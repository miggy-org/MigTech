#include "pch.h"
#include "GameScripts.h"
#include "DemoCuboingoScreens.h"

using namespace MigTech;
using namespace Cuboingo;
using namespace tinyxml2;

///////////////////////////////////////////////////////////////////////////
// XML tags

const char* TAG_SCRIPTS = "Scripts";
const char* TAG_SCRIPT = "Script";
const char* TAG_TIMER_CFG = "TimerCfg";
const char* TAG_BACKGROUND = "Background";
const char* TAG_OVERLAY = "Overlay";
const char* TAG_VISUAL = "Visual";
const char* TAG_CUBE = "Cube";
const char* TAG_LEVEL = "Level";
const char* TAG_COLOR = "Color";
const char* TAG_SCORE = "Score";

///////////////////////////////////////////////////////////////////////////
// XML attribute names

// script attributes
const char* ATTR_NAME = "Name";
const char* ATTR_UNIQUE_ID = "UniqueID";
const char* ATTR_AUTHOR = "Author";
const char* ATTR_DIFFICULTY = "Difficulty";
const char* ATTR_SCORED = "Scored";
const char* ATTR_DESC1 = "Desc1";
const char* ATTR_DESC2 = "Desc2";
const char* ATTR_DESC3 = "Desc3";
const char* ATTR_DESC4 = "Desc4";
const char* ATTR_IMAGE_ID = "Image";
const char* ATTR_MUSIC_ID = "Music";
const char* ATTR_DEMO_ID = "Demo";
const char* ATTR_START_LEVEL = "StartLevel";
const char* ATTR_TIP_PREFIX = "Tip";

// visual attributes
const char* ATTR_CUBE_TEXTURE = "Texture";
const char* ATTR_CUBE_REFLECT = "Reflect";
const char* ATTR_CUBE_COLOR = "Color";
const char* ATTR_CUBE_INTENSITY = "Intensity";

// per level attributes
const char* ATTR_ROTATE_TIME = "RotateTime";
const char* ATTR_FALL_TIME = "FallTime";
const char* ATTR_GLOW_TIME = "GlowTime";
const char* ATTR_IDLE_TIME = "IdleTime";
const char* ATTR_RESET_TIME = "ResetTime";
const char* ATTR_NUM_COLORS = "NumColors";
const char* ATTR_DIRECTIONS = "Directions";
const char* ATTR_GLOW_COUNT = "GlowCount";
const char* ATTR_GRID_LEVEL = "GridLevel";
const char* ATTR_GRID_FACE_LOCK = "GridFaceLock";
const char* ATTR_FALLING_PIECES = "FallingPieces";
const char* ATTR_HOLE_LIMIT = "FallingHoleLimit";
const char* ATTR_RAND_FALLING = "RandomizeFallingPieces";
const char* ATTR_LAUNCH_STYLE = "LaunchAxisStyle";
const char* ATTR_MISS_COUNT = "MissCount";
const char* ATTR_TIME_LIMIT = "TimeLimit";

// per level attributes for special falling pieces
const char* ATTR_EVIL_PROB = "EvilProb";
const char* ATTR_EVIL_FALL_TIME = "EvilFallTime";
const char* ATTR_EVIL_PERIOD = "EvilPeriod";
const char* ATTR_EVIL_STYLE = "EvilStyle";
const char* ATTR_EVIL_AXIS_LOCK = "EvilAxisLock";
const char* ATTR_EVIL_MAX_COLOR = "EvilMaxColor";
const char* ATTR_WILD_PROB = "WildProb";
const char* ATTR_WILD_FALL_TIME = "WildFallTime";
const char* ATTR_WILD_PERIOD = "WildPeriod";
const char* ATTR_WILD_STYLE = "WildStyle";
const char* ATTR_WILD_AXIS_LOCK = "WildAxisLock";

// per level burst attributes
const char* ATTR_BURST_PROB = "BurstProb";
const char* ATTR_BURST_FALL_TIME = "BurstFallTime";
const char* ATTR_EVIL_BURST_PROB = "EvilBurstProb";
const char* ATTR_EVIL_BURST_FALL_TIME = "EvilBurstFallTime";
const char* ATTR_EVIL_BURST_PERIOD = "EvilBurstPeriod";

