#include "pch.h"
#include "ScreenBase.h"
#include "MigConst.h"
#include "MigUtil.h"

using namespace MigTech;
using namespace tinyxml2;

ScreenBase::ScreenBase(const std::string& name)
	: _screenName(name),
	_bgHandler(nullptr),
	_overlayHandler(nullptr),
	_fadeOverlay(nullptr),
	_fadeDuration(MigTech::defFadeDuration),
	_overlay(nullptr),
	_expiredOverlay(nullptr),
	_musicFade(false),
	_musicStopOnExit(true),
	_swipeLocked(SWIPE_NONE),
	_localFont(nullptr),
	_controls(this)
{
	_lcList.addToList(_controls);

	// the threshold values determine what constitutes a swipe/tap
	_maxThreshold = 0.1f;
	_tapThreshold = 0.1f;
}

ScreenBase::~ScreenBase()
{
	if (_localFont != nullptr)
		delete _localFont;
}

// override to provide the next screen after this one closes
ScreenBase* ScreenBase::getNextScreen()
{
	return nullptr;
}

// returns screen name for debugging
const std::string& ScreenBase::getScreenName()
{
	return _screenName;
}

// called to create the screen from scratch, but doesn't load graphics
void ScreenBase::create()
{
	// set the animation list, which will replace any existing one
	if (MigUtil::theAnimList != nullptr)
		delete MigUtil::theAnimList;
	MigUtil::theAnimList = new AnimList();

	// open screen configuration doc
	std::string xmlFile = _screenName + ".xml";
	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument(xmlFile);
	if (pdoc != nullptr)
	{
		XMLElement* elem = pdoc->FirstChildElement("Screen");
		if (elem != nullptr)
		{
			const char* bg = elem->Attribute("Background");
			if (bg != nullptr)
				initBackgroundScreen(bg);

			const char* music = elem->Attribute("Music");
			if (music != nullptr)
				startMusic(music);

			const char* font = elem->Attribute("Font");
			if (font != nullptr)
			{
				// need to call Font::create() before the controls call create()
				_localFont = new Font(font);
				_localFont->create();
			}

			const char* musicStop = elem->Attribute("MusicStopOnExit");
			_musicStopOnExit = MigUtil::parseBool(musicStop, _musicStopOnExit);

			const char* fd = elem->Attribute("FadeDuration");
			_fadeDuration = MigUtil::parseInt(fd, _fadeDuration);

			const char* fadeColor = elem->Attribute("FadeColor");
			_fadeColor = MigUtil::parseColorString(fadeColor, MigTech::colBlack);

			const char* clearColor = elem->Attribute("ClearColor");
			_clearColor = MigUtil::parseColorString(clearColor, MigTech::colBlack);

			XMLElement* background = elem->FirstChildElement("Background");
			if (background != nullptr)
				initBackgroundScreen(background);

			XMLElement* controls = elem->FirstChildElement("Controls");
			if (controls != nullptr)
				_controls.loadFromXML(controls, _localFont);
		}
	}

	// lifecycle events
	_lcList.create();
	if (_localFont != nullptr)
		_lcList.addToList(*_localFont);

	// set up the fade in animation
	startFadeIn(_fadeDuration);
}

// called to create needed graphic resources, will be called again to restore graphics
void ScreenBase::createGraphics()
{
	// create the texture object and assign the shaders
	_fadeOverlay = MigUtil::theRend->createObject();
	_fadeOverlay->addShaderSet(MIGTECH_VSHADER_POS_NO_TRANSFORM, MIGTECH_PSHADER_COLOR);

	// load mesh vertices (position only)
	VertexPosition txtVertices[4];
	txtVertices[0] = VertexPosition(Vector3(-1, -1, 0));
	txtVertices[1] = VertexPosition(Vector3(-1, 1, 0));
	txtVertices[2] = VertexPosition(Vector3(1, -1, 0));
	txtVertices[3] = VertexPosition(Vector3(1, 1, 0));
	_fadeOverlay->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION);

	// load mesh indices
	const unsigned short txtIndices[] =
	{
		0, 2, 1,
		1, 2, 3,
	};
	_fadeOverlay->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	_fadeOverlay->setCulling(FACE_CULLING_BACK);

	// overlay
	if (_overlay != nullptr)
		_overlay->createGraphics();

	// lifecycle events
	_lcList.createGraphics();
}

