#include "pch.h"
#include "OverlayBase.h"
#include "MigUtil.h"

using namespace MigTech;
using namespace tinyxml2;

OverlayBase::OverlayBase(const std::string& name) : _overlayName(name),
	_parentAlpha(0.5F), _fadeDuration(MigTech::defFastFadeDuration), _fadeDurationInter(MigTech::defFastFadeDuration), _fadeMusic(false),
	_callback(nullptr), _nextOverlay(nullptr), _transType(OVERLAY_TRANSITION_NONE),
	_alpha(0), _scaleX(0), _scaleY(0), _rotateX(0), _rotateY(0), _rotateZ(0),
	_animDuration(0),
	_localFont(nullptr), _controls(this)
{
	_lcList.addToList(_controls);
}

OverlayBase::~OverlayBase()
{
	if (_localFont != nullptr)
		delete _localFont;
}

void OverlayBase::create()
{
	// open overlay configuration doc
	std::string xmlFile = _overlayName + ".xml";
	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument(xmlFile);
	if (pdoc != nullptr)
	{
		XMLElement* elem = pdoc->FirstChildElement("Overlay");
		if (elem != nullptr)
		{
			const char* font = elem->Attribute("Font");
			if (font != nullptr)
			{
				// need to call Font::create() before the controls call create()
				_localFont = new Font(font);
				_localFont->create();
			}

			const char* pa = elem->Attribute("ParentAlpha");
			_parentAlpha = MigUtil::parseFloat(pa, _parentAlpha);

			const char* fd = elem->Attribute("FadeDuration");
			_fadeDuration = MigUtil::parseInt(fd, _fadeDuration);

			const char* fdi = elem->Attribute("FadeDurationInter");
			_fadeDurationInter = MigUtil::parseInt(fdi, _fadeDurationInter);

			const char* fm = elem->Attribute("FadeMusic");
			_fadeMusic = MigUtil::parseBool(fm, _fadeMusic);

			XMLElement* controls = elem->FirstChildElement("Controls");
			if (controls != nullptr)
				_controls.loadFromXML(controls, _localFont);
		}
	}

	// lifecycle events
	_lcList.create();
	if (_localFont != nullptr)
		_lcList.addToList(*_localFont);
}

void OverlayBase::createGraphics()
{
	// lifecycle events
	_lcList.createGraphics();
}

void OverlayBase::windowSizeChanged()
{
	// lifecycle events
	_lcList.windowSizeChanged();
}

void OverlayBase::visibilityChanged(bool vis)
{
	// lifecycle events
	_lcList.visibilityChanged(vis);
}

void OverlayBase::suspend()
{
	// lifecycle events
	_lcList.suspend();
}

void OverlayBase::resume()
{
	// lifecycle events
	_lcList.resume();
}

void OverlayBase::destroyGraphics()
{
	// lifecycle events
	_lcList.destroyGraphics();
}

void OverlayBase::destroy()
{
	// lifecycle events
	_lcList.destroyGraphics();
	_lcList.removeAll();
	_controls.removeAllControls();
}

bool OverlayBase::pointerPressed(float x, float y)
{
	return _controls.pointerPressed(x, y);
}

void OverlayBase::pointerReleased(float x, float y)
{
	_controls.pointerReleased(x, y);
}

void OverlayBase::pointerMoved(float x, float y, bool isInContact)
{
	_controls.pointerMoved(x, y, isInContact);
}

void OverlayBase::keyDown(VIRTUAL_KEY key)
{
}

void OverlayBase::keyUp(VIRTUAL_KEY key)
{
}

bool OverlayBase::update()
{
	return true;
}

void OverlayBase::render()
{
	static Matrix localMat;
	localMat.identity();
	if (_scaleX != 1 || _scaleY != 1)
		localMat.scale(_scaleX, _scaleY, 1);
	if (_rotateX != 0)
		localMat.rotateX(_rotateX);
	if (_rotateY != 0)
		localMat.rotateY(_rotateY);
	if (_rotateZ != 0)
		localMat.rotateZ(_rotateZ);

	// draw the UI controls first
	_controls.draw(_alpha, localMat);

	MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
	MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

	draw(_alpha, localMat);
}