// per level stamp attributes
const char* ATTR_STAMP_COUNT = "StampCount";
const char* ATTR_STAMP_PERIOD = "StampPeriod";
const char* ATTR_STAMP_PROB = "StampProb";
const char* ATTR_STAMP_DURATION = "StampDuration";
const char* ATTR_STAMP_CYCLE = "StampCycle";
const char* ATTR_STAMP_STYLE = "StampStyle";

// per level power up attributes
const char* ATTR_POWERUP_MULTIPLIER = "PowerUpMultiplier";
const char* ATTR_POWERUP_MULTIPLIER_DURATION = "PowerUpMultiplierDuration";
const char* ATTR_POWERUP_INVULNERABILITY_DURATION = "PowerUpInvulnerabilityDuration";
const char* ATTR_POWERUP_INVULNERABILITY_MAXIMUM = "PowerUpInvulnerabilityMaximum";

// per level scoring attributes
const char* ATTR_SCORE_PIECE_MATCH = "PieceMatch";
const char* ATTR_SCORE_SIDE_COMPLETE = "SideComplete";
const char* ATTR_SCORE_STAMP_MATCH = "StampMatch";
const char* ATTR_SCORE_TAP_MAX = "TapMax";
const char* ATTR_SCORE_PIECE_MISS = "PieceMiss";
const char* ATTR_SCORE_STAMP_MISS = "StampMiss";
const char* ATTR_SCORE_EVIL_MULTIPLIER = "EvilMultiplier";
const char* ATTR_SCORE_STAMP_CHAIN_MULTIPLIER = "StampChainMultiplier";
const char* ATTR_SCORE_STAMP_CHAIN_TIME = "StampChainTime";
const char* ATTR_SCORE_BONUS_TIME = "TimedBonusPerSecond";
const char* ATTR_SCORE_BONUS_NO_MISS = "NoMissBonus";
const char* ATTR_SCORE_BONUS_ALL_STAMPS = "AllStampsHitBonus";
const char* ATTR_SCORE_GOOD_COLOR = "GoodColor";
const char* ATTR_SCORE_BAD_COLOR = "BadColor";
const char* ATTR_SCORE_HEADER_COLOR = "HeaderColor";
const char* ATTR_SCORE_VALUE_COLOR = "ValueColor";

///////////////////////////////////////////////////////////////////////////
// score configuratino

ScoreConfig::ScoreConfig()
{
	pieceMatch = 0;
	sideComplete = 0;
	stampMatch = 0;
	tapMax = 0;

	pieceMiss = 0;
	stampMiss = 0;

	timedBonusPerSecond = 0;
	noMissBonus = 0;
	allStampsHitBonus = 0;

	evilMultiplier = 1;
	stampChainMultiplier = 1;
	stampChainTime = 0;

	headerColor = Color(0.35f, 0.35f, 0.35f);
	valueColor = Color(0.37f, 0.60f, 0.78f);
	goodColor = Color(0, 1, 0);
	badColor = Color(1, 0, 0);
}

ScoreConfig::ScoreConfig(const ScoreConfig& rhs)
{
	copyScoreConfig(rhs);
}

void ScoreConfig::copyScoreConfig(const ScoreConfig& toCopy)
{
	pieceMatch = toCopy.pieceMatch;
	sideComplete = toCopy.sideComplete;
	stampMatch = toCopy.stampMatch;
	tapMax = toCopy.tapMax;

	pieceMiss = toCopy.pieceMiss;
	stampMiss = toCopy.stampMiss;

	timedBonusPerSecond = toCopy.timedBonusPerSecond;
	noMissBonus = toCopy.noMissBonus;
	allStampsHitBonus = toCopy.allStampsHitBonus;

	evilMultiplier = toCopy.evilMultiplier;
	stampChainMultiplier = toCopy.stampChainMultiplier;
	stampChainTime = toCopy.stampChainTime;

	headerColor = toCopy.headerColor;
	valueColor = toCopy.valueColor;
	goodColor = toCopy.goodColor;
	badColor = toCopy.badColor;
}

