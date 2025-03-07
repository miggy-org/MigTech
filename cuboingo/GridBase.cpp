#include "pch.h"
#include "GridBase.h"
#include "CubeConst.h"
#include "CubeUtil.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

const std::string silverMapName = "silvermap.jpg";
const std::string holeMapName = "holemap.png";
const std::string gridVertexShader = "cvs_Grid";
const std::string gridPixelShader = "cps_Grid";

// center coordinate for slots, depending on (dimension - 1)
const float CENTER_BY_LVL[3] = { 0.0f, 0.5f, 0.65f };

// radius for the slots, depending on (dimension - 1)
const float RADIUS_BY_LVL[3] = { 0.9f, 0.4f, 0.24f };

// static init
Object* GridBase::_objFace = nullptr;
Object* GridBase::_objSides = nullptr;

// used to determine the X center of a slot 
static float getSlotXCenterByIndex(int dimen, int index)
{
	float xCenter = 0;
	if (dimen == 1)
		xCenter = CENTER_BY_LVL[0];
	else if (dimen == 2)
		xCenter = (index % 2 == 0 ? -CENTER_BY_LVL[1] : CENTER_BY_LVL[1]);
	else if (dimen == 3)
	{
		if (index % 3 == 0)
			xCenter = -CENTER_BY_LVL[2];
		else if (index % 3 == 2)
			xCenter = CENTER_BY_LVL[2];
	}
	return xCenter;
}

// used to determine the Y center of a slot 
static float getSlotYCenterByIndex(int dimen, int index)
{
	float yCenter = 0;
	if (dimen == 1)
		yCenter = CENTER_BY_LVL[0];
	else if (dimen == 2)
		yCenter = (index < 2 ? CENTER_BY_LVL[1] : -CENTER_BY_LVL[1]);
	else if (dimen == 3)
	{
		if (index < 3)
			yCenter = CENTER_BY_LVL[2];
		else if (index > 5)
			yCenter = -CENTER_BY_LVL[2];
	}
	return yCenter;
}

///////////////////////////////////////////////////////////////////////////
// used to pass around a description of a grid from one grid to another

GridInfo::GridInfo()
{
	uniqueID = 0;
	orient = AXISORIENT_NONE;
	dimen = 0;
	mapIndex = -1;
	fillList = nullptr;
}

GridInfo::GridInfo(const GridInfo& rhs)
{
	*this = rhs;
}

GridInfo::~GridInfo()
{
	delete[] fillList;
}

void GridInfo::operator=(const GridInfo& rhs)
{
	uniqueID = rhs.uniqueID;
	orient = rhs.orient;
	dimen = rhs.dimen;
	mapIndex = rhs.mapIndex;
	emptyCol = rhs.emptyCol;
	fillCol = rhs.fillCol;
	fillList = new bool[dimen*dimen];
	copyFillList(rhs);
}

void GridInfo::init(AxisOrient newOrient, int newDimen, int newMapIndex)
{
	orient = newOrient;
	dimen = newDimen;
	mapIndex = newMapIndex;

	fillList = new bool[dimen*dimen];
	setFillList(true);
}

void GridInfo::initFromGrid(const GridBase& grid, bool incReservedInFillList)
{
	orient = grid._orient;
	dimen = grid._dimen;
	mapIndex = grid._mapIndex;
	emptyCol = grid._emptyCol;
	fillCol = grid._fillCol;

	int len = dimen*dimen;
	fillList = new bool[len];
	for (int i = 0; i < len; i++)
	{
		fillList[i] = (grid._theSlots[i].filled || (incReservedInFillList && grid._theSlots[i].reserved > 0));
	}
}

void GridInfo::setFillList(bool newValue)
{
	int len = fillListLen();
	for (int i = 0; i < len; i++)
		fillList[i] = newValue;
}

void GridInfo::copyFillList(const GridInfo& other)
{
	int len = fillListLen();
	for (int i = 0; i < len; i++)
		fillList[i] = other.fillList[i];
}