void ScreenBase::windowSizeChanged()
{
	// lifecycle events
	_lcList.windowSizeChanged();

	// overlay
	if (_overlay != nullptr)
		_overlay->windowSizeChanged();
}

void ScreenBase::visibilityChanged(bool vis)
{
	if (MigUtil::theMusic != nullptr)
	{
		if (vis)
			MigUtil::theMusic->resumeSound();
		else
			MigUtil::theMusic->pauseSound();
	}

	// lifecycle events
	_lcList.visibilityChanged(vis);

	// overlay
	if (_overlay != nullptr)
		_overlay->visibilityChanged(vis);
}

void ScreenBase::suspend()
{
	//if (MigUtil::theMusic != nullptr)
	//	MigUtil::theMusic->pauseSound();

	// lifecycle events
	_lcList.suspend();

	// overlay
	if (_overlay != nullptr)
		_overlay->suspend();
}

void ScreenBase::resume()
{
	//if (MigUtil::theMusic != nullptr)
	//	MigUtil::theMusic->resumeSound();

	// lifecycle events
	_lcList.resume();

	// overlay
	if (_overlay != nullptr)
		_overlay->resume();
}

// destroys graphic resources
void ScreenBase::destroyGraphics()
{
	if (_fadeOverlay != nullptr)
		MigUtil::theRend->deleteObject(_fadeOverlay);
	_fadeOverlay = nullptr;

	if (_overlay != nullptr)
		_overlay->destroyGraphics();

	// lifecycle events
	_lcList.destroyGraphics();
}

// destroy all of the data
void ScreenBase::destroy()
{
	freeOverlay();

	// lifecycle events
	_lcList.destroy();
	_lcList.removeAll();
	_controls.removeAllControls();

	if (_bgHandler != nullptr)
		delete _bgHandler;
	_bgHandler = nullptr;
	if (_overlayHandler != nullptr)
		delete _overlayHandler;
	_overlayHandler = nullptr;

	if (MigUtil::theAudio != nullptr && _musicStopOnExit)
	{
		if (MigUtil::theMusic != nullptr)
			MigUtil::theMusic->stopSound();
		MigUtil::theAudio->deleteMedia(MigUtil::theMusic);
		MigUtil::theMusic = nullptr;
	}
}

bool ScreenBase::pointerPressed(float x, float y)
{
	// start swipe tracking
	_startPt.x = x;
	_startPt.y = y;

	if (_overlay != nullptr)
		return _overlay->pointerPressed(x, y);
	return _controls.pointerPressed(x, y);
}

void ScreenBase::pointerReleased(float x, float y)
{
	if (_overlay != nullptr)
		_overlay->pointerReleased(x, y);
	else
		_controls.pointerReleased(x, y);

	// reset this so the chosen swipe axis is no longer locked
	_swipeLocked = SWIPE_NONE;

	// is this just a tap?
	if (abs(x - _startPt.x) < _tapThreshold && abs(y - _startPt.y) < _tapThreshold)
	{
		if (_overlay != nullptr)
			_overlay->onTap(_startPt.x, _startPt.y);
		else
			onTap(_startPt.x, _startPt.y);
	}
}

void ScreenBase::pointerMoved(float x, float y, bool isInContact)
{
	if (_overlay != nullptr)
		_overlay->pointerMoved(x, y, isInContact);
	else
		_controls.pointerMoved(x, y, isInContact);

	if (isInContact)
	{
		// are we locked on a particular swipe yet?
		if (_swipeLocked == SWIPE_NONE)
		{
			float xDelta = x - _startPt.x;
			float yDelta = y - _startPt.y;

			if (abs(xDelta) > _maxThreshold)
			{
				// horizontal swipe detected
				_swipeLocked = SWIPE_HORIZONTAL;
			}
			else if (abs(yDelta) > _maxThreshold)
			{
				// vertical swipe detected
				_swipeLocked = SWIPE_VERTICAL;
			}

			if (_swipeLocked != SWIPE_NONE)
			{
				if (_overlay != nullptr)
					_overlay->onSwipe(x, y, xDelta, yDelta, _swipeLocked);
				else
					onSwipe(x, y, xDelta, yDelta, _swipeLocked);
			}
		}
	}
}

