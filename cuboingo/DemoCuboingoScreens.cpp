#include "pch.h"
#include "CubeUtil.h"
#include "DemoCuboingoScreens.h"
#include "GameScreen.h"
#include "SplashScreen.h"
#include "../core/Font.h"

using namespace Cuboingo;
using namespace MigTech;

// default light used for this screen (rotated to compensate for the -45 degree rotation in the view matrix)
static const Vector3 lightDir = Vector3(-0.7f, -1, -0.7f).normalize();

///////////////////////////////////////////////////////////////////////////
// DemoGameCube

void DemoGameCube::fillCubeSlots(int face, bool* fillList, int fillListLen)
{
	if (face < NUM_GRIDS)
	{
		const GridBase* theGrid = _theGrids[face];
		if (fillListLen == theGrid->getSlotCount())
		{
			for (int i = 0; i < fillListLen; i++)
			{
				Slot& theSlot = theGrid->getSlot(i);
				theSlot.filled = fillList[i];
				theSlot.setColor(fillList[i] ? theGrid->getFillColor() : theGrid->getEmptyColor());
			}
		}
	}
}

GridInfo DemoGameCube::newGridCandidate(int face)
{
	GridInfo chosen;
	chosen.initFromGrid(*(_theGrids[face]), false);

	// invert the filled bit
	chosen.invertFillList();
	return chosen;
}

///////////////////////////////////////////////////////////////////////////
// DemoGameplay base class

static const int ID_LINE_TEXT_1 = 1;
static const int ID_LINE_TEXT_2 = 2;
static const int ID_LINE_TEXT_3 = 3;
static const int ID_NEXT_BUTTON = 10;

DemoGameplay::DemoGameplay(const std::string& name) : DemoScreen(name),
	_gameCube(this), _launcher(_gameCube), _stamps(nullptr),
	_line1Text(nullptr), _line2Text(nullptr), _line3Text(nullptr), _nextBtn(nullptr),
	_textAlpha(0), _abortAll(false)
{
	_lcList.addToList(_gameCube);
	_lcList.addToList(_launcher);
	_lcList.addToList(_stamps);
}

static TextButton* getTextButtonSafe(Controls& ctrls, int id)
{
	ControlBase* ctrl = ctrls.getControlByID(id);
	if (ctrl != nullptr && ctrl->getType() == TEXT_BUTTON_CONTROL)
		return (TextButton*)ctrl;
	return nullptr;
}

void DemoGameplay::create()
{
	DemoScreen::create();

	// get lines of text
	_line1Text = getTextButtonSafe(_controls, ID_LINE_TEXT_1);
	_line2Text = getTextButtonSafe(_controls, ID_LINE_TEXT_2);
	_line3Text = getTextButtonSafe(_controls, ID_LINE_TEXT_3);
	_nextBtn = getTextButtonSafe(_controls, ID_NEXT_BUTTON);

	// initialize the launcher
	_launcher.init(this);
	_launcher.configControls(_controls);

	// initialize stamps
	_stamps.init();

	// load sound effects
	_sounds.loadSound(WAV_ROTATE);

	// preload the image here to add an alpha channel
	MigUtil::theRend->loadImage("cursor.png", "cursor.png", LOAD_IMAGE_ADD_ALPHA);
	_cursor.init("cursor.png", 1, 2, 0.05f, 0.05f, false);
	_cursor.setAlpha(0);
}

void DemoGameplay::createGraphics()
{
	DemoScreen::createGraphics();

	// set up lighting for the cube shader (since no other shader on this screen uses lighting, we can do this here)
	MigUtil::theRend->setLightDirPos(0, lightDir, true);
}

void DemoGameplay::windowSizeChanged()
{
	DemoScreen::windowSizeChanged();

	// default matrices
	CubeUtil::loadDefaultPerspectiveMatrix(_projMatrix);
	CubeUtil::loadDefaultViewMatrix(_viewMatrix, MigUtil::convertToRadians(-45));
}

