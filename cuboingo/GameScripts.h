#pragma once

#include "../core/MigInclude.h"
#include "CubeConst.h"

using namespace MigTech;

namespace Cuboingo
{
	///////////////////////////////////////////////////////////////////////////
	// score configuratino

	class ScoreConfig
	{
	public:
		ScoreConfig();
		ScoreConfig(const ScoreConfig& rhs);

		void copyScoreConfig(const ScoreConfig& toCopy);

		bool isBonusSummaryRequired() const;
		bool isValid() const;

	public:
		int pieceMatch;
		int sideComplete;
		int stampMatch;
		int tapMax;

		int pieceMiss;
		int stampMiss;

		int timedBonusPerSecond;
		int noMissBonus;
		int allStampsHitBonus;

		float evilMultiplier;
		float stampChainMultiplier;
		int stampChainTime;

		Color headerColor;
		Color valueColor;
		Color goodColor;
		Color badColor;
	};

	///////////////////////////////////////////////////////////////////////////
	// level configuration

	class Level
	{
	public:
		Level();
		Level(const Level& rhs);

		void copyLevel(const Level& toCopy);

	public:
		int rotTime;			// time to rotate the cube 90 degrees (ms)

		int fallTime;			// time for a grid piece to fall (ms)
		int glowTime;			// time for the shafts to glow (ms)
		int idleTime;			// idle time between shaft and falling grid (ms)
		int resetTime;			// reset time if nothing in the launcher (ms)

		int numColors;			// maximum number of colors allowed
		int gridLevel;			// grid dimension (1, 2 or 3)
		int directions;		    // number of directions grids will fall from
		int glowCount;			// number of times the hint pieces will glow
		int fallingPieces;		// number of falling pieces at one time
		int fallingHoleLimit;	// limit on the number of holes in any falling piece, 0 means none
		bool randFallingPieces; // randomize orientation of falling pieces
		int missCount;			// number of allowed misses, 0 means unrestricted
		int timeLimit;			// time limit for this level (seconds)
		GridLock gridLock;		// is grid face locking on for this level
		LaunchStyle launchAxisStyle;	// launch axis style (see CubeConst.launchStyle*)

		int evilProbability;	// the probability that an evil falling piece will instead of a normal one (%)
		int evilFallTime;		// time for an evil falling piece to fall (ms)
		int evilPeriod;		    // time the evil pieces display one color (ms)
		int evilMaxColors;		// maximum colors to cycle through, 0 means no limit
		EvilGridStyle evilStyle;  // style (see EvilGridStyle)
		AxisLock evilAxisLock;	// style of axis lock (see CubeConst.axisLock*)

		int wildProbability;	// the probability that a wild card falling piece will instead of a normal one (%)
		int wildFallTime;		// time for a wild card falling piece to fall (ms)
		int wildPeriod;		    // time the wild card pieces display one color (ms)
		WildGridStyle wildStyle; // style (not in use yet, reserved for future)
		AxisLock wildAxisLock;	// style of axis lock (see CubeConst.axisLock*)

		int burstProbability;	// probability of a burst launch
		int burstFallTime;		// fall time for burst launches
		int evilBurstProbability;	// probabiliby of an evil burst launch
		int evilBurstFallTime;	// fall time for an evil burst launch
		int evilBurstPeriod;	// cycle time for an evil burst launch

		int stampCount;		    // number of on-screen stamps at any time
		int stampPeriod;		// the period between each stamp probability check (ms)
		int stampProbability;	// the probability that a stamp will appear, if possible (%)
		int stampDuration;		// the length of time a stamp will display (ms)
		int stampCycle;			// the length of time a single power-up iteration will display (ms)
		StampStyle stampStyle;  // style of stamping (see StampStyle)

		int powerUpMult;		// power up score multiplier (POWERUP_MULTIPLIER)
		int powerUpMultDur;		// power up multiplier duration (POWERUP_MULTIPLIER)
		int powerUpInvDur;		// power up invulnerability duration (POWERUP_INVULNERABLE)
		int powerUpInvMax;		// power up invulnerability maximum misses(POWERUP_INVULNERABLE)

		Color dimColors[6];
		Color maxColors[6];

		ScoreConfig scoreCfg;
	};

	///////////////////////////////////////////////////////////////////////////
	// single script configuration

	class Script
	{
	public:
		Script();

		void clear();
		bool isLoaded() const;
		int getMaxLevelNumber() const;

		bool load(const std::string& scrID, bool headerOnly);

	public:
		std::string scriptID;
		std::string name;
		std::string uniqueID;
		std::string author;
		std::string desc1;
		std::string desc2;
		std::string desc3;
		std::string desc4;
		std::string difficulty;
		std::string imageID;
		std::string musicID;
		std::string demoID;
		int missCount;			// number of allowed misses (0 means none, else overrides level setting)
		int timeLimit;			// time limit for the game (seconds)
		bool scoring;			// is scoring on for this script

		std::vector<std::string> tips;  // tips to be displayed at the beginning of the script

		std::vector<Level> levels;	// levels
		int startLevel;

		std::string textureFile;
		std::string reflectFile;
		Color colCube;
		float reflectIntensity;

		BgBase* bgHandler;
		BgBase* overlayHandler;
	};

	///////////////////////////////////////////////////////////////////////////
	// game scripts public interface

	// implement this interface to receive game script events
	class IGameScriptUpdate
	{
	public:
		// new level loaded
		virtual void newLevel(int level) = 0;
	};

	class GameScripts
	{
	public:
		static void init(tinyxml2::XMLElement* rootCfg);
		static void clear();

		static bool loadTestScript();
		static bool loadGameScript(const std::string& scriptID);
		static void clearGameScript();
		static bool isGameScriptLoaded();

		static bool addScriptHeader(const std::string& scriptID);
		static int getScriptHeaderCount();
		static const Script& getScriptHeader(int index);

		static const Script& getCurrScript();
		static const Level& getCurrLevel();
		static int getCurrLevelIndex();

		static bool isLastLevel();
		static bool isBonusScreenRequired();
		static bool nextLevel();

		static void addCallback(IGameScriptUpdate* newItem);
		static bool removeCallback(IGameScriptUpdate* delItem);

		static ScreenBase* demoStringToDemoScreen(const std::string& res);
	};
}