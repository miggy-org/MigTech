#include "pch.h"
#include "CubeUtil.h"
#include "CreditsScreen.h"
#include "SplashScreen.h"
#include "GameScripts.h"
#include "../core/Timer.h"

using namespace Cuboingo;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// credits background handler

// animation lengths
static const int FLASH_ANIM_LENGTH = 2000;
static const int FADE1_ANIM_LENGTH = 4000;
static const int MAIN_ANIM_LENGTH = 2000;
static const int FADE2_ANIM_LENGTH = 2000;

CreditsBg::CreditsBg(ICreditsEvents* events) : CycleBg(), _events(events)
{
	CycleBg::init("credits_bg1.jpg", "credits_bg2.jpg", 0);
}

void CreditsBg::fadeOut()
{
	// clear any existing animation
	_idAnim.clearAnim();

	// just fade out
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, FADE2_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idAnim = MigUtil::theAnimList->addItem(animItem);

	// last stage
	_stage = STAGE_ACTION_FADE_OUT2;
	_colOther = Color(0, 0, 0, 0);
}

void CreditsBg::create()
{
	CycleBg::create();

	// start the first stage animation
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, (float)FLASH_ANIM_LENGTH, FLASH_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idAnim = MigUtil::theAnimList->addItem(animItem);

	// first stage
	_stage = STAGE_ACTION_FLASH;
}

void CreditsBg::createGraphics()
{
	CycleBg::createGraphics();

	// create the fade object and assign the shaders
	Object* fadeObj = MigUtil::theRend->createObject();
	fadeObj->addShaderSet(MIGTECH_VSHADER_POS_NO_TRANSFORM, MIGTECH_PSHADER_COLOR);

	// load mesh vertices (position / color)
	VertexPosition txtVertices[4];
	txtVertices[0].pos = Vector3(-1, -1, 0);
	txtVertices[1].pos = Vector3(-1,  1, 0);
	txtVertices[2].pos = Vector3( 1, -1, 0);
	txtVertices[3].pos = Vector3( 1,  1, 0);
	fadeObj->loadVertexBuffer(txtVertices, ARRAYSIZE(txtVertices), MigTech::VDTYPE_POSITION);

	// load mesh indices
	static const unsigned short txtIndices[] =
	{
		0, 2, 1,
		1, 2, 3,
	};
	fadeObj->loadIndexBuffer(txtIndices, ARRAYSIZE(txtIndices), MigTech::PRIMITIVE_TYPE_TRIANGLE_LIST);

	// culling
	fadeObj->setCulling(FACE_CULLING_BACK);
	_fadePoly = fadeObj;
}

void CreditsBg::destroyGraphics()
{
	if (_fadePoly != nullptr)
		MigUtil::theRend->deleteObject(_fadePoly);
	_fadePoly = nullptr;

	CycleBg::destroyGraphics();
}

bool CreditsBg::doFrame(int id, float newVal, void* optData)
{
	if (_idAnim == id)
	{
		if (_stage == STAGE_ACTION_FLASH)
		{
			// newVal is actually milliseconds
			newVal = newVal - 200 * ((int)newVal / 200);
			_colOther.a = (newVal > 100 ? 200 - newVal : newVal) / 100.0f;
		}
		else if (_stage == STAGE_ACTION_FADE_OUT1 || _stage == STAGE_ACTION_FADE_IN2 || _stage == STAGE_ACTION_FADE_OUT2)
		{
			// newVal is an alpha transparency
			_colOther.a = newVal;
		}
		else if (_stage == STAGE_ACTION_MAIN)
		{
			// newVal is an alpha transparency but needs to be modulated
			newVal = newVal - 2 * ((int)newVal / 2);
			_colOther.a = (newVal > 1 ? 2 - newVal : newVal);
		}
	}

	// don't call base class
	return true;
}

static void loadNewImage(Object* poly, const std::string& img)
{
	if (MigUtil::theRend->loadImage(img, img, LOAD_IMAGE_NONE) != nullptr)
		poly->setImage(0, img, TXT_FILTER_LINEAR, TXT_FILTER_LINEAR, TXT_WRAP_CLAMP);
}

