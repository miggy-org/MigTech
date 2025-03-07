#include "pch.h"
#include "AnimList.h"
#include "Timer.h"
#include "MigUtil.h"

using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// AnimID

AnimID::AnimID() : _id(0)
{
}

AnimID::~AnimID()
{
	clearAnim();
}

// this will clear the animation from the list
bool AnimID::clearAnim()
{
	bool wasActive = isActive();
	if (_id != 0 && MigUtil::theAnimList != nullptr)
		MigUtil::theAnimList->removeItem(_id);
	_id = 0;
	return wasActive;
}

///////////////////////////////////////////////////////////////////////////
// AnimItem

AnimItem::AnimItem(IAnimTarget* target)
{
	animTarget = target;
	startVal = endVal = 0;
	duration = 0;
	cycle = 1;
	type = ANIM_TYPE_NONE;
	optData = nullptr;
	animID = 0;
	startTimeTicks = 0;
	diffVal = 0;
	useSystemTime = false;
	paramArray = nullptr;
	paramArraySize = 0;
}

AnimItem::AnimItem(const AnimItem& other)
{
	*this = other;
}

AnimItem::~AnimItem()
{
	delete[] paramArray;
}

void AnimItem::operator=(const AnimItem& rhs)
{
	startVal = rhs.startVal;
	endVal = rhs.endVal;
	duration = rhs.duration;
	cycle = rhs.cycle;
	type = rhs.type;
	optData = rhs.optData;
	animTarget = rhs.animTarget;
	animID = rhs.animID;
	startTimeTicks = rhs.startTimeTicks;
	diffVal = rhs.diffVal;
	useSystemTime = rhs.useSystemTime;

	if (rhs.paramArray != nullptr && rhs.paramArraySize > 0)
	{
		paramArray = new float[rhs.paramArraySize];
		for (int i = 0; i < rhs.paramArraySize; i++)
			paramArray[i] = rhs.paramArray[i];
		paramArraySize = rhs.paramArraySize;
	}
	else
	{
		paramArray = nullptr;
		paramArraySize = 0;
	}
}

void AnimItem::configSimpleAnim(float start, float end, long dur, int ltype)
{
	startVal = start;
	endVal = end;
	duration = dur;
	type = ltype;
}

void AnimItem::configSimpleAnim(float start, float end, long dur, int ltype, void* opt)
{
	configSimpleAnim(start, end, dur, ltype);
	optData = opt;
}

void AnimItem::configTimer(long dur, bool isInfinite)
{
	configSimpleAnim(0, 1, dur, (isInfinite ? ANIM_TYPE_TIMER_INFINITE : ANIM_TYPE_TIMER));
}

void AnimItem::configTimer(long dur, bool isInfinite, void* opt)
{
	configSimpleAnim(0, 1, dur, (isInfinite ? ANIM_TYPE_TIMER_INFINITE : ANIM_TYPE_TIMER));
	optData = opt;
}

void AnimItem::configParametricAnim(float start, float end, long dur, const float* fParams, int sizeParams)
{
	startVal = start;
	endVal = end;
	duration = dur;
	type = ANIM_TYPE_PARAMETRIC;

	if (fParams != nullptr && sizeParams > 0)
	{
		paramArray = new float[sizeParams];
		for (int i = 0; i < sizeParams; i++)
			paramArray[i] = fParams[i];
		paramArraySize = sizeParams;
	}
	else
		throw std::invalid_argument("(AnimItem::configParametricAnim) missing parameters");
}

void AnimItem::configParametricAnim(float start, float end, long dur, const float* fParams, int sizeParams, void* opt)
{
	configParametricAnim(start, end, dur, fParams, sizeParams);
	optData = opt;
}

///////////////////////////////////////////////////////////////////////////
// AnimList

AnimList::AnimList()
{
	_newAnimID = 1;
	_isListProcessing = false;
}

// call to add a new animation item
int AnimList::addItem(AnimItem& newItem, bool useSystemTime)
{
	newItem.animID = _newAnimID++;
	newItem.startTimeTicks = (useSystemTime ? Timer::systemTime() : Timer::gameTime());
	newItem.diffVal = newItem.endVal - newItem.startVal;
	newItem.useSystemTime = useSystemTime;

	// if we're in the middle of processing the animation list it's not safe to add it here
	if (!_isListProcessing)
		_itemList.push_back(newItem);
	else
		_addList.push_back(newItem);

	//LOGINFO("New animation item added, there are now %d items in the list", itemList.size());
	return newItem.animID;
}

// call to remove an existing animation item
bool AnimList::removeItem(int id)
{
	vector<AnimItem>::iterator iter;
	for (iter = _itemList.begin(); iter != _itemList.end(); iter++)
	{
		if (iter->animID == id)
		{
			// if we're in the middle of processing the animation list it's not safe to remove it here
			if (!_isListProcessing)
				_itemList.erase(iter);
			else
				_delList.push_back(id);
			return true;
		}
	}
	return false;
}

