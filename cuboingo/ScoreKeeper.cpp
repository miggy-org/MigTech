#include "pch.h"
#include "CubeConst.h"
#include "ScoreKeeper.h"
#include "../core/MigUtil.h"
#include "CubeUtil.h"

using namespace MigTech;
using namespace Cuboingo;

///////////////////////////////////////////////////////////////////////////
// PopupBonusText

// the size of bonus pop-up text
static const float BONUS_HEIGHT = 0.12f;

// the amount of distance that pop-up text will travel
static const float DIST_GOOD = 0.2f;
static const float DIST_BAD = -0.4f;

// durations of pop-up animations
static const int ANIM_LENGTH_SCALE = 500;
static const int ANIM_LENGTH_GOOD = 1000;
static const int ANIM_LENGTH_BAD = 2000;

PopupBonusText::PopupBonusText()
{
	isGood = false;
	param = 0;
	size = 0;
	sizeScalar = 1;
}

void PopupBonusText::init(bool good, const std::string& text, float u, float v, float scalar, const ScoreConfig& cfg)
{
	isGood = good;
	theText.init(text, JUSTIFY_CENTER);
	loc = MigUtil::screenPercentToCameraPlane(u, v);
	sizeScalar = scalar;

	// color
	col = Color(isGood ? cfg.goodColor : cfg.badColor);

	// set up the animation for the scaling
	float fs[] = { 0.0f, 0.50f, 0.80f, 1.00f, 1.10f, 1.17f, 1.20f, 1.17f, 1.10f, 1.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, BONUS_HEIGHT, ANIM_LENGTH_SCALE, fs, ARRAYSIZE(fs));
	idScaleAnim = MigUtil::theAnimList->addItem(animItemS);

	// set up the animation for everything else
	AnimItem animItemP(this);
	if (isGood)
	{
		// simple linear animation
		animItemP.configSimpleAnim(0, 1, ANIM_LENGTH_GOOD, AnimItem::ANIM_TYPE_LINEAR);
	}
	else
	{
		// a more dramatic animation
		float fy[] = { 1.0f, 0.99f, 0.98f, 0.96f, 0.94f, 0.90f, 0.80f, 0.65f, 0.40f, 0.0f };
		animItemP.configParametricAnim(1, 0, ANIM_LENGTH_BAD, fy, ARRAYSIZE(fy));
	}
	idParamAnim = MigUtil::theAnimList->addItem(animItemP);
}

bool PopupBonusText::doFrame(int id, float newVal, void* optData)
{
	if (idParamAnim == id)
	{
		param = newVal;
		col.a = (1.0f - newVal);
	}
	else if (idScaleAnim == id)
	{
		size = newVal;
	}
	return true;
}

void PopupBonusText::animComplete(int id, void* optData)
{
	if (idScaleAnim == id)
		idScaleAnim = 0;
	if (idParamAnim == id)
		idParamAnim = 0;
}

void PopupBonusText::draw()
{
	float lu = loc.x;
	float lv = loc.y + (isGood ? DIST_GOOD : DIST_BAD)*param;

	theText.transform(sizeScalar*size, 1.0f, lu, lv, 0, 0, 0, 0);
	theText.draw(col);

	//MigUtil::theFont->draw(text, col.r, col.g, col.b, col.a, u, v, sizeScalar*size, JUSTIFY_CENTER);
}

///////////////////////////////////////////////////////////////////////////
// ScoreKeeper

// number of digits to display
static const int SCORE_DIGITS = 6;

// the length of time for the score to animate when it updates
static const int SCORE_ANIM_DURATION = 1000;

// screen percentages for side bonus pop-up positions
static const float fSideScoreU[] = { 0, 0.3f, 0.7f, 0.5f };
static const float fSideScoreV[] = { 0, 0.7f, 0.7f, 0.4f };

