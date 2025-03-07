#pragma once

#include "../core/MigInclude.h"
#include "../core/ScreenBase.h"
#include "../core/MigUtil.h"
#include "../core/Font.h"
#include "GameCube.h"
#include "Launcher.h"
#include "ScoreKeeper.h"
#include "Particles.h"
#include "Stamp.h"
#include "ShadowPass.h"

using namespace MigTech;

namespace Cuboingo
{
	class GameScreen : public MigTech::ScreenBase, public IGameCubeCallback, public ILauncherCallback
	{
	public:
		GameScreen();

		// screen management
		virtual ScreenBase* getNextScreen();

		// life cycle events
		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();
		virtual void visibilityChanged(bool vis);

		// frame events
		virtual bool update();
		virtual bool render();
		virtual bool renderPass(int index, RenderPass* pass);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IOverlayCallback
		virtual void onOverlayExit(long duration);
		virtual void onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration);
		virtual void onOverlayCustom(int id, void* data);

		// IGameCubeCallback
		virtual void onCubeAnimComplete();
		virtual void onCubeStampComplete(AxisOrient axis);
		virtual void onCubeGameOverAnimComplete(bool win);

		// ILauncherCallback
		virtual void onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier);
		virtual void onDisplayHint(const GridInfo& gridInfo);

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe);
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);

		// allocator for custom controls
		virtual ControlBase* allocControl(const char* tag);

		// fade control
		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);

		bool startNextLevel();
		void startLauncher();
		void doLaunch();
		void doStamp();
		void startCubeRotate(GameCube::ROTATE_AXIS axis, int dir);
		void startCubeStamp(AxisOrient axis);
		void clearGameElements();
		std::string startGameWin();
		std::string startGameLose();
		void handlePause();
		std::string handleCubeHit(bool needObsoleteCheck, int mapIndex);
		std::string handleCubeMiss();
		void checkForLauncherReset();

	private:
		// local copy of the view matrix
		Matrix _viewMatrix;
		Matrix _projMatrix;

		// the game cube
		GameCube _cube;

		// for playing sounds
		SoundCache _sounds;

		// launcher
		Launcher _launcher;
		AnimID _idLaunchTimer;
		AnimID _idResetTimer;

		// stamps
		StampList _stamps;
		AnimID _idStampTimer;

		// score keeper
		ScoreKeeper _scoreKeeper;

		// particles
		SparkList _sparks;

		// shadow catcher
		ShadowPass _shadowPass;

		// losing animation
		AnimID _idLosingAnim;
		bool _gameIsOver;
		bool _gameWon;

		bool _tipsDisplayed;
	};
}