// performs the parametric computation (p must be >= 0 and < 1)
float AnimList::doParametricFunction(float* vals, int sizeVals, float p)
{
	float step = 1.0f / (sizeVals - 1);
	//int start = 0;
	//int end = vals.Length - 1;
	int start = (int)(p / step);
	int end = start + 1;
	float fRet = 0;
	for (int i = start; i <= end; i++)
	{
		float fTemp = (float) (1.0f - fabs(p - step * i) / step);
		if (fTemp > 0)
			fRet += fTemp * vals[i];
	}

	//LOGINFO("doParametricFunction() returning %f (fParam is %f, step is %f)", fRet, p, step);
	return fRet;
}

// called by GameScreen to process all active animations
bool AnimList::doAnimations()
{
	// starting list processing
	_isListProcessing = true;

	// cycle through the list and do each animation, note it's backwards to make removal easy
	vector<AnimItem>::iterator iter = _itemList.begin();
	for ( ; iter != _itemList.end(); )
	{
		// do not process any animation items that have been added to the delete list
		bool isOk = true;
		if (!_delList.empty())
		{
			std::vector<int>::const_iterator delIter = _delList.begin();
			while (delIter != _delList.end() && isOk)
			{
				if (*delIter == iter->animID)
					isOk = false;
				delIter++;
			}
		}

		bool needToIter = true;
		if (isOk)
		{
			// some animation types never end unless explicitly told to do so
			bool isInfinite = (
				iter->type == AnimItem::ANIM_TYPE_LINEAR_INFINITE ||
				iter->type == AnimItem::ANIM_TYPE_LINEAR_INFINITE_BOUNCE ||
				iter->type == AnimItem::ANIM_TYPE_TIMER_INFINITE);

			// and some animation types are just timers
			bool isTimer = (
				iter->type == AnimItem::ANIM_TYPE_TIMER ||
				iter->type == AnimItem::ANIM_TYPE_TIMER_INFINITE);

			// elapsed time for this animation
			uint64 diffTicks = (iter->useSystemTime ? Timer::systemTime() : Timer::gameTime()) - iter->startTimeTicks;
			long diff = Timer::ticksToMilliSeconds(diffTicks);

			// is this animation done?
			bool done = (diff >= iter->duration && !isInfinite);

			// compute the 0->1 parameter which then computes the actual animated value
			float param = (done ? 1 : iter->cycle * (diff / (float)iter->duration));
			if (param > 1 && !isInfinite)
				param = param - (int)param;	// in case cycle > 1
			if (iter->type == AnimItem::ANIM_TYPE_LINEAR_BOUNCE)
			{
				// some animation types bounce
				param = 2 * (param <= 0.5 ? param : 1 - param);
			}
			else if (iter->type == AnimItem::ANIM_TYPE_LINEAR_INFINITE_BOUNCE)
			{
				// some animation types bounce
				param = param - (int)param;
				param = 2 * (param <= 0.5 ? param : 1 - param);
			}
			else if (iter->type == AnimItem::ANIM_TYPE_PARAMETRIC)
			{
				// pass this through a parametric function
				if (!done)
					param = doParametricFunction(iter->paramArray, iter->paramArraySize, param);
				else
					param = iter->paramArray[iter->paramArraySize - 1];
			}
			float val = iter->startVal + param * iter->diffVal;

			// the timer only calls doFrame() when complete
			if (!isTimer || done || param >= 1)
			{
				bool remove = !iter->animTarget->doFrame(iter->animID, val, iter->optData);
				if (remove || done)
				{
					// signal that this animation is done
					iter->animTarget->animComplete(iter->animID, iter->optData);

					// remove this animation from the list because it's done
					iter = _itemList.erase(iter);
					needToIter = false;
					//LOGINFO("Removed animation item, there are now %d items remaining", mItemList.size());
				}
				else if (iter->type == AnimItem::ANIM_TYPE_TIMER_INFINITE)
				{
					// reset the timer animation type
					iter->startTimeTicks = (iter->useSystemTime ? Timer::systemTime() : Timer::gameTime());
				}
			}
		}

		if (needToIter)
			++iter;
	}

	// list processing is complete
	_isListProcessing = false;

	// remove any queued items
	int nItems = _delList.size();
	if (nItems > 0)
	{
		for (int n = 0; n < nItems; n++)
			removeItem(_delList[n]);
		_delList.clear();
	}

	// add any queued items
	nItems = _addList.size();
	if (nItems > 0)
	{
		for (int n = 0; n < nItems; n++)
			_itemList.push_back(_addList[n]);
		_addList.clear();
	}

	return true;
}
