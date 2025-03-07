#include "pch.h"
#include "DemoBase.h"
#include "MigUtil.h"
#include "Timer.h"

using namespace MigTech;
using namespace tinyxml2;

// fade anim duration
static const int FADE_ANIM_DURATION = 500;

// XML tags
static const char* TAG_DEMO		= "Demo";
static const char* TAG_POINT	= "Pt";
static const char* TAG_TAP		= "Tap";

// XML attribute names
static const char* ATTR_PERIOD	= "Period";
static const char* ATTR_X		= "x";
static const char* ATTR_Y		= "y";
static const char* ATTR_TIME	= "time";

///////////////////////////////////////////////////////////////////////////
// DemoScript

DemoScript::DemoScript(IDemoScriptCallback* callback)
{
	_callback = callback;
	_u = _v = 0;
	_alpha = 0;
	_isFingerDown = false;
	_uArray = nullptr;
	_vArray = nullptr;
	_numUV = 0;
	_dur = 0;
	_taps = nullptr;
	_numTaps = 0;
	_startTime = 0;
	_nextEvent = 0;
	_isStarted = false;
	_isDone = false;
}

DemoScript::~DemoScript()
{
	stop();

	delete _uArray;
	delete _vArray;
	delete _taps;
}

static float* floatListToArray(const std::vector<float>& fList, int& num)
{
	num = fList.size();
	float* newArray = new float[num];
	for (int i = 0; i < num; i++)
		newArray[i] = fList[i];
	return newArray;
}

static long* longListToArray(const std::vector<long>& lList, int& num)
{
	num = lList.size();
	long* newArray = new long[num];
	for (int i = 0; i < num; i++)
		newArray[i] = lList[i];
	return newArray;
}

// loads the demo script from an XML configuration file and starts
bool DemoScript::start(const std::string& xmlID)
{
	LOGINFO("(DemoScript::start) Loading demo XML script '%s'", xmlID.c_str());

	tinyxml2::XMLDocument* pdoc = XMLDocFactory::loadDocument(xmlID);
	if (pdoc != nullptr)
	{
		XMLElement* demo = pdoc->FirstChildElement(TAG_DEMO);
		if (demo != nullptr)
		{
			std::vector<float> uList;
			std::vector<float> vList;
			std::vector<long> tapList;

			// period
			long period = atoi(demo->Attribute(ATTR_PERIOD));

			// points
			XMLElement* point = demo->FirstChildElement(TAG_POINT);
			while (point != nullptr)
			{
				float u = (float)atof(point->Attribute(ATTR_X)) / 100.0f;
				float v = (float)atof(point->Attribute(ATTR_Y)) / 100.0f;
				uList.push_back(u);
				vList.push_back(1 - v);

				point = point->NextSiblingElement(TAG_POINT);
			}

			// taps
			XMLElement* tap = demo->FirstChildElement(TAG_TAP);
			while (tap != nullptr)
			{
				long t = atoi(tap->Attribute(ATTR_TIME));
				tapList.push_back(t);

				tap = tap->NextSiblingElement(TAG_TAP);
			}

			int numUV, numTaps;
			float* uArray = floatListToArray(uList, numUV);
			float* vArray = floatListToArray(vList, numUV);
			long* tapEvents = longListToArray(tapList, numTaps);
			long dur = (numUV - 1) * period;
			start(dur, uArray, vArray, numUV, tapEvents, numTaps);

			LOGDBG("(DemoScript::start) Done reading demo XML script '%s'", xmlID.c_str());
		}
		else
			throw std::runtime_error("(DemoScript::start) Root demo element not found");

		delete pdoc;
	}
	else
		throw std::invalid_argument("(DemoScript::start) Unable to open demo XML script");

	return true;
}