// screen percentages for stamp bonus pop-up positions
static const float fStampScoreU[] = { 0, 0.75f, 0.25f, 0.50f };
static const float fStampScoreV[] = { 0, 0.45f, 0.45f, 0.80f };

ScoreKeeper::ScoreKeeper()
{
	_isScored = false;
	_isLegacy = false;
	_currScore = 0;
	_realScore = 0;
	_gameMissCount = 0;
	_levelMissCount = 0;
	_stampHitCount = 0;
	_stampMissCount = 0;
	_gameStartTime = 0;
	_levelStartTime = 0;
	_lastStampMatch = 0;
	_stampChainCount = 0;
}

ScoreKeeper::~ScoreKeeper()
{
	GameScripts::removeCallback(this);
}

// all of the display text fields are initialized here
void ScoreKeeper::initText()
{
	// if we ever get updated graphics from Chris for all scripts, we can rip out the legacy stuff
	const Script& script = GameScripts::getCurrScript();
	_isLegacy = (script.imageID.empty() ? true : false);
	float sizeHeader = 0.80f;
	float sizeValue = 1.00f;
	float sizeScore = 0.60f;

	// current score text
	_currScoreText.init(MigUtil::intToPaddedString(_currScore, SCORE_DIGITS), JUSTIFY_CENTER);
	_currScoreText.transform(sizeScore, 1.0f, 0, -3, 0, -rad90, rad45, 0);

	// the raw coordinates below are for FHD layout, adjust for real aspect ratio
	Size size = MigUtil::theRend->getOutputSize();
	float screenAspect = size.width / size.height;
	float adjust = (screenAspect / 1.78f);

	if (!_isLegacy)
	{
		// remaining time header
		_timeHeader.init(MigUtil::getString("time", "TIME"), JUSTIFY_CENTER);
		_timeHeader.transform(sizeHeader, 1.0f, -2.0f*adjust, 2, -7, 0, rad90, 0);

		// remaining time text
		_timeText.init("0:00", JUSTIFY_CENTER);
		_timeText.transform(sizeValue, 1.0f, -2.0f*adjust, 0.8f, -7, 0, rad90, 0);

		// cube level header
		_cubeHeader.init(MigUtil::getString("cube", "CUBE"), JUSTIFY_CENTER);
		_cubeHeader.transform(sizeHeader, 1.0f, -2.8f*adjust, -0.7f, -7, 0, rad90, 0);

		// cube level text
		_cubeText.init("01", JUSTIFY_CENTER);
		_cubeText.transform(sizeValue, 1.0f, -2.8f*adjust, -2.0f, -7, 0, rad90, 0);
	}
	else
	{
		// remaining time header
		_timeHeader.init(MigUtil::getString("time", "TIME"), JUSTIFY_CENTER);
		_timeHeader.transform(sizeHeader, 1.0f, -2.8f*adjust, -0.5f, -7, 0, rad90, 0);

		// remaining time text
		_timeText.init("0:00", JUSTIFY_CENTER);
		_timeText.transform(sizeValue, 1.0f, -2.8f*adjust, -1.8f, -7, 0, rad90, 0);
	}

	if (!_isLegacy)
	{
		// bump count header
		_bumpHeader.init(MigUtil::getString("bumps", "BUMPS"), JUSTIFY_CENTER);
		_bumpHeader.transform(sizeHeader, 1.0f, 2.0f*adjust, 2, -7, 0, 0, 0);

		// bump count text
		_bumpText.init("00", JUSTIFY_CENTER);
		_bumpText.transform(sizeValue, 1.0f, 2.0f*adjust, 0.8f, -7, 0, 0, 0);

		// slip count header
		_slipHeader.init(MigUtil::getString("slips", "SLIPS"), JUSTIFY_CENTER);
		_slipHeader.transform(sizeHeader, 1.0f, 2.8f*adjust, -0.7f, -7, 0, 0, 0);

		// slip count text
		_slipText.init("0/0", JUSTIFY_CENTER);
		_slipText.transform(sizeValue, 1.0f, 2.8f*adjust, -2.0f, -7, 0, 0, 0);
	}
	else
	{
		// slip count header
		_slipHeader.init(MigUtil::getString("slips", "SLIPS"), JUSTIFY_CENTER);
		_slipHeader.transform(sizeHeader, 1.0f, 2.8f*adjust, -0.5f, -7, 0, 0, 0);

		// slip count text
		_slipText.init("0/0", JUSTIFY_CENTER);
		_slipText.transform(sizeValue, 1.0f, 2.8f*adjust, -1.8f, -7, 0, 0, 0);
	}

	updateSlipText();
	updateBumpText();
}

