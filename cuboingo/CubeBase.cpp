#include "pch.h"
#include "CubeBase.h"
#include "CubeConst.h"
#include "CubeUtil.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

const std::string defSilverMapName = "silvermap.jpg";
const std::string defReflectMapName = "reflect.jpg";
const std::string defVertexShader = "cvs_Cube";
const std::string defPixelShader = "cps_Cube";

///////////////////////////////////////////////////////////////////////////
// CubeBase

CubeBase::CubeBase(float radX, float radY, float radZ)
	: _radiusX(radX), _radiusY(radY), _radiusZ(radZ), _isRounded(true),
	_cubeObj(nullptr),
	_rotX(0), _rotY(0), _rotZ(0), _scale(1), _color(colWhite),
	_useShadows(false), _writeDepth(true),
	_reflectIntensity(0.5f)
{
	_vertexShader = defVertexShader;
	_pixelShader = defPixelShader;
	_textureName = defSilverMapName;
	_reflectName = defReflectMapName;
}

VertexPositionNormalTexture* CubeBase::createVertices(bool useRoundedEdges, unsigned int& arraySize)
{
	static VertexPositionNormalTexture verts[4 * 6];

	Vector2 uv1(0, 0);
	Vector2 uv2(1, 0);
	Vector2 uv3(1, 1);
	Vector2 uv4(0, 1);
	Vector3 norm;

	// the radius of the sides will shrink to make room for rounded edges, if we're using them
	float radiusX = _radiusX - (useRoundedEdges ? Cuboingo::defCubeRoundedDelta : 0);
	float radiusY = _radiusY - (useRoundedEdges ? Cuboingo::defCubeRoundedDelta : 0);
	float radiusZ = _radiusZ - (useRoundedEdges ? Cuboingo::defCubeRoundedDelta : 0);

	// front
	norm = Vector3(0, 0, 1);
	verts[0].pos = Vector3(-radiusX, -radiusY, _radiusZ); verts[0].norm = norm; verts[0].uv = uv1;
	verts[1].pos = Vector3( radiusX, -radiusY, _radiusZ); verts[1].norm = norm; verts[1].uv = uv2;
	verts[2].pos = Vector3( radiusX,  radiusY, _radiusZ); verts[2].norm = norm; verts[2].uv = uv3;
	verts[3].pos = Vector3(-radiusX,  radiusY, _radiusZ); verts[3].norm = norm; verts[3].uv = uv4;

	// back
	norm = Vector3(0, 0, -1);
	verts[4].pos = Vector3( radiusX, -radiusY, -_radiusZ); verts[4].norm = norm; verts[4].uv = uv1;
	verts[5].pos = Vector3(-radiusX, -radiusY, -_radiusZ); verts[5].norm = norm; verts[5].uv = uv2;
	verts[6].pos = Vector3(-radiusX,  radiusY, -_radiusZ); verts[6].norm = norm; verts[6].uv = uv3;
	verts[7].pos = Vector3( radiusX,  radiusY, -_radiusZ); verts[7].norm = norm; verts[7].uv = uv4;

	// top
	norm = Vector3(0, 1, 0);
	verts[8].pos  = Vector3(-radiusX, _radiusY,  radiusZ); verts[8].norm  = norm; verts[8].uv  = uv1;
	verts[9].pos  = Vector3( radiusX, _radiusY,  radiusZ); verts[9].norm  = norm; verts[9].uv  = uv2;
	verts[10].pos = Vector3( radiusX, _radiusY, -radiusZ); verts[10].norm = norm; verts[10].uv = uv3;
	verts[11].pos = Vector3(-radiusX, _radiusY, -radiusZ); verts[11].norm = norm; verts[11].uv = uv4;

	// bottom
	norm = Vector3(0, -1, 0);
	verts[12].pos = Vector3(-radiusX, -_radiusY, -radiusZ); verts[12].norm = norm; verts[12].uv = uv1;
	verts[13].pos = Vector3( radiusX, -_radiusY, -radiusZ); verts[13].norm = norm; verts[13].uv = uv2;
	verts[14].pos = Vector3( radiusX, -_radiusY,  radiusZ); verts[14].norm = norm; verts[14].uv = uv3;
	verts[15].pos = Vector3(-radiusX, -_radiusY,  radiusZ); verts[15].norm = norm; verts[15].uv = uv4;

	// right
	norm = Vector3(1, 0, 0);
	verts[16].pos = Vector3(_radiusX, -radiusY,  radiusZ); verts[16].norm = norm; verts[16].uv = uv1;
	verts[17].pos = Vector3(_radiusX, -radiusY, -radiusZ); verts[17].norm = norm; verts[17].uv = uv2;
	verts[18].pos = Vector3(_radiusX,  radiusY, -radiusZ); verts[18].norm = norm; verts[18].uv = uv3;
	verts[19].pos = Vector3(_radiusX,  radiusY,  radiusZ); verts[19].norm = norm; verts[19].uv = uv4;

	// left
	norm = Vector3(-1, 0, 0);
	verts[20].pos = Vector3(-_radiusX, -radiusY, -radiusZ); verts[20].norm = norm; verts[20].uv = uv1;
	verts[21].pos = Vector3(-_radiusX, -radiusY,  radiusZ); verts[21].norm = norm; verts[21].uv = uv2;
	verts[22].pos = Vector3(-_radiusX,  radiusY,  radiusZ); verts[22].norm = norm; verts[22].uv = uv3;
	verts[23].pos = Vector3(-_radiusX,  radiusY, -radiusZ); verts[23].norm = norm; verts[23].uv = uv4;

	arraySize = ARRAYSIZE(verts);
	return verts;
}

