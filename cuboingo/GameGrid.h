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
	class GameGrid : public GridBase, public IAnimTarget
	{
	public:
		GameGrid();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		void updateConfig(AxisOrient newOrient, int newDir);
		void startFleeAnimation(int length);
		void cancelSlotAnim();

		void shuffleSlots(bool cw);
		void invertSlots();

		void doHint(const GridInfo& gridInfo);
		bool doCollision(const GridInfo* item, bool isHit);

		bool isFilled() const { return _isGridFilled; }
		void setVisibility(bool visible);

	protected:
		virtual void applyOffset(Matrix& mat) const;
		virtual const Color& getSlotDrawColor(const Slot& theSlot, int index) const;

		void clearSlotAnimations();

	protected:
		AnimID _idHintAnim;
		GridInfo _hintGrid;
		Color _hintEmptyCol, _hintFillCol;

		AnimID _idShrinkAnim, _idGrowAnim;
		bool _isGridFilled;

		AnimID _idFleeAnim;
		float _fleeTrans;
	};
}