#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Controls.h"
#include "GameScripts.h"
#include "GameCube.h"
#include "HintGrid.h"
#include "FallingGrid.h"
#include "LightBeam.h"

using namespace MigTech;

namespace Cuboingo
{
	// call back interface to receive events from the launcher
	class ILauncherCallback
	{
	public:
		// invoked when a falling piece is done falling
		virtual void onFallingPieceComplete(const GridInfo* gridInfo, int numGrids, float tapBonus, float scoreMultiplier) = 0;

		// invoked when a hint needs to be displayed
		virtual void onDisplayHint(const GridInfo& gridInfo) = 0;
	};

	// sub-class of pic button to handle the glowing animations
	class LaunchButton : public PicButton
	{
	private:
		static const int ANIM_DUR_GLOW = 1000;
		static const int ANIM_DUR_TAP = 500;

	public:
		LaunchButton(int id) : PicButton(id) { }

		virtual bool init(tinyxml2::XMLElement* xml, const Font* font);
		void init(float u, float v, float h, float rot);
		void startAnim(const Color& col);
		void startTapAnim();
		void update(const Color& col);
		void stop();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	private:
		void cancelExistingAnims();

	private:
		AnimID _idGlowAnim;
		AnimID _idTapAnim;
	};

	// this class is used to shepherd a falling grid through the three stages listed below
	class LaunchStage
	{
	public:
		// the three major stages of a falling grid
		enum TYPE { ANIM_TYPE_IDLE, ANIM_TYPE_GLOW, ANIM_TYPE_FALL };

		LaunchStage()
		{
			glowGrid = nullptr;
			fallGrid = nullptr;
			animType = ANIM_TYPE_IDLE;
		}
		~LaunchStage()
		{
			delete glowGrid;
			delete fallGrid;
		}

		//GridInfo gridInfo;
		HintGrid* glowGrid;
		FallingGrid* fallGrid;
		TYPE animType;
		AnimID idAnim;
	};

	class Launcher : public MigBase, public IAnimTarget, public IGameScriptUpdate, public IFallingGridCallback, public IEvilFallingGridCallback
	{
	public:
		Launcher(const GameCube& cube);
		~Launcher();

		void init(ILauncherCallback* callback);
		void configControls(Controls& ctrls);
		void configLaunchSettings(int idle, int glow, int fall, int count);

		void createGraphics();
		void destroyGraphics();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IGameScriptUpdate
		virtual void newLevel(int level);

		// IFallingGridCallback
		virtual void rejectAnimComplete(FallingGrid* rejGrid);
		virtual void obsoleteAnimComplete(FallingGrid* fallGrid);

		// IEvilFallingGridCallback
		virtual void onGridInfoChange(AxisOrient axis, const GridInfo& newGridInfo);

		bool canLaunch() const;
		int getPieceCount() const { return _idleList.size() + _hintList.size() + _fallList.size(); }
		void addRejection(const GridInfo& rejGridInfo);
		void checkForObsoleteFallingPieces(int mapIndex);

		bool startGridLaunch(GridInfo& newGrid, AxisOrient orient, int idleTime, bool isBurst);
		bool startGridLaunch(GridInfo& newGrid, AxisOrient orient);
		bool startGridLaunch(GridInfo& newGrid);
		bool startEvilGridLaunch(GridInfo* newGrids, int numGrids, EvilGridStyle evilStyle, AxisOrient orient);
		bool startEvilGridLaunch(GridInfo* newGrids, int numGrids, EvilGridStyle evilStyle);
		bool startEvilGridLaunch(GridInfo* newGrids, int numGrids);
		bool startWildGridLaunch(GridInfo* newGrids, int numGrids, WildGridStyle wildStyle);
		bool startWildGridLaunch(GridInfo* newGrids, int numGrids);
		bool startBurstLaunch(GridInfo* newGrids, int numGrids);
		bool startEvilBurstLaunch(GridInfo* newGrids, int numGrids);

		bool onTap(float x, float y, const Matrix& proj, const Matrix& view);
		bool onClick(int id);
		bool onKey(VIRTUAL_KEY key);
		void clearAllItems();

		void draw(const Matrix& mat) const;
		void draw() const;

	protected:
		bool findFirstPieceColor(AxisOrient orient, Color& col) const;
		LaunchButton* axisToLaunchButton(AxisOrient axis);
		void evalLaunchButton(AxisOrient axis);
		void evalLaunchButtons();
		void invokeCollisionCallback(const FallingGrid* fallGrid);
		const std::list<AxisOrient>& getUnlockedAxisList() const;
		AxisOrient pickRandomOrientation();
		AxisOrient pickSequentialOrientation();
		AxisOrient pickSequentialByColorOrientation(const GridInfo& grid);

		void startIdleTimer(LaunchStage* stage, int idleTime);
		void startObsoleteAnim(LaunchStage* stage);
		void checkListForObsoleteFallingPieces(std::list<LaunchStage*>& list, int mapIndex, bool doObsoleteAnim);
		void startTapAnim(FallingGrid* hitGrid);
		bool doPieceAccel(AxisOrient orient);

	protected:
		const GameCube& _gameCube;
		LightBeam _lightBeam;

		// lists of objects to render (or simply for tracking)
		std::list<LaunchStage*> _idleList;
		std::list<LaunchStage*> _hintList;
		std::list<LaunchStage*> _fallList;
		std::list<FallingGrid*> _obsoleteList;
		std::list<FallingGrid*> _rejList;

		// basic script settings (that can be overridden)
		int _idleTime;
		int _glowTime;
		int _fallTime;
		int _glowCount;

		// list of potential axis directions to launch falling pieces along
		std::list<AxisOrient> _axisDirections;
		int _lastAxis;	// used during sequential axis style only
		int _lastIndex; // used during sequential-by-color axis style only

		// for playing sounds
		SoundEffect* _tapSound;

		// buttons for accelerating falling pieces
		LaunchButton* _leftBtn;
		LaunchButton* _rightBtn;
		LaunchButton* _centerBtn;

		// animation IDs
		AnimID _idGlobalGlow;

		// call back
		ILauncherCallback* _callback;

		// burst modes
		enum LaunchBurstMode { BURST_NONE, BURST_NORMAL, BURST_EVIL };
		LaunchBurstMode _burstMode;
	};
}
