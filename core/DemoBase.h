#pragma once

#include "AnimList.h"
#include "MovieClip.h"
#include "ScreenBase.h"

namespace MigTech
{
	enum DEMO_EVENT { DEMO_EVENT_NONE, DEMO_EVENT_FINGER_DOWN, DEMO_EVENT_FINGER_UP, DEMO_EVENT_MOVE };

	class IDemoScriptCallback
	{
	public:
		virtual void onStartScript() = 0;
		virtual void onMove(float u, float v, DEMO_EVENT evt) = 0;
		virtual void onStopScript() = 0;
	};

	class DemoScript : public IAnimTarget
	{
	public:
		DemoScript(IDemoScriptCallback* callback);
		~DemoScript();

		bool start(const std::string& xmlID);
		void start(long dur, float* uArray, float* vArray, int numUV, long* tapEvents, int numTaps);
		void update();
		void stop();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		float getU() const { return _u; }
		float getV() const { return _v; }
		float getAlpha() const { return _alpha; }
		bool isFingerDown() const { return _isFingerDown; }
		bool isStarted() const { return _isStarted; }
		bool isDone() const { return _isDone; }

	protected:
		void startFadeAnim(float start, float end);
		void startCursorAnim();

	protected:
		// current position/alpha/finger (can be directly accessed)
		float _u, _v;
		float _alpha;
		bool _isFingerDown;

		// position animation
		float* _uArray;
		float* _vArray;
		int _numUV;
		AnimID _idAnimU, _idAnimV;
		long _dur;

		// fade animation
		AnimID _idFadeAnim;

		// tap incidents
		long* _taps;
		int _numTaps;
		long _startTime;
		int _nextEvent;

		IDemoScriptCallback* _callback;
		bool _isStarted;
		bool _isDone;
	};

	class DemoScreen : public ScreenBase, public IDemoScriptCallback
	{
	public:
		DemoScreen(const std::string& name);
		virtual ~DemoScreen();

		// IDemoScriptCallback
		virtual void onStartScript();
		virtual void onMove(float u, float v, DEMO_EVENT evt);
		virtual void onStopScript();

		// raw input events
		virtual bool pointerPressed(float x, float y);
		virtual void pointerReleased(float x, float y);
		virtual void pointerMoved(float x, float y, bool isInContact);

		// frame events
		virtual bool update();

	protected:
		// demo script
		DemoScript _script;

		// the on-screen cursor
		MovieClip _cursor;
		int _fingerUpFrame;
		int _fingerDownFrame;

		// locks out the user from interacting with the demo
		bool _userLockout;
	};
}