void CreditsBg::animComplete(int id, void* optData)
{
	if (_idAnim == id)
	{
		_idAnim = 0;

		// signal stage action complete
		if (_events != nullptr)
			_events->stageActionComplete(_stage);

		if (_stage == STAGE_ACTION_FLASH)
		{
			// start a fade out animation
			AnimItem animItem(this);
			animItem.configSimpleAnim(0, 1, FADE1_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
			_idAnim = MigUtil::theAnimList->addItem(animItem);

			_stage = STAGE_ACTION_FADE_OUT1;
			_colOther = Color(0, 0, 0, 0);
		}
		else if (_stage == STAGE_ACTION_FADE_OUT1)
		{
			// start a fade in animation
			AnimItem animItem(this);
			animItem.configSimpleAnim(1, 0, MAIN_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
			_idAnim = MigUtil::theAnimList->addItem(animItem);

			// new background image
			BgBase::init("credits_bg3.jpg");
			loadNewImage(_screenPoly, _bgResID);

			_stage = STAGE_ACTION_FADE_IN2;
		}
		else if (_stage == STAGE_ACTION_FADE_IN2)
		{
			// start fading between the two main images
			AnimItem animItem(this);
			animItem.configSimpleAnim(0, 2, MAIN_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
			_idAnim = MigUtil::theAnimList->addItem(animItem);

			// new other background image
			_bgRes2ID = "credits_bg4.jpg";
			loadNewImage(_screenPoly2, _bgRes2ID);

			_stage = STAGE_ACTION_MAIN;
			_colOther = Color(1, 1, 1, 0);
		}
		else if (_stage == STAGE_ACTION_FADE_OUT2)
		{
			_idAnim = 0;

			_stage = STAGE_ACTION_BLANK;
		}
	}
}

void CreditsBg::render() const
{
	if (_stage == STAGE_ACTION_FADE_OUT1 || _stage == STAGE_ACTION_FADE_IN2 || _stage == STAGE_ACTION_FADE_OUT2)
	{
		BgBase::render();

		if (_fadePoly != nullptr)
		{
			MigUtil::theRend->setObjectColor(_colOther);
			MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
			MigUtil::theRend->setDepthTesting(DEPTH_TEST_STATE_NONE, false);

			_fadePoly->render();
		}
	}
	else if (_stage != STAGE_ACTION_BLANK)
		CycleBg::render();
	else
		MigUtil::theRend->setClearColor(colBlack);
}

///////////////////////////////////////////////////////////////////////////
// credits cube

static const int INTRO_ANIM_LENGTH = 1000;
static const int PAUSE_ANIM_LENGTH = 2000;
static const int MOVE_ANIM_LENGTH = 1500;
static const int EXIT_ANIM_LENGTH = 1000;

void CreditsCube::start()
{
	startIntroAnimation(INTRO_ANIM_LENGTH);
}

void CreditsCube::exit()
{
	startExitAnimation(EXIT_ANIM_LENGTH);
}

void CreditsCube::animComplete(int id, void* optData)
{
	if (_idScale == id)
	{
		if (_scale > 0)
		{
			// finished intro animation
			AnimItem animItem(this);
			animItem.configTimer(PAUSE_ANIM_LENGTH, false);
			_idPause = MigUtil::theAnimList->addItem(animItem);
		}
		else if (_events != nullptr)
		{
			// finished exit animation
			_events->cubeExitComplete();
		}
	}
	else if (_idPause == id)
	{
		// finished initial post-intro pause
		startMoveAnimation(MOVE_ANIM_LENGTH);
	}
	else if (_idTransX == id)
	{
		// finished post-intro move, start rotating
		startRotation();
	}

	SplashCube::animComplete(id, optData);
}

void CreditsCube::applyTransform(Matrix& worldMatrix) const
{
	// we need the transformation applied a little differently here
	static Matrix locMatrix;
	locMatrix.identity();
	if (_scale != 1)
		locMatrix.scale(_scale, _scale, _scale);
	if (_rotX != 0)
		locMatrix.rotateX(_rotX);
	if (_rotY != 0)
		locMatrix.rotateY(_rotY);
	if (_rotZ != 0)
		locMatrix.rotateZ(_rotZ);
	if (_translate.x != 0 || _translate.y != 0 || _translate.z != 0)
		locMatrix.translate(_translate);
	locMatrix.multiply(worldMatrix);
	worldMatrix.copy(locMatrix);

	MigUtil::theRend->setModelMatrix(worldMatrix);
}

void CreditsCube::startIntroAnimation(int dur)
{
	AnimItem animItemX(this);
	animItemX.configSimpleAnim(MigUtil::convertToRadians(50), 0, dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotX = MigUtil::theAnimList->addItem(animItemX);

	AnimItem animItemY(this);
	animItemY.configSimpleAnim(MigUtil::convertToRadians(300), 0, dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotY = MigUtil::theAnimList->addItem(animItemY);

	AnimItem animItemZ(this);
	animItemZ.configSimpleAnim(MigUtil::convertToRadians(37.5f), 0, dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotZ = MigUtil::theAnimList->addItem(animItemZ);

	float fs[] = { 0.35f, 0.65f, 0.85f, 1.0f, 1.15f, 1.2f, 1.25f, 1.2f, 1.12f, 1.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, 1, dur, fs, ARRAYSIZE(fs));
	_idScale = MigUtil::theAnimList->addItem(animItemS);

	// play the intro animation sound
	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->playMedia(WAV_START, AudioBase::AUDIO_CHANNEL_SOUND);
}

void CreditsCube::startMoveAnimation(int dur)
{
	float ft[] = { 0.01f, 0.03f, 0.05f, 0.08f, 0.12f, 0.17f, 0.23f, 0.29f, 0.38f, 0.5f,
		0.62f, 0.71f, 0.77f, 0.83f, 0.88f, 0.92f, 0.95f, 0.97f, 0.99f, 1.0f };

	AnimItem animItemX(this);
	animItemX.configParametricAnim(0, -3, dur, ft, ARRAYSIZE(ft));
	_idTransX = MigUtil::theAnimList->addItem(animItemX);

	AnimItem animItemY(this);
	animItemY.configParametricAnim(0, 0.8f, dur, ft, ARRAYSIZE(ft));
	_idTransY = MigUtil::theAnimList->addItem(animItemY);
}

void CreditsCube::startExitAnimation(int dur)
{
	// cancel any existing animations
	_idRotY.clearAnim();

	AnimItem animItemX(this);
	animItemX.configSimpleAnim(0, MigUtil::convertToRadians(25), dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotX = MigUtil::theAnimList->addItem(animItemX);

	AnimItem animItemY(this);
	animItemY.configSimpleAnim(_rotY, _rotY + MigUtil::convertToRadians(200), dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotY = MigUtil::theAnimList->addItem(animItemY);

	AnimItem animItemZ(this);
	animItemZ.configSimpleAnim(0, MigUtil::convertToRadians(20), dur, AnimItem::ANIM_TYPE_LINEAR);
	_idRotZ = MigUtil::theAnimList->addItem(animItemZ);

	//float fs[] = { 0.35f, 0.65f, 0.85f, 1.0f, 1.15f, 1.2f, 1.25f, 1.2f, 1.12f, 1.0f };
	float fs[] = { 1.0f, 1.12f, 1.2f, 1.25f, 1.2f, 1.15f, 1.0f, 0.85f, 0.65f, 0.35f, 0.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, 1, dur, fs, ARRAYSIZE(fs));
	_idScale = MigUtil::theAnimList->addItem(animItemS);

	// play the intro animation sound
	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->playMedia(WAV_SPLASH_EXIT, AudioBase::AUDIO_CHANNEL_SOUND);
}

///////////////////////////////////////////////////////////////////////////
// credits screen

// default light used for this screen
static const Vector3 lightDir = Vector3(0, -1, -0.7f).normalize();

static const int ANIM_FADE_DURATION = 1000;

CreditsScreen::CreditsScreen() : ScreenBase("CreditsScreen"),
	_cube(this), _currEvent(0)
{
	_lcList.addToList(_cube);
	_lcList.addToList(_clip);
}

ScreenBase* CreditsScreen::getNextScreen()
{
	return new SplashScreen();
}

void CreditsScreen::create()
{
	// init background
	initBackgroundScreen(new CreditsBg(this));

	// create() will initialize the background handler
	ScreenBase::create();

	// preload the music
	if (MigUtil::theAudio != nullptr)
		MigUtil::theMusic = MigUtil::theAudio->loadMedia("credits.mp3", AudioBase::AUDIO_CHANNEL_MUSIC);

	// init splash cube
	_cube.init();
	_cube.start();
	_cubeVisible = true;

	// init the event list
	initEventList();
}

void CreditsScreen::createGraphics()
{
	ScreenBase::createGraphics();

	// set up lighting for the cube shader (since no other shader on this screen uses lighting, we can do this here)
	MigUtil::theRend->setLightDirPos(0, lightDir, true);
}

void CreditsScreen::windowSizeChanged()
{
	ScreenBase::windowSizeChanged();

	// default matrices
	CubeUtil::loadDefaultPerspectiveMatrix(_projMatrix);
	CubeUtil::loadDefaultViewMatrix(_viewMatrix, 0);
}

bool CreditsScreen::render()
{
	// background screen
	drawBackgroundScreen();

	// cube
	MigUtil::theRend->setProjectionMatrix(_projMatrix);
	MigUtil::theRend->setViewMatrix(_viewMatrix);
	_cube.draw();

	// movie clip
	MigUtil::theRend->setProjectionMatrix(nullptr);
	MigUtil::theRend->setViewMatrix(nullptr);
	_clip.draw();

	return true;
}

bool CreditsScreen::doFrame(int id, float newVal, void* optData)
{
	if (_idFade == id)
	{
		// set the color of the movie clip
		_clip.setAlpha(newVal);
		if (newVal == 0)
			_clip.setVisible(false);
	}

	return ScreenBase::doFrame(id, newVal, optData);
}

void CreditsScreen::animComplete(int id, void* optData)
{
	if (_idTimer == id)
	{
		_idTimer.clearAnim();

		if (_clip.isVisible())
		{
			// create a fade out
			AnimItem animItem(this);
			animItem.configSimpleAnim(1, 0, ANIM_FADE_DURATION / 2, AnimItem::ANIM_TYPE_LINEAR);
			_idFade = MigUtil::theAnimList->addItem(animItem);
		}
		else
		{
			// on to the next movie clip event
			_currEvent++;
			if (_currEvent < (int) _events.size())
				nextEvent();
		}
	}
	else if (_idFade == id)
	{
		_idFade.clearAnim();

		if (_clip.isVisible())
		{
			// create a timer
			AnimItem animItem(this);
			animItem.configTimer(_events[_currEvent].dur - ANIM_FADE_DURATION, false);
			_idTimer = MigUtil::theAnimList->addItem(animItem);
		}
		else
		{
			// on to the next movie clip event
			_currEvent++;
			if (_currEvent < (int)_events.size())
				nextEvent();
			else
				startFadeOut();
		}
	}

	ScreenBase::animComplete(id, optData);
}

void CreditsScreen::stageActionComplete(int action)
{
	if (action == CreditsBg::STAGE_ACTION_FADE_OUT1)
	{
		// start music
		if (MigUtil::theMusic != nullptr)
			MigUtil::theMusic->playSound(false);

		// also start playing the movie clip events
		startEvents();
	}
	else if (action == CreditsBg::STAGE_ACTION_FADE_OUT2)
	{
		_cube.exit();
	}
}

void CreditsScreen::cubeExitComplete()
{
	// the last image needs to be centered
	_clipPos.x = 0;

	// this is the signal that the cube is gone
	_cubeVisible = false;

	// this will start the final event
	nextEvent();
}

void CreditsScreen::onTap(float x, float y)
{
	ScreenBase::onTap(x, y);

	startFadeOut();
}

void CreditsScreen::initEventList()
{
	// configure the list of events
	_events.push_back(CreditsEvent(15000, "credits_01.jpg", ""));
	_events.push_back(CreditsEvent(3000, "credits_02.jpg", ""));
	_events.push_back(CreditsEvent(9000, "credits_03.jpg", ""));
	_events.push_back(CreditsEvent(9000, "credits_04.jpg", ""));
	_events.push_back(CreditsEvent(9000, "credits_05.jpg", ""));
	_events.push_back(CreditsEvent(8000, "credits_06.jpg", ""));
	_events.push_back(CreditsEvent(2500, "credits_07.jpg", ""));
	_events.push_back(CreditsEvent(8000, "credits_08.jpg", WAV_FILL1.c_str()));
	_events.push_back(CreditsEvent(8000, "credits_09.jpg", WAV_FILL2.c_str()));
	_events.push_back(CreditsEvent(8000, "credits_10.jpg", WAV_FILL3.c_str()));
	_events.push_back(CreditsEvent(8000, "credits_11.jpg", WAV_FILL4.c_str()));
	_events.push_back(CreditsEvent(8000, "credits_12.jpg", WAV_FILL5.c_str()));
	_events.push_back(CreditsEvent(8000, "credits_13.jpg", WAV_FILL6.c_str()));
	_events.push_back(CreditsEvent(1000, "", ""));
	_events.push_back(CreditsEvent(5000, "credits_15.jpg", ""));
	_events.push_back(CreditsEvent(5000, "credits_16.jpg", ""));
	_events.push_back(CreditsEvent(4000, "credits_17.jpg", ""));
	_events.push_back(CreditsEvent(5000, "credits_18.jpg", ""));
	_events.push_back(CreditsEvent(2000, "", ""));
	_events.push_back(CreditsEvent(5000, "credits_19.jpg", ""));     // this last event is delayed until after the cube exits
}

void CreditsScreen::startEvents()
{
	// set up the matrix to offset the movie clips, won't change throughout the events (except for the last one)
	float dx = MigUtil::screenPercentWidthToCameraPlane(0.25f);
	_clipPos = Vector3(dx, 0, 0);

	// start the first event
	_currEvent = 0;
	nextEvent();
}

void CreditsScreen::nextEvent()
{
	if (_currEvent < (int)_events.size() - 1 || !_cubeVisible)
	{
		if (!_events[_currEvent].image.empty())
		{
			// compute width and height of the movie clip
			float w = MigUtil::screenPercentWidthToCameraPlane(0.4f);
			float h = MigUtil::screenPercentHeightToCameraPlane((_currEvent == 0 ? 0.5f : 0.3f));

			// create the movie clip
			_clip.init(_events[_currEvent].image, w, h);
			_clip.setAlpha(0);
			_clip.setPos(_clipPos);
			_clip.setVisible(true);

			// if this is the first clip, create the graphics
			if (_currEvent == 0)
				_clip.createGraphics();

			// create a fade in
			AnimItem animItem(this);
			animItem.configSimpleAnim(0, 1, ANIM_FADE_DURATION / 2, AnimItem::ANIM_TYPE_LINEAR);
			_idFade = MigUtil::theAnimList->addItem(animItem);
		}
		else
		{
			// no movie clip to display for this event
			_clip.setVisible(false);

			// create a timer
			AnimItem animItem(this);
			animItem.configTimer(_events[_currEvent].dur, false);
			_idTimer = MigUtil::theAnimList->addItem(animItem);
		}

		// finally, play a sound if necessary
		if (!_events[_currEvent].sound.empty() && MigUtil::theAudio != nullptr)
			MigUtil::theAudio->playMedia(_events[_currEvent].sound, AudioBase::AUDIO_CHANNEL_SOUND);
	}
	else
	{
		// the last event is to be played after the fade out and cube exit animations are complete
		((CreditsBg*)_bgHandler)->fadeOut();
	}
}
