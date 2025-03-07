#include "pch.h"
#include "BgBase.h"
#include "MigUtil.h"

using namespace MigTech;
using namespace tinyxml2;

///////////////////////////////////////////////////////////////////////////
// static function to load configurable background handlers from XML strings

BgBase* BgBase::loadHandlerFromXML(tinyxml2::XMLElement* xml)
{
	if (xml == nullptr)
		throw std::invalid_argument("(BgBase::loadHandlerFromXML) XML element missing");
	BgBase* bgHandler = nullptr;

	const char* type = xml->Attribute("Type");
	if (!strcmp(type, "overlay"))
	{
		const char* image = xml->Attribute("Image");
		const char* overlay = xml->Attribute("Overlay");
		const char* per1 = xml->Attribute("Period");
		const char* per2 = xml->Attribute("PeriodOverlay");

		float imagePeriods[2];
		MigUtil::parseFloats(per1, imagePeriods, 2);
		float overlayPeriods[2];
		MigUtil::parseFloats(per2, overlayPeriods, 2);

		OverlayBg* bg = new OverlayBg();
		bg->initImage(image, imagePeriods[0], imagePeriods[1]);
		bg->initOverlay(overlay, overlayPeriods[0], overlayPeriods[1]);
		bgHandler = bg;
	}
	else if (!strcmp(type, "cycle"))
	{
		const char* image1 = xml->Attribute("Image");
		const char* image2 = xml->Attribute("Image2");
		long period = MigUtil::parseInt(xml->Attribute("Period"), 2000);
		bgHandler = new CycleBg(image1, image2, period);
	}
	else
	{
		const char* image = xml->Attribute("Image");
		bgHandler = new BgBase(image);
	}

	return bgHandler;
}

///////////////////////////////////////////////////////////////////////////
// base class for background handler

BgBase::BgBase()
	: _screenPoly(nullptr), _isVisible(true), _isOpaque(true)
{
}

BgBase::BgBase(const std::string& bgResID)
	: _screenPoly(nullptr), _isVisible(true), _isOpaque(true)
{
	init(bgResID);
}

BgBase::~BgBase()
{
}

bool BgBase::init(const std::string& bgResID)
{
	_bgResID = bgResID;
	return true;
}