void GridInfo::invertFillList()
{
	int len = fillListLen();
	for (int i = 0; i < len; i++)
		fillList[i] = !fillList[i];
}

void GridInfo::randomizeFillList(int maxFilled)
{
	int filledCount = 0;
	int fallbackIndex = -1;
	int len = fillListLen();
	for (int i = 0; i < len; i++)
	{
		bool isFilled = fillList[i];
		if (isFilled)
		{
			// if a fill max is being enforced then don't exceed that
			if (maxFilled > 0 && filledCount == maxFilled)
				isFilled = false;
			else
			{
				// randomly turn off so we don't always fill the entire grid
				isFilled = (MigUtil::rollAgainstPercent(50));
				if (isFilled)
					filledCount++;
				else if (filledCount == 0)
				{
					// keep track of a fallback in case we turn them ALL off
					if (fallbackIndex == -1 || MigUtil::rollAgainstPercent(50))
						fallbackIndex = i;
				}
			}
		}
		fillList[i] = isFilled;
	}

	// if we cleared them all, then make sure at least one is set
	if (filledCount == 0 && fallbackIndex >= 0)
		fillList[fallbackIndex] = true;
}

void GridInfo::assignFillList(const int* compArray, int compValue)
{
	int len = fillListLen();
	for (int i = 0; i < len; i++)
	{
		fillList[i] = (compArray[i] == compValue ? true : false);
	}
}

///////////////////////////////////////////////////////////////////////////
// represents a single slot on the grid, which can be empty or filled

Slot::Slot()
{
	filled = false;
	invis = false;
	animating = false;
	reserved = 0;
	scale = 1;
}

void Slot::configSlot(float x, float y, float z)
{
	ptCenter.x = x;
	ptCenter.y = y;
	ptCenter.z = z;
}

void Slot::configSlot(const Vector3& pt)
{
	ptCenter = pt;
}

void Slot::setColor(float r, float g, float b)
{
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = 1;
}

void Slot::setColor(const Color& newColor)
{
	color = newColor;
}

void Slot::copySlotInfo(const Slot& rhs)
{
	setColor(rhs.color);
	filled = rhs.filled;
	reserved = rhs.reserved;
	animating = rhs.animating;
}

///////////////////////////////////////////////////////////////////////////
// GridBase represents a single grid of slots for one side of a cube

GridBase::GridBase()
{
	_orient = AXISORIENT_NONE;
	_dimen = 0;
	_mapIndex = -1;
	_distFromOrigin = 0;
	_slotRadius = 0;
	_slotRadiusScale = 1;
	_gridDepth = 0;
	_theSlots = nullptr;
}

GridBase::~GridBase()
{
}

bool GridBase::init(int dimension, int index, AxisOrient orientation, float dist)
{
	if (orientation == AXISORIENT_NONE)
		return false;
	if (dimension < 1 || dimension > 3)
		return false;

	_orient = orientation;
	_dimen = dimension;
	_mapIndex = index;
	_distFromOrigin = dist;
	_slotRadius = RADIUS_BY_LVL[_dimen - 1] * _slotRadiusScale;
	if (_slotRadius > 1)
		_slotRadius = 1;

	// create the slot classes
	float absDist = (float) fabs(_distFromOrigin);
	int nSlots = getSlotCount();
	_theSlots = new Slot[nSlots];
	for (int i = 0; i < nSlots; i++)
	{
		_theSlots[i].configSlot(
			getSlotXCenterByIndex(_dimen, i),
			getSlotYCenterByIndex(_dimen, i),
			absDist);
		_theSlots[i].setColor(_emptyCol);
	}

	return true;
}

