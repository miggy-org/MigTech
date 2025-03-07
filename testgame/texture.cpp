#include "pch.h"
#include "texture.h"
#include "../core/MigInclude.h"

using namespace TestGame;
using namespace MigTech;

Texture::Texture() :
	m_txtObj(nullptr)
{
}

Texture::~Texture()
{
}

void Texture::CreateDeviceDependentResources()
{
	// load the shaders first
	MigUtil::theRend->loadVertexShader("TextureVertexShader", VDTYPE_POSITION_TEXTURE, SHADER_HINT_MVP);
	MigUtil::theRend->loadPixelShader("TexturePixelShader", SHADER_HINT_NONE);

	// load the texture map
	std::string textureName = "pngtest.png";
	MigUtil::theRend->loadImage(textureName, textureName, LOAD_IMAGE_NONE);

	// create the texture object and assign the shaders
	Object* txtObj = MigUtil::theRend->createObject();
	txtObj->addShaderSet("TextureVertexShader", "TexturePixelShader");

	// Load mesh vertices. Each vertex has a position and a color.
	static const VertexPositionTexture txtVertices[] = 
	{
		{ Vector3(-1.0f, -0.8f, -1.0f), Vector2(0.0f, 1.0f) },
		{ Vector3(-1.0f,  0.8f, -1.0f), Vector2(0.0f, 0.0f) },
		{ Vector3( 1.0f, -0.8f, -1.0f), Vector2(1.0f, 1.0f) },
		{ Vector3( 1.0f,  0.8f, -1.0f), Vector2(1.0f, 0.0f) },
	};
	txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

	// Load mesh indices. Each trio of indices represents
	// a triangle to be rendered on the screen.
	// For example: 0,2,1 means that the vertices with indexes
	// 0, 2 and 1 from the vertex buffer compose the 
	// first triangle of this mesh.
	static const unsigned short txtIndices [] =
	{
		0,2,1,
		1,2,3,
	};
	txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// texturing
	txtObj->setImage(0, textureName, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

	// culling
	txtObj->setCulling(FACE_CULLING_BACK);

	m_txtObj = txtObj;
}

void Texture::ReleaseDeviceDependentResources()
{
	if (m_txtObj)
		MigUtil::theRend->deleteObject(m_txtObj);
	m_txtObj = nullptr;
}

// Renders one frame using the vertex and pixel shaders.
void Texture::Render()
{
	if (m_txtObj)
	{
		MigUtil::theRend->setModelMatrix(nullptr);

		MigUtil::theRend->setObjectColor(Color(1, 1, 1, 1));
		//MigUtil::theRend->setBlending(BLEND_STATE_NONE);
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);

		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

		m_txtObj->render();
	}
}
