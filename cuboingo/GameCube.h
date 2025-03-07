#pragma once

#include "CubeBase.h"

using namespace MigTech;

namespace Cuboingo
{
	// implement this interface to receive events from the game cube
	class IGameCubeCallback
	{
	public:
		// intro or exit animation complete
		virtual void onCubeAnimComplete() = 0;

		// stamping action is complete
		virtual void onCubeStampComplete(AxisOrient axis) = 0;

		// game win animation is complete
		virtual void onCubeGameOverAnimComplete(bool win) = 0;
	};

	// info about the results of a falling grid/cube collision
	struct CollisionResult
	{
		bool wasHit;
		bool wasRealHit;			// ignores shield
		bool wasPieceFilled;		// only if wasHit is true
		bool wasSideComplete;		// only if wasHit is true
		bool needObsoleteCheck;		// only if wasHit is true
		GridInfo rejGrid;			// only if wasHit is false
		std::string soundToPlay;
		int mapIndexHit;
		AxisOrient orientHit;
		Color colHit;
	};

	// info about the results of a stamp/cube collision
	struct StampResult
	{
		bool wasHit;
		bool wasRealHit;			// ignores shield
		bool wasSideComplete;		// only if wasHit is true
		bool needObsoleteCheck;		// only if wasHit is true
		GridInfo hitGridInfo;		// only if wasHit is true, and if stamp had no grid of it's own
		std::string soundToPlay;
	};

	// used to visualize the invulnerability power-up
	class GameCubeShield : public CubeBase
	{
	public:
		GameCubeShield();

		void doHitAnimation();
		bool isVisible() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual void draw(Matrix& mat) const;

	protected:
		AnimID _idHitAnim;
		float _flashParam;
	};

	class GameCube : public CubeGrid
	{
	public:
		enum ROTATE_AXIS { ROTATE_AXIS_NONE, ROTATE_AXIS_X, ROTATE_AXIS_Y, ROTATE_AXIS_Z };
		static const int ROTATE_NONE = 0;
		static const int ROTATE_CCW = 1;
		static const int ROTATE_CW = -1;

	public:
		GameCube(IGameCubeCallback* cb);

		void initCubeGrids(int gridLevel, int numColors, const Color* dim, const Color* max, const int* indArray);
		void initCubeGrids(int gridLevel, int numColors, const Color* dim, const Color* max);
		void initCubeGrids();

		virtual void create();
		virtual void createGraphics();
		virtual void destroyGraphics();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual void draw(Matrix& mat) const;
		virtual void draw() const;

		int startIntroAnimation();
		int startExitAnimation();
		int startWinAnimation();
		int startLoseAnimation();

		bool startRotate(ROTATE_AXIS axis, int dir, int dur);
		bool startRotate(ROTATE_AXIS axis, int dir);
		bool startStamp(AxisOrient axis, float dist, int dur);
		bool startStamp(AxisOrient axis, float dist);

		bool newFallingGridCandidate(GridInfo& info, bool allowEmpty, bool incReserved);
		bool newGridCandidate(GridInfo& info, bool allowEmpty, bool allowFilled, bool allowPartial);
		GridInfo* newGridCandidatesList(bool allowEmptyGrids, bool allowFilledGrids, bool allowPartialGrids, bool incReserved, int& numGrids);
		GridInfo* newBurstCandidatesList(bool allowEmptyGrids, int& numGrids);
		GridInfo* newEvilBurstCandidatesList(int& numGrids);

		void doHint(const GridInfo& gridInfo);
		bool doCollision(CollisionResult& res, const GridInfo* pieces, int numPieces);
		bool doStamp(StampResult& res, AxisOrient axis, GridInfo* stampGrid);

		bool canRotate() const;
		bool canStamp() const;
		bool isGameOverAnimating() const;
		bool isStationary() const;
		bool isVisible() const;
		bool isFilled() const;

	protected:
		void handleShuffling();
		void startRealExitAnimation(int length);
		void startCubeHitAnimation(bool isHit, AxisOrient orient);
		GridBase* gridFaceByOrientation(AxisOrient orient, bool frontFacing);

	protected:
		// cube rotation control
		ROTATE_AXIS _rotAxis;
		int _rotDir;
		bool _shuffleComplete;

		// animation IDs used during the hit jiggle
		AnimID _idJiggleRotX, _idJiggleRotY;
		AnimID _idJiggleScaleAnim;
		AnimID _idExitDelay;

		// animation IDs for win/lose animations
		AnimID _idWinAnim;
		AnimID _idLoseAnim;

		// used for tagging falling grids w/ unique identifiers
		int _nextFallingGridID;

		// the last chosen grid face when making candidates (used by grid locking only)
		GridBase* _lastChosenGrid;

		// stamping (this is a copy of existing animations)
		int _idStampAnimation;

		// invulnerability shield display
		GameCubeShield _shield;

		// we only support a single callback for now
		IGameCubeCallback* _callback;
	};
}