bool ScoreConfig::isBonusSummaryRequired() const
{
	return (timedBonusPerSecond > 0 || noMissBonus > 0 || allStampsHitBonus > 0);
}

bool ScoreConfig::isValid() const
{
	return (pieceMatch > 0 || sideComplete > 0 || stampMatch > 0 || tapMax > 0 || timedBonusPerSecond > 0 || noMissBonus > 0 || allStampsHitBonus > 0);
}

///////////////////////////////////////////////////////////////////////////
// level configuration

Level::Level()
{
	rotTime = 0;
	fallTime = 0;
	glowTime = 0;
	idleTime = 0;
	resetTime = 0;

	numColors = 0;
	gridLevel = 0;
	directions = 0;
	glowCount = 0;
	fallingPieces = 0;
	fallingHoleLimit = 0;
	randFallingPieces = true;
	missCount = 0;
	timeLimit = 0;
	gridLock = GRIDLOCK_NONE;
	launchAxisStyle = LAUNCHSTYLE_NORMAL;

	evilProbability = 0;
	evilFallTime = 0;
	evilPeriod = 0;
	evilMaxColors = 0;
	evilStyle = EVILGRIDSTYLE_COLOR_ONLY;
	evilAxisLock = AXISLOCK_NONE;

	wildProbability = 0;
	wildFallTime = 0;
	wildPeriod = 0;
	wildStyle = WILDGRIDSTYLE_NORMAL;
	wildAxisLock = AXISLOCK_NONE;

	burstProbability = 0;
	burstFallTime = 0;
	evilBurstProbability = 0;
	evilBurstFallTime = 0;
	evilBurstPeriod = 0;

	stampCount = 0;
	stampPeriod = 0;
	stampProbability = 0;
	stampDuration = 0;
	stampCycle = 0;
	stampStyle = STAMPSTYLE_BONUS_RANDOM;

	powerUpMult = 3;
	powerUpMultDur = 10000;
	powerUpInvDur = 0;
	powerUpInvMax = 0;
}

Level::Level(const Level& rhs)
{
	copyLevel(rhs);
}

void Level::copyLevel(const Level& toCopy)
{
	rotTime = toCopy.rotTime;
	fallTime = toCopy.fallTime;
	glowTime = toCopy.glowTime;
	idleTime = toCopy.idleTime;
	resetTime = toCopy.resetTime;

	numColors = toCopy.numColors;
	gridLevel = toCopy.gridLevel;
	directions = toCopy.directions;
	glowCount = toCopy.glowCount;
	fallingPieces = toCopy.fallingPieces;
	fallingHoleLimit = toCopy.fallingHoleLimit;
	randFallingPieces = toCopy.randFallingPieces;
	missCount = toCopy.missCount;
	timeLimit = toCopy.timeLimit;
	gridLock = toCopy.gridLock;
	launchAxisStyle = toCopy.launchAxisStyle;

	evilProbability = toCopy.evilProbability;
	evilFallTime = toCopy.evilFallTime;
	evilPeriod = toCopy.evilPeriod;
	evilMaxColors = toCopy.evilMaxColors;
	evilStyle = toCopy.evilStyle;
	evilAxisLock = toCopy.evilAxisLock;

	wildProbability = toCopy.wildProbability;
	wildFallTime = toCopy.wildFallTime;
	wildPeriod = toCopy.wildPeriod;
	wildStyle = toCopy.wildStyle;
	wildAxisLock = toCopy.wildAxisLock;

	burstProbability = toCopy.burstProbability;
	burstFallTime = toCopy.burstFallTime;
	evilBurstProbability = toCopy.evilBurstProbability;
	evilBurstFallTime = toCopy.evilBurstFallTime;
	evilBurstPeriod = toCopy.evilBurstPeriod;

	stampCount = toCopy.stampCount;
	stampPeriod = toCopy.stampPeriod;
	stampProbability = toCopy.stampProbability;
	stampDuration = toCopy.stampDuration;
	stampCycle = toCopy.stampCycle;
	stampStyle = toCopy.stampStyle;

	powerUpMult = toCopy.powerUpMult;
	powerUpMultDur = toCopy.powerUpMultDur;
	powerUpInvDur = toCopy.powerUpInvDur;
	powerUpInvMax = toCopy.powerUpInvMax;

	for (int i = 0; i < 6; i++)
	{
		dimColors[i] = toCopy.dimColors[i];
		maxColors[i] = toCopy.maxColors[i];
	}

	scoreCfg.copyScoreConfig(toCopy.scoreCfg);
}

