#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"
#include "CubeConst.h"
#include "GridBase.h"

using namespace MigTech;

namespace Cuboingo
{
	class FallingGrid;

	class IFallingGridCallback
	{
	public:
		virtual void rejectAnimComplete(FallingGrid* rejGrid) = 0;
		virtual void obsoleteAnimComplete(FallingGrid* fallGrid) = 0;
	};

	class FallingGrid : public GridBase, public IAnimTarget
	{
	public:
		FallingGrid(const GridInfo& info, bool isBurst);

		virtual bool init(float dist);
		float checkTap(float x, float y, const Matrix& proj, const Matrix& view) const;
		void setBaseDist(float dist) { _baseDist = dist; }
		bool isTapped() const { return (_accel > 0 || _idTapAnim.isActive() ? true : false); }
		float getAccel() const { return _accel; }
		float getTotalDist() const
		{
			float total = _baseDist - defFallStartPos *_accel;
			return (total > 0 ? total : 0);
		}

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual const GridInfo* getGrids(int& count) const;
		virtual const Color& getDisplayColor() const;
		virtual bool canLock() const;
		virtual int getFallTime() const;
		virtual float getScoreMultiplier() const;
		virtual bool checkObsolete(int mapIndexObsolete);

		virtual bool startTapAnim(int animTime);
		virtual bool startObsoleteAnim(int animTime, IFallingGridCallback* callback);
		virtual bool startRejectAnim(int animTime, IFallingGridCallback* callback);

	protected:
		virtual void applyOffset(Matrix& mat) const;

		void updateSlotColorsForTap(float accel);

	protected:
		GridInfo _origInfo;
		bool _isBurst;

		IFallingGridCallback* _callback;
		float _baseDist;
		float _accel;
		float _rejectTrans;
		AnimID _idTapAnim;
		AnimID _idObsoleteAnim;
		AnimID _idAnimFade, _idAnimBounce;
	};

	class IEvilFallingGridCallback
	{
	public:
		// invoked on evil grid info change
		virtual void onGridInfoChange(AxisOrient axis, const GridInfo& newGridInfo) = 0;
	};

	class EvilFallingGrid : public FallingGrid
	{
	public:
		EvilFallingGrid(GridInfo* grids, int numGrids, bool isBurst, IEvilFallingGridCallback* callback);
		~EvilFallingGrid();

		virtual bool init(float dist);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual const GridInfo* getGrids(int& count) const;
		virtual const Color& getDisplayColor() const;
		virtual int getFallTime() const;
		virtual bool canLock() const;
		virtual float getScoreMultiplier() const;
		virtual bool checkObsolete(int mapIndexObsolete);
		virtual bool startTapAnim(int animTime);
		virtual bool startObsoleteAnim(int animTime, IFallingGridCallback* callback);

	protected:
		void startIntroAnim();
		void startTimerAnim();
		void startExitAnim();
		void clearAnim();
		void updateSlotColors(const Color& newColor);
		void updateSlotVisibility(const bool* fillList);
		void updateAllForNewGrid();
		void generateGridIndexList();

	protected:
		GridInfo* _grids;
		std::vector<int> _indexList;
		int _numRealGrids;
		int _numGrids;
		int _currGrid;
		AnimID _idIntro, _idTimer, _idExit;

		IEvilFallingGridCallback* _callback;
	};

	class WildFallingGrid : public FallingGrid
	{
	public:
		WildFallingGrid(GridInfo* grids, int numGrids, bool isBurst);
		~WildFallingGrid();

		virtual bool init(float dist);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);

		virtual const GridInfo* getGrids(int& count) const;
		virtual int getFallTime() const;
		virtual bool canLock() const;
		virtual bool checkObsolete(int mapIndexObsolete);

	protected:
		void updateSlotColors(float r, float g, float b);
		void clearAnim();

	protected:
		GridInfo* _grids;
		int _numGrids;
		AnimID _idColorAnim;
	};
}