bool DemoGameplay::doFrame(int id, float newVal, void* optData)
{
	if (_idTextAnim == id)
	{
		if (_textAlpha == 1)
		{
			// animate the lengths of the lines of text
			int len = (int)newVal;
			if (_line1Text != nullptr)
				_line1Text->updateText(CubeUtil::getUpperSubString(_line1Str, len));
			if (_line2Text != nullptr)
				_line2Text->updateText(CubeUtil::getUpperSubString(_line2Str, len));
			if (_line3Text != nullptr)
				_line3Text->updateText(CubeUtil::getUpperSubString(_line3Str, len));
		}
		else
		{
			// animate the transparency of the lines of text
			_textAlpha = newVal;
		}

		if (_line1Text != nullptr)
			_line1Text->setAlpha(_textAlpha);
		if (_line2Text != nullptr)
			_line2Text->setAlpha(_textAlpha);
		if (_line3Text != nullptr)
			_line3Text->setAlpha(_textAlpha);
	}

	return DemoScreen::doFrame(id, newVal, optData);
}

void DemoGameplay::animComplete(int id, void* optData)
{
	if (_idPreDelayAnim == id)
	{
		// pre-animation delay is complete, start the text animation
		startTextAnim();
		_idPreDelayAnim = 0;
	}
	else if (_idTextAnim == id)
	{
		// text animations is complete, start the post-animation delay
		if (_postDelay > 0)
			_idPostDelayAnim = startDelayAnim(_postDelay);
		else
			onTextAnimComplete(_textAlpha > 0);

		_idTextAnim = 0;
	}
	else if (_idPostDelayAnim == id)
	{
		// post-animation delay is complete, invoke the call-back
		onTextAnimComplete(_textAlpha > 0);
		_idPostDelayAnim = 0;
	}

	DemoScreen::animComplete(id, optData);
}

bool DemoGameplay::update()
{
	// glow the NEXT button
	if (_nextBtn != nullptr)
	{
		int glow = Timer::gameTimeMillis() % 2000;
		glow = (glow > 1000 ? (2000 - glow) : glow);
		_nextBtn->setAlpha(glow / 1000.0f);
	}

	return DemoScreen::update();
}

bool DemoGameplay::render()
{
	// background screen
	drawBackgroundScreen();

	// set up the projection matrix
	MigUtil::theRend->setProjectionMatrix(_projMatrix);
	MigUtil::theRend->setViewMatrix(_viewMatrix);

	// game cube
	_gameCube.draw();

	// falling grids, lights, etc
	_launcher.draw();

	// stamps
	_stamps.draw();

	// reset the projection matrix
	MigUtil::theRend->setProjectionMatrix(nullptr);
	MigUtil::theRend->setViewMatrix(nullptr);

	// draw the cursor
	_cursor.draw();

	return true;
}

// overridden to allow the NEXT button to work
bool DemoGameplay::pointerPressed(float x, float y)
{
	if (_nextBtn != nullptr && _nextBtn->getRect().Contains(x, y))
		startFadeOut();
	return DemoScreen::pointerPressed(x, y);
}

void DemoGameplay::onCubeAnimComplete()
{
}

void DemoGameplay::onCubeStampComplete(AxisOrient axis)
{
	if (axis != AXISORIENT_NONE)
	{
		// get the stamp grid (might be empty) and apply the stamping to the cube
		GridInfo stampGridInfo;
		bool success = _stamps.getStampGridInfo(stampGridInfo, axis, true);
		success &= (stampGridInfo.dimen > 0);

		StampResult res;
		if (_gameCube.doStamp(res, axis, (success ? &stampGridInfo : nullptr)))
		{
			// the stamp will animate based upon the hit/miss
			_stamps.doStampResult(axis, res.wasHit, res.hitGridInfo);

			// play the sound
			if (!res.soundToPlay.empty())
				_sounds.playSound(res.soundToPlay);
		}
	}
}

void DemoGameplay::onCubeGameOverAnimComplete(bool win)
{
}

void DemoGameplay::onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier)
{
	if (gridInfo != nullptr)
	{
		// send the grid to the game cube for collision management
		CollisionResult res;
		if (_gameCube.doCollision(res, gridInfo, 1))
		{
			// add a rejection grid that will bounce off the cube
			if (!res.wasHit)
				_launcher.addRejection(res.rejGrid);

			// play the sound
			if (!res.soundToPlay.empty())
				_sounds.playSound(res.soundToPlay);
		}
	}
}

void DemoGameplay::onDisplayHint(const GridInfo& gridInfo)
{
	_gameCube.doHint(gridInfo);
}

