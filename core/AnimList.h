#pragma once

#include <vector>
using namespace std;

namespace MigTech
{
	// implement this interface to receive animation events
	class IAnimTarget
	{
	public:
		// return false to remove this item from the list
		virtual bool doFrame(int id, float newVal, void* optData) = 0;

		// indicates that the given animation is complete
		virtual void animComplete(int id, void* optData) = 0;
	};

	class AnimID
	{
	public:
		AnimID();
		~AnimID();

		// assignment won't clear the previous ID
		void operator=(int id) { _id = id; }
		bool operator==(int id) const { return (_id == id); }
		bool operator!=(int id) const { return (_id != id); }
		operator bool() const { return isActive(); }
		operator int() const { return _id; }

		bool isActive() const { return (_id != 0); }

		// this will clear the animation from the list
		bool clearAnim();

	protected:
		int _id;
	};

	class AnimItem
	{
	public:
		// animation types supported
		//  TODO: consider changing to an enum
		static const int ANIM_TYPE_NONE = 0;
		static const int ANIM_TYPE_LINEAR = 1;
		static const int ANIM_TYPE_LINEAR_BOUNCE = 2;
		static const int ANIM_TYPE_LINEAR_INFINITE = 3;
		static const int ANIM_TYPE_LINEAR_INFINITE_BOUNCE = 4;
		static const int ANIM_TYPE_PARAMETRIC = 5;
		static const int ANIM_TYPE_TIMER = 98;
		static const int ANIM_TYPE_TIMER_INFINITE = 99;

	public:
		// controls animation type
		float startVal;
		float endVal;
		long duration;
		long cycle;
		int type;
		void* optData;

		// these will be filled in by AnimList
		IAnimTarget* animTarget;
		int animID;
		uint64 startTimeTicks;
		float diffVal;
		bool useSystemTime;

		// parametric only
		float* paramArray;
		int paramArraySize;

	public:
		AnimItem(IAnimTarget* target);
		AnimItem(const AnimItem& other);
		~AnimItem();

		void operator=(const AnimItem& rhs);

		void configSimpleAnim(float start, float end, long dur, int ltype);
		void configSimpleAnim(float start, float end, long dur, int ltype, void* opt);
		void configTimer(long dur, bool isInfinite);
		void configTimer(long dur, bool isInfinite, void* opt);
		void configParametricAnim(float start, float end, long dur, const float* fParams, int sizeParams);
		void configParametricAnim(float start, float end, long dur, const float* fParams, int sizeParams, void* opt);
	};

	class AnimList
	{
	private:
		// the master list of active animations
		vector<AnimItem> _itemList;

		// used to cache items to add/remove
		vector<AnimItem> _addList;
		vector<int> _delList;
		bool _isListProcessing;

		// tracks the next animation ID to return
		int _newAnimID;

	private:
		float doParametricFunction(float* vals, int sizeVals, float p);

	public:
		AnimList();

		// call to add a new animation item
		int addItem(AnimItem& newItem, bool useSystemTime = false);

		// call to remove an existing animation item
		bool removeItem(int id);

		// called by MigGame to process all active animations
		bool doAnimations();
	};
}