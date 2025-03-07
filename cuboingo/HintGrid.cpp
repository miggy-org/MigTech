#include "pch.h"
#include "HintGrid.h"
#include "CubeConst.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

HintGrid::HintGrid(const GameCube& cube, const LightBeam& beam, const GridInfo& info) : GridBase(),
	_gameCube(cube), _lightBeam(beam), _origInfo(info),
	_cancel(false), _animGlowParam(0), _animGrowing(true), _sortedRenderingOrder(nullptr)
{
	_slotRadiusScale = (_origInfo.dimen == 1 ? 1.05f : 1.15f);		// this will make the hint slots larger
}

HintGrid::~HintGrid()
{
	delete [] _sortedRenderingOrder;
}

static void copyArray4(int* ind, int i1, int i2, int i3, int i4)
{
	ind[0] = i1;
	ind[1] = i2;
	ind[2] = i3;
	ind[3] = i4;
}

static void copyArray9(int* ind, int i1, int i2, int i3, int i4, int i5, int i6, int i7, int i8, int i9)
{
	ind[0] = i1;
	ind[1] = i2;
	ind[2] = i3;
	ind[3] = i4;
	ind[4] = i5;
	ind[5] = i6;
	ind[6] = i7;
	ind[7] = i8;
	ind[8] = i9;
}

// when blending the slots need to be sorted by distance
static int* getSortedSlotRenderingOrder(const GridInfo& grid)
{
	int* newInts = nullptr;
	if (grid.dimen == 2)
	{
		newInts = new int[4];
		if (grid.orient == AXISORIENT_X)
			copyArray4(newInts, 3, 2, 1, 0);
		else if (grid.orient == AXISORIENT_Y)
			copyArray4(newInts, 0, 1, 2, 3);
		else if (grid.orient == AXISORIENT_Z)
			copyArray4(newInts, 2, 3, 0, 1);
	}
	else if (grid.dimen == 3)
	{
		newInts = new int[9];
		if (grid.orient == AXISORIENT_X)
			copyArray9(newInts, 8, 5, 7, 2, 4, 6, 1, 3, 0);
		else if (grid.orient == AXISORIENT_Y)
			copyArray9(newInts, 0, 3, 1, 6, 4, 2, 7, 5, 8);
		else if (grid.orient == AXISORIENT_Z)
			copyArray9(newInts, 6, 7, 3, 8, 4, 0, 5, 1, 2);
	}
	else
	{
		newInts = new int[1];
		newInts[0] = 0;
	}
	return newInts;
}

bool HintGrid::init()
{
	// configure the new falling grid from the info passed in
	if (!GridBase::init(_origInfo.dimen, -1, _origInfo.orient, defCubeRadius + 0.01f))
	{
		LOGWARN("(HintGrid::init) init() returned false!");
		return false;
	}
	setAllColors(0.5f, 0.5f, 0.5f, 1, 1, 1);
	//_mapIndex = _origInfo.mapIndex;	// note we didn't pass this into init()

	// hide slots in this hint grid that aren't filled in
	updateGridInfo(_origInfo);

	// we need to sort the rendering order from back to front
	_sortedRenderingOrder = getSortedSlotRenderingOrder(_origInfo);
	return true;
}

void HintGrid::updateGridInfo(const GridInfo& info)
{
	// hide slots in this hint grid that aren't filled in
	int listLen = _origInfo.fillListLen();
	if (listLen == getSlotCount())
	{
		for (int i = 0; i < listLen; i++)
			_theSlots[i].invis = !info.fillList[i];
	}
}

bool HintGrid::setGlowParam(float newVal, bool isGrowing)
{
	bool isMax = (!isGrowing && _animGrowing);

	_animGlowParam = newVal;
	_animGrowing = isGrowing;

	float red = _emptyCol.r + (_fillCol.r - _emptyCol.r) * newVal;
	float grn = _emptyCol.g + (_fillCol.g - _emptyCol.g) * newVal;
	float blu = _emptyCol.b + (_fillCol.b - _emptyCol.b) * newVal;

	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].setColor(red, grn, blu);

	return isMax;
}

// this causes the glow to continue during the falling stage
void HintGrid::setFallParam(float newVal)
{
	setGlowParam(newVal, false);
}

void HintGrid::draw(const Matrix& mat, bool showLightBeams) const
{
	MigUtil::theRend->setBlending(BLEND_STATE_NONE);

	// do not draw the glowing grid if the cube is moving at all
	if (_gameCube.isStationary() && !_animGrowing)
		GridBase::draw(mat);

	if (showLightBeams)
	{
		// activate blending for the light beams
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);

		// render the shafts from back to front since we're blending
		int lenOrder = _origInfo.fillListLen();
		for (int i = 0; i < lenOrder; i++)
		{
			int index = _sortedRenderingOrder[i];
			if (_origInfo.fillList[index])
			{
				_lightBeam.draw(_origInfo, _theSlots[index].ptCenter.x, _theSlots[index].ptCenter.y, _animGlowParam, _animGrowing, mat);
			}
		}
	}
}