void DemoGameplay::onTap(float x, float y)
{
	DemoScreen::onTap(x, y);

	if (!_userLockout)
	{
		// pass the tap to the stamp list to see if it hit any of those
		AxisOrient stampLocked = _stamps.processTap(x, y);
		if (stampLocked == AXISORIENT_NONE)
		{
			// pass the tap along to the launcher instead
			//mLauncher.processTap(x, y);   // disable because launcher buttons are doing this instead
		}
		else
		{
			// compute the distance the cube will have to travel
			float dist = (stampLocked == AXISORIENT_Y ? defStampDistY : defStampDistXZ);
			dist -= (defCubeRadius + defStampDepth);
			_gameCube.startStamp(stampLocked, -dist, 500);

			// start the stamp sound
			// TODO: need a new sound for stamping
			_sounds.playSound(WAV_STAMP);
		}
	}
}

void DemoGameplay::onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe)
{
	DemoScreen::onSwipe(x, y, dx, dy, swipe);

	// don't allow new rotations if the cube is already rotating
	if (_gameCube.canRotate())
	{
		GameCube::ROTATE_AXIS axis = GameCube::ROTATE_AXIS_NONE;
		int dir = GameCube::ROTATE_NONE;

		// determine rotation axis and direction
		if (swipe == SWIPE_HORIZONTAL)
		{
			axis = GameCube::ROTATE_AXIS_Y;
			dir = (dx > 0 ? GameCube::ROTATE_CCW : GameCube::ROTATE_CW);
		}
		else if (swipe == SWIPE_VERTICAL)
		{
			axis = (x < 0.5f ? GameCube::ROTATE_AXIS_X : GameCube::ROTATE_AXIS_Z);
			dir = GameCube::ROTATE_CCW;
			if (axis == GameCube::ROTATE_AXIS_X && dy < 0)
				dir = GameCube::ROTATE_CW;
			else if (axis == GameCube::ROTATE_AXIS_Z && dy > 0)
				dir = GameCube::ROTATE_CW;
		}

		if (axis != GameCube::ROTATE_AXIS_NONE)
			startCubeRotate(axis, dir);
	}
}

void DemoGameplay::onKey(VIRTUAL_KEY key)
{
	ScreenBase::onKey(key);

	// the user can always tap on the NEXT button
	if (key == ESCAPE)
	{
		_abortAll = true;
		//startFadeOut();
	}
	else if (!_userLockout)
	{
		// don't allow new rotations if the cube is already rotating
		if (_gameCube.canRotate())
		{
			if (key == NUMPAD4 || key == LEFT)
				startCubeRotate(GameCube::ROTATE_AXIS_Y, GameCube::ROTATE_CW);
			else if (key == NUMPAD6 || key == RIGHT)
				startCubeRotate(GameCube::ROTATE_AXIS_Y, GameCube::ROTATE_CCW);
			else if (key == NUMPAD7 || key == UP || key == HOME)
				startCubeRotate(GameCube::ROTATE_AXIS_Z, GameCube::ROTATE_CCW);
			else if (key == NUMPAD3 || key == DOWN || key == PAGEDOWN)
				startCubeRotate(GameCube::ROTATE_AXIS_Z, GameCube::ROTATE_CW);
			else if (key == NUMPAD9 || key == PAGEUP)
				startCubeRotate(GameCube::ROTATE_AXIS_X, GameCube::ROTATE_CW);
			else if (key == NUMPAD1 || key == END)
				startCubeRotate(GameCube::ROTATE_AXIS_X, GameCube::ROTATE_CCW);
		}
	}
}

bool DemoGameplay::onBackKey()
{
	_abortAll = true;
	return DemoScreen::onBackKey();
}

void DemoGameplay::onClick(int id, ControlBase* sender)
{
	_launcher.onClick(id);
}

// overloaded to support launch buttons
ControlBase* DemoGameplay::allocControl(const char* tag)
{
	if (!strcmp(tag, "LaunchButton"))
		return new LaunchButton(0);
	return nullptr;
}

// call-back invoked when any text animation is complete
void DemoGameplay::onTextAnimComplete(bool fadeIn)
{
}

