#include "pch.h"
#include "LightBeam.h"
#include "CubeConst.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

// radius for the slots, note that they're the same as grids but that could change
static const float RADIUS_BY_LVL[] = { 0.9f, 0.4f, 0.24f };

// distances from the center of the cube 
static const float SHAFT_START = 0.9f;
static const float SHAFT_END = 5;
//static const float ACCENT_DEPTH = 0.5f;

static const std::string lightBeamVertexShader("cvs_Beam");
static const std::string lightBeamPixelShader("cps_Beam");

LightBeam::LightBeam() : _objBeam(nullptr)
{
}

LightBeam::~LightBeam()
{
}

static VertexPositionTexture* createVertices(unsigned int& arraySize)
{
	static VertexPositionTexture verts[8];

	// front
	verts[0].pos = Vector3(-1, -1, SHAFT_START); verts[0].uv = Vector2(1, 1);
	verts[1].pos = Vector3( 1, -1, SHAFT_START); verts[1].uv = Vector2(0, 1);
	verts[2].pos = Vector3( 1,  1, SHAFT_START); verts[2].uv = Vector2(1, 1);
	verts[3].pos = Vector3(-1,  1, SHAFT_START); verts[3].uv = Vector2(0, 1);
	verts[4].pos = Vector3(-1, -1, SHAFT_END);   verts[4].uv = Vector2(1, 0);
	verts[5].pos = Vector3( 1, -1, SHAFT_END);   verts[5].uv = Vector2(0, 0);
	verts[6].pos = Vector3( 1,  1, SHAFT_END);   verts[6].uv = Vector2(1, 0);
	verts[7].pos = Vector3(-1,  1, SHAFT_END);   verts[7].uv = Vector2(0, 0);

	arraySize = ARRAYSIZE(verts);
	return verts;
}

static unsigned short* createIndices(unsigned int& arraySize)
{
	static unsigned short indices[] = {
		0, 1, 5, 0, 5, 4,
		1, 2, 6, 1, 6, 5,
		2, 3, 7, 2, 7, 6,
		3, 0, 4, 3, 4, 7
	};
	arraySize = ARRAYSIZE(indices);
	return indices;
}

void LightBeam::createGraphics()
{
	// load the shaders first
	MigUtil::theRend->loadVertexShader(lightBeamVertexShader, VDTYPE_POSITION_TEXTURE, SHADER_HINT_MVP);
	MigUtil::theRend->loadPixelShader(lightBeamPixelShader, SHADER_HINT_NONE);

	// create the cube object and assign the shaders
	Object* beamObj = MigUtil::theRend->createObject();
	beamObj->addShaderSet(lightBeamVertexShader, lightBeamPixelShader);

	// load mesh vertices
	unsigned int vertArraySize = 0;
	VertexPositionTexture* txtVertices = createVertices(vertArraySize);
	beamObj->loadVertexBuffer(txtVertices, vertArraySize, MigTech::VDTYPE_POSITION_TEXTURE);

	// load mesh indices
	unsigned int indArraySize = 0;
	unsigned short* txtIndices = createIndices(indArraySize);
	beamObj->loadIndexBuffer(txtIndices, indArraySize, MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	beamObj->setCulling(FACE_CULLING_BACK);
	_objBeam = beamObj;
}

void LightBeam::destroyGraphics()
{
	if (_objBeam != nullptr)
		MigUtil::theRend->deleteObject(_objBeam);
	_objBeam = nullptr;
}

static void loadMatrix(const GridInfo& gridInfo, float xCenter, float yCenter, float zOffset, float glowParam, const Matrix& worldMatrix)
{
	static Matrix locMatrix;
	locMatrix.identity();

	// make sure the center of the shaft is always facing the user
	if (gridInfo.orient == AXISORIENT_X || gridInfo.orient == AXISORIENT_Y)
		locMatrix.rotateZ(rad90);

	// scale the shaft to the correct size
	float scale = glowParam * (RADIUS_BY_LVL[gridInfo.dimen - 1]);
	locMatrix.scale(scale, scale, 1);

	// translate the shaft to the center of the slot
	locMatrix.translate(xCenter, yCenter, zOffset);

	// rotate the shaft to the correct orientation
	if (gridInfo.orient == AXISORIENT_X)
		locMatrix.rotateY(rad90);
	else if (gridInfo.orient == AXISORIENT_Y)
		locMatrix.rotateX(-rad90);

	locMatrix.multiply(worldMatrix);
	MigUtil::theRend->setModelMatrix(locMatrix);
}

void LightBeam::draw(const GridInfo& gridInfo, float xCenter, float yCenter, float glowParam, bool isGrowing, const Matrix& worldMatrix) const
{
	Color drawColor = gridInfo.fillCol;
	//drawColor.a = 0.5f * glowParam;
	MigUtil::theRend->setObjectColor(drawColor);
	MigUtil::theRend->setMiscValue(0, glowParam);
	MigUtil::theRend->setMiscValue(1, isGrowing);

	loadMatrix(gridInfo, xCenter, yCenter, 0, glowParam, worldMatrix);

	_objBeam->render();
}
