#include "pch.h"
#include "PerfMon.h"
#include "MigUtil.h"
#include "Timer.h"

using namespace MigTech;

// static initialization
bool PerfMon::showFramesPerSecond = false;
long PerfMon::delayFramesPerSecond = 1000;
Text* PerfMon::lastFPS = nullptr;
uint64 PerfMon::startTimeStamp = 0;
uint64 PerfMon::lastTimeStamp = 0;
long PerfMon::frameCount = 0;
long PerfMon::totalFrameCount = 0;
double PerfMon::bestFPS = 0;
double PerfMon::worstFPS = 0;

void PerfMon::init(Font* font)
{
	lastFPS = nullptr;

	if (font != nullptr)
	{
		lastFPS = font->createText();
		lastFPS->init("", 0.85f, 0.95f, 0.05f, JUSTIFY_LEFT);
	}
}

void PerfMon::showFPS(bool show)
{
	showFramesPerSecond = show;
}

bool PerfMon::isFPSOn()
{
	return showFramesPerSecond;
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

// not thread safe
static char* formatFPSString(double fps)
{
	static char temp[40];
	sprintf(temp, "%f", fps);

	char* dot = strchr(temp, '.');
	if (dot != nullptr)
		*(dot + 2) = 0;
	return temp;
}

#pragma warning(pop)

void PerfMon::doFPS()
{
	totalFrameCount++;
	if (startTimeStamp == 0)
		startTimeStamp = Timer::gameTime();

	// are we even keeping track
	if (showFramesPerSecond)
	{
		// reset the modeling matrix
		MigUtil::theRend->setProjectionMatrix(nullptr);
		MigUtil::theRend->setViewMatrix(nullptr);

		// increment the current frame count
		frameCount++;

		// make sure we've started keeping track
		uint64 gameTime = Timer::gameTime();
		if (lastTimeStamp > 0)
		{
			// has at least 1 second passed
			long diff = Timer::ticksToMilliSeconds(gameTime - lastTimeStamp);
			if (diff >= delayFramesPerSecond)
			{
				// compute the new FPS
				double fps = (1000 * frameCount) / (double)diff;
				if (lastFPS != nullptr)
					lastFPS->update(formatFPSString(fps));

				// reset
				lastTimeStamp = gameTime;
				frameCount = 0;

				// keep track of best/worst
				if (fps < worstFPS || worstFPS == 0)
					worstFPS = fps;
				if (fps > bestFPS)
					bestFPS = fps;
				//LOGDBG("FPS: %s", formatFPSString(fps));
			}
		}
		else
			lastTimeStamp = Timer::gameTime();

		// draw the most recent value
		if (lastFPS != nullptr)
			lastFPS->draw(0.5f, 0.5f, 0.5f, 1);
	}
}

void PerfMon::doReport()
{
	// spit out a report
	long diff = Timer::ticksToMilliSeconds(Timer::gameTime() - startTimeStamp);
	double aveFPS = (1000 * totalFrameCount) / (double)diff;
	LOGINFO("(PerfMon::doReport) Average FPS was %s", formatFPSString(aveFPS));
	if (bestFPS > 0)
		LOGINFO("(PerfMon::doReport) Best FPS was %s", formatFPSString(bestFPS));
	if (worstFPS > 0)
		LOGINFO("(PerfMon::doReport) Worst FPS was %s", formatFPSString(worstFPS));

	// reset everything that's global
	startTimeStamp = 0;
	totalFrameCount = 0;
	bestFPS = worstFPS = 0;
	frameCount = 0;
	lastTimeStamp = 0;
}

///////////////////////////////////////////////////////////////////////////
// private performance monitor

PerfMon::PerfMon()
{
	_perfTimeList = nullptr;
	_numSlices = 0;
}

PerfMon::PerfMon(int numSlices)
{
	_perfTimeList = nullptr;
	initSlicePerfTest(numSlices);
}

PerfMon::~PerfMon()
{
	if (_perfTimeList != nullptr)
		delete[] _perfTimeList;
}

void PerfMon::initSlicePerfTest(int numSlices)
{
	if (_perfTimeList != nullptr)
		delete[] _perfTimeList;
	_perfTimeList = new long[numSlices];
	_numSlices = numSlices;
	_currTimeIndex = 0;
}

void PerfMon::startSlicePerfTest()
{
	_lastPerfStamp = Timer::gameTime();
}

void PerfMon::endSlicePerfTest()
{
	if (_perfTimeList != nullptr)
	{
		_perfTimeList[_currTimeIndex] = Timer::ticksToMilliSeconds(Timer::gameTime() - _lastPerfStamp);
		_totalTimeCount++;

		long totalTime = 0;
		int count = 0;
		for (int i = 0; i < _numSlices; i++)
		{
			if (_perfTimeList[i] > 0)
			{
				totalTime += _perfTimeList[i];
				count++;
			}
		}
		LOGWARN("(PerfMon::endSlicePerfTest) slice = %d ms, ave = %f ms (%d samples out of %d)", _perfTimeList[_currTimeIndex], (totalTime / (double)count), count, _totalTimeCount);

		_currTimeIndex++;
		if (_currTimeIndex >= _numSlices)
			_currTimeIndex = 0;
	}
}