int DemoGameplay::startDelayAnim(int delay)
{
	AnimItem animItem(this);
	animItem.configTimer(delay, false);
	return MigUtil::theAnimList->addItem(animItem);
}

void DemoGameplay::initLinesOfText(const std::string& text1, const std::string& text2, const std::string& text3)
{
	// initialize the text strings, and then clear them - they'll retain the sizing and placement
	if (_line1Text != nullptr && !text1.empty())
	{
		_line1Text->updateText(text1, JUSTIFY_CENTER);
		_line1Text->updateText("");
	}
	if (_line2Text != nullptr && !text1.empty())
	{
		_line2Text->updateText(text2, JUSTIFY_CENTER);
		_line2Text->updateText("");
	}
	if (_line3Text != nullptr && !text1.empty())
	{
		_line3Text->updateText(text3, JUSTIFY_CENTER);
		_line3Text->updateText("");
	}
}

int DemoGameplay::getMaxTextLength()
{
	int len = _line1Str.length();
	len = max((int)_line2Str.length(), len);
	len = max((int)_line3Str.length(), len);
	return len;
}

void DemoGameplay::startTextAnim()
{
	AnimItem animItem(this);
	if (_textAlpha == 1)
	{
		// text in animation
		int len = getMaxTextLength();
		animItem.configSimpleAnim(0, (float)len, _textAnimDur, AnimItem::ANIM_TYPE_LINEAR);
	}
	else
	{
		// text out animation
		animItem.configSimpleAnim(_textAlpha, 0, _textAnimDur, AnimItem::ANIM_TYPE_LINEAR);
	}
	_idTextAnim = MigUtil::theAnimList->addItem(animItem);
}

// starts animating in up to 3 lines of text to be displayed wherever the sub-class defines them
void DemoGameplay::startTextAnimIn(const std::string& text1, const std::string& text2, const std::string& text3, int preDelay, int duration, int postDelay)
{
	_line1Str = MigUtil::getString(text1, text1);
	_line2Str = MigUtil::getString(text2, text2);
	_line3Str = MigUtil::getString(text3, text3);
	_textAnimDur = duration;
	_postDelay = postDelay;
	_textAlpha = 1;	// in addition to the alpha blending value, this is our sign that this is the in animation

	initLinesOfText(_line1Str, _line2Str, _line3Str);

	if (preDelay > 0)
		_idPreDelayAnim = startDelayAnim(preDelay);
	else
		startTextAnim();
}

// starts animating out the same text as above
void DemoGameplay::startTextAnimOut(int preDelay, int duration, int postDelay)
{
	_textAnimDur = duration;
	_postDelay = postDelay;
	_textAlpha = 0.99f;	// in addition to the alpha blending value, this is our sign that this is the out animation

	if (preDelay > 0)
		_idPreDelayAnim = startDelayAnim(preDelay);
	else
		startTextAnim();
}

void DemoGameplay::startCubeRotate(GameCube::ROTATE_AXIS axis, int dir)
{
	// start the cube rotation
	_gameCube.startRotate(axis, dir, 500);

	// start the rotate sound
	_sounds.playSound(WAV_ROTATE);
}

///////////////////////////////////////////////////////////////////////////
// DemoGameplay1

ScreenBase* DemoGameplay1::getNextScreen()
{
	if (_abortAll)
		return new SplashScreen();
	return new DemoGameplay2();
}

void DemoGameplay1::create()
{
	DemoGameplay::create();

	// initialize the cube according to a preset pattern
	int indArray[] = { 0, 1, 0, 1, 0, 1 };
	Color dim[2];
	dim[0] = Color(0.5f, 0, 0);
	dim[1] = Color(0, 0, 0.5f);
	Color max[2];
	max[0] = Color(1, 0, 0);
	max[1] = Color(0, 0, 1);
	_gameCube.initCubeGrids(1, 2, dim, max, indArray);
}

void DemoGameplay1::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
	DemoGameplay::fadeComplete(fadeAlpha, fadeStyle);

	// screen fade in is complete, start animating in the text
	if (fadeStyle == FADE_STYLE_IN)
		startTextAnimIn("demo1_line1", "demo1_line2", "", 1000, 1000, 1000);
}

