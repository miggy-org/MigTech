#include "pch.h"
#include "CubeUtil.h"
#include "SplashScreen.h"
#include "GameScreen.h"
#include "StartOverlays.h"

using namespace Cuboingo;
using namespace MigTech;

const bool SPLASH_LAUNCH = false;

const int LAUNCH_PERIOD = 4000;
const int LAUNCH_FALL_TIME = 10000;
const int MAXIMUM_HOLE_LIMIT = 3;
const float DIST_START_FADE = 3.5f;
const float DIST_STOP_FALL = 2.5f;

// default light used for this screen (rotated to compensate for the -45 degree rotation in the view matrix)
static const Vector3 lightDir = Vector3(-0.7f, -1, -0.7f).normalize();

///////////////////////////////////////////////////////////////////////////
// SplashLauncher

SplashLauncher::SplashLauncher()
{
	_lastOrient = AXISORIENT_Z;
}

SplashLauncher::~SplashLauncher()
{
	std::list<FallingGrid*>::iterator iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		delete (*iter);
		iter++;
	}
	_fallList.clear();
}

void SplashLauncher::init()
{
}

void SplashLauncher::startGridLaunch(GridInfo& newGrid)
{
	newGrid.orient = _lastOrient;
	FallingGrid* fallGrid = new FallingGrid(newGrid, false);
	fallGrid->init(defFallStartPos);
	_fallList.push_back(fallGrid);

	// new animation
	AnimItem animItem(this);
	animItem.configSimpleAnim(defFallStartPos, 0, LAUNCH_FALL_TIME, AnimItem::ANIM_TYPE_LINEAR, fallGrid);
	MigUtil::theAnimList->addItem(animItem);

	_lastOrient = (_lastOrient == AXISORIENT_X ? AXISORIENT_Z : AXISORIENT_X);
}

void SplashLauncher::draw() const
{
	static Matrix identity;

	// draw the obsolete falling grids, while they are still visible
	std::list<FallingGrid*>::const_iterator iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		(*iter)->draw(identity);
		iter++;
	}
}

bool SplashLauncher::doFrame(int id, float newVal, void* optData)
{
	FallingGrid* fallGrid = (FallingGrid*)optData;
	if (fallGrid != nullptr)
	{
		fallGrid->setBaseDist(newVal);
		if (fallGrid->getTotalDist() <= DIST_STOP_FALL)
			return false;

		if (newVal < DIST_START_FADE)
		{
			// animate the alpha transparency of the slots
			float alpha = 1.0f - (DIST_START_FADE - newVal) / (DIST_START_FADE - DIST_STOP_FALL);
			int numSlots = fallGrid->getSlotCount();
			for (int i = 0; i < numSlots; i++)
				fallGrid->getSlot(i).color.a = alpha;
		}
	}
	return true;
}

void SplashLauncher::animComplete(int id, void* optData)
{
	FallingGrid* fallGrid = (FallingGrid*)optData;
	if (fallGrid != nullptr)
	{
		_fallList.remove(fallGrid);
		delete fallGrid;
	}
}

///////////////////////////////////////////////////////////////////////////
// SplashScreen

SplashScreen::SplashScreen()
	: ScreenBase("SplashScreen")
{
	_lcList.addToList(_cube);
	_lcList.addToList(_launcher);
	_lcList.addToList(_shadowPass);
}

ScreenBase* SplashScreen::getNextScreen()
{
	const Script& script = GameScripts::getCurrScript();
	ScreenBase* demoScreen = GameScripts::demoStringToDemoScreen(script.demoID);
	return (demoScreen != nullptr ? demoScreen : new GameScreen());
}