///////////////////////////////////////////////////////////////////////////
// single script configuration

Script::Script()
{
	clear();
}

void Script::clear()
{
	levels.clear();
	name = "";
	author = "";
	desc1 = desc2 = desc3 = desc4 = "";
	difficulty = "";
	missCount = 0;
	timeLimit = 0;
	imageID = "";
	musicID = "";
	demoID = "";
	scoring = false;
	tips.clear();
	textureFile = "";
	reflectFile = "";
	colCube = colWhite;
	reflectIntensity = 0.5f;

	// note that we don't delete these, GameScreen is responsible for that
	bgHandler = nullptr;
	overlayHandler = nullptr;
}

bool Script::isLoaded() const
{
	return (levels.size() > 0);
}

int Script::getMaxLevelNumber() const
{
	return levels.size() - 1;
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

static const char* getAttributeSafe(XMLElement* pe, const char* attr)
{
	const char* p = pe->Attribute(attr);
	return (p != nullptr ? p : "");
}

bool Script::load(const std::string& scrID, bool headerOnly)
{
	LOGINFO("(Script::load) Loading script %s, header only is %s", scrID.c_str(), (headerOnly ? "true" : "false"));

	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument(scrID);
	if (pdoc == nullptr)
	{
		LOGWARN("(Script::load) Unable to load '%s'", scrID.c_str());
		return false;
	}

	XMLElement* pscript = pdoc->FirstChildElement(TAG_SCRIPT);
	if (pscript != nullptr)
	{
		//LOGINFO("Font tag found");
		name = getAttributeSafe(pscript, ATTR_NAME);
		uniqueID = getAttributeSafe(pscript, ATTR_UNIQUE_ID);
		author = getAttributeSafe(pscript, ATTR_AUTHOR);
		desc1 = getAttributeSafe(pscript, ATTR_DESC1);
		desc2 = getAttributeSafe(pscript, ATTR_DESC2);
		desc3 = getAttributeSafe(pscript, ATTR_DESC3);
		desc4 = getAttributeSafe(pscript, ATTR_DESC4);
		difficulty = getAttributeSafe(pscript, ATTR_DIFFICULTY);
		imageID = getAttributeSafe(pscript, ATTR_IMAGE_ID);
		musicID = getAttributeSafe(pscript, ATTR_MUSIC_ID);
		demoID = getAttributeSafe(pscript, ATTR_DEMO_ID);
		startLevel = MigUtil::parseInt(getAttributeSafe(pscript, ATTR_START_LEVEL), 1);
		missCount = MigUtil::parseInt(getAttributeSafe(pscript, ATTR_MISS_COUNT), 0);
		timeLimit = MigUtil::parseInt(getAttributeSafe(pscript, ATTR_TIME_LIMIT), 0);
		scoring = MigUtil::parseBool(getAttributeSafe(pscript, ATTR_SCORED), false);

		// load the tips block
		char buf[40];
		for (int i = 1; true; i++)
		{
			sprintf(buf, "%s%d", ATTR_TIP_PREFIX, i);
			std::string tip = getAttributeSafe(pscript, buf);
			if (tip.length() > 0)
				tips.push_back(tip);
			else
				break;
		}

		if (!headerOnly)
		{
			XMLElement* pvisual = pscript->FirstChildElement(TAG_VISUAL);
			if (pvisual != nullptr)
			{
				XMLElement* pcube = pvisual->FirstChildElement(TAG_CUBE);
				if (pcube != nullptr)
				{
					textureFile = getAttributeSafe(pcube, ATTR_CUBE_TEXTURE);
					reflectFile = getAttributeSafe(pcube, ATTR_CUBE_REFLECT);
					colCube = MigUtil::parseColorString(getAttributeSafe(pcube, ATTR_CUBE_COLOR), colCube);
					reflectIntensity = MigUtil::parseFloat(getAttributeSafe(pcube, ATTR_CUBE_INTENSITY), reflectIntensity);
				}

				XMLElement* pbg = pvisual->FirstChildElement(TAG_BACKGROUND);
				if (pbg != nullptr)
					bgHandler = BgBase::loadHandlerFromXML(pbg);

				XMLElement* poverlay = pvisual->FirstChildElement(TAG_OVERLAY);
				if (poverlay != nullptr)
					overlayHandler = BgBase::loadHandlerFromXML(poverlay);
			}

			if (bgHandler == nullptr)
				bgHandler = new BgBase(!imageID.empty() ? imageID : "ingame.jpg");

			XMLElement* plevel = pscript->FirstChildElement(TAG_LEVEL);
			while (plevel != nullptr)
			{
				int newLevelIndex = levels.size();

				// this should create a new empty level and return a reference to it
				levels.resize(newLevelIndex + 1);
				Level& lvl = levels[newLevelIndex];

				// if there's a previous level, copy it's attributes first
				int prevLevelIndex = newLevelIndex - 1;
				if (prevLevelIndex >= 0)
					lvl.copyLevel(levels[prevLevelIndex]);

				lvl.rotTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_ROTATE_TIME), lvl.rotTime);
				lvl.fallTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_FALL_TIME), lvl.fallTime);
				lvl.glowTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_GLOW_TIME), lvl.glowTime);
				lvl.idleTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_IDLE_TIME), lvl.idleTime);
				lvl.resetTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_RESET_TIME), lvl.resetTime);
				lvl.numColors = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_NUM_COLORS), lvl.numColors);
				lvl.directions = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_DIRECTIONS), lvl.directions);
				lvl.glowCount = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_GLOW_COUNT), lvl.glowCount);
				lvl.gridLevel = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_GRID_LEVEL), lvl.gridLevel);
				lvl.gridLock = (GridLock)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_GRID_FACE_LOCK), (int)lvl.gridLock);
				lvl.fallingPieces = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_FALLING_PIECES), lvl.fallingPieces);
				lvl.fallingHoleLimit = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_HOLE_LIMIT), lvl.fallingHoleLimit);
				lvl.randFallingPieces = MigUtil::parseBool(getAttributeSafe(plevel, ATTR_RAND_FALLING), lvl.randFallingPieces);
				lvl.launchAxisStyle = (LaunchStyle)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_LAUNCH_STYLE), (int)lvl.launchAxisStyle);
				lvl.missCount = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_MISS_COUNT), lvl.missCount);
				lvl.timeLimit = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_TIME_LIMIT), lvl.timeLimit);
				lvl.evilProbability = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_PROB), lvl.evilProbability);
				lvl.evilFallTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_FALL_TIME), lvl.evilFallTime);
				lvl.evilPeriod = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_PERIOD), lvl.evilPeriod);
				lvl.evilMaxColors = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_MAX_COLOR), lvl.evilMaxColors);
				lvl.evilStyle = (EvilGridStyle)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_STYLE), (int)lvl.evilStyle);
				lvl.evilAxisLock = (AxisLock)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_AXIS_LOCK), (int)lvl.evilAxisLock);
				lvl.wildProbability = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_WILD_PROB), lvl.wildProbability);
				lvl.wildFallTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_WILD_FALL_TIME), lvl.wildFallTime);
				lvl.wildPeriod = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_WILD_PERIOD), lvl.wildPeriod);
				lvl.wildStyle = (WildGridStyle)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_WILD_STYLE), (int)lvl.wildStyle);
				lvl.wildAxisLock = (AxisLock)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_WILD_AXIS_LOCK), (int)lvl.wildAxisLock);
				lvl.burstProbability = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_BURST_PROB), lvl.burstProbability);
				lvl.burstFallTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_BURST_FALL_TIME), lvl.burstFallTime);
				lvl.evilBurstProbability = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_BURST_PROB), lvl.evilBurstProbability);
				lvl.evilBurstFallTime = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_BURST_FALL_TIME), lvl.evilBurstFallTime);
				lvl.evilBurstPeriod = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_EVIL_BURST_PERIOD), lvl.evilBurstPeriod);
				lvl.stampCount = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_COUNT), lvl.stampCount);
				lvl.stampPeriod = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_PERIOD), lvl.stampPeriod);
				lvl.stampProbability = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_PROB), lvl.stampProbability);
				lvl.stampDuration = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_DURATION), lvl.stampDuration);
				lvl.stampCycle = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_CYCLE), lvl.stampCycle);
				lvl.stampStyle = (StampStyle)MigUtil::parseInt(getAttributeSafe(plevel, ATTR_STAMP_STYLE), (int)lvl.stampStyle);
				lvl.powerUpMult = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_POWERUP_MULTIPLIER), lvl.powerUpMult);
				lvl.powerUpMultDur = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_POWERUP_MULTIPLIER_DURATION), lvl.powerUpMultDur);
				lvl.powerUpInvDur = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_POWERUP_INVULNERABILITY_DURATION), lvl.powerUpInvDur);
				lvl.powerUpInvMax = MigUtil::parseInt(getAttributeSafe(plevel, ATTR_POWERUP_INVULNERABILITY_MAXIMUM), lvl.powerUpInvMax);

				int nextColor = 0;
				XMLElement* pcolor = plevel->FirstChildElement(TAG_COLOR);
				while (pcolor != nullptr)
				{
					float vals[6];
					int found = MigUtil::parseFloats(pcolor->GetText(), vals, 6);
					if (found == 6)
					{
						lvl.dimColors[nextColor] = Color(vals[0], vals[1], vals[2], 1);
						lvl.maxColors[nextColor] = Color(vals[3], vals[4], vals[5], 1);
					}
					else
						LOGWARN("(Script::load) Error parsing color string, only found %d floats", found);

					pcolor = pcolor->NextSiblingElement(TAG_COLOR);
					nextColor++;
				}

				XMLElement* pscore = plevel->FirstChildElement(TAG_SCORE);
				if (pscore != nullptr)
				{
					ScoreConfig& score = lvl.scoreCfg;
					score.pieceMatch = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_PIECE_MATCH), score.pieceMatch);
					score.sideComplete = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_SIDE_COMPLETE), score.sideComplete);
					score.stampMatch = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_STAMP_MATCH), score.stampMatch);
					score.tapMax = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_TAP_MAX), score.tapMax);
					score.pieceMiss = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_PIECE_MISS), score.pieceMiss);
					score.stampMiss = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_STAMP_MISS), score.stampMiss);
					score.evilMultiplier = MigUtil::parseFloat(getAttributeSafe(pscore, ATTR_SCORE_EVIL_MULTIPLIER), score.evilMultiplier);
					score.stampChainMultiplier = MigUtil::parseFloat(getAttributeSafe(pscore, ATTR_SCORE_STAMP_CHAIN_MULTIPLIER), score.stampChainMultiplier);
					score.stampChainTime = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_STAMP_CHAIN_TIME), score.stampChainTime);
					score.timedBonusPerSecond = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_BONUS_TIME), score.timedBonusPerSecond);
					score.noMissBonus = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_BONUS_NO_MISS), score.noMissBonus);
					score.allStampsHitBonus = MigUtil::parseInt(getAttributeSafe(pscore, ATTR_SCORE_BONUS_ALL_STAMPS), score.allStampsHitBonus);

					score.headerColor = MigUtil::parseColorString(getAttributeSafe(pscore, ATTR_SCORE_HEADER_COLOR), score.headerColor);
					score.valueColor = MigUtil::parseColorString(getAttributeSafe(pscore, ATTR_SCORE_VALUE_COLOR), score.valueColor);
					score.goodColor = MigUtil::parseColorString(getAttributeSafe(pscore, ATTR_SCORE_GOOD_COLOR), score.goodColor);
					score.badColor = MigUtil::parseColorString(getAttributeSafe(pscore, ATTR_SCORE_BAD_COLOR), score.badColor);
				}

				plevel = plevel->NextSiblingElement(TAG_LEVEL);
			}
		}
	}

	delete pdoc;

	scriptID = scrID;
	if (levels.size() > 0 && scoring)
		scoring = (levels[0].scoreCfg.isValid());
	return true;
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// scripts interface

