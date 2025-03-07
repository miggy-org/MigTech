#pragma once

#include "MigConst.h"
#include "MigGame.h"
#include "RenderBase.h"
#include "AudioBase.h"
#include "PersistBase.h"
#include "Dialog.h"

namespace MigTech
{
	class MigUtil
	{
	public:

	///////////////////////////////////////////////////////////////////////////
	// globals

		// global game that is currently running
		static MigGame* theGame;

		// real time renderer
		static RenderBase* theRend;

		// audio manager
		static AudioBase* theAudio;

		// global music handle
		static SoundEffect* theMusic;

		// persistent data manager
		static PersistBase* thePersist;

		// animation list for the current screen
		static AnimList* theAnimList;

		// default font
		static Font* theFont;

		// singleton dialog, when it's on-screen and active
		static DialogBase* theDialog;

	///////////////////////////////////////////////////////////////////////////
	// init MigUtil

		static bool init();

	///////////////////////////////////////////////////////////////////////////
	// logging

		static void debug(const char* msg, ...);  // use LOGD below
		static void info(const char* msg, ...);
		static void warn(const char* msg, ...);
		static void error(const char* msg, ...);
		static void fatal(const char* msg, ...);

		static bool dumpLogToFile(const char* msg = nullptr);
		static bool dumpLogFileToDebugger();
		static bool dumpLogFileToExternalStorage();
		static bool dumpLogFileToStrings(std::vector<std::string>& log);
		static bool dumpLogFileExists();
		static bool deleteLogFile();

	///////////////////////////////////////////////////////////////////////////
	// watchdog

		static void setWatchdog(int periodInSeconds = -1, int maxDelayInSeconds = 1);
		static int getWatchdogPeriod();
		static void petWatchdog();
		static bool checkWatchdog();
		static void suspendWatchdog();

	///////////////////////////////////////////////////////////////////////////
	// conversion

		static float convertToRadians(float degrees);
		static float convertToDegrees(float radians);

		// camera plane is defined as the default (-1,1) / (-1,1)
		static Vector3 screenPercentToCameraPlane(float u, float v);
		static float screenPercentWidthToCameraPlane(float w);
		static float screenPercentHeightToCameraPlane(float h);
		static float cameraPlaneWidthToScreenPercent(float w);
		static float cameraPlaneHeightToScreenPercent(float h);
		static Vector2 cameraPlaneToScreenPercent(const Vector3& pt);
		static Rect screenPercentToRectangle(float u, float v, float w, float h);

		static const char* intToString(int val);
		static const char* intToPaddedString(int val, int padding);

	///////////////////////////////////////////////////////////////////////////
	// parsing

		static int parseInt(const char* intStr, int def);
		static float parseFloat(const char* floatStr, float def);
		static bool parseBool(const char* boolStr, bool def);
		static int parseFloats(const char* floatStr, float* fArray, int len);
		static Color parseColorString(const char* colorStr, const Color& def);

	///////////////////////////////////////////////////////////////////////////
	// localization and string handling

		static std::string getString(const std::string& name, const std::string& def = "");
		static std::string toUpper(const std::string& input);

	///////////////////////////////////////////////////////////////////////////
	// other

		static bool rollAgainstPercent(int percent);
		static int pickRandom(int max);
		static float pickFloat();
		static Color blendColors(const Color& col1, const Color& col2, float blend);
	};

// macros for logging messages
#ifdef NDEBUG
#define LOGDBG(msg, ...)
#else
#define LOGDBG(msg, ...) MigUtil::debug(msg, ##__VA_ARGS__)
#endif // NDEBUG
#define LOGINFO(msg, ...) MigUtil::info(msg, ##__VA_ARGS__)
#define LOGWARN(msg, ...) MigUtil::warn(msg, ##__VA_ARGS__)
#define LOGERR(msg, ...) MigUtil::error(msg, ##__VA_ARGS__)
}