void OverlayBase::startIntroAnimation(long duration)
{
	// -1 means use the default
	if (duration == -1)
		duration = _fadeDuration;

	// inform the callback
	if (_callback != nullptr)
		_callback->onOverlayIntro(duration);

	if (duration > 0)
	{
		// create animations to scale/fade in
		AnimItem animAlpha(this);
		animAlpha.configSimpleAnim(0, 1, duration, AnimItem::ANIM_TYPE_LINEAR);
		_idAlphaAnim = MigUtil::theAnimList->addItem(animAlpha);

		float fsx[] = { 0.0f, 0.50f, 0.80f, 1.0f, 1.15f, 1.2f, 1.15f, 1.1f, 1.05f, 1.0f };
		AnimItem animScaleX(this);
		animScaleX.configParametricAnim(0, 1, duration, fsx, ARRAYSIZE(fsx));
		_idScaleXAnim = MigUtil::theAnimList->addItem(animScaleX);

		float fsy[] = { 0.0f, 0.17f, 0.34f, 0.5f, 0.64f, 0.77f, 0.87f, 0.94f, 0.98f, 1.0f };
		AnimItem animScaleY(this);
		animScaleY.configParametricAnim(0, 1, duration, fsy, ARRAYSIZE(fsy));
		_idScaleYAnim = MigUtil::theAnimList->addItem(animScaleY);

		AnimItem animRotateZ(this);
		animRotateZ.configSimpleAnim(MigUtil::convertToRadians(10), 0, duration, AnimItem::ANIM_TYPE_LINEAR);
		_idRotateZAnim = MigUtil::theAnimList->addItem(animRotateZ);

		// create a simple timer animation to track the end of the transition
		AnimItem timerAnim(this);
		timerAnim.configTimer(duration, false, (void*) 1);
		_idTimerAnim = MigUtil::theAnimList->addItem(timerAnim);

		_transType = OVERLAY_TRANSITION_INTRO;
		_animDuration = duration;
	}
	else
	{
		onAnimComplete(OVERLAY_TRANSITION_INTRO);
		if (_callback != nullptr)
			_callback->onOverlayComplete(OVERLAY_TRANSITION_INTRO, duration);
		_scaleX = _scaleY = 1;
		_alpha = 1;
	}
}

void OverlayBase::startInterAnimation(OVERLAY_TRANSITION_TYPE transType, long duration, bool isIncoming)
{
	// -1 means use the default
	if (duration == -1)
		duration = _fadeDurationInter;

	if (duration > 0)
	{
		if (transType == OVERLAY_TRANSITION_ROTATE_NEXT || transType == OVERLAY_TRANSITION_ROTATE_BACK)
		{
			int dir = ((transType == OVERLAY_TRANSITION_ROTATE_NEXT) ? 1 : -1);
			_rotateY = (isIncoming ? -dir * MigUtil::convertToRadians(90) : 0);

			// create animations to rotate
			AnimItem animItem(this);
			if (isIncoming)
				animItem.configSimpleAnim(_rotateY, 0, duration, AnimItem::ANIM_TYPE_LINEAR);
			else
				animItem.configSimpleAnim(_rotateY, dir * MigUtil::convertToRadians(90), duration, AnimItem::ANIM_TYPE_LINEAR);
			_idRotateYAnim = MigUtil::theAnimList->addItem(animItem);
		}
		else if (transType == OVERLAY_TRANSITION_FADE)
		{
			_scaleX = _scaleY = (isIncoming ? 0.9f : 1.0f);
			_alpha = (isIncoming ? 0.0f : 1.0f);

			// create animations to fade/scale
			AnimItem animAlpha(this);
			animAlpha.configSimpleAnim(_alpha, isIncoming ? 1.0f : 0.0f, duration, AnimItem::ANIM_TYPE_LINEAR);
			_idAlphaAnim = MigUtil::theAnimList->addItem(animAlpha);
			AnimItem animScale(this);
			animScale.configSimpleAnim(_scaleX, isIncoming ? 1.0f : 0.9f, duration, AnimItem::ANIM_TYPE_LINEAR);
			_idScaleXAnim = MigUtil::theAnimList->addItem(animScale);
			_idScaleYAnim = MigUtil::theAnimList->addItem(animScale);
		}

		// create a simple timer animation to track the end of the transition
		AnimItem timerAnim(this);
		timerAnim.configTimer(duration, false, reinterpret_cast<void*>(isIncoming ? 1 : 0));
		_idTimerAnim = MigUtil::theAnimList->addItem(timerAnim);

		_transType = transType;
		_animDuration = duration;
	}
}