unsigned short* CubeBase::createIndices(bool useRoundedEdges, unsigned int& arraySize)
{
	if (useRoundedEdges)
	{
		static unsigned short indices[] = {
			0, 1, 2, 0, 2, 3,		// front face
			4, 5, 6, 4, 6, 7,		// back face
			8, 9, 10, 8, 10, 11,    // top face
			12, 13, 14, 12, 14, 15, // bottom face
			16, 17, 18, 16, 18, 19, // right face
			20, 21, 22, 20, 22, 23, // left face

			// edges around the 4 sides
			1, 16, 19, 1, 19, 2,
			21, 0, 3, 21, 3, 22,
			4, 7, 18, 4, 18, 17,
			20, 23, 6, 20, 6, 5,

			// edges attached to the top
			2, 9, 8, 2, 8, 3,
			6, 11, 10, 6, 10, 7,
			8, 11, 23, 8, 23, 22,
			9, 19, 18, 9, 18, 10,

			// edges attached to the bottom
			0, 15, 14, 0, 14, 1,
			4, 13, 12, 4, 12, 5,
			13, 17, 16, 13, 16, 14,
			12, 15, 21, 12, 21, 20,

			// corners
			2, 19, 9,
			3, 8, 22,
			6, 23, 11,
			7, 10, 18,
			4, 17, 13,
			5, 12, 20,
			0, 21, 15,
			1, 14, 16,
		};
		arraySize = ARRAYSIZE(indices);
		return indices;
	}
	else
	{
		static unsigned short indices[] = {
			0, 1, 2, 0, 2, 3,		// front face
			4, 5, 6, 4, 6, 7,		// back face
			8, 9, 10, 8, 10, 11,    // top face
			12, 13, 14, 12, 14, 15, // bottom face
			16, 17, 18, 16, 18, 19, // right face
			20, 21, 22, 20, 22, 23, // left face
		};
		arraySize = ARRAYSIZE(indices);
		return indices;
	}
}