// list of script headers
static std::vector<Script> _scripts;

// currently loaded script
static Script _loadedScript;

// current level w/in the loaded script
static int _currLevel;

// list of objects to receive script events
static std::vector<IGameScriptUpdate*> _eventList;

static void invokeLevelChangeCallback()
{
	std::vector<IGameScriptUpdate*>::iterator iter = _eventList.begin();
	while (iter != _eventList.end())
	{
		(*iter)->newLevel(_currLevel);
		iter++;
	}
}

void GameScripts::init(tinyxml2::XMLElement* rootCfg)
{
	if (rootCfg != nullptr)
	{
		XMLElement* pscripts = rootCfg->FirstChildElement(TAG_SCRIPTS);
		if (pscripts != nullptr)
		{
			XMLElement* pscript = pscripts->FirstChildElement(TAG_SCRIPT);
			while (pscript != nullptr)
			{
				std::string name = pscript->GetText();
				if (!name.empty())
					addScriptHeader(name);

				pscript = pscript->NextSiblingElement(TAG_SCRIPT);
			}
		}
	}
	else
	{
		LOGWARN("(GameScripts::init) No script directory found");

		// no scripts directory
		addScriptHeader("script1.xml");
		addScriptHeader("script2.xml");
		addScriptHeader("script3.xml");
		addScriptHeader("script4.xml");
		addScriptHeader("script5.xml");
		addScriptHeader("script6.xml");
	}
}

