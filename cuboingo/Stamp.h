#pragma once

#include "CubeBase.h"
#include "GameScripts.h"
#include "ScoreKeeper.h"
#include "PowerUp.h"

using namespace MigTech;

namespace Cuboingo
{
	// implement this interface to receive stamp grid events
	class IStampGridUpdate
	{
	public:
		virtual void gridAnimComplete(bool animIn) = 0;
		virtual void lifecycleExpired() = 0;
	};

	class StampGrid : public GridBase, public IAnimTarget
	{
	public:
		StampGrid(IStampGridUpdate* callback);

		bool init(const GridInfo& info, float dist);
		bool init(GridInfo* infoArray, int numArray, float dist);

		void setSlotRadius(float newVal);
		void setSlotAlpha(float newVal);

		void updateSlotColors(const Color& newColor);
		void updateSlotVisibility(const bool* fillList);
		void updateAllForNewGrid();

		void startGridAnim(int duration, bool fadeIn);
		void startFadeAnim(int duration, bool fadeIn);
		void cancelAnim();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual void draw(const Matrix& mat) const;

		const GridInfo& getStampGridInfo() const { return _grids[_currGrid]; }
		int getMapIndex() const { return getStampGridInfo().mapIndex; }
		float getSlotRadius() const { return _slotRadius; }
		bool isVisible() const { return _visible; }
		void setVisible(bool vis) { _visible = vis; }

	protected:
		GridInfo* _grids;
		int _numGrids;
		int _currGrid;
		float _slotRadius;
		bool _visible;

		// reserved for the animations
		AnimID _idSlotAnim;
		AnimID _idSlotFade;
		AnimID _idLifeCycle;
		AnimID _idInternalCycle;

		IStampGridUpdate* _callback;
	};

	// implement this interface to receive stamp events
	class IStampUpdate
	{
	public:
		// a stamp has expired
		virtual void stampExpired(AxisOrient orient) = 0;
	};

	class Stamp : public CubeBase, public IStampGridUpdate
	{
	public:
		Stamp(AxisOrient orient, IStampUpdate* callback);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IStampGridUpdate
		virtual void gridAnimComplete(bool animIn);
		virtual void lifecycleExpired();

		virtual void draw(Matrix& mat) const;
		virtual void draw() const;

		void showStamp(const GridInfo& grid);
		void showStamp(GridInfo* infoArray, int numArray);
		void showStamp();
		void doCollision(bool wasHit, const GridInfo& newGrid);
		void hideStamp(bool doSlotAnim = true);
		void resetTranslation();

		bool isVisible() const { return (_color.a > 0); }
		bool isStampable() const { return (_color.a == 1 && !_idTransZ.isActive()); }
		bool isInStamp(float x, float y) const { return (isStampable() && _screenRect.Contains(x, y)); }
		int getMapIndex() const { return (isVisible() ? _stampGrid.getMapIndex() : -1); }
		AxisOrient getOrient() const { return _orient; }
		bool isStampGridVisible() const { return _stampGrid.isVisible(); }
		const GridInfo& getStampGridInfo() const { return _stampGrid.getStampGridInfo(); }
		const Rect& getScreenRect() const { return _screenRect; }
		void setScreenRect(const Rect& rect) { _screenRect = rect; }

	protected:
		AxisOrient _orient;
		IStampUpdate* _callback;

		// screen rect the stamp will occupy
		Rect _screenRect;

		// grid to display on this stamp
		StampGrid _stampGrid;
	};

	class StampList : public MigBase, public IGameScriptUpdate, public IStampUpdate
	{
	public:
		StampList(ScoreKeeper* sk);
		~StampList();

		bool init();
		void createGraphics();
		void destroyGraphics();
		void draw() const;
		void drawIcons() const;

		// IGameScriptUpdate
		virtual void newLevel(int level);

		// IStampUpdate
		virtual void stampExpired(AxisOrient orient);

		bool isStampingActive() const;
		bool isStampAvailable() const;
		bool isStampAxisActive(AxisOrient axis) const;
		bool isStampAxisStampable(AxisOrient axis) const;
		bool isMapIndexUsed(int mapIndex) const;
		AxisOrient processTap(float x, float y) const;

		Stamp* getRandomAvailableStamp();
		bool startNewStamp(const GridInfo& newGrid);
		bool startNewStamp(const GridInfo& newGrid, AxisOrient axis);
		bool startNewStamp(AxisOrient axis);
		bool startNewStamp(GridInfo* newGrids, int numGrids);
		bool getStampGridInfo(GridInfo& info, AxisOrient axis, bool needToInvert);
		void doStampResult(AxisOrient axis, bool wasHit, const GridInfo& newGrid);
		void checkForObsoleteStamps(int mapIndex);
		void clearAllStamps();

	protected:
		Stamp* axisToStamp(AxisOrient axis);
		const Stamp* axisToStamp(AxisOrient axis) const;

	protected:
		ScoreKeeper* _scoreKeeper;

		// stamps
		Stamp _stampX;
		Stamp _stampY;
		Stamp _stampZ;

		// power ups
		std::map<AxisOrient, PowerUp> _powerUps;
	};
}