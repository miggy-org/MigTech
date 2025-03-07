#include "pch.h"
#include "cube.h"
#include "../core/MigInclude.h"

using namespace TestGame;
using namespace MigTech;

Cube::Cube() :
	m_cube(nullptr), m_translateX(0), m_translateY(0), m_rotateDir(1)
{
	// setup the animation(s)
	AnimItem animItem1(this);
	animItem1.configSimpleAnim(0, 45, 1000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
	m_idAnim1 = MigUtil::theAnimList->addItem(animItem1);
	AnimItem animItem2(this);
	animItem2.configSimpleAnim(0, 1, 2000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
	m_idAnim2 = MigUtil::theAnimList->addItem(animItem2);
}

Cube::~Cube()
{
	MigUtil::theAnimList->removeItem(m_idAnim1);
	MigUtil::theAnimList->removeItem(m_idAnim2);
}

void Cube::CreateDeviceDependentResources()
{
	// load the shaders first
	MigUtil::theRend->loadVertexShader("SampleVertexShader", VDTYPE_POSITION_COLOR, SHADER_HINT_MVP);
	MigUtil::theRend->loadPixelShader("SamplePixelShader", SHADER_HINT_NONE);

	// create the cube object and assign the shaders
	Object* cube = MigUtil::theRend->createObject();
	cube->addShaderSet("SampleVertexShader", "SamplePixelShader");

	// Load mesh vertices. Each vertex has a position and a color.
	static const VertexPositionColor cubeVertices[] = 
	{
		{ Vector3(-0.5f, -0.5f, -0.5f), Color(0.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3(-0.5f, -0.5f,  0.5f), Color(0.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3(-0.5f,  0.5f, -0.5f), Color(0.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3(-0.5f,  0.5f,  0.5f), Color(0.0f, 1.0f, 1.0f, 1.0f) },
		{ Vector3( 0.5f, -0.5f, -0.5f), Color(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f, -0.5f,  0.5f), Color(1.0f, 0.0f, 1.0f, 1.0f) },
		{ Vector3( 0.5f,  0.5f, -0.5f), Color(1.0f, 1.0f, 0.0f, 1.0f) },
		{ Vector3( 0.5f,  0.5f,  0.5f), Color(1.0f, 1.0f, 1.0f, 1.0f) },
	};
	cube->loadVertexBuffer(cubeVertices, ARRAYSIZE(cubeVertices), MigTech::VDTYPE_POSITION_COLOR);

	// Load mesh indices. Each trio of indices represents
	// a triangle to be rendered on the screen.
	// For example: 0,2,1 means that the vertices with indexes
	// 0, 2 and 1 from the vertex buffer compose the 
	// first triangle of this mesh.
	static const unsigned short cubeIndices [] =
	{
		0,2,1, // -x
		1,2,3,

		4,5,6, // +x
		5,7,6,

		0,1,5, // -y
		0,5,4,

		2,6,7, // +y
		2,7,3,

		0,4,6, // -z
		0,6,2,

		1,3,7, // +z
		1,7,5,
	};
	cube->loadIndexBuffer(cubeIndices, ARRAYSIZE(cubeIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	cube->setCulling(FACE_CULLING_FRONT);

	m_cube = cube;
}

void Cube::ReleaseDeviceDependentResources()
{
	if (m_cube)
		MigUtil::theRend->deleteObject(m_cube);
	m_cube = nullptr;
}

bool Cube::doFrame(int id, float newVal, void* optData)
{
	if (id == m_idAnim1)
	{
		m_rotateY = static_cast<float>(fmod(MigUtil::convertToRadians(newVal), 2 * MigTech::PI));
	}
	else if (id == m_idAnim2)
	{
		newVal = (float) fmod(newVal, 2);
		if (newVal > 1)
			newVal = 2 - newVal;
		//m_translateX = newVal - 0.5f;
		m_alpha = newVal;
	}

	return true;
}

void Cube::animComplete(int id, void* optData)
{
}

void Cube::Translate(float x, float y)
{
	m_translateX = x;
	m_translateY = y;
}

void Cube::RotateDir(float r)
{
	m_rotateDir = r;
}

// Renders one frame using the vertex and pixel shaders.
void Cube::Render()
{
	if (m_cube)
	{
		m_matrix.identity();
		m_matrix.rotateY(m_rotateDir*m_rotateY);
		m_matrix.translate(m_translateX, m_translateY, 0);
		MigUtil::theRend->setModelMatrix(m_matrix);

		MigUtil::theRend->setObjectColor(Color(1, 1, 1, m_alpha));
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
		//MigUtil::theRend->setBlending(BLEND_STATE_NONE);

		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_LESS, true);

		m_cube->render();
	}
}