void SplashScreen::create()
{
	ScreenBase::create();

	_cube.init();
	_cube.startRotation();

	GameScripts::clearGameScript();

	if (_shadowPass.init())
		_renderPasses.push_back(&_shadowPass);

	if (SPLASH_LAUNCH)
	{
		// start a launch timer
		AnimItem animItem(this);
		animItem.configTimer(LAUNCH_PERIOD, true);
		_idLaunch = MigUtil::theAnimList->addItem(animItem);

		// and launch the first piece
		doLaunch();
	}
}

void SplashScreen::createGraphics()
{
	ScreenBase::createGraphics();

	// set up lighting for the cube shader (since no other shader on this screen uses lighting, we can do this here)
	MigUtil::theRend->setLightDirPos(0, lightDir, true);
}

void SplashScreen::windowSizeChanged()
{
	ScreenBase::windowSizeChanged();

	// default matrices
	CubeUtil::loadDefaultPerspectiveMatrix(_projMatrix);
	CubeUtil::loadDefaultViewMatrix(_viewMatrix, MigUtil::convertToRadians(-45));
}

void SplashScreen::onTap(float x, float y)
{
	ScreenBase::onTap(x, y);

	if (!GameScripts::isGameScriptLoaded())
		startNewOverlay(new StartOverlay());
}

void SplashScreen::onKey(VIRTUAL_KEY key)
{
	ScreenBase::onKey(key);

	if (key == SPACE || key == ENTER)
		startNewOverlay(new StartOverlay());
}

bool SplashScreen::onBackKey()
{
	// we don't consume this, so pressing the back key will exit on phones
	return false;
}

bool SplashScreen::render()
{
	// background screen
	drawBackgroundScreen();

	MigUtil::theRend->setProjectionMatrix(_projMatrix);
	MigUtil::theRend->setViewMatrix(_viewMatrix);
	_shadowPass.draw();
	_cube.draw();

	// activate blending for the falling pieces
	MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);
	_launcher.draw();
	MigUtil::theRend->setBlending(BLEND_STATE_NONE);

	return true;
}

bool SplashScreen::renderPass(int index, RenderPass* pass)
{
	_cube.draw();
	return true;
}

void SplashScreen::doLaunch()
{
	// launch a falling piece
	GridInfo newGrid;
	if (_cube.newFallingGridCandidate(newGrid, MAXIMUM_HOLE_LIMIT))
		_launcher.startGridLaunch(newGrid);
}

bool SplashScreen::doFrame(int id, float newVal, void* optData)
{
	if (_idLaunch == id)
		doLaunch();
	return ScreenBase::doFrame(id, newVal, optData);
}

void SplashScreen::animComplete(int id, void* optData)
{
	if (_idTimer == id)
	{
		startFadeOut(defFadeDuration);
		_idTimer = 0;
	}
	ScreenBase::animComplete(id, optData);
}

// we need this to detect if the overlay is complete and a script was chosen
void SplashScreen::onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration)
{
	ScreenBase::onOverlayComplete(transType, duration);

	if (transType == OVERLAY_TRANSITION_EXIT)
	{
		// only start the game if we have a loaded script
		if (GameScripts::isGameScriptLoaded() || !GameScripts::getCurrScript().demoID.empty())
		{
			// start the exit animation (spinning shrinking cube)
			startExitAnimation();
		}
	}
}

// the options overlay will use this message to update the main music volume
void SplashScreen::onOverlayCustom(int id, void* data)
{
	if (id == 1 && data != nullptr && MigUtil::theMusic != nullptr)
	{
		float* pf = (float*)data;
		MigUtil::theMusic->setVolume(*pf);
	}
}

void SplashScreen::startExitAnimation()
{
	// play the exit animation sound
	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->playMedia(WAV_SPLASH_EXIT, AudioBase::AUDIO_CHANNEL_SOUND);

	// start the exit animation (spinning shrinking cube)
	int dur = _cube.startExitAnimation();

	// set a timer to fire after the exit animation
	AnimItem animItem(this);
	animItem.configTimer(dur, false);
	_idTimer = MigUtil::theAnimList->addItem(animItem);
}