static VertexPositionNormalTexture* createVertices(float radius, float depth)
{
	VertexPositionNormalTexture* verts = new VertexPositionNormalTexture[24];

	Vector2 uv0(0.1f, 0.6f);
	Vector2 uv1(0.5f, 0.4f);
	Vector2 uv2(0.9f, 0.6f);
	Vector2 uv3(0.5f, 0.8f);
	Vector2 uv4(0.1f, 0.4f);
	Vector2 uv5(0.5f, 0.2f);
	Vector2 uv6(0.9f, 0.4f);
	Vector2 uv7(0.5f, 0.6f);
	Vector3 norm;

	// front
	norm = Vector3(0, 0, 1);
	verts[0].pos = Vector3(-radius, -radius, depth);
	verts[0].norm = norm;
	verts[0].uv = Vector2(0, 0);
	verts[1].pos = Vector3( radius, -radius, depth);
	verts[1].norm = norm;
	verts[1].uv = Vector2(1, 0);
	verts[2].pos = Vector3( radius,  radius, depth);
	verts[2].norm = norm;
	verts[2].uv = Vector2(1, 1);
	verts[3].pos = Vector3(-radius,  radius, depth);
	verts[3].norm = norm;
	verts[3].uv = Vector2(0, 1);

	// back
	norm = Vector3(0, 0, -1);
	verts[4].pos = Vector3(-radius, -radius, 0);
	verts[4].norm = norm;
	verts[4].uv = Vector2(0, 0);
	verts[5].pos = Vector3( radius, -radius, 0);
	verts[5].norm = norm;
	verts[5].uv = Vector2(1, 0);
	verts[6].pos = Vector3( radius,  radius, 0);
	verts[6].norm = norm;
	verts[6].uv = Vector2(1, 1);
	verts[7].pos = Vector3(-radius,  radius, 0);
	verts[7].norm = norm;
	verts[7].uv = Vector2(0, 1);

	// top
	norm = Vector3(0, 1, 0);
	verts[8].pos  = Vector3(-radius,  radius, depth);
	verts[8].norm = norm;
	verts[8].uv = uv7;
	verts[9].pos  = Vector3( radius,  radius, depth);
	verts[9].norm = norm;
	verts[9].uv = uv6;
	verts[10].pos = Vector3( radius,  radius, 0);
	verts[10].norm = norm;
	verts[10].uv = uv2;
	verts[11].pos = Vector3(-radius,  radius, 0);
	verts[11].norm = norm;
	verts[11].uv = uv3;

	// bottom
	norm = Vector3(0, -1, 0);
	verts[12].pos = Vector3(-radius, -radius, 0);
	verts[12].norm = norm;
	verts[12].uv = uv0;
	verts[13].pos = Vector3( radius, -radius, 0);
	verts[13].norm = norm;
	verts[13].uv = uv1;
	verts[14].pos = Vector3( radius, -radius, depth);
	verts[14].norm = norm;
	verts[14].uv = uv5;
	verts[15].pos = Vector3(-radius, -radius, depth);
	verts[15].norm = norm;
	verts[15].uv = uv4;

	// right
	norm = Vector3(1, 0, 0);
	verts[16].pos = Vector3(radius, -radius, depth);
	verts[16].norm = norm;
	verts[16].uv = uv5;
	verts[17].pos = Vector3(radius, -radius, 0);
	verts[17].norm = norm;
	verts[17].uv = uv1;
	verts[18].pos = Vector3(radius,  radius, 0);
	verts[18].norm = norm;
	verts[18].uv = uv2;
	verts[19].pos = Vector3(radius,  radius, depth);
	verts[19].norm = norm;
	verts[19].uv = uv6;

	// left
	norm = Vector3(-1, 0, 0);
	verts[20].pos = Vector3(-radius, -radius, 0);
	verts[20].norm = norm;
	verts[20].uv = uv0;
	verts[21].pos = Vector3(-radius, -radius, depth);
	verts[21].norm = norm;
	verts[21].uv = uv4;
	verts[22].pos = Vector3(-radius,  radius, depth);
	verts[22].norm = norm;
	verts[22].uv = uv7;
	verts[23].pos = Vector3(-radius,  radius, 0);
	verts[23].norm = norm;
	verts[23].uv = uv3;

	return verts;
}

