#pragma once

#include "../core/MigInclude.h"
#include "../core/DemoBase.h"
#include "../core/MigUtil.h"
#include "GameCube.h"
#include "Launcher.h"
#include "Stamp.h"
#include "GameScripts.h"
#include "ScoreKeeper.h"

using namespace MigTech;

namespace Cuboingo
{
	class DemoGameCube : public GameCube
	{
	public:
		DemoGameCube(IGameCubeCallback* cb) : GameCube(cb) { _scale = 1; }

		void fillCubeSlots(int face, bool* fillList, int fillListLen);
		GridInfo newGridCandidate(int face);
	};

	class DemoGameplay : public DemoScreen, public IGameCubeCallback, public ILauncherCallback
	{
	public:
		DemoGameplay(const std::string& name);

		// life cycle events
		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// frame events
		virtual bool update();
		virtual bool render();

		// raw input events
		virtual bool pointerPressed(float x, float y);

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

		// callback invoked when a text animation is complete
		virtual void onTextAnimComplete(bool fadeIn);

	protected:
		int startDelayAnim(int delay);
		int getMaxTextLength();
		void initLinesOfText(const std::string& text1, const std::string& text2, const std::string& text3);
		void startTextAnim();
		void startTextAnimIn(const std::string& text1, const std::string& text2, const std::string& text3, int preDelay, int duration, int postDelay);
		void startTextAnimOut(int preDelay, int duration, int postDelay);
		void startCubeRotate(GameCube::ROTATE_AXIS axis, int dir);

	protected:
		DemoGameCube _gameCube;
		Launcher _launcher;
		StampList _stamps;
		SoundCache _sounds;

		// local copy of the view/proj matrix
		Matrix _viewMatrix;
		Matrix _projMatrix;

		// up to 3 lines of text
		TextButton* _line1Text;
		std::string _line1Str;
		TextButton* _line2Text;
		std::string _line2Str;
		TextButton* _line3Text;
		std::string _line3Str;

		// for fading out the 3 lines of text
		float _textAlpha;

		// the NEXT button
		TextButton* _nextBtn;

		// animation stuff
		AnimID _idPreDelayAnim;
		AnimID _idTextAnim;
		int _textAnimDur;
		AnimID _idPostDelayAnim;
		int _postDelay;

		// aborts all demos
		bool _abortAll;
	};

	class DemoGameplay1 : public DemoGameplay
	{
	public:
		DemoGameplay1() : DemoGameplay("DemoGameplay1") { }

		virtual ScreenBase* getNextScreen();

		virtual void create();

		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);
		virtual void onTextAnimComplete(bool fadeIn);
		virtual void onStopScript();
	};

	class DemoGameplay2 : public DemoGameplay
	{
	public:
		DemoGameplay2() : DemoGameplay("DemoGameplay2"), _phase(0)  { }

		virtual ScreenBase* getNextScreen();

		virtual void create();

		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);
		virtual void onTextAnimComplete(bool fadeIn);
		virtual void onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier);

	protected:
		void startDemoPhase(int cubeFace, AxisOrient launchAxis, const char* demoScriptID);

	protected:
		int _phase;
	};

	class DemoGameplay3 : public DemoGameplay
	{
	public:
		DemoGameplay3() : DemoGameplay("DemoGameplay3"), _phase(0) {}

		virtual ScreenBase* getNextScreen();

		virtual void create();

		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);
		virtual void onTextAnimComplete(bool fadeIn);
		virtual void onCubeStampComplete(AxisOrient axis);
		virtual void onStopScript();

	protected:
		int _phase;
	};

	class DemoGameplay4 : public DemoGameplay
	{
	public:
		DemoGameplay4() : DemoGameplay("DemoGameplay4"), _phase(0) {}

		virtual ScreenBase* getNextScreen();

		virtual void create();

		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);
		virtual void onTextAnimComplete(bool fadeIn);
		virtual void onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier);

	protected:
		void startDemoPhase(AxisOrient launchAxis, const char* demoScriptID);

	protected:
		int _phase;
	};
}
