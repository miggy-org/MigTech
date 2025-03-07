#include "pch.h"
#include "ShadowPass.h"
#include "CubeUtil.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

// shadow map settings
static const std::string shadowTexName = "_shadowTex";
static const float shadowAlpha = 0.5f;
static const int shadowTexWidth = 512;
static const int shadowTexHeight = 512;
static const float shadowCameraHeight = 10;
static const float shadowNearPlane = 2;
static const float shadowCatchFloor = -2;
static const float shadowCatchRadius = 5;

ShadowPass::ShadowPass() : RenderPass(RENDER_PASS_PRE)
{
	_shadowPoly = nullptr;
	_shadowObjColor = Color(colBlack, shadowAlpha);
}

bool ShadowPass::init()
{
	if (!RenderPass::init(shadowTexName, IMG_FORMAT_ALPHA, shadowTexWidth, shadowTexHeight, 0))
	{
		LOGWARN("(ShadowPass::init) Failed to create shadow target using alpha, trying RGBA...");
		if (!RenderPass::init(shadowTexName, IMG_FORMAT_RGBA, shadowTexWidth, shadowTexHeight, 0))
		{
			LOGWARN("(ShadowPass::init) Failed to create shadow target using RGBA, there will be no shadows");
			return false;
		}
	}

	// create the shadow rendering pass matrices
	float nearDist = shadowCameraHeight - shadowNearPlane;
	float farDist = shadowCameraHeight - shadowCatchFloor;
	float nearPlaneRadius = shadowCatchRadius * (nearDist / farDist);
	_proj.loadPerspectiveFovRH(
		(float)(2 * atan(nearPlaneRadius / (shadowCameraHeight - shadowNearPlane))),
		1, nearDist, farDist);
	_view.loadLookAtRH(
		Vector3(0, shadowCameraHeight, 0),
		MigTech::ptOrigin,
		MigTech::unitZ);

	return true;
}

void ShadowPass::createGraphics()
{
	RenderPass::createGraphics();

	// note that we still create the object if CubeUtil::useShadows is off
	if (_shadowPoly == nullptr && RenderPass::isValid())
	{
		// create the texture object and assign the shaders
		Object* txtObj = MigUtil::theRend->createObject();
		txtObj->addShaderSet(MIGTECH_VSHADER_POS_TEX_TRANSFORM, MIGTECH_PSHADER_TEX);

		// UV coords change based upon whether the render target is top/down or botom/up
		bool isBottomUp = ((_target->getCaps() & IMAGE_CAPS_BOTTOM_UP) ? true : false);
		Vector2 uv1 = (isBottomUp ? Vector2(1.0f, 1.0f) : Vector2(1.0f, 0.0f));
		Vector2 uv2 = (isBottomUp ? Vector2(0.0f, 1.0f) : Vector2(0.0f, 0.0f));
		Vector2 uv3 = (isBottomUp ? Vector2(0.0f, 0.0f) : Vector2(0.0f, 1.0f));
		Vector2 uv4 = (isBottomUp ? Vector2(1.0f, 0.0f) : Vector2(1.0f, 1.0f));

		// load mesh vertices (position / color)
		VertexPositionTexture txtVertices[4];
		txtVertices[0] = VertexPositionTexture(Vector3(-shadowCatchRadius, shadowCatchFloor,  shadowCatchRadius), uv1);
		txtVertices[1] = VertexPositionTexture(Vector3( shadowCatchRadius, shadowCatchFloor,  shadowCatchRadius), uv2);
		txtVertices[2] = VertexPositionTexture(Vector3( shadowCatchRadius, shadowCatchFloor, -shadowCatchRadius), uv3);
		txtVertices[3] = VertexPositionTexture(Vector3(-shadowCatchRadius, shadowCatchFloor, -shadowCatchRadius), uv4);
		txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

		// load mesh indices
		const unsigned short txtIndices[] =
		{
			0, 1, 2, 0, 2, 3,
		};
		txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

		// texturing
		txtObj->setImage(0, shadowTexName, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

		// culling
		txtObj->setCulling(FACE_CULLING_BACK);
		_shadowPoly = txtObj;
	}
}

void ShadowPass::destroyGraphics()
{
	if (_shadowPoly != nullptr && MigUtil::theRend != nullptr)
		MigUtil::theRend->deleteObject(_shadowPoly);
	_shadowPoly = nullptr;

	RenderPass::destroyGraphics();
}

bool ShadowPass::isValid() const
{
	return (CubeUtil::useShadows ? RenderPass::isValid() : false);
}

bool ShadowPass::preRender()
{
	// flag indicates the shadow pass
	CubeUtil::renderPass = RENDER_PASS_SHADOW;
	return RenderPass::preRender();
}

void ShadowPass::postRender()
{
	// flag indicates the final pass
	CubeUtil::renderPass = RENDER_PASS_FINAL;
	RenderPass::postRender();
}

void ShadowPass::draw()
{
	if (_shadowPoly && CubeUtil::useShadows)
	{
		MigUtil::theRend->setModelMatrix(nullptr);
		//MigUtil::theRend->setViewMatrix(_view);
		//MigUtil::theRend->setProjectionMatrix(_proj);

		MigUtil::theRend->setObjectColor(_shadowObjColor);
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

		_shadowPoly->render();
	}
}