void GridBase::createGraphics()
{
	// load the shaders first
	MigUtil::theRend->loadVertexShader(gridVertexShader, VDTYPE_POSITION_NORMAL_TEXTURE, SHADER_HINT_MVP);
	MigUtil::theRend->loadPixelShader(gridPixelShader, SHADER_HINT_NONE);

	// load the maps
	MigUtil::theRend->loadImage(silverMapName, silverMapName, LOAD_IMAGE_NONE);
	MigUtil::theRend->loadImage(holeMapName, holeMapName, LOAD_IMAGE_NONE);

	// create the objects and assign the shaders
	Object* faceObj = MigUtil::theRend->createObject();
	faceObj->addShaderSet(gridVertexShader, gridPixelShader);
	faceObj->addShaderSet(MIGTECH_VSHADER_POS_TRANSFORM, MIGTECH_PSHADER_COLOR);
	Object* sideObj = MigUtil::theRend->createObject();
	sideObj->addShaderSet(gridVertexShader, gridPixelShader);
	sideObj->addShaderSet(MIGTECH_VSHADER_POS_TRANSFORM, MIGTECH_PSHADER_COLOR);
	
	// load mesh vertices
	VertexPositionNormalTexture* txtVertices = createVertices(1, 1);
	faceObj->loadVertexBuffer(txtVertices, 24, MigTech::VDTYPE_POSITION_NORMAL_TEXTURE);
	sideObj->loadVertexBuffer(txtVertices, 24, MigTech::VDTYPE_POSITION_NORMAL_TEXTURE);

	// load mesh indices
	unsigned short faceIndices[] = {
		0, 1, 2, 0, 2, 3,		// front face
		4, 7, 6, 4, 6, 5,		// back face
	};
	faceObj->loadIndexBuffer(faceIndices, ARRAYSIZE(faceIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);
	unsigned short sideIndices[] = {
		8, 9, 10, 8, 10, 11,    // top face
		12, 13, 14, 12, 14, 15, // bottom face
		16, 17, 18, 16, 18, 19, // right face
		20, 21, 22, 20, 22, 23, // left face
	};
	sideObj->loadIndexBuffer(sideIndices, ARRAYSIZE(sideIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// assign texturing
	faceObj->setImage(0, holeMapName, TXT_FILTER_NEAREST, TXT_FILTER_NEAREST, TXT_WRAP_CLAMP);
	sideObj->setImage(0, silverMapName, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

	// culling
	faceObj->setCulling(FACE_CULLING_BACK);
	sideObj->setCulling(FACE_CULLING_BACK);
	_objFace = faceObj;
	_objSides = sideObj;
}

void GridBase::destroyGraphics()
{
	if (_objFace != nullptr)
		MigUtil::theRend->deleteObject(_objFace);
	_objFace = nullptr;
	if (_objSides != nullptr)
		MigUtil::theRend->deleteObject(_objSides);
	_objSides = nullptr;

	MigUtil::theRend->unloadImage(silverMapName);
	MigUtil::theRend->unloadImage(holeMapName);
}

void GridBase::draw(const Matrix& mat) const
{
	static Color sideColor(0.5f, 0.5f, 0.5f, 1);
	static Matrix locMatrix;

	MigUtil::theRend->setBlending(BLEND_STATE_NONE);
	MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_LESS, true);
	MigUtil::theRend->setMiscValue(0, (float)_mapIndex);

	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
	{
		const Slot& theSlot = _theSlots[i];
		if (!theSlot.invis)
		{
			applyTransform(locMatrix, mat, theSlot);
			MigUtil::theRend->setModelMatrix(locMatrix);

			MigUtil::theRend->setObjectColor(getSlotDrawColor(theSlot, i));
			_objFace->render(CubeUtil::renderPass);

			if (_gridDepth > 0)
			{
				sideColor.a = theSlot.color.a;
				MigUtil::theRend->setObjectColor(sideColor);
				_objSides->render(CubeUtil::renderPass);
			}
		}
	}
}

bool GridBase::setEmptyColors(float r, float g, float b)
{
	_emptyCol = Color(r, g, b);
	if (_theSlots != nullptr)
	{
		int len = getSlotCount();
		for (int i = 0; i < len; i++)
			_theSlots[i].setColor(r, g, b);
	}
	return true;
}

bool GridBase::setEmptyColors(const Color& col)
{
	return setEmptyColors(col.r, col.g, col.b);
}

bool GridBase::setFilledColors(float r, float g, float b)
{
	_fillCol = Color(r, g, b);
	return true;
}

bool GridBase::setFilledColors(const Color& col)
{
	return setFilledColors(col.r, col.g, col.b);
}

bool GridBase::setAllColors(float er, float eg, float eb, float fr, float fg, float fb)
{
	setEmptyColors(er, eg, eb);
	setFilledColors(fr, fg, fb);
	return true;
}

bool GridBase::setAllColors(const Color& eCol, const Color& fCol)
{
	setEmptyColors(eCol);
	setFilledColors(fCol);
	return true;
}

bool GridBase::isFilled(bool incReserved) const
{
	int len = getSlotCount();
	for (int i = 0; i < len; i++)
	{
		if (!(_theSlots[i].filled || (incReserved && _theSlots[i].reserved > 0)))
			return false;
	}
	return true;
}

bool GridBase::isEmpty(bool incReserved) const
{
	int len = getSlotCount();
	for (int i = 0; i < len; i++)
	{
		if (_theSlots[i].filled || (incReserved && _theSlots[i].reserved > 0))
			return false;
	}
	return true;
}

void GridBase::reserveSlots(const GridInfo& info)
{
	int lenSlots = getSlotCount();
	for (int i = 0; i < lenSlots; i++)
	{
		if (info.fillList[i])
			_theSlots[i].reserved = info.uniqueID;
	}
}

void GridBase::clearReserveSlots(int uniqueID)
{
	int numSlots = getSlotCount();
	for (int j = 0; j < numSlots; j++)
	{
		if (_theSlots[j].reserved == uniqueID || uniqueID == -1)
			_theSlots[j].reserved = 0;
	}
}

GridBase::FillState GridBase::getFillState(bool incReserved) const
{
	bool notEmpty = false;
	bool notFilled = false;
	int len = getSlotCount();
	for (int i = 0; i < len; i++)
	{
		if (_theSlots[i].filled || (incReserved && _theSlots[i].reserved > 0))
			notEmpty = true;
		else
			notFilled = true;
	}
	return ((notFilled && notEmpty) ? FILLSTATE_PARTIAL : (notEmpty ? FILLSTATE_FILLED : FILLSTATE_EMPTY));
}

void GridBase::applyTransform(Matrix& outMatrix, const Matrix& worldMatrix, const Slot& theSlot) const
{
	outMatrix.identity();

	float radius = _slotRadius * theSlot.scale;
	outMatrix.scale(radius, radius, _gridDepth);

	outMatrix.translate(theSlot.ptCenter);
	applyOffset(outMatrix);

	switch (_orient)
	{
	case AXISORIENT_Z:
		if (_distFromOrigin < 0)
			outMatrix.rotateY(rad180);
		break;
	case AXISORIENT_X:
		outMatrix.rotateY(_distFromOrigin >= 0 ? rad90 : -rad90);
		break;
	case AXISORIENT_Y:
		outMatrix.rotateX(_distFromOrigin >= 0 ? -rad90 : rad90);
		break;
	default:
		break;
	}

	outMatrix.multiply(worldMatrix);
}

void GridBase::applyOffset(Matrix& mat) const
{
}

const Color& GridBase::getSlotDrawColor(const Slot& theSlot, int index) const
{
	return theSlot.color;
}

void GridBase::updateSlotRadius(float newVal, bool onlyIfAnimating)
{
	// we animate all of the slots to catch any errors that occur during shuffling (grid rotation)
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
	{
		Slot& theSlot = _theSlots[i];
		if (!theSlot.invis)
			theSlot.scale = ((theSlot.animating || !onlyIfAnimating) ? newVal : 1);
	}
}

void GridBase::updateMapIndex(int newIndex)
{
	_mapIndex = newIndex;
}
