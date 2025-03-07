#include "pch.h"
#include "../core/MigUtil.h"
#include "../core/Timer.h"
#include "AndroidApp.h"

using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// platform specific configuration bits

unsigned int plat_getBits()
{
	return PLAT_ANDROID;
}

///////////////////////////////////////////////////////////////////////////
// platform specific timer functions

static double startTime;
static double lastTime;
static double maxDelta;
static uint64 elapsedTicks;
static uint64 totalTicks;

bool plat_initTimer()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	startTime = ts.tv_sec + ts.tv_nsec / 1E9;

	//maxDelta = CLOCKS_PER_SEC / 10;
	maxDelta = 0.1;
	return true;
}

uint64 plat_getRawTicks()
{
	// query the current time
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double currentTime = ts.tv_sec + ts.tv_nsec / 1E9;

	// compute elapsed time since last update
	double timeDelta = currentTime - startTime;

	// convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
	timeDelta *= Timer::ticksPerSecond;
	return timeDelta;
}

uint64 plat_getCurrentTicks()
{
	// query the current time
	//clock_t currentTime = clock();
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	double currentTime = ts.tv_sec + ts.tv_nsec/1E9;
	//LOGDBG("currentTime is %f\n", currentTime);

	// compute elapsed time since last update
	double timeDelta = currentTime - lastTime;
	lastTime = currentTime;

	// clamp excessively large time deltas (e.g. after paused in the debugger)
	if (timeDelta > maxDelta)
		timeDelta = maxDelta;

	// convert clock units into a canonical tick format
	timeDelta *= Timer::ticksPerSecond;
	//timeDelta /= CLOCKS_PER_SEC;

	elapsedTicks = timeDelta;
	totalTicks += timeDelta;
	//LOGDBG("Elapsed time is %f\n", (currTimeTicks / (double) Timer::ticksPerSecond));
	return totalTicks;
}

///////////////////////////////////////////////////////////////////////////
// platform specific logging functions

#include <android/log.h>

#define  LOG_TAG    "migtech"

bool plat_isDebuggerPresent()
{
	// TODO: is there a way to determine if a debugger is attached?
	return true;
}

//#pragma warning(push)
//#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS
void plat_outputDebugString(const char* msg, int level)
{
	if (level == 0)
		__android_log_write(ANDROID_LOG_DEBUG, LOG_TAG, msg);
	else if (level == 1)
		__android_log_write(ANDROID_LOG_INFO, LOG_TAG, msg);
	else if (level == 2)
		__android_log_write(ANDROID_LOG_WARN, LOG_TAG, msg);
	else if (level == 3)
		__android_log_write(ANDROID_LOG_ERROR, LOG_TAG, msg);
	else if (level == 4)
		__android_log_write(ANDROID_LOG_FATAL, LOG_TAG, msg);
}
//#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// platform specific file IO utilities

byte* plat_loadFileBuffer(const char* filePath, int& length)
{
	// get the asset file size
	length = AndroidUtil_getAssetBuffer(filePath, nullptr, 0);
	if (length == -1)
	{
		LOGWARN("(::plat_loadFileBuffer) file '%s' doesn't exist\n", filePath);
		return nullptr;
	}
	//LOGDBG("(plat_loadFileBuffer) file '%s' size is %d\n", filePath, length);

	// load the image into a memory buffer
	byte* pFile = new byte[length];
	if (AndroidUtil_getAssetBuffer(filePath, pFile, length) == -1)
	{
		LOGWARN("(::plat_loadFileBuffer) image '%s' failed to load\n", filePath);
		delete [] pFile;
		return nullptr;
	}

	return pFile;
}

const std::string& plat_getFilesDir()
{
	return AndroidUtil_getFilesDir();
}

const std::string& plat_getExternalFilesDir()
{
	return AndroidUtil_getExtFilesDir();
}