// initialize the score keeper object (must be called after the script is loaded)
void ScoreKeeper::init()
{
	// score init
	_currScore = 0;
	_stampHitCount = 0;

	// init all of the display text
	initText();

	// we want to know when new levels are loaded
	GameScripts::addCallback(this);

	LOGINFO("Scoring for this script is %s", (GameScripts::getCurrScript().scoring ? "on" : "off"));
}

// used to load an external score configuration, outside of a game script
void ScoreKeeper::loadScoreConfig(const ScoreConfig& cfg)
{
	_scoreCfg = cfg;
	_isScored = _scoreCfg.isValid();
}

// AnimTarget override - callback for animation processing
bool ScoreKeeper::doFrame(int id, float newVal, void* optData)
{
	if (_idScoreAnim == id)
	{
		_currScore = (int)newVal;
		_currScoreText.update(MigUtil::intToPaddedString(_currScore, SCORE_DIGITS));
	}

	return true;
}

// AnimTarget override - indicates that the given animation is complete
void ScoreKeeper::animComplete(int id, void* optData)
{
	if (_idScoreAnim == id)
		_idScoreAnim = 0;
}

// GameScriptUpdate override - a new level is starting
void ScoreKeeper::newLevel(int level)
{
	// cache the score config reference
	_scoreCfg = GameScripts::getCurrLevel().scoreCfg;
	_isScored = _scoreCfg.isValid();
	_levelMissCount = 0;
	_stampMissCount = 0;

	// reset the level start time (and game time, if not yet done)
	_levelStartTime = _countdown.getElapsedTimeMillis();
	if (_gameStartTime == 0)
		_gameStartTime = _levelStartTime;

	// update the current time limits
	_gameTimeLimit = 1000 * GameScripts::getCurrScript().timeLimit;
	_levelTimeLimit = 1000 * GameScripts::getCurrLevel().timeLimit;

	// update the cube level display
	if (!_isLegacy)
		_cubeText.update(MigUtil::intToPaddedString(level + 1, 2), JUSTIFY_CENTER);

	// update the slip display
	updateSlipText();

	// pause the game timer (game screen will resume it when play starts)
	pauseGameTimer();
}

void ScoreKeeper::updateSlipText()
{
	int missTotal = GameScripts::getCurrScript().missCount;
	int missCount = _gameMissCount;
	if (missTotal == 0)
	{
		const Level& lvl = GameScripts::getCurrLevel();
		missTotal = lvl.missCount;
		missCount = _levelMissCount;
	}

	if (missTotal > 0)
	{
		std::string missTotalStr = MigUtil::intToString(missCount);
		missTotalStr += "/";
		missTotalStr += MigUtil::intToString(missTotal);
		_slipText.update(missTotalStr, JUSTIFY_CENTER);
	}
	else
		_slipText.update("-/-", JUSTIFY_CENTER);
}

void ScoreKeeper::updateBumpText()
{
	if (!_isLegacy)
	{
		const Level& lvl = GameScripts::getCurrLevel();
		if (lvl.stampCount > 0)
			_bumpText.update(MigUtil::intToPaddedString(_stampHitCount, 2), JUSTIFY_CENTER);
		else
			_bumpText.update("- -", JUSTIFY_CENTER);
	}
}

