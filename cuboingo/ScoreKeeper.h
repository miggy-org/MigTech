#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Timer.h"
#include "../core/Font.h"
#include "GameScripts.h"

using namespace MigTech;

namespace Cuboingo
{
	class PopupBonusText : public IAnimTarget
	{
	public:
		PopupBonusText();

		void init(bool good, const std::string& text, float u, float v, float scalar, const ScoreConfig& cfg);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		void draw();

	public:
		bool isGood;
		float param;
		Vector3 loc;
		float size, sizeScalar;
		//std::string text;
		Text theText;
		Color col;
		AnimID idScaleAnim;
		AnimID idParamAnim;
	};

	class ScoreKeeper : public IAnimTarget, public IGameScriptUpdate
	{
	public:
		ScoreKeeper();
		~ScoreKeeper();

		void init();
		void loadScoreConfig(const ScoreConfig& cfg);

		void addToScore(int value);
		void pieceMatch(AxisOrient orient, float multiplier);
		void sideComplete(AxisOrient orient, float multiplier);
		void stampMatch(AxisOrient orient);
		void tapComplete(AxisOrient orient, float scale, float multiplier);
		void pieceMiss(AxisOrient orient);
		void stampMiss(AxisOrient orient);
		void stampExpired(AxisOrient orient);

		bool checkMissesForGameOver() const;
		bool checkTimerForGameOver() const;
		long getElapsedTime() const;
		int getRemainingTime() const;

		// pause/resume the game timer
		void pauseGameTimer() { _countdown.pauseTime(); }
		void resumeGameTimer() { _countdown.resumeTime(); }

		// statistics
		int getRealScore() const { return _realScore; }
		int getGameMissCount() const { return _gameMissCount; }
		int getLevelMissCount() const { return _levelMissCount; }
		int getStampMissCount() const { return _stampMissCount; }

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IGameScriptUpdate
		virtual void newLevel(int level);

		void update();
		void draw3D() const;
		void drawOverlay() const;

	protected:
		void initText();
		void updateSlipText();
		void updateBumpText();
		void startScoreAnim();
		void startPopupAnim(bool isGood, const char* text, float u, float v, float scalar);
		void startPopupAnim(bool isGood, bool isStamp, const char* text, AxisOrient orient, float scalar);
		int applyPowerUp(int score) const;

	protected:
		// cache of the scoring configuration
		ScoreConfig _scoreCfg;
		bool _isScored;
		bool _isLegacy;

		// current score (differs from the real score if animating)
		int _currScore;
		Text _currScoreText;

		// the real score, independent of animation
		int _realScore;

		// score animation ID
		AnimID _idScoreAnim;

		// hit/miss counts
		int _gameMissCount;
		int _levelMissCount;
		int _stampHitCount;
		int _stampMissCount;

		// game timer
		long _gameStartTime;
		long _levelStartTime;
		long _gameTimeLimit;
		long _levelTimeLimit;
		Timer _countdown;

		// 4 corner text display
		Text _timeHeader;
		Text _timeText;
		Text _cubeHeader;
		Text _cubeText;
		Text _slipHeader;
		Text _slipText;
		Text _bumpHeader;
		Text _bumpText;

		// stamp chain tracking
		long _lastStampMatch;
		int _stampChainCount;

		// list of pop-up animations
		std::list<PopupBonusText*> _popupList;
	};
}