void GameScripts::clear()
{
	_scripts.clear();
	_eventList.clear();
	_currLevel = 0;
	_loadedScript.clear();
}

bool GameScripts::loadTestScript()
{
	LOGINFO("(GameScripts::loadTestScript) Loading test script");

	_loadedScript.uniqueID = "hardcoded_script";
	_loadedScript.scoring = true;
	_loadedScript.timeLimit = 300;
	_loadedScript.imageID = "space.png";
	_loadedScript.bgHandler = new BgBase(_loadedScript.imageID);
	_loadedScript.overlayHandler = nullptr;
	_currLevel = 0;

	// initial quick level
	Level lvl;
	lvl.rotTime = 500;
	lvl.fallTime = 4000;
	lvl.glowTime = 4000;
	lvl.numColors = 2;
	lvl.directions = 3;
	lvl.gridLevel = 1;
	lvl.glowCount = 1;
	lvl.fallingPieces = 2;
	lvl.missCount = 3;
	//lvl.fallingHoleLimit = 1;
	//lvl.timeLimit = 300;
	//lvl.gridLock = GridLock.NO_WAIT;
	//lvl.launchAxisStyle = LaunchStyle.BY_COLOR;

	//lvl.evilProbability = 25;
	//lvl.evilFallTime = 16000;
	//lvl.evilPeriod = 2000;
	//lvl.evilMaxColors = 0;
	//lvl.evilStyle = EvilGridStyle.RANDOMIZED;
	//lvl.evilAxisLock = AxisLock.ALL;

	//lvl.wildProbability = 25;
	//lvl.wildFallTime = 7000;
	//lvl.wildPeriod = 250;
	//lvl.wildAxisLock = AxisLock.SINGLE;

	//lvl.burstProbability = 0;
	//lvl.burstFallTime = 6000;
	//lvl.evilBurstProbability = 0;
	//lvl.evilBurstFallTime = 10000;
	//lvl.evilBurstPeriod = 1000;

	lvl.stampCount = 2;
	lvl.stampPeriod = 4000;
	lvl.stampProbability = 100;
	lvl.stampDuration = 5000;
	lvl.stampCycle = 1000;
	lvl.stampStyle = STAMPSTYLE_BONUS_RANDOM;

	lvl.dimColors[0] = Color(0.65f, 0.50f, 0.00f);
	lvl.dimColors[1] = Color(0.50f, 0.00f, 0.50f);
	lvl.dimColors[2] = Color(0.50f, 0.00f, 0.00f);
	lvl.dimColors[3] = Color(0.00f, 0.00f, 0.50f);
	lvl.dimColors[4] = Color(0.50f, 0.50f, 0.50f);
	lvl.dimColors[5] = Color(0.00f, 0.50f, 0.25f);
	lvl.maxColors[0] = Color(1.00f, 0.80f, 0.00f);
	lvl.maxColors[1] = Color(1.00f, 0.00f, 1.00f);
	lvl.maxColors[2] = Color(1.00f, 0.00f, 0.00f);
	lvl.maxColors[3] = Color(0.00f, 0.00f, 1.00f);
	lvl.maxColors[4] = Color(1.00f, 1.00f, 1.00f);
	lvl.maxColors[5] = Color(0.00f, 1.00f, 0.50f);

	lvl.scoreCfg.pieceMatch = 100;
	lvl.scoreCfg.sideComplete = 500;
	lvl.scoreCfg.stampMatch = 1000;
	lvl.scoreCfg.tapMax = 100;
	lvl.scoreCfg.pieceMiss = 1000;
	lvl.scoreCfg.stampMiss = 1000;
	lvl.scoreCfg.evilMultiplier = 3;
	lvl.scoreCfg.stampChainMultiplier = 2;
	lvl.scoreCfg.stampChainTime = 2000;
	lvl.scoreCfg.timedBonusPerSecond = 10;
	lvl.scoreCfg.noMissBonus = 2000;
	lvl.scoreCfg.allStampsHitBonus = 5000;
	lvl.scoreCfg.goodColor = Color(0, 1, 0);
	lvl.scoreCfg.badColor = Color(1, 0, 0);

	_loadedScript.levels.push_back(lvl);
	_loadedScript.scoring = true;

	invokeLevelChangeCallback();
	return true;
}