void DemoGameplay1::onTextAnimComplete(bool fadeIn)
{
	if (fadeIn)
	{
		// text is animated in, start the demo script only if this is the first round of text
		if (_userLockout)
			_script.start("demo1.xml");
	}
	else
	{
		// user can now interact w/ the screen
		startTextAnimIn("demo1_line3", "demo1_line4", "", 0, 1000, 1000);
		_userLockout = false;
	}
}

void DemoGameplay1::onStopScript()
{
	DemoGameplay::onStopScript();

	// script is complete, animate out the first round of text
	startTextAnimOut(0, 1000, 1000);
}

///////////////////////////////////////////////////////////////////////////
// DemoGameplay2

ScreenBase* DemoGameplay2::getNextScreen()
{
	if (_abortAll)
		return new SplashScreen();
	return new DemoGameplay3();
}

void DemoGameplay2::create()
{
	DemoGameplay::create();

	// initialize the cube according to a preset pattern
	int indArray[] = { 0, 1, 0, 1, 0, 1 };
	Color dim[2];
	dim[0] = Color(0.5f, 0, 0);
	dim[1] = Color(0, 0, 0.5f);
	Color max[2];
	max[0] = Color(1, 0, 0);
	max[1] = Color(0, 0, 1);
	_gameCube.initCubeGrids(2, 2, dim, max, indArray);

	bool fillList[] = { true, false, false, true };
	_gameCube.fillCubeSlots(CubeBase::FACE_FRONT, fillList, ARRAYSIZE(fillList));
	_gameCube.fillCubeSlots(CubeBase::FACE_TOP, fillList, ARRAYSIZE(fillList));

	// initialize the launcher
	_launcher.configLaunchSettings(0, 2000, 5000, 2);

	_phase = 1;
}

void DemoGameplay2::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
	DemoGameplay::fadeComplete(fadeAlpha, fadeStyle);

	// screen fade in is complete, start animating in the text
	if (fadeStyle == FADE_STYLE_IN)
		startTextAnimIn("demo2_line1", "demo2_line2", "", 1000, 1000, 1000);
}

void DemoGameplay2::startDemoPhase(int cubeFace, AxisOrient launchAxis, const char* demoScriptID)
{
	// launch a grid
	GridInfo newGrid = _gameCube.newGridCandidate(cubeFace);
	_launcher.startGridLaunch(newGrid, launchAxis);

	// and start the demo script
	_script.start(demoScriptID);
}

void DemoGameplay2::onTextAnimComplete(bool fadeIn)
{
	if (fadeIn)
	{
		if (_phase == 1)
			startDemoPhase(CubeBase::FACE_FRONT, AXISORIENT_X, "demo2_part1.xml");
		else if (_phase == 2)
			startDemoPhase(CubeBase::FACE_TOP, AXISORIENT_Z, "demo2_part2.xml");
		else if (_phase == 3)
			startDemoPhase(CubeBase::FACE_TOP, AXISORIENT_X, "demo2_part3.xml");
	}
	else
	{
		// text animation out is complete, on to the next phase
		if (_phase == 1)
			startTextAnimIn("demo2_line3", "demo2_line4", "", 500, 1000, 1000);
		else if (_phase == 2)
			startTextAnimIn("demo2_line5", "", "", 500, 500, 1000);
		else
			startFadeOut();
		_phase++;
	}
}

void DemoGameplay2::onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier)
{
	DemoGameplay::onFallingPieceComplete(gridInfo, numGrids, tapBonus, scoreMultiplier);

	// since the fall is complete, animate out
	startTextAnimOut(2000, 1000, 1000);
}

///////////////////////////////////////////////////////////////////////////
// DemoGameplay3

ScreenBase* DemoGameplay3::getNextScreen()
{
	if (_abortAll)
		return new SplashScreen();
	return new DemoGameplay4();
}

void DemoGameplay3::create()
{
	DemoGameplay::create();

	// initialize the cube according to a preset pattern
	int indArray[] = { 0, 1, 0, 1, 0, 1 };
	Color dim[2];
	dim[0] = Color(0.5f, 0, 0);
	dim[1] = Color(0, 0, 0.5f);
	Color max[2];
	max[0] = Color(1, 0, 0);
	max[1] = Color(0, 0, 1);
	_gameCube.initCubeGrids(2, 2, dim, max, indArray);

	bool fillList[] = { true, true, true, true };
	_gameCube.fillCubeSlots(CubeBase::FACE_FRONT, fillList, ARRAYSIZE(fillList));
	_gameCube.fillCubeSlots(CubeBase::FACE_RIGHT, fillList, ARRAYSIZE(fillList));

	// initialize stamps since we'll be using them
	_stamps.startNewStamp(AXISORIENT_X);
	_stamps.startNewStamp(AXISORIENT_Z);

	_phase = 1;
}

