#include "pch.h"
#include "CubeUtil.h"
#include "GameScreen.h"
#include "GameScripts.h"
#include "GameOverlays.h"
#include "EndGameScreens.h"
#include "SplashScreen.h"
#include "../core/Timer.h"

using namespace Cuboingo;
using namespace MigTech;

// default light used for this screen (rotated to compensate for the -45 degree rotation in the view matrix)
static const Vector3 lightDir = Vector3(-0.7f, -1, -0.7f).normalize();

GameScreen::GameScreen()
	: ScreenBase("GameScreen"), _cube(this), _launcher(_cube), _stamps(&_scoreKeeper),
	_gameIsOver(false), _gameWon(false), _tipsDisplayed(false)
{
	_lcList.addToList(_cube);
	_lcList.addToList(_launcher);
	_lcList.addToList(_stamps);
	_lcList.addToList(_sparks);
	_lcList.addToList(_shadowPass);
}

ScreenBase* GameScreen::getNextScreen()
{
	// if the game didn't actually end (ESC), go back to splash screen
	if (!_gameIsOver)
		return new SplashScreen();

	// win screen
	if (_gameWon)
		return new WinScreen(_scoreKeeper.getRealScore());

	// lose screen
	return new LoseScreen();
}

void GameScreen::create()
{
	// debugging aid - if no script has been loaded, then load the testing one
	const Script& script = GameScripts::getCurrScript();
	if (!script.isLoaded())
		GameScripts::loadTestScript();

	// load a background handler based upon a token in the script (we take ownership of the handler from the script)
	initBackgroundScreen(script.bgHandler);
	initOverlayScreen(script.overlayHandler);

	// this will call create() on the background/overlay handlers
	ScreenBase::create();

	// preload sound effects and update the current time since this could take a while
	_sounds.loadSound("rotate");
	_sounds.loadSound("match");
	_sounds.loadSound("same");
	_sounds.loadSound("error");

	// initialize the launcher
	_launcher.init(this);
	_launcher.configControls(_controls);

	// initialize the score keeper
	_scoreKeeper.init();

	// initialize the particles
	_sparks.init();

	// initialize the tamps
	_stamps.init();

	// initialize power-ups
	CubeUtil::currPowerUp.clear();

	// initialize the shadow rendering pass
	if (_shadowPass.init())
		_renderPasses.push_back(&_shadowPass);

	// load the in game music, if it hasn't started already
	if (MigUtil::theMusic == nullptr && MigUtil::theAudio != nullptr)
	{
		std::string musicID = GameScripts::getCurrScript().musicID;
		if (musicID.empty())
			musicID = "ingame_music.mp3";
		MigUtil::theMusic = MigUtil::theAudio->loadMedia(musicID, AudioBase::AUDIO_CHANNEL_MUSIC);
	}
}

void GameScreen::createGraphics()
{
	ScreenBase::createGraphics();

	// set up lighting for the cube shader (since no other shader on this screen uses lighting, we can do this here)
	MigUtil::theRend->setLightDirPos(0, lightDir, true);
}

void GameScreen::windowSizeChanged()
{
	ScreenBase::windowSizeChanged();

	// default matrices
	CubeUtil::loadDefaultPerspectiveMatrix(_projMatrix);
	CubeUtil::loadDefaultViewMatrix(_viewMatrix, MigUtil::convertToRadians(-45));
}

void GameScreen::visibilityChanged(bool vis)
{
	if (vis)
	{
		if (_overlay == nullptr)
		{
			// pause game time and display the pause overlay
			handlePause();
		}
	}
	else
		ScreenBase::visibilityChanged(false);
}

void GameScreen::onTap(float x, float y)
{
	// launcher buttons get first pass
	if (!_controls.onTap(x, y))
	{
		// then the falling pieces themselves
		if (!_launcher.onTap(x, y, _projMatrix, _viewMatrix))
		{
			// don't allow a new stamp action if one is in progress
			if (_cube.canStamp())
			{
				// pass the tap to the stamp list to see if it hit any of those
				AxisOrient stampLocked = _stamps.processTap(x, y);
				if (stampLocked != AXISORIENT_NONE)
					startCubeStamp(stampLocked);
			}
		}
	}
}