void ScreenBase::keyDown(VIRTUAL_KEY key)
{
	if (_overlay != nullptr)
		_overlay->keyDown(key);
}

void ScreenBase::keyUp(VIRTUAL_KEY key)
{
	if (_overlay != nullptr)
		_overlay->keyUp(key);

	// in theory we could do key processing here, if ever needed
	if (_overlay != nullptr)
		_overlay->onKey(key);
	else
		onKey(key);
}

// returns true if the back key was handled
bool ScreenBase::backKey()
{
	return (_overlay != nullptr ? _overlay->onBackKey() : onBackKey());
}

// returns false if it's time for this screen to die
bool ScreenBase::update()
{
	// delete any expired overlay
	if (_expiredOverlay != nullptr)
	{
		delete _expiredOverlay;
		_expiredOverlay = nullptr;
	}

	if (_bgHandler != nullptr)
		_bgHandler->update();
	if (_overlayHandler != nullptr)
		_overlayHandler->update();
	if (_overlay != nullptr)
		_overlay->update();

	return (_fadeColor.a < 1.0 ? true : (_fadeStyle != FADE_STYLE_OUT));
}

// this is a basic implementation, derived classes should provide their own
bool ScreenBase::render()
{
	// background screen
	drawBackgroundScreen();

	return true;
}

// renders overlays after all rendering passes are complete
bool ScreenBase::renderOverlays()
{
	// overlay
	drawOverlays();

	return true;
}

// multi-pass render support
const std::vector<RenderPass*>& ScreenBase::getPassList() const
{
	return _renderPasses;
}

// multi-pass render support
bool ScreenBase::renderPass(int index, RenderPass* pass)
{
	return false;
}

bool ScreenBase::doFrame(int id, float newVal, void* optData)
{
	if (_idFadeAnim == id)
	{
		// this controls the fade screen opacity (0=no fading, 1=fade is complete)
		_fadeColor.a = newVal;
	}
	else if (_idMusicAnim == id)
	{
		// music volume
		if (MigUtil::theMusic != nullptr)
			MigUtil::theMusic->fadeVolume(newVal);
	}

	return true;
}

void ScreenBase::animComplete(int id, void* optData)
{
	if (_idFadeAnim == id)
	{
		_idFadeAnim = 0;

		// indicate that the fade is complete
		fadeComplete(_fadeColor.a, _fadeStyle);
	}
	else if (_idMusicAnim == id)
	{
		_idMusicAnim = 0;

		// stop playing if the volume faded out completely
		if (MigUtil::theMusic != nullptr && MigUtil::theMusic->getVolume() == 0)
			MigUtil::theMusic->stopSound();
	}
}

void ScreenBase::onOverlayIntro(long duration)
{
}

void ScreenBase::onOverlayExit(long duration)
{
	// restore the fade back to normal
	startFadeScreen(duration, _fadeColor.a, 0.0f, _musicFade);
	_musicFade = false;
}

void ScreenBase::onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration)
{
	if (transType != OVERLAY_TRANSITION_INTRO && _overlay != nullptr)
	{
		OverlayBase* nextOverlay = _overlay->_nextOverlay;

		// clear the existing overlay
		freeOverlay();

		// initialize the next overlay
		if (nextOverlay != nullptr)
		{
			LOGINFO("(ScreenBase::onOverlayComplete) Starting next overlay (%s)", nextOverlay->getOverlayName().c_str());

			_overlay = nextOverlay;
			_overlay->_callback = this;
			_overlay->create();
			_overlay->createGraphics();

			// by definition we start the inter-overlay transition
			_overlay->startInterAnimation(transType, duration, true);
		}
	}
}

void ScreenBase::onOverlayCustom(int id, void* data)
{
}

void ScreenBase::onTap(float x, float y)
{
	LOGINFO("(ScreenBase::onTap) Tap at (%f, %f) detected", x, y);
	_controls.onTap(x, y);
}

void ScreenBase::onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe)
{
	LOGINFO("(ScreenBase::onSwipe) Swipe at (%f, %f, %d) detected", x, y, swipe);
}