void DemoGameplay3::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
	DemoGameplay::fadeComplete(fadeAlpha, fadeStyle);

	// screen fade in is complete, start animating in the text
	if (fadeStyle == FADE_STYLE_IN)
		startTextAnimIn("demo3_line1", "demo3_line2", "", 500, 1000, 1000);
}

void DemoGameplay3::onTextAnimComplete(bool fadeIn)
{
	if (fadeIn)
	{
		if (_phase == 1)
			_script.start("demo3_part1.xml");
		else if (_phase == 2)
			_script.start("demo3_part2.xml");
	}
	else
	{
		// text animation out is complete, on to the next phase
		if (_phase == 1)
			startTextAnimIn("demo3_line3", "demo3_line4", "", 500, 1000, 1000);
		else
			startFadeOut();
		_phase++;
	}
}

void DemoGameplay3::onCubeStampComplete(AxisOrient axis)
{
	// since the stamp is complete, animate out
	startTextAnimOut(2000, 1000, 1000);

	DemoGameplay::onCubeStampComplete(axis);
}

void DemoGameplay3::onStopScript()
{
	DemoGameplay::onStopScript();

	if (_phase >= 2)
		startTextAnimOut(2000, 1000, 1000);
}

///////////////////////////////////////////////////////////////////////////
// DemoGameplay4

ScreenBase* DemoGameplay4::getNextScreen()
{
	// if this script has an actual game (and not just a tutorial, then go to the game screen)
	if (GameScripts::getCurrScript().isLoaded())
		return new GameScreen();
	return new SplashScreen();
}

void DemoGameplay4::create()
{
	DemoGameplay::create();

	// initialize the cube according to a preset pattern
	int indArray[] = { 0, 1, 0, 1, 0, 1 };
	Color dim[2];
	dim[0] = Color(0.5f, 0, 0);
	dim[1] = Color(0, 0, 0.5f);
	Color max[2];
	max[0] = Color(1, 0, 0);
	max[1] = Color(0, 0, 1);
	_gameCube.initCubeGrids(2, 2, dim, max, indArray);

	// initialize the launcher
	_launcher.configLaunchSettings(0, 2000, 5000, 2);

	_phase = 1;
}

void DemoGameplay4::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
	DemoGameplay::fadeComplete(fadeAlpha, fadeStyle);

	// screen fade in is complete, start animating in the text
	if (fadeStyle == FADE_STYLE_IN)
		startTextAnimIn("demo4_line1", "", "", 500, 1000, 1000);
}

void DemoGameplay4::startDemoPhase(AxisOrient launchAxis, const char* demoScriptID)
{
	// launch an evil grid
	int numGrids = 0;
	GridInfo* newGrids = _gameCube.newGridCandidatesList(true, false, true, false, numGrids);
	_launcher.startEvilGridLaunch(newGrids, numGrids, EVILGRIDSTYLE_COLOR_ONLY, launchAxis);

	// and start the demo script
	_script.start(demoScriptID);
}

void DemoGameplay4::onTextAnimComplete(bool fadeIn)
{
	if (fadeIn)
	{
		if (_phase == 1)
			startDemoPhase(AXISORIENT_X, "demo4_part1.xml");
		else
			startDemoPhase(AXISORIENT_Z, "demo4_part2.xml");
	}
	else
	{
		// text animation out is complete, on to the next phase
		if (_phase == 1)
			startTextAnimIn("demo4_line2", "", "", 500, 1000, 1000);
		else
			startFadeOut();
		_phase++;
	}
}

void DemoGameplay4::onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier)
{
	DemoGameplay::onFallingPieceComplete(gridInfo, numGrids, tapBonus, scoreMultiplier);

	if (_phase == 1)
		startTextAnimOut(1000, 1000, 1000);
	else
		startTextAnimOut(2000, 1000, 1000);
}
