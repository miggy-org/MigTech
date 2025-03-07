#include "pch.h"
#include "GameGrid.h"
#include "CubeConst.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

GameGrid::GameGrid() : GridBase(), _isGridFilled(false), _fleeTrans(0)
{
}

// sets the new slot colors for any slot that was impacted by a collision
static void setNewSlotColors(Slot* slots, int numSlots, const Color& fillCol, const Color& emptyCol)
{
	for (int i = 0; i < numSlots; i++)
	{
		Slot& theSlot = slots[i];
		theSlot.color = (theSlot.filled ? fillCol : emptyCol);
	}
}

bool GameGrid::doFrame(int id, float newVal, void* optData)
{
	if (_idHintAnim == id)
	{
		const Color& hintCol = _hintGrid.fillCol;  // colWhite;
		_hintEmptyCol = MigUtil::blendColors(_emptyCol, hintCol, newVal);
		_hintFillCol = MigUtil::blendColors(_fillCol, hintCol, newVal);
	}
	else if (_idFleeAnim == id)
		_fleeTrans = newVal;
	else if (_idShrinkAnim == id || _idGrowAnim == id)
		updateSlotRadius(newVal, true);
	return true;
}

void GameGrid::animComplete(int id, void* optData)
{
	if (_idHintAnim == id)
	{
		_idHintAnim = 0;
	}
	else if (_idShrinkAnim == id)
	{
		_idShrinkAnim = 0;

		// set up a new animation to grow the slot sizes
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 1, defSlotAnimDuration, AnimItem::ANIM_TYPE_LINEAR);
		_idGrowAnim = MigUtil::theAnimList->addItem(animItem);

		// update the color of the impacted slots
		setNewSlotColors(_theSlots, getSlotCount(), _fillCol, _emptyCol);
	}
	else if (_idGrowAnim == id)
	{
		// animation is complete, clear animating bits
		_idGrowAnim = 0;
		clearSlotAnimations();
	}
	else if (_idFleeAnim == id)
	{
		// make all slots invisible
		_idFleeAnim = 0;
		setVisibility(false);
	}
}

// updates the orientation
void GameGrid::updateConfig(AxisOrient newOrient, int newDir)
{
	_orient = newOrient;
	_distFromOrigin = (float)(newDir * fabs(_distFromOrigin));
}

// starts the flee animation
void GameGrid::startFleeAnimation(int length)
{
	float ft[] = { 0, 0.05f, 0.1f, 0.2f, 0.4f, 1 };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, defFallStartPos, length, ft, ARRAYSIZE(ft));
	_idFleeAnim = MigUtil::theAnimList->addItem(animItem);
}

// cancels any existing slot animations
void GameGrid::cancelSlotAnim()
{
	clearSlotAnimations();

	// set the slot sizes back to full size
	updateSlotRadius(1, true);
}

// shuffles the slots w/in a grid (2x2)
static void shuffleSlots(Slot* slots, int s1, int s2, int s3, int s4)
{
	static Slot tmpSlot;
	tmpSlot.copySlotInfo(slots[s4]);
	slots[s4].copySlotInfo(slots[s3]);
	slots[s3].copySlotInfo(slots[s2]);
	slots[s2].copySlotInfo(slots[s1]);
	slots[s1].copySlotInfo(tmpSlot);
}

// shuffles the slots w/in a grid (3x3)
static void shuffleSlots(Slot* slots, int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8)
{
	shuffleSlots(slots, s1, s2, s3, s4);
	shuffleSlots(slots, s5, s6, s7, s8);
}

void GameGrid::shuffleSlots(bool cw)
{
	if (_dimen == 2)
	{
		if (cw)
			::shuffleSlots(_theSlots, 0, 2, 3, 1);
		else
			::shuffleSlots(_theSlots, 0, 1, 3, 2);
	}
	else if (_dimen == 3)
	{
		if (cw)
			::shuffleSlots(_theSlots, 0, 6, 8, 2, 1, 3, 7, 5);
		else
			::shuffleSlots(_theSlots, 0, 2, 8, 6, 1, 5, 7, 3);
	}
}

// swaps 2 slots
static void swapSlots(Slot* slots, int s1, int s2)
{
	static Slot tmpSlot;
	tmpSlot.copySlotInfo(slots[s1]);
	slots[s1].copySlotInfo(slots[s2]);
	slots[s2].copySlotInfo(tmpSlot);
}

void GameGrid::invertSlots()
{
	if (_dimen == 2)
	{
		::swapSlots(_theSlots, 0, 3);
		::swapSlots(_theSlots, 1, 2);
	}
	else if (_dimen == 3)
	{
		::swapSlots(_theSlots, 0, 8);
		::swapSlots(_theSlots, 1, 7);
		::swapSlots(_theSlots, 2, 6);
		::swapSlots(_theSlots, 3, 5);
	}
}

void GameGrid::doHint(const GridInfo& gridInfo)
{
	_hintGrid = gridInfo;
	_hintEmptyCol = _hintFillCol = _hintGrid.fillCol;  // colWhite;

	float ft[] = { 0, 0.05f, 0.1f, 0.2f, 0.4f, 1 };
	AnimItem animItem(this);
	animItem.configParametricAnim(1, 0, defSlotAnimDuration, ft, ARRAYSIZE(ft));
	_idHintAnim = MigUtil::theAnimList->addItem(animItem);
}

bool GameGrid::doCollision(const GridInfo* item, bool isHit)
{
	cancelSlotAnim();
	_isGridFilled = true;

	// set the filled bit for each impacted slot
	bool isAnythingAnimating = false;
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
	{
		// we're not going to animate any slot that didn't actually change
		if ((item == nullptr || item->fillList[i]) && _theSlots[i].filled != isHit)
		{
			_theSlots[i].filled = isHit;
			_theSlots[i].animating = true;
			isAnythingAnimating = true;
		}

		// if any slot is not filled, then the grid is incomplete
		if (!_theSlots[i].filled)
			_isGridFilled = false;
	}

	if (isAnythingAnimating)
	{
		// set up an animation to shrink the slots
		AnimItem animItem(this);
		animItem.configSimpleAnim(1, 0, defSlotAnimDuration, AnimItem::ANIM_TYPE_LINEAR);
		_idShrinkAnim = MigUtil::theAnimList->addItem(animItem);
	}

	return isAnythingAnimating;
}

// overridden to apply the flee animation translation
void GameGrid::applyOffset(Matrix& mat) const
{
	if (_fleeTrans > 0)
		mat.translate(0, 0, _fleeTrans);
}

const Color& GameGrid::getSlotDrawColor(const Slot& theSlot, int index) const
{
	if (_idHintAnim.isActive() && !theSlot.animating)
	{
		bool isHint = _hintGrid.fillList[index];
		if (isHint)
			return (theSlot.filled ? _hintFillCol : _hintEmptyCol);
	}
	return GridBase::getSlotDrawColor(theSlot, index);
}

void GameGrid::clearSlotAnimations()
{
	if (_idShrinkAnim.isActive())
	{
		_idShrinkAnim.clearAnim();

		// update the slot colors since the shrink animation was in progress
		setNewSlotColors(_theSlots, getSlotCount(), _fillCol, _emptyCol);
	}
	_idGrowAnim.clearAnim();

	// animation is complete, clear animating bits
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].animating = false;
}

// can be used to show/hide the grid
void GameGrid::setVisibility(bool visible)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].invis = !visible;
}