void DemoScript::startFadeAnim(float start, float end)
{
	AnimItem animItem(this);
	animItem.configSimpleAnim(start, end, FADE_ANIM_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idFadeAnim = MigUtil::theAnimList->addItem(animItem);
}

void DemoScript::startCursorAnim()
{
	// remember the tap event time stamps
	_startTime = Timer::gameTimeMillis();

	// create animations for the u/v lists
	AnimItem animItemU(this);
	animItemU.configParametricAnim(0, 1, _dur, _uArray, _numUV);
	_idAnimU = MigUtil::theAnimList->addItem(animItemU);

	AnimItem animItemV(this);
	animItemV.configParametricAnim(0, 1, _dur, _vArray, _numUV);
	_idAnimV = MigUtil::theAnimList->addItem(animItemV);
}

// loads a list of u/v coordinates (0->1) and tap events (time stamps)
void DemoScript::start(long dur, float* uArray, float* vArray, int numUV, long* tapEvents, int numTaps)
{
	if (uArray == nullptr || vArray == nullptr && tapEvents == nullptr || numUV == 0 || numTaps == 0)
		throw std::invalid_argument("(DemoScript::start) UV and/or taps array are empty");

	_uArray = uArray;
	_vArray = vArray;
	_numUV = numUV;
	_taps = tapEvents;
	_numTaps = numTaps;
	_dur = dur;
	_nextEvent = 0;

	// set the first position
	_u = uArray[0];
	_v = vArray[0];
	_alpha = 0;
	_isFingerDown = false;

	// start the fade in animation
	startFadeAnim(0, 1);

	_isStarted = true;
	_isDone = false;
	if (_callback != nullptr)
		_callback->onStartScript();
}

// updates the script for each frame
void DemoScript::update()
{
	// only send events up if the cursor animation has really started, and not just fading
	if (_idAnimU.isActive())
	{
		DEMO_EVENT evt = DEMO_EVENT_MOVE;

		// note the event if it's time for the finger to tap or release
		long elapsedTime = Timer::gameTimeMillis() - _startTime;
		if (_nextEvent < _numTaps && elapsedTime >= _taps[_nextEvent])
		{
			_nextEvent++;
			_isFingerDown = !_isFingerDown;

			evt = (_isFingerDown ? DEMO_EVENT_FINGER_DOWN : DEMO_EVENT_FINGER_UP);
		}

		if (_callback != nullptr)
			_callback->onMove(_u, _v, evt);
	}
}

// aborts the script
void DemoScript::stop()
{
	_idFadeAnim.clearAnim();
	_idAnimU.clearAnim();
	_idAnimV.clearAnim();
}

// callback used by the animation interface
bool DemoScript::doFrame(int id, float newVal, void* optData)
{
	if (_idAnimU == id)
		_u = newVal;
	else if (_idAnimV == id)
		_v = newVal;
	else if (_idFadeAnim == id)
		_alpha = newVal;
	return true;
}

// indicates that the given animation is complete
void DemoScript::animComplete(int id, void* optData)
{
	if (_idAnimU == id)
	{
		_idAnimU = 0;
	}
	else if (_idAnimV == id)
	{
		_idAnimV = 0;

		// start the fade out animation
		startFadeAnim(1, 0);
	}
	else if (_idFadeAnim == id)
	{
		// if this was a fade in then start the cursor animation
		if (_alpha > 0)
			startCursorAnim();
		else
		{
			_isDone = true;

			if (_callback != nullptr)
				_callback->onStopScript();
		}
		_idFadeAnim = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
// DemoScreen

DemoScreen::DemoScreen(const std::string& name) : ScreenBase(name), _script(this)
{
	// by default, the user will be locked out of interacting w/ the demo screens
	_userLockout = true;
	_fingerUpFrame = 0;
	_fingerDownFrame = 1;

	_lcList.addToList(_cursor);
	_cursor.setAlpha(0);
}

DemoScreen::~DemoScreen()
{
}

void DemoScreen::onStartScript()
{
	LOGINFO("(DemoScreen::onStartScript) Demo script is starting");

	_cursor.setPos(MigUtil::screenPercentToCameraPlane(_script.getU(), _script.getV()));
	_cursor.setAlpha(_script.getAlpha());
}

void DemoScreen::onMove(float u, float v, DEMO_EVENT evt)
{
	_cursor.setPos(MigUtil::screenPercentToCameraPlane(u, v));

	if (evt == DEMO_EVENT_FINGER_DOWN)
		_cursor.jumpToFrame((float)_fingerDownFrame);
	else if (evt == DEMO_EVENT_FINGER_UP)
		_cursor.jumpToFrame((float)_fingerUpFrame);

	if (_script.isFingerDown() || evt == DEMO_EVENT_FINGER_UP)
	{
		bool lockout = _userLockout;
		_userLockout = false;
		if (evt == DEMO_EVENT_FINGER_DOWN)
			pointerPressed(u, v);
		else if (evt == DEMO_EVENT_FINGER_UP)
			pointerReleased(u, v);
		else
			pointerMoved(u, v, true);
		_userLockout = lockout;
	}
}

void DemoScreen::onStopScript()
{
	LOGINFO("(DemoScreen::onStopScript) Demo script is complete");

	_cursor.setAlpha(0);
}

bool DemoScreen::pointerPressed(float x, float y)
{
	return (!_userLockout ? ScreenBase::pointerPressed(x, y) : false);
}

void DemoScreen::pointerReleased(float x, float y)
{
	if (!_userLockout)
		ScreenBase::pointerReleased(x, y);
}

void DemoScreen::pointerMoved(float x, float y, bool isInContact)
{
	if (!_userLockout)
		ScreenBase::pointerMoved(x, y, isInContact);
}

bool DemoScreen::update()
{
	bool ret = ScreenBase::update();

	// let the demo script update, if necessary
	if (_script.isStarted() && !_script.isDone())
	{
		_script.update();
		_cursor.setAlpha(_script.getAlpha());
	}
	return ret;
}