void GameScreen::onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe)
{
	// don't allow new rotations if the cube is already rotating
	if (_cube.canRotate())
	{
		GameCube::ROTATE_AXIS axisLocked = GameCube::ROTATE_AXIS_NONE;
		int dir = GameCube::ROTATE_NONE;

		// determine rotation axis and direction
		if (swipe == SWIPE_HORIZONTAL)
		{
			axisLocked = GameCube::ROTATE_AXIS_Y;
			dir = (dx > 0 ? GameCube::ROTATE_CCW : GameCube::ROTATE_CW);
		}
		else if (swipe == SWIPE_VERTICAL)
		{
			axisLocked = (x < 0.5f ? GameCube::ROTATE_AXIS_X : GameCube::ROTATE_AXIS_Z);
			dir = GameCube::ROTATE_CCW;
			if (axisLocked == GameCube::ROTATE_AXIS_X && dy < 0)
				dir = GameCube::ROTATE_CW;
			else if (axisLocked == GameCube::ROTATE_AXIS_Z && dy > 0)
				dir = GameCube::ROTATE_CW;
		}

		if (axisLocked != GameCube::ROTATE_AXIS_NONE)
			startCubeRotate(axisLocked, dir);
	}
}

void GameScreen::onKey(VIRTUAL_KEY key)
{
	ScreenBase::onKey(key);

	// don't allow new rotations if the cube is already rotating
	if (_cube.canRotate())
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
		else
			_launcher.onKey(key);
	}
}

bool GameScreen::onBackKey()
{
	LOGINFO("(GameScreen::onBackKey) Back key detected");

	if (_overlay == nullptr)
	{
		// pause game time and display the pause overlay
		handlePause();
	}

	// this means we are consuming the event
	return true;
}

void GameScreen::onClick(int id, ControlBase* sender)
{
	_launcher.onClick(id);
}

// overloaded to support launch buttons
ControlBase* GameScreen::allocControl(const char* tag)
{
	if (!strcmp(tag, "LaunchButton"))
		return new LaunchButton(0);
	return nullptr;
}

void GameScreen::fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle)
{
	if (fadeStyle == FADE_STYLE_IN && !_cube.isVisible())
	{
		if (!_tipsDisplayed && GameScripts::getCurrScript().tips.size() > 0 && GameScripts::getCurrLevelIndex() == 0)
		{
			// we need to display tips before starting the game
			startNewOverlay(new TipsOverlay());
			_tipsDisplayed = true;
		}
		else if (startNextLevel())
			_sounds.playSound(WAV_START);
	}

	ScreenBase::fadeComplete(fadeAlpha, fadeStyle);
}

bool GameScreen::update()
{
	_scoreKeeper.update();

	// if there's no launcher and stamping then the game is already over
	if (!_gameIsOver)
	{
		// check for game or level time finished
		if (_scoreKeeper.checkTimerForGameOver())
		{
			// time up, game over
			std::string wavLose = startGameLose();
			if (!wavLose.empty() && MigUtil::theAudio != nullptr)
				MigUtil::theAudio->playMedia(wavLose, AudioBase::AUDIO_CHANNEL_SOUND);
		}
	}

	return ScreenBase::update();
}

bool GameScreen::render()
{
	// background screen
	drawBackgroundScreen();

	// set up the view matrix
	MigUtil::theRend->setProjectionMatrix(_projMatrix);
	MigUtil::theRend->setViewMatrix(_viewMatrix);

	// draw any 3D text strings
	if (_bgHandler != nullptr && _bgHandler->isVisible())
		_scoreKeeper.draw3D();

	// shadow catcher
	_shadowPass.draw();

	// game cube
	_cube.draw();

	// falling grids, lights, etc
	_launcher.draw();

	// stamps
	_stamps.draw();

	// particles
	_sparks.draw();

	// back to 2D drawing (TODO: should this be in renderOverlays() instead?)
	MigUtil::theRend->setProjectionMatrix(nullptr);
	MigUtil::theRend->setViewMatrix(nullptr);

	// 2D power-up icons
	_stamps.drawIcons();

	// draw any 2D text strings (which are overlaid on the final image)
	if (_bgHandler != nullptr && _bgHandler->isVisible())
		_scoreKeeper.drawOverlay();

	return true;
}

bool GameScreen::renderPass(int index, RenderPass* pass)
{
	// just the cube and falling pieces
	_cube.draw();
	_launcher.draw();

	return true;
}