void ScreenBase::onKey(VIRTUAL_KEY key)
{
	LOGINFO("(ScreenBase::onKey) Key press (%d) detected", key);

	if (key == ESCAPE)
	{
		// ESC by default kills the current screen
		startFadeOut();
	}
}

bool ScreenBase::onBackKey()
{
	LOGINFO("(ScreenBase::onBackKey) Back key detected");

	// by default, fade out and consume the key
	startFadeOut();
	return true;
}

void ScreenBase::onClick(int id, ControlBase* sender)
{
	LOGINFO("(ScreenBase::onClick) Control click detected by ID=%d", id);
}

void ScreenBase::onSlide(int id, ControlBase* sender, float val)
{
	LOGINFO("(ScreenBase::onSlide) Control slide detected by ID=%d, val=%f", id, val);
}

ControlBase* ScreenBase::allocControl(const char* tag)
{
	return nullptr;
}

// starts fading in the screen
void ScreenBase::startFadeIn(long duration)
{
	// -1 means use the default
	if (duration == -1)
		duration = _fadeDuration;

	startFadeScreen(duration, 1, 0, _musicFade);
	_musicFade = false;
}

// starts fading out the screen
void ScreenBase::startFadeOut(long duration)
{
	// -1 means use the default
	if (duration == -1)
		duration = _fadeDuration;

	startFadeScreen(duration, 0, 1, _musicStopOnExit);
}

// starts a fade animation
void ScreenBase::startFadeScreen(long duration, float start, float end, bool musicToo)
{
	// prevent a divide by zero
	if (duration <= 0)
		duration = 1;

	// set up the fade animation
	AnimItem animItem(this);
	animItem.configSimpleAnim(start, end, duration, AnimItem::ANIM_TYPE_LINEAR);
	_idFadeAnim = MigUtil::theAnimList->addItem(animItem);

	// set up a separate animation for music
	if (musicToo)
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(1 - start, 1 - end, duration, AnimItem::ANIM_TYPE_LINEAR);
		_idMusicAnim = MigUtil::theAnimList->addItem(animItem);
	}

	// remember the fade style
	_fadeStyle = (end < start ? FADE_STYLE_IN : FADE_STYLE_OUT);
	_fadeColor.a = start;
}

// sets the fade value instantly
void ScreenBase::doFade(float end)
{
	_fadeColor.a = end;
}

// indicates a fade is complete
void ScreenBase::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
}

// loads the background handler, call this before create()
void ScreenBase::initBackgroundScreen(BgBase* bgHandler)
{
	if (_bgHandler == nullptr && bgHandler != nullptr)
	{
		_bgHandler = bgHandler;
		_lcList.addToList(*_bgHandler);
	}
}

// this just initializes a static background handler for an image
void ScreenBase::initBackgroundScreen(const std::string& resID)
{
	if (_bgHandler == nullptr)
		initBackgroundScreen(new BgBase(resID));
}

// this initializes more complicated background handlers
void ScreenBase::initBackgroundScreen(tinyxml2::XMLElement* xml)
{
	if (_bgHandler == nullptr)
		initBackgroundScreen(BgBase::loadHandlerFromXML(xml));
}

// loads the overlay handler, call this before create()
void ScreenBase::initOverlayScreen(BgBase* overlayHandler)
{
	if (_overlayHandler == nullptr && overlayHandler != nullptr)
	{
		_overlayHandler = overlayHandler;
		_overlayHandler->setOpaque(false);
		_lcList.addToList(*_overlayHandler);
	}
}

// this just initializes a static overlay handler for an image
void ScreenBase::initOverlayScreen(const std::string& resID)
{
	if (_overlayHandler == nullptr)
		initOverlayScreen(new BgBase(resID));
}

// this initializes more complicated overlay handlers
void ScreenBase::initOverlayScreen(tinyxml2::XMLElement* xml)
{
	if (_overlayHandler == nullptr)
		initOverlayScreen(BgBase::loadHandlerFromXML(xml));
}