void CubeBase::createGraphics()
{
	// load the shaders first
	MigUtil::theRend->loadVertexShader(_vertexShader, VDTYPE_POSITION_NORMAL_TEXTURE, SHADER_HINT_MVP | SHADER_HINT_MODEL | SHADER_HINT_LIGHTS);
	MigUtil::theRend->loadPixelShader(_pixelShader, SHADER_HINT_NONE);

	// load the silver map
	MigUtil::theRend->loadImage(_textureName, _textureName, LOAD_IMAGE_NONE);
	MigUtil::theRend->loadImage(_reflectName, _reflectName, LOAD_IMAGE_NONE);

	// create the cube object and assign the shaders
	Object* cubeObj = MigUtil::theRend->createObject();
	cubeObj->addShaderSet(_vertexShader, _pixelShader);
	if (_useShadows)
		cubeObj->addShaderSet(MIGTECH_VSHADER_POS_TRANSFORM, MIGTECH_PSHADER_COLOR);

	// load mesh vertices
	unsigned int vertArraySize = 0;
	VertexPositionNormalTexture* txtVertices = createVertices(_isRounded, vertArraySize);
	cubeObj->loadVertexBuffer(txtVertices, vertArraySize, MigTech::VDTYPE_POSITION_NORMAL_TEXTURE);

	// load mesh indices
	unsigned int indArraySize = 0;
	unsigned short* txtIndices = createIndices(_isRounded, indArraySize);
	cubeObj->loadIndexBuffer(txtIndices, indArraySize, MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// assign texturing
	cubeObj->setImage(0, _textureName, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
	cubeObj->setImage(1, _reflectName, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_REPEAT);

	// culling
	cubeObj->setCulling(FACE_CULLING_BACK);
	_cubeObj = cubeObj;
}

void CubeBase::destroyGraphics()
{
	if (_cubeObj != nullptr)
		MigUtil::theRend->deleteObject(_cubeObj);
	_cubeObj = nullptr;
	//MigUtil::theRend->unloadImage(_textureName);
	//MigUtil::theRend->unloadImage(_reflectName);
}

void CubeBase::draw(Matrix& mat, const Color& col) const
{
	if (_cubeObj != nullptr && _scale > 0 && col.a > 0)
	{
		applyTransform(mat);

		MigUtil::theRend->setBlending(col.a < 1 ? BLEND_STATE_SRC_ALPHA : BLEND_STATE_NONE);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_LESS, _writeDepth);
		MigUtil::theRend->setObjectColor(col);

		// reflection mapping settings
		MigUtil::theRend->setMiscValue(0, defEye.x);
		MigUtil::theRend->setMiscValue(1, defEye.y);
		MigUtil::theRend->setMiscValue(2, defEye.z);
		MigUtil::theRend->setMiscValue(3, (CubeUtil::useReflections && CubeUtil::renderPass == RENDER_PASS_FINAL ? _reflectIntensity : 0));

		if (CubeUtil::renderPass == RENDER_PASS_FINAL)
			_cubeObj->render();
		else if (CubeUtil::renderPass == RENDER_PASS_SHADOW && _useShadows)
			_cubeObj->render(1);	// this refers to the second shader set, which should be the shadow shaders
	}
}

void CubeBase::draw(Matrix& mat) const
{
	draw(mat, _color);
}

void CubeBase::draw() const
{
	static Matrix locMatrix;
	locMatrix.identity();
	draw(locMatrix);
}

bool CubeBase::doFrame(int id, float newVal, void* optData)
{
	if (_idRotX == id)
		_rotX = newVal;
	else if (_idRotY == id)
		_rotY = newVal;
	else if (_idRotZ == id)
		_rotZ = newVal;
	else if (_idScale == id)
		_scale = newVal;
	else if (_idTransX == id)
		_translate.x = newVal;
	else if (_idTransY == id)
		_translate.y = newVal;
	else if (_idTransZ == id)
		_translate.z = newVal;
	else if (_idOpacity == id)
		_color.a = newVal;

	return true;
}

void CubeBase::animComplete(int id, void* optData)
{
	if (_idRotX == id)
		_idRotX = 0;
	else if (_idRotY == id)
		_idRotY = 0;
	else if (_idRotZ == id)
		_idRotZ = 0;
	else if (_idScale == id)
		_idScale = 0;
	else if (_idTransX == id)
		_idTransX = 0;
	else if (_idTransY == id)
		_idTransY = 0;
	else if (_idTransZ == id)
		_idTransZ = 0;
	else if (_idOpacity == id)
		_idOpacity = 0;
}

void CubeBase::applyTransform(Matrix& worldMatrix) const
{
	static Matrix locMatrix;
	locMatrix.identity();
	if (_translate.x != 0 || _translate.y != 0 || _translate.z != 0)
		locMatrix.translate(_translate);
	if (_rotX != 0)
		locMatrix.rotateX(_rotX);
	if (_rotY != 0)
		locMatrix.rotateY(_rotY);
	if (_rotZ != 0)
		locMatrix.rotateZ(_rotZ);
	if (_scale != 1)
		locMatrix.scale(_scale, _scale, _scale);
	locMatrix.multiply(worldMatrix);
	MigUtil::theRend->setModelMatrix(locMatrix);

	// after this call the worldMatrix will have the cube transformation applied
	worldMatrix.copy(locMatrix);
}

///////////////////////////////////////////////////////////////////////////
// CubeGrid

CubeGrid::CubeGrid(float radius)
	: CubeBase(radius, radius, radius)
{
	for (int i = 0; i < NUM_GRIDS; i++)
		_theGrids[i] = nullptr;
}

CubeGrid::~CubeGrid()
{
	deleteAllGrids();
}

void CubeGrid::draw(Matrix& mat, const Color& col) const
{
	CubeBase::draw(mat, col);

	if (_scale > 0 && col.a > 0 && CubeUtil::renderPass == RENDER_PASS_FINAL)
	{
		for (int i = 0; i < NUM_GRIDS; i++)
		{
			if (_theGrids[i] != nullptr)
				_theGrids[i]->draw(mat);
		}
	}
}

void CubeGrid::draw(Matrix& mat) const
{
	CubeBase::draw(mat);
}

void CubeGrid::draw() const
{
	CubeBase::draw();
}

void CubeGrid::deleteAllGrids()
{
	for (int i = 0; i < NUM_GRIDS; i++)
	{
		if (_theGrids[i] != nullptr)
			delete _theGrids[i];
		_theGrids[i] = nullptr;
	}
}