void BgBase::createGraphics()
{
	if (_screenPoly == nullptr && _bgResID.length() > 0)
	{
		// load the texture map
		if (MigUtil::theRend->loadImage(_bgResID, _bgResID, LOAD_IMAGE_NONE) != nullptr)
		{
			// create the texture object and assign the shaders
			Object* txtObj = MigUtil::theRend->createObject();
			txtObj->addShaderSet(MIGTECH_VSHADER_POS_TEX_NO_TRANSFORM, MIGTECH_PSHADER_TEX);

			// load mesh vertices (position / color)
			VertexPositionTexture txtVertices[4];
			txtVertices[0] = VertexPositionTexture(Vector3(-1, -1, 0), Vector2(0.0f, 1.0f));
			txtVertices[1] = VertexPositionTexture(Vector3(-1, 1, 0), Vector2(0.0f, 0.0f));
			txtVertices[2] = VertexPositionTexture(Vector3(1, -1, 0), Vector2(1.0f, 1.0f));
			txtVertices[3] = VertexPositionTexture(Vector3(1, 1, 0), Vector2(1.0f, 0.0f));
			txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

			// load mesh indices
			const unsigned short txtIndices[] =
			{
				0, 2, 1,
				1, 2, 3,
			};
			txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

			// texturing
			txtObj->setImage(0, _bgResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

			// culling
			txtObj->setCulling(FACE_CULLING_BACK);
			_screenPoly = txtObj;
		}
		else
			LOGWARN("(BgBase::createGraphics) Could not load image '%s'", _bgResID.c_str());
	}
}

void BgBase::destroyGraphics()
{
	if (_screenPoly)
		MigUtil::theRend->deleteObject(_screenPoly);
	_screenPoly = nullptr;
	MigUtil::theRend->unloadImage(_bgResID);
}

void BgBase::update()
{
}

void BgBase::render() const
{
	if (_screenPoly && _isVisible)
	{
		//MigUtil::theRend->setModelMatrix(nullptr);
		//MigUtil::theRend->setViewMatrix(nullptr);
		//MigUtil::theRend->setProjectionMatrix(nullptr);

		MigUtil::theRend->setObjectColor(MigTech::colWhite);
		MigUtil::theRend->setBlending(_isOpaque ? BLEND_STATE_NONE : BLEND_STATE_SRC_ALPHA);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

		_screenPoly->render();
	}
}

///////////////////////////////////////////////////////////////////////////
// background handler that cycles through 2 images

CycleBg::CycleBg() : _screenPoly2(nullptr)
{
	_colOther = colWhite;
}

CycleBg::CycleBg(const std::string& image1, const std::string& image2, long duration) : _screenPoly2(nullptr)
{
	_colOther = colWhite;
	init(image1, image2, duration);
}

bool CycleBg::init(const std::string& image1, const std::string& image2, long duration)
{
	BgBase::init(image1);
	_bgRes2ID = image2;
	_dur = duration;

	return true;
}

void CycleBg::create()
{
	BgBase::create();

	// set up the fade animation, if specified
	if (_dur > 0)
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 2, _dur, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
		_idAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

void CycleBg::createGraphics()
{
	BgBase::createGraphics();

	if (!_bgRes2ID.empty() && _screenPoly2 == nullptr)
	{
		// load the texture map (set alpha to full in case it's empty since we'll need blending)
		if (MigUtil::theRend->loadImage(_bgRes2ID, _bgRes2ID, LOAD_IMAGE_SET_ALPHA) != nullptr)
		{
			// create the texture object and assign the shaders
			Object* txtObj = MigUtil::theRend->createObject();
			txtObj->addShaderSet(MIGTECH_VSHADER_POS_TEX_NO_TRANSFORM, MIGTECH_PSHADER_TEX);

			// load mesh vertices (position / color)
			VertexPositionTexture txtVertices[4];
			txtVertices[0] = VertexPositionTexture(Vector3(-1, -1, 0), Vector2(0.0f, 1.0f));
			txtVertices[1] = VertexPositionTexture(Vector3(-1, 1, 0), Vector2(0.0f, 0.0f));
			txtVertices[2] = VertexPositionTexture(Vector3(1, -1, 0), Vector2(1.0f, 1.0f));
			txtVertices[3] = VertexPositionTexture(Vector3(1, 1, 0), Vector2(1.0f, 0.0f));
			txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

			// load mesh indices
			static const unsigned short txtIndices[] =
			{
				0, 2, 1,
				1, 2, 3,
			};
			txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

			// texturing
			txtObj->setImage(0, _bgRes2ID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);

			// culling
			txtObj->setCulling(FACE_CULLING_BACK);
			_screenPoly2 = txtObj;
		}
		else
			LOGWARN("(BgBase::createGraphics) Could not load image '%s'", _bgRes2ID.c_str());
	}
}

void CycleBg::destroyGraphics()
{
	BgBase::destroyGraphics();

	if (_screenPoly2)
		MigUtil::theRend->deleteObject(_screenPoly2);
	_screenPoly2 = nullptr;
	MigUtil::theRend->unloadImage(_bgRes2ID);
}

void CycleBg::destroy()
{
	_idAnim.clearAnim();

	BgBase::destroy();
}

void CycleBg::render() const
{
	BgBase::render();

	if (_screenPoly2 && _isVisible)
	{
		MigUtil::theRend->setObjectColor(_colOther);
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

		_screenPoly2->render();
	}
}

bool CycleBg::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
	{
		newVal = newVal - 2*((int)newVal / 2);
		_colOther.a = (newVal > 1 ? 2 - newVal : newVal);
	}
	return true;
}

void CycleBg::animComplete(int id, void* optData)
{
	_idAnim = 0;
}

///////////////////////////////////////////////////////////////////////////
// background handler that is a composite of 2 images, each of which can animate

static const std::string VSHADER_OVERLAY = "mtvs_OverlayBackground";

OverlayBg::OverlayBg() : BgBase(),
	_overlayPoly(nullptr),
	_uPeriodBg(0), _vPeriodBg(0),
	_uPeriodOv(0), _vPeriodOv(0)
{
}

bool OverlayBg::initImage(const std::string& image, float uPeriod, float vPeriod)
{
	BgBase::init(image);
	_uPeriodBg = uPeriod;
	_vPeriodBg = vPeriod;
	return true;
}

bool OverlayBg::initOverlay(const std::string& image, float uPeriod, float vPeriod)
{
	_bgResOverlay = image;
	_uPeriodOv = uPeriod;
	_vPeriodOv = vPeriod;
	return true;
}

void OverlayBg::create()
{
	BgBase::create();

	_uBgOffset = _vBgOffset = 0;
	_uOvOffset = _vOvOffset = 0;

	// set up the timer animation, if needed
	if (_uPeriodBg != 0 || _vPeriodBg != 0 || _uPeriodOv != 0 || _vPeriodOv != 0)
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 1, 1000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
		_idAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

void OverlayBg::createGraphics()
{
	// this class uses a special shader
	MigUtil::theRend->loadVertexShader(VSHADER_OVERLAY, VDTYPE_POSITION_TEXTURE, SHADER_HINT_NONE);

	// don't call BgBase::createGraphics() because we use our own special vertex shader
	if (_screenPoly == nullptr && _bgResID.length() > 0)
	{
		// load the texture map
		if (MigUtil::theRend->loadImage(_bgResID, _bgResID, LOAD_IMAGE_NONE) != nullptr)
		{
			// create the texture object and assign the shaders
			Object* txtObj = MigUtil::theRend->createObject();
			txtObj->addShaderSet(VSHADER_OVERLAY, MIGTECH_PSHADER_TEX);

			// load mesh vertices (position / color)
			VertexPositionTexture txtVertices[4];
			txtVertices[0] = VertexPositionTexture(Vector3(-1, -1, 0), Vector2(0.0f, 1.0f));
			txtVertices[1] = VertexPositionTexture(Vector3(-1, 1, 0), Vector2(0.0f, 0.0f));
			txtVertices[2] = VertexPositionTexture(Vector3(1, -1, 0), Vector2(1.0f, 1.0f));
			txtVertices[3] = VertexPositionTexture(Vector3(1, 1, 0), Vector2(1.0f, 0.0f));
			txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

			// load mesh indices
			const unsigned short txtIndices[] =
			{
				0, 2, 1,
				1, 2, 3,
			};
			txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

			// texturing
			txtObj->setImage(0, _bgResID, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_REPEAT);

			// culling
			txtObj->setCulling(FACE_CULLING_BACK);
			_screenPoly = txtObj;
		}
		else
			LOGWARN("(OverlayBg::createGraphics) Could not load image '%s'", _bgResID.c_str());
	}

	if (!_bgResOverlay.empty() && _overlayPoly == nullptr)
	{
		// load the texture map
		if (MigUtil::theRend->loadImage(_bgResOverlay, _bgResOverlay, LOAD_IMAGE_NONE) != nullptr)
		{
			// create the texture object and assign the shaders
			Object* txtObj = MigUtil::theRend->createObject();
			txtObj->addShaderSet(VSHADER_OVERLAY, MIGTECH_PSHADER_TEX);

			// load mesh vertices (position / color)
			VertexPositionTexture txtVertices[4];
			txtVertices[0] = VertexPositionTexture(Vector3(-1, -1, 0), Vector2(0.0f, 1.0f));
			txtVertices[1] = VertexPositionTexture(Vector3(-1, 1, 0), Vector2(0.0f, 0.0f));
			txtVertices[2] = VertexPositionTexture(Vector3(1, -1, 0), Vector2(1.0f, 1.0f));
			txtVertices[3] = VertexPositionTexture(Vector3(1, 1, 0), Vector2(1.0f, 0.0f));
			txtObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION_TEXTURE);

			// load mesh indices
			static const unsigned short txtIndices[] =
			{
				0, 2, 1,
				1, 2, 3,
			};
			txtObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

			// texturing
			txtObj->setImage(0, _bgResOverlay, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_REPEAT);

			// culling
			txtObj->setCulling(FACE_CULLING_BACK);
			_overlayPoly = txtObj;
		}
		else
			LOGWARN("(OverlayBg::createGraphics) Could not load image '%s'", _bgResOverlay.c_str());
	}
}

void OverlayBg::destroyGraphics()
{
	BgBase::destroyGraphics();

	if (_overlayPoly)
		MigUtil::theRend->deleteObject(_overlayPoly);
	_overlayPoly = nullptr;
	MigUtil::theRend->unloadImage(_bgResOverlay);
}

void OverlayBg::destroy()
{
	_idAnim.clearAnim();

	BgBase::destroy();
}

void OverlayBg::render() const
{
	MigUtil::theRend->setMiscValue(0, _uBgOffset);
	MigUtil::theRend->setMiscValue(1, _vBgOffset);
	BgBase::render();

	if (_overlayPoly && _isVisible)
	{
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);
		MigUtil::theRend->setMiscValue(0, _uOvOffset);
		MigUtil::theRend->setMiscValue(1, _vOvOffset);
		_overlayPoly->render();
	}
}

static float computeOffset(float value, float period)
{
	if (period == 0)
		return 0;

	float offset = (float) (value / fabs(period));
	if (period > 0)
		offset = -offset;
	return offset;
}

bool OverlayBg::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
	{
		_uBgOffset = computeOffset(newVal, _uPeriodBg);
		_vBgOffset = computeOffset(newVal, _vPeriodBg);
		_uOvOffset = computeOffset(newVal, _uPeriodOv);
		_vOvOffset = computeOffset(newVal, _vPeriodOv);
	}

	return true;
}

void OverlayBg::animComplete(int id, void* optData)
{
	if (_idAnim == id)
		_idAnim = 0;
}