static const char* buildTimeString(int time)
{
	static std::string buf;
	buf = MigUtil::intToString(time / 60);
	buf += ':';
	buf += MigUtil::intToPaddedString(time % 60, 2);
	return buf.c_str();
}

void ScoreKeeper::update()
{
	// remove obsolete pop-ups
	if (_isScored && !_popupList.empty())
	{
		std::list<PopupBonusText*>::iterator iter = _popupList.begin();
		while (iter != _popupList.end())
		{
			PopupBonusText* popup = (*iter);
			iter++;

			if (popup->param >= 1)
				_popupList.remove(popup);
		}
	}

	// -1 means no time limit
	int remain = getRemainingTime();
	if (remain != -1)
	{
		// convert to seconds (only update when this changes)
		static int lastRemainSeconds = -1;
		int remainSeconds = remain / 1000;
		if (remainSeconds != lastRemainSeconds)
		{
			lastRemainSeconds = remainSeconds;
			_timeText.update(buildTimeString(remainSeconds));
		}
	}
}

// draws the current score
void ScoreKeeper::draw3D() const
{
	// 0 means no time limit
	if (_gameTimeLimit > 0 || _levelTimeLimit > 0)
	{
		// draw the time limit text
		_timeHeader.draw(_scoreCfg.headerColor);
		_timeText.draw(_scoreCfg.valueColor);
	}

	// draw the cube level
	if (!_isLegacy)
	{
		_cubeHeader.draw(_scoreCfg.headerColor);
		_cubeText.draw(_scoreCfg.valueColor);
	}

	// draw the slip count
	_slipHeader.draw(_scoreCfg.headerColor);
	_slipText.draw(_scoreCfg.valueColor);

	// draw the bump count
	if (!_isLegacy)
	{
		_bumpHeader.draw(_scoreCfg.headerColor);
		_bumpText.draw(_scoreCfg.valueColor);
	}

	if (GameScripts::getCurrScript().scoring)
	{
		// draw the main score
		Color scoreColor = _scoreCfg.valueColor;
		if (CubeUtil::currPowerUp == POWERUPTYPE_MULTIPLIER)
		{
			float blend = Timer::gameTimeMillis() / 1000.0f;
			blend = 1 - (blend - (int)blend);
			scoreColor = MigUtil::blendColors(colBlack, colWhite, blend);
		}
		_currScoreText.draw(scoreColor);
	}
}

// draws any overlay text strings (not 3D text)
void ScoreKeeper::drawOverlay() const
{
	if (_isScored && !_popupList.empty())
	{
		// draw the pop-up scores
		std::list<PopupBonusText*>::const_iterator iter = _popupList.begin();
		while (iter != _popupList.end())
		{
			(*iter)->draw();
			iter++;
		}
	}
}