bool GameScreen::doFrame(int id, float newVal, void* optData)
{
	if (_idLaunchTimer == id)
	{
		doLaunch();
		return true;
	}
	else if (_idStampTimer == id)
	{
		doStamp();
		return true;
	}
	else if (_idLosingAnim == id)
	{
		// this creates a flashing screen that shows during the second half of the animation
		float col = ((((int)newVal) % 100 < 50) ? 1.0f : 0.0f);
		_clearColor = Color(col, col, col);
		if (_bgHandler != nullptr)
			_bgHandler->setVisible(newVal < 500);
	}

	return ScreenBase::doFrame(id, newVal, optData);
}

void GameScreen::animComplete(int id, void* optData)
{
	if (_idLosingAnim == id)
	{
		// restore the background image
		_clearColor = Color(0, 0, 0);
		if (_bgHandler != nullptr)
			_bgHandler->setVisible(true);
		_idLosingAnim = 0;
	}
	else if (_idResetTimer == id)
	{
		startLauncher();
		_idResetTimer = 0;
	}

	ScreenBase::animComplete(id, optData);
}

void GameScreen::onOverlayExit(long duration)
{
	if (!Timer::isGameTimePaused())
	{
		// restore the fade back to normal
		if (!_gameIsOver && !GameScripts::isLastLevel())
			startFadeScreen(defFadeDuration, 0.5f, 0.0f, true);
		else
			startFadeOut(defFadeDuration);
	}
	else
	{
		// game was paused so fade back in quickly
		startFadeScreen(defFastFadeDuration, 0.5f, 0.0f, true);
	}
}

void GameScreen::onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration)
{
	if (transType == OVERLAY_TRANSITION_EXIT)
	{
		if (!Timer::isGameTimePaused())
		{
			// summary is over, start the next level (unless the user won)
			if (!_gameIsOver && startNextLevel())
			{
				// play the intro sound again
				_sounds.playSound(WAV_START);
			}
		}
		else
		{
			LOGINFO("(GameScreen::onOverlayComplete) Game is resuming");

			// the pause overlay is what ended
			Timer::resumeGameTime();
			if (MigUtil::theMusic != nullptr)
				MigUtil::theMusic->resumeSound();
		}
	}

	ScreenBase::onOverlayComplete(transType, duration);
}

void GameScreen::onOverlayCustom(int id, void* data)
{
	// for now the only overlay that sends this is the pause overlay to quit the game
	startFadeOut();
}

// called by GameCube when a start or exit animation is complete
void GameScreen::onCubeAnimComplete()
{
	if (_cube.isFilled())
	{
		// is a game summary screen required
		if (GameScripts::isBonusScreenRequired())
		{
			// yes, create it
			startNewOverlay(new SummaryOverlay(_scoreKeeper));
		}
		else
		{
			// this was an exit animation so load the next level
			startNextLevel();
		}
	}
	else if (!_gameIsOver)
	{
		// resume the game timer (was paused when the new level loaded)
		_scoreKeeper.resumeGameTimer();

		// launch the first piece
		startLauncher();

		// start the stamp timer, if applicable
		const Level& lvl = GameScripts::getCurrLevel();
		if (lvl.stampPeriod > 0 && lvl.stampProbability > 0)
		{
			AnimItem animItem(this);
			animItem.configTimer(lvl.stampPeriod, true);
			_idStampTimer = MigUtil::theAnimList->addItem(animItem);
		}

		// play the in game music, if it hasn't started already
		if (MigUtil::theMusic != nullptr && !MigUtil::theMusic->isPlaying())
			MigUtil::theMusic->playSound(true);
	}
}

// called by GameCube when a stamping action is complete
void GameScreen::onCubeStampComplete(AxisOrient axis)
{
	if (axis != AXISORIENT_NONE)
	{
		// get the stamp grid (might be empty) and apply the stamping to the cube
		GridInfo stampGridInfo;
		bool isNotEmpty = _stamps.getStampGridInfo(stampGridInfo, axis, true);

		StampResult res;
		if (_cube.doStamp(res, axis, (isNotEmpty ? &stampGridInfo : nullptr)))
		{
			// stamping rules change depending on whether the stamp had a grid assigned to it
			if (isNotEmpty)
			{
				if (res.wasHit)
				{
					// handle hit logic
					std::string soundID = handleCubeHit(res.needObsoleteCheck, stampGridInfo.mapIndex);
					if (!soundID.empty())
						res.soundToPlay = soundID;	// overrides the default sound

					// score keeping
					const Level& lvl = GameScripts::getCurrLevel();
					if (res.wasSideComplete || (lvl.stampStyle == STAMPSTYLE_BONUS_RANDOM))
						_scoreKeeper.stampMatch(stampGridInfo.orient);
				}
				else
				{
					// score keeping
					_scoreKeeper.stampMiss(stampGridInfo.orient);

					// handle miss logic
					std::string soundID = handleCubeMiss();
					if (!soundID.empty())
						res.soundToPlay = soundID;	// overrides the default sound
				}
			}
			else
			{
				// score keeping
				if (res.wasHit)
					_scoreKeeper.stampMatch(axis);
			}

			// the stamp will animate based upon the hit/miss
			_stamps.doStampResult(axis, res.wasRealHit, res.hitGridInfo);

			// play the sound
			if (!res.soundToPlay.empty())
				_sounds.playSound(res.soundToPlay);
		}
	}
}

void GameScreen::onCubeGameOverAnimComplete(bool win)
{
	// is a game summary screen required
	if (GameScripts::isBonusScreenRequired() && win)
	{
		// yes, create it
		startNewOverlay(new SummaryOverlay(_scoreKeeper));
	}
	else
	{
		// fade to black
		startFadeOut(defFadeDuration);
	}
}

void GameScreen::onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier)
{
	if (gridInfo != nullptr)
	{
		// send the grid to the game cube for collision management
		//CubeUtil.info("doCollision: " + grid.toString());
		CollisionResult res;
		if (_cube.doCollision(res, gridInfo, numGrids))
		{
			if (res.wasHit)
			{
				// handle hit logic
				string soundID = handleCubeHit(res.needObsoleteCheck, res.mapIndexHit);
				if (soundID != "")
					res.soundToPlay = soundID;	// overrides the default sound

				// score keeping
				_scoreKeeper.tapComplete(res.orientHit, tapBonus, scoreMultiplier);
				if (res.wasSideComplete)
					_scoreKeeper.sideComplete(res.orientHit, scoreMultiplier);
				else if (res.wasPieceFilled)
					_scoreKeeper.pieceMatch(res.orientHit, scoreMultiplier);

				// particles
				if (res.wasSideComplete)
					_sparks.sideComplete(res.orientHit, res.colHit);
			}
			else
			{
				// score keeping
				_scoreKeeper.pieceMiss(res.orientHit);

				// handle miss logic
				string soundID = handleCubeMiss();
				if (soundID != "")
					res.soundToPlay = soundID;	// overrides the default sound

				// add a rejection grid that will bounce off the cube
				_launcher.addRejection(res.rejGrid);
			}

			// play the sound
			if (!res.soundToPlay.empty())
				_sounds.playSound(res.soundToPlay);

			// check for a possible launcher reset
			checkForLauncherReset();
		}
	}
}

void GameScreen::onDisplayHint(const GridInfo& gridInfo)
{
	_cube.doHint(gridInfo);
}

bool GameScreen::startNextLevel()
{
	bool ok = true;

	// only start the next level if the cube is filled, else we're probably at game start
	if (_cube.isFilled())
	{
		// load the next level
		ok = GameScripts::nextLevel();
		LOGINFO("(GameScreen::startNextLevel) Starting next level %d", GameScripts::getCurrLevelIndex() + 1);
	}

	if (ok)
	{
		// load the new grids for the new level
		_cube.initCubeGrids();

		// start a new intro animation
		_cube.startIntroAnimation();
	}
	return ok;
}

void GameScreen::startLauncher()
{
	// launch the first piece
	doLaunch();

	// start the launch timer
	const Level& lvl = GameScripts::getCurrLevel();
	int duration = (lvl.idleTime + lvl.glowTime + lvl.fallTime) / lvl.fallingPieces;
	AnimItem animItem(this);
	animItem.configTimer(duration, true);
	_idLaunchTimer = MigUtil::theAnimList->addItem(animItem);
}

void GameScreen::doLaunch()
{
	if (_launcher.canLaunch())
	{
		bool doNormalLaunch = true;

		// determine if it's time to launch an evil or wild card falling piece
		const Level& lvl = GameScripts::getCurrLevel();
		if (MigUtil::rollAgainstPercent(lvl.evilProbability))
		{
			// launch an evil falling piece, if possible
			int numGrids;
			GridInfo* newGrids = _cube.newGridCandidatesList(true, false, true, false, numGrids);
			if (_launcher.startEvilGridLaunch(newGrids, numGrids))
			{
				// don't do a normal launch
				doNormalLaunch = false;
			}
		}
		else if (MigUtil::rollAgainstPercent(lvl.wildProbability))
		{
			// launch a wild-card falling piece, if possible
			int numGrids;
			GridInfo* newGrids = _cube.newGridCandidatesList(true, false, true, false, numGrids);
			if (_launcher.startWildGridLaunch(newGrids, numGrids))
			{
				// don't do a normal launch
				doNormalLaunch = false;
			}
		}
		else if (MigUtil::rollAgainstPercent(lvl.burstProbability))
		{
			// launch a burst, if possible
			int numGrids;
			GridInfo* newGrids = _cube.newBurstCandidatesList(true, numGrids);
			if (_launcher.startBurstLaunch(newGrids, numGrids))
			{
				// don't do a normal launch
				doNormalLaunch = false;
			}
		}
		else if (MigUtil::rollAgainstPercent(lvl.evilBurstProbability))
		{
			// launch an evil burst, if possible
			int numGrids;
			GridInfo* newGrids = _cube.newEvilBurstCandidatesList(numGrids);
			if (_launcher.startEvilBurstLaunch(newGrids, numGrids))
			{
				// don't do a normal launch
				doNormalLaunch = false;
			}
		}

		if (doNormalLaunch)
		{
			// launch a normal falling piece
			GridInfo newGrid;
			if (_cube.newFallingGridCandidate(newGrid, true, true))
				_launcher.startGridLaunch(newGrid);
		}
	}
}

void GameScreen::doStamp()
{
	if (_stamps.isStampAvailable())
	{
		const Level& lvl = GameScripts::getCurrLevel();
		if (MigUtil::rollAgainstPercent(lvl.stampProbability))
		{
			if (lvl.stampStyle != STAMPSTYLE_POWERUP_COLOR &&
				lvl.stampStyle != STAMPSTYLE_POWERUP_FACE &&
				lvl.stampStyle != STAMPSTYLE_POWERUP_FILLED_COLOR)
			{
				bool allowFilled = (lvl.stampStyle == STAMPSTYLE_BONUS_RANDOM);
				bool allowPartial = (lvl.stampStyle == STAMPSTYLE_COMPLETES_FACE);

				GridInfo newGrid;
				if (_cube.newGridCandidate(newGrid, false, allowFilled, allowPartial))
				{
					if (!_stamps.isMapIndexUsed(newGrid.mapIndex))
					{
						// invert the fill list if the piece is meant to complete a face
						if (lvl.stampStyle == STAMPSTYLE_COMPLETES_FACE)
							newGrid.invertFillList();

						_stamps.startNewStamp(newGrid);
					}
				}
			}
			else
			{
				bool allowFilled = (lvl.stampStyle == STAMPSTYLE_POWERUP_COLOR || lvl.stampStyle == STAMPSTYLE_POWERUP_FILLED_COLOR);
				bool allowEmpty = (lvl.stampStyle == STAMPSTYLE_POWERUP_FILLED_COLOR);
				bool allowPartial = (lvl.stampStyle == STAMPSTYLE_POWERUP_FACE || lvl.stampStyle == STAMPSTYLE_POWERUP_FILLED_COLOR);

				int numGrids;
				GridInfo* newGrids = _cube.newGridCandidatesList(allowEmpty, allowFilled, allowPartial, true, numGrids);
				if (newGrids != nullptr && numGrids > 0)
				{
					// invert the fill lists if the piece is meant to complete a face
					if (lvl.stampStyle == STAMPSTYLE_POWERUP_FACE)
					{
						for (int i = 0; i < numGrids; i++)
							newGrids[i].invertFillList();
					}
					else if (lvl.stampStyle == STAMPSTYLE_POWERUP_FILLED_COLOR)
					{
						for (int i = 0; i < numGrids; i++)
							newGrids[i].setFillList(true);
					}

					_stamps.startNewStamp(newGrids, numGrids);
				}
			}
		}
	}
}