bool GameScripts::addScriptHeader(const std::string& scriptID)
{
	// load the header only and add it to the list
	Script script;
	if (script.load(scriptID, true))
	{
		_scripts.push_back(script);
		return true;
	}
	return false;
}

int GameScripts::getScriptHeaderCount()
{
	return _scripts.size();
}

const Script& GameScripts::getScriptHeader(int index)
{
	return _scripts[index];
}

bool GameScripts::loadGameScript(const std::string& scriptID)
{
	_currLevel = 0;

	if (_loadedScript.load(scriptID, false))
	{
		_currLevel = _loadedScript.startLevel - 1;

		invokeLevelChangeCallback();
	}
	return (_loadedScript.isLoaded());
}

void GameScripts::clearGameScript()
{
	_loadedScript.clear();
}

bool GameScripts::isGameScriptLoaded()
{
	return _loadedScript.isLoaded();
}

const Script& GameScripts::getCurrScript()
{
	return _loadedScript;
}

const Level& GameScripts::getCurrLevel()
{
	if (_currLevel < 0 || _currLevel > _loadedScript.getMaxLevelNumber())
		throw std::out_of_range("(GameScripts::getCurrLevel) Current level out of bounds");
	return _loadedScript.levels[_currLevel];
}

int GameScripts::getCurrLevelIndex()
{
	return _currLevel;
}