// animates the score display to catch up w/ the real score
void ScoreKeeper::startScoreAnim()
{
	// cancel any existing animation
	_idScoreAnim.clearAnim();

	// this is a simple sin curve
	float fy[] = { 0.0f, 0.17f, 0.34f, 0.5f, 0.64f, 0.77f, 0.87f, 0.94f, 0.98f, 1.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim((float)_currScore, (float)_realScore, SCORE_ANIM_DURATION, fy, ARRAYSIZE(fy));
	_idScoreAnim = MigUtil::theAnimList->addItem(animItem);
}

// creates a pop-up animation
void ScoreKeeper::startPopupAnim(bool isGood, const char* text, float u, float v, float scalar)
{
	if (CubeUtil::showPopupScores)
	{
		std::string finalText = (isGood ? "+" : "-");
		finalText += text;
		LOGINFO("(ScoreKeeper::startPopupAnim) Creating score pop-up '%s'", finalText.c_str());

		PopupBonusText* popup = new PopupBonusText();
		popup->init(isGood, finalText, u, v, scalar, _scoreCfg);
		_popupList.push_back(popup);
	}
}

static int orientEnumToIndex(AxisOrient orient)
{
	if (orient == AXISORIENT_X)
		return 2;
	else if (orient == AXISORIENT_Y)
		return 3;
	else if (orient == AXISORIENT_Z)
		return 1;
	return 0;
}

// creates a pop-up animation
void ScoreKeeper::startPopupAnim(bool isGood, bool isStamp, const char* text, AxisOrient orient, float scalar)
{
	int index = orientEnumToIndex(orient);
	if (isStamp)
		startPopupAnim(isGood, text, fStampScoreU[index], fStampScoreV[index], scalar);
	else
		startPopupAnim(isGood, text, fSideScoreU[index], fSideScoreV[index], scalar);
}

// alters a relative score by the current power up
int ScoreKeeper::applyPowerUp(int score) const
{
	return score*CubeUtil::currPowerUp.getScoreMultiplier();
}

// adds a value to the score and animates it
void ScoreKeeper::addToScore(int value)
{
	if (value != 0)
	{
		_realScore += value;
		if (_realScore < 0)
			_realScore = 0;
		startScoreAnim();
		LOGINFO("(ScoreKeeper::addToScore) Score is now %d", _realScore);
	}
}

// a falling piece matched
void ScoreKeeper::pieceMatch(AxisOrient orient, float multiplier)
{
	if (_isScored)
	{
		int score = (int)(_scoreCfg.pieceMatch * multiplier);
		addToScore(score);
		//CubeUtil.info("ScoreKeeper.pieceMatch: score is now " + mRealScore);
	}
}

// a cube side has completed
void ScoreKeeper::sideComplete(AxisOrient orient, float multiplier)
{
	if (_isScored)
	{
		int score = applyPowerUp((int)(_scoreCfg.sideComplete * multiplier));
		addToScore(score);
		startPopupAnim(true, false, MigUtil::intToString(score), orient, 1);
		//CubeUtil.info("sideComplete: score is now " + mRealScore);
	}
}

// a successful stamp
void ScoreKeeper::stampMatch(AxisOrient orient)
{
	float stampChainMultiplier = 1;

	// update stamp match
	_stampHitCount++;
	updateBumpText();

	// we need to update or reset the stamp chain multiplier
	if (_isScored)
	{
		stampChainMultiplier = _scoreCfg.stampChainMultiplier;

		// is there a stamp chain time even set?
		if (_scoreCfg.stampChainTime > 0)
		{
			// is this stamp action w/in the chain time?  if so, update the multiplier, else reset it
			long stampMatchTime = Timer::gameTimeMillis();
			if (stampMatchTime < _lastStampMatch + _scoreCfg.stampChainTime)
				_stampChainCount++;
			else
				_stampChainCount = 1;

			_lastStampMatch = stampMatchTime;
		}
		else
			_stampChainCount = 1;

		int stampScore = applyPowerUp((int)(_scoreCfg.stampMatch * pow(stampChainMultiplier, _stampChainCount - 1)));
		addToScore(stampScore);

		float stampScalar = (float)pow(1.1, _stampChainCount - 1);
		startPopupAnim(true, true, MigUtil::intToString(stampScore), orient, stampScalar);
		//CubeUtil.info("stampMatch: score is now " + mRealScore);
	}
}

// a falling piece that matched was tapped
void ScoreKeeper::tapComplete(AxisOrient orient, float scale, float multiplier)
{
	if (_isScored)
	{
		_realScore += (int)(_scoreCfg.tapMax * scale * multiplier);
		// note that we don't do an animation because this should always be followed by either pieceMatch() or sideComplete()
		//CubeUtil.info("tapComplete: scale is " + scale + ", score is now " + mRealScore);
	}
}

// a falling piece missed
void ScoreKeeper::pieceMiss(AxisOrient orient)
{
	if (_isScored)
	{
		addToScore(-_scoreCfg.pieceMiss);
		startPopupAnim(false, false, MigUtil::intToString(_scoreCfg.pieceMiss), orient, 1);
		//CubeUtil.info("ScoreKeeper.pieceMiss: score is now " + mRealScore);
	}

	_gameMissCount++;
	_levelMissCount++;

	// update the slip display
	updateSlipText();
}

// a bad stamp
void ScoreKeeper::stampMiss(AxisOrient orient)
{
	if (_isScored)
	{
		addToScore(-_scoreCfg.stampMiss);
		startPopupAnim(false, true, MigUtil::intToString(_scoreCfg.stampMiss), orient, 1);
		//CubeUtil.info("stampMiss: score is now " + mRealScore);
	}

	_gameMissCount++;
	_levelMissCount++;
	_stampMissCount++;

	// update the slip display
	updateSlipText();
}

// a stamp expired
void ScoreKeeper::stampExpired(AxisOrient orient)
{
	//CubeUtil.info("stampExpired: score is now " + mRealScore);
	_stampMissCount++;
}

// returns true if the game is over due to misses
bool ScoreKeeper::checkMissesForGameOver() const
{
	bool gameOver = false;

	int missCount = GameScripts::getCurrScript().missCount;
	if (missCount > 0)
	{
		// determine if the game is over due to a script miss cap
		if (_gameMissCount >= missCount)
		{
			LOGINFO("Miss count has exceeded the game miss limit (%d), game is over", missCount);
			gameOver = true;
		}
	}
	else
	{
		// determine if the game is over due to a level miss cap
		const Level& lvl = GameScripts::getCurrLevel();
		if (lvl.missCount > 0 && _levelMissCount >= lvl.missCount)
		{
			LOGINFO("Miss count has exceeded the level miss limit (%d), game is over", lvl.missCount);
			gameOver = true;
		}
	}
	return gameOver;
}

// returns true if the game is over due to running out of time
bool ScoreKeeper::checkTimerForGameOver() const
{
	bool gameOver = false;

	if (_gameTimeLimit > 0)
	{
		// a game time limit is being enforced, is the game time over?
		long elapsed = _countdown.getElapsedTimeMillis() - _gameStartTime;
		if (elapsed > _gameTimeLimit)
		{
			LOGINFO("Game time has exceeded the game time limit (%d seconds), game is over", _gameTimeLimit);
			gameOver = true;
		}
	}
	else
	{
		if (_levelTimeLimit > 0)
		{
			// a level time limit is being enforced, is the level time over?
			long elapsed = _countdown.getElapsedTimeMillis() - _levelStartTime;
			if (elapsed > _levelTimeLimit)
			{
				LOGINFO("Game time has exceeded the level time limit (%d seconds), game is over", _levelTimeLimit);
				gameOver = true;
			}
		}
	}
	return gameOver;
}

// retrieves the elapsed time (which depends on script or level timed limits), 0 if no limit
long ScoreKeeper::getElapsedTime() const
{
	long elapsed = 0;
	int timeLimit = GameScripts::getCurrScript().timeLimit;
	if (timeLimit > 0 || GameScripts::getCurrLevel().timeLimit > 0)
	{
		long startTime = (timeLimit > 0 ? _gameStartTime : _levelStartTime);
		elapsed = _countdown.getElapsedTimeMillis() - startTime;
	}
	return elapsed;
}

// retrieves the remaining time (which depends on script or level timed limits), -1 if no limit
int ScoreKeeper::getRemainingTime() const
{
	long timeLimit = (_gameTimeLimit > 0 ? _gameTimeLimit : _levelTimeLimit);
	if (timeLimit == 0)
		return -1;

	long elapsed = getElapsedTime();
	int remain = (int)(timeLimit > elapsed ? timeLimit - elapsed : 0);
	return remain;
}