// starts playing the background music immediately
void ScreenBase::startMusic(const std::string& musicResourceID, bool loop)
{
	// was a song provided
	if (MigUtil::theAudio != nullptr && !musicResourceID.empty())
	{
		// if a song is already playing, make sure it's the same
		if (MigUtil::theMusic == nullptr || musicResourceID.compare(MigUtil::theMusic->getName()) != 0)
		{
			if (MigUtil::theMusic != nullptr)
				MigUtil::theMusic->stopSound();
			MigUtil::theAudio->deleteMedia(MigUtil::theMusic);

			LOGINFO("(ScreenBase::startMusic) Loading new music (%s)", musicResourceID.c_str());
			MigUtil::theMusic = MigUtil::theAudio->loadMedia(musicResourceID, AudioBase::AUDIO_CHANNEL_MUSIC);
			if (MigUtil::theMusic != nullptr)
			{
				MigUtil::theMusic->playSound(loop);
				MigUtil::theMusic->fadeVolume(0);	// we will fade it in
			}
			else
				LOGWARN("(ScreenBase::startMusic) Unable to play (%s)", musicResourceID.c_str());

			// this indicates that we need to fade the music in
			_musicFade = true;
		}
	}
}

// clears the screen using the clear color
void ScreenBase::clearScreen()
{
	MigUtil::theRend->setClearColor(_clearColor);
}

// draws the background
void ScreenBase::drawBackgroundScreen()
{
	clearScreen();

	if (_bgHandler != nullptr)
		_bgHandler->render();
}

// draws any overlays
void ScreenBase::drawOverlays()
{
	MigUtil::theRend->setProjectionMatrix(nullptr);
	MigUtil::theRend->setViewMatrix(nullptr);

	// draw the UI controls first
	_controls.draw(1);

	// overlay image next
	if (_overlayHandler != nullptr)
		_overlayHandler->render();

	if (_fadeOverlay && _fadeColor.a > 0)
	{
		MigUtil::theRend->setObjectColor(_fadeColor);
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
		MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

		_fadeOverlay->render();
	}

	if (_overlay)
		_overlay->render();
}

// starts a new overlay complete w/ intro animation and the fading out of the main screen
void ScreenBase::startNewOverlay(OverlayBase* newOverlay, float newAlpha, long duration, bool musicToo)
{
	if (newOverlay == nullptr)
		throw std::invalid_argument("(ScreenBase::startNewOverlay) New overlay cannot be null");
	LOGINFO("(ScreenBase::startNewOverlay) Starting new overlay (%s)", newOverlay->getOverlayName().c_str());

	// set the overlay
	_overlay = newOverlay;
	_overlay->_callback = this;
	_overlay->create();
	_overlay->createGraphics();
	startNewOverlayAnimation(newAlpha, duration, musicToo);
}

// starts a new overlay complete w/ intro animation and the fading out of the main screen (uses settings specified in the overlay XML)
void ScreenBase::startNewOverlay(OverlayBase* newOverlay)
{
	if (newOverlay == nullptr)
		throw std::invalid_argument("(ScreenBase::startNewOverlay) New overlay cannot be null");
	LOGINFO("(ScreenBase::startNewOverlay) Starting new overlay (%s)", newOverlay->getOverlayName().c_str());

	// set the overlay
	_overlay = newOverlay;
	_overlay->_callback = this;
	_overlay->create();
	_overlay->createGraphics();
	startNewOverlayAnimation(newOverlay->_parentAlpha, newOverlay->_fadeDuration, newOverlay->_fadeMusic);
}

// starts overlay intro animation and the fading out of the main screen
void ScreenBase::startNewOverlayAnimation(float newAlpha, long duration, bool musicToo)
{
	// start the overlay intro animatnion
	_overlay->startIntroAnimation(duration);

	// also, fade the main screen
	if (duration > 0)
		startFadeScreen(duration, 0.0f, newAlpha, musicToo);
	else
		doFade(newAlpha);
	_musicFade = musicToo;
}

// releases an active overlay
void ScreenBase::freeOverlay()
{
	if (_overlay != nullptr)
	{
		LOGINFO("(ScreenBase::freeOverlay) Freeing overlay (%s)", _overlay->getOverlayName().c_str());

		_overlay->destroyGraphics();
		_overlay->destroy();

		// deleting the overlay now is dangerous since this function is often called by an overlay callback
		_expiredOverlay = _overlay;
	}
	_overlay = nullptr;
}

// returns if an overlay screen is visible
bool ScreenBase::isOverlayVisible() const
{
	return (_overlay != nullptr);
}