bool GameScripts::isLastLevel()
{
	return (_currLevel >= getCurrScript().getMaxLevelNumber());
}

bool GameScripts::isBonusScreenRequired()
{
	return (getCurrScript().scoring && getCurrLevel().scoreCfg.isBonusSummaryRequired());
}

bool GameScripts::nextLevel()
{
	if (_currLevel < _loadedScript.getMaxLevelNumber())
	{
		_currLevel++;
		invokeLevelChangeCallback();
		return true;
	}
	return false;
}

void GameScripts::addCallback(IGameScriptUpdate* newItem)
{
	_eventList.push_back(newItem);

	// automatically invoke the newLevel() callback if a script has been loaded
	if (_loadedScript.isLoaded() && _loadedScript.getMaxLevelNumber() >= 0)
		newItem->newLevel(_currLevel);
}

bool GameScripts::removeCallback(IGameScriptUpdate* delItem)
{
	std::vector<IGameScriptUpdate*>::iterator iter = _eventList.begin();
	while (iter != _eventList.end())
	{
		if (delItem == (*iter))
		{
			_eventList.erase(iter);
			return true;
		}
		iter++;
	}
	return false;
}

ScreenBase* GameScripts::demoStringToDemoScreen(const std::string& res)
{
	if (!res.empty())
	{
		if (res.compare("intro_demo") == 0)
			return new DemoGameplay1();
	}
	return nullptr;
}