void GameScreen::startCubeRotate(GameCube::ROTATE_AXIS axis, int dir)
{
	//LOGDBG("(GameScreen::startCubeRotate) Starting cube rotate (%d, %d)", axis, dir);

	// start the cube rotation
	_cube.startRotate(axis, dir);

	// start the rotate sound
	_sounds.playSound(WAV_ROTATE);
}

void GameScreen::startCubeStamp(AxisOrient axis)
{
	//LOGDBG("(GameScreen::startCubeStamp) Starting cube stamp (%d)", axis);

	// compute the distance the cube will have to travel
	float dist = (axis == AXISORIENT_Y ? defStampDistY : defStampDistXZ);
	dist -= (defCubeRadius + defStampDepth);
	_cube.startStamp(axis, -dist);

	// start the stamp sound
	_sounds.playSound(WAV_STAMP);
}

void GameScreen::clearGameElements()
{
	// tell the launcher/stamp list to remove any outstanding grids
	_launcher.clearAllItems();
	_stamps.clearAllStamps();

	// clear the launch timer
	_idLaunchTimer.clearAnim();

	// clear the stamp timer
	_idStampTimer.clearAnim();
}

std::string GameScreen::startGameWin()
{
	LOGINFO("(GameScreen::startGameWin) Starting game win sequence");

	_gameIsOver = true;
	_gameWon = true;

	// clear the game elements
	clearGameElements();

	// game win animation
	_cube.startWinAnimation();

	return WAV_WIN;
}

std::string GameScreen::startGameLose()
{
	LOGINFO("(GameScreen::startGameLose) Starting game lose sequence");

	_gameIsOver = true;

	// clear the game elements
	clearGameElements();

	// game over animation
	_cube.startLoseAnimation();

	// set up an animation to flash the screen
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1000, 1000, AnimItem::ANIM_TYPE_LINEAR);
	_idLosingAnim = MigUtil::theAnimList->addItem(animItem);

	return WAV_DEATH;
}

void GameScreen::handlePause()
{
	LOGINFO("(GameScreen::handlePause) Game is paused");

	// pause game time and display the pause overlay
	Timer::pauseGameTime();
	startNewOverlay(new PauseOverlay(), 0.5f, 0, true);
	if (MigUtil::theMusic != nullptr)
		MigUtil::theMusic->pauseSound();
	_sounds.playSound(WAV_PAUSE);
}

std::string GameScreen::handleCubeHit(bool needObsoleteCheck, int mapIndex)
{
	std::string id = "";

	// if the cube is filled then we need to move to the next level
	if (_cube.isFilled())
	{
		// pause the countdown timer
		_scoreKeeper.pauseGameTimer();

		if (GameScripts::isLastLevel())
		{
			// last level, start the game win animation
			startGameWin();

			// and we'll need to play this sound
			id = WAV_WIN;
		}
		else
		{
			// clear the game elements
			clearGameElements();

			// start the exit animation
			_cube.startExitAnimation();
		}
	}
	else if (needObsoleteCheck)
	{
		// the falling piece was a hit and might have rendered other falling pieces and/or stamps obsolete
		_launcher.checkForObsoleteFallingPieces(mapIndex);
		_stamps.checkForObsoleteStamps(mapIndex);
	}

	return id;
}

std::string GameScreen::handleCubeMiss()
{
	std::string id = "";

	if (_scoreKeeper.checkMissesForGameOver())
	{
		// too many misses, start the game lose animation
		startGameLose();

		// and we'll need to play this sound
		id = WAV_DEATH;
	}

	return id;
}

void GameScreen::checkForLauncherReset()
{
	if (!_gameIsOver && _launcher.getPieceCount() == 0 && _idLaunchTimer.isActive())
	{
		const Level& lvl = GameScripts::getCurrLevel();
		if (lvl.resetTime > 0)
		{
			// clear the launch timer
			_idLaunchTimer.clearAnim();

			// set a new timer that will restart the launcher
			AnimItem animItem(this);
			animItem.configTimer(lvl.resetTime, false);
			_idResetTimer = MigUtil::theAnimList->addItem(animItem);
		}
	}
}
