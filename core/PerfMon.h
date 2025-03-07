#pragma once

#include "MigDefines.h"
#include "Font.h"

namespace MigTech
{
	class PerfMon
	{
	private:
		// display the FPS
		static bool showFramesPerSecond;

		// delay between updates to the FPS display
		static long delayFramesPerSecond;

		// FPS counter
		static Text* lastFPS;
		static uint64 startTimeStamp;
		static uint64 lastTimeStamp;
		static long frameCount;
		static long totalFrameCount;
		static double bestFPS;
		static double worstFPS;

	public:
		// static interface, used by Game class
		static void init(Font* font);
		static void showFPS(bool show);
		static bool isFPSOn();
		static void doFPS();
		static void doReport();

	public:
		PerfMon();
		PerfMon(int numSlices);
		~PerfMon();

		// interface for slice performance testing
		void initSlicePerfTest(int numSlices);
		void startSlicePerfTest();
		void endSlicePerfTest();

	protected:
		// variables for tracking slice performance tests
		long* _perfTimeList;
		int _numSlices;
		uint64 _lastPerfStamp;
		int _totalTimeCount;
		int _currTimeIndex;
	};
}