void OverlayBase::startExitAnimation(long duration)
{
	// -1 means use the default
	if (duration == -1)
		duration = _fadeDuration;

	// inform the callback
	if (_callback != nullptr)
		_callback->onOverlayExit(duration);

	if (duration > 0)
	{
		// create animations to scale/fade out
		AnimItem animAlpha(this);
		animAlpha.configSimpleAnim(1, 0, duration, AnimItem::ANIM_TYPE_LINEAR);
		_idAlphaAnim = MigUtil::theAnimList->addItem(animAlpha);

		float fsx[] = { 1.0f, 1.05f, 1.1f, 1.15f, 1.2f, 1.15f, 1.0f, 0.8f, 0.5f, 0.0f };
		AnimItem animScaleX(this);
		animScaleX.configParametricAnim(0, 1, duration, fsx, ARRAYSIZE(fsx));
		_idScaleXAnim = MigUtil::theAnimList->addItem(animScaleX);

		float fsy[] = { 1.0f, 0.98f, 0.94f, 0.87f, 0.77f, 0.64f, 0.5f, 0.34f, 0.17f, 0.0f };
		AnimItem animScaleY(this);
		animScaleY.configParametricAnim(0, 1, duration, fsy, ARRAYSIZE(fsy));
		_idScaleYAnim = MigUtil::theAnimList->addItem(animScaleY);

		AnimItem animRotateZ(this);
		animRotateZ.configSimpleAnim(0, MigUtil::convertToRadians(-10), duration, AnimItem::ANIM_TYPE_LINEAR);
		_idRotateZAnim = MigUtil::theAnimList->addItem(animRotateZ);

		// create a simple timer animation to track the end of the transition
		AnimItem timerAnim(this);
		timerAnim.configTimer(duration, false, (void*)0);
		_idTimerAnim = MigUtil::theAnimList->addItem(timerAnim);

		_transType = OVERLAY_TRANSITION_EXIT;
		_animDuration = duration;
	}
	else
	{
		onAnimComplete(OVERLAY_TRANSITION_EXIT);
		if (_callback != nullptr)
			_callback->onOverlayComplete(OVERLAY_TRANSITION_EXIT, duration);
		_scaleX = _scaleY = 0;
	}
}

bool OverlayBase::doFrame(int id, float newVal, void* optData)
{
	if (_idAlphaAnim == id)
		_alpha = newVal;
	else if (_idScaleXAnim == id)
		_scaleX = newVal;
	else if (_idScaleYAnim == id)
		_scaleY = newVal;
	else if (_idRotateXAnim == id)
		_rotateX = newVal;
	else if (_idRotateYAnim == id)
		_rotateY = newVal;
	else if (_idRotateZAnim == id)
		_rotateZ = newVal;
	return true;
}

void OverlayBase::animComplete(int id, void* optData)
{
	if (_idTimerAnim == id)
	{
		_idTimerAnim = 0;

		onAnimComplete(_transType);
		if (_callback != nullptr && optData == (void*)0)
			_callback->onOverlayComplete(_transType, _animDuration);

		_transType = OVERLAY_TRANSITION_NONE;
	}
	else if (_idAlphaAnim == id)
	{
		_idAlphaAnim = 0;
	}
	else if (_idScaleXAnim == id)
	{
		_idScaleXAnim = 0;
	}
	else if (_idScaleYAnim == id)
	{
		_idScaleYAnim = 0;
	}
	else if (_idRotateXAnim == id)
	{
		_idRotateXAnim = 0;
	}
	else if (_idRotateYAnim == id)
	{
		_idRotateYAnim = 0;
	}
	else if (_idRotateZAnim == id)
	{
		_idRotateZAnim = 0;
	}
}

void OverlayBase::onTap(float x, float y)
{
	LOGINFO("(OverlayBase::onTap) Tap at (%f, %f) detected", x, y);
	_controls.onTap(x, y);
}

void OverlayBase::onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe)
{
	LOGINFO("(OverlayBase::onSwipe) Swipe at (%f, %f, %d) detected", x, y, swipe);
}

void OverlayBase::onKey(VIRTUAL_KEY key)
{
	LOGINFO("(OverlayBase::onKey) Key press (%d) detected", key);
	if (key == ESCAPE)
		onBackKey();
}

bool OverlayBase::onBackKey()
{
	LOGINFO("(OverlayBase::onBackKey) Back key detected");
	return false;
}

void OverlayBase::onClick(int id, ControlBase* sender)
{
	LOGINFO("(OverlayBase::onClick) Control click detected by ID=%d", id);
	if (id == Button::ID_BACK_BUTTON)
		onBackKey();
}

void OverlayBase::onSlide(int id, ControlBase* sender, float val)
{
	LOGINFO("(OverlayBase::onSlide) Control slide detected by ID=%d, val=%f", id, val);
}

ControlBase* OverlayBase::allocControl(const char* tag)
{
	return nullptr;
}

void OverlayBase::onAnimComplete(OVERLAY_TRANSITION_TYPE transType)
{
}

void OverlayBase::draw(float alpha, const Matrix& mat)
{
}
