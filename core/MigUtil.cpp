#include "pch.h"
#include "MigUtil.h"
#include "MigConst.h"
#include "Timer.h"

#include <time.h>

using namespace MigTech;
using namespace tinyxml2;

// static declarations
MigGame* MigUtil::theGame = nullptr;
RenderBase* MigUtil::theRend = nullptr;
AudioBase* MigUtil::theAudio = nullptr;
SoundEffect* MigUtil::theMusic = nullptr;
PersistBase* MigUtil::thePersist = nullptr;
AnimList* MigUtil::theAnimList = nullptr;
Font* MigUtil::theFont = nullptr;
DialogBase* MigUtil::theDialog = nullptr;

///////////////////////////////////////////////////////////////////////////
// volatile const definitions (see compiler error C2864)

//const float MigConst::PI = 3.141592654f;

///////////////////////////////////////////////////////////////////////////
// platform specific

extern uint64 plat_getRawTicks();
extern bool plat_isDebuggerPresent();
extern void plat_outputDebugString(const char* msg, int level);
extern const std::string& plat_getFilesDir();
extern const std::string& plat_getExternalFilesDir();

///////////////////////////////////////////////////////////////////////////
// logging

// remove this to disable log tracking
#define LOG_TRACKING

#define LOG_NUM_LINES   100
#define LOG_LINE_LENGTH 256
#define LOG_CRASH_DUMP_FILE "mtlog.txt"

#ifdef LOG_TRACKING
// static list of the most recent <X> logging lines
static char logBufs[LOG_NUM_LINES][LOG_LINE_LENGTH];
static int lastLine = 0;
#endif // LOG_TRACKING

bool MigUtil::init()
{
#ifdef LOG_TRACKING
	for (int i = 0; i < LOG_NUM_LINES; i++)
		logBufs[i][0] = 0;
#endif // LOG_TRACKING
	return true;
}

static char* getLoggingBuf()
{
#ifdef LOG_TRACKING
	// move to the next available line
	char* outbuf = logBufs[lastLine];
	lastLine = (++lastLine % LOG_NUM_LINES);
#else
	static char outbuf[LOG_LINE_LENGTH];
#endif // LOG_TRACKING
	return outbuf;
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

void MigUtil::debug(const char* msg, ...)
{
#ifndef NDEBUG
	if (plat_isDebuggerPresent())
	{
		// note that these don't appear in crash dumps
		static char outbuf[1024];

		va_list args;
		va_start(args, msg);
		vsprintf(outbuf, msg, args);
		va_end(args);

		plat_outputDebugString(outbuf, 0);
	}
#endif // !NDEBUG
}

void MigUtil::info(const char* msg, ...)
{
	char* outbuf = getLoggingBuf();

	va_list args;
	va_start(args, msg);
	vsprintf(outbuf, msg, args);
	va_end(args);

	if (plat_isDebuggerPresent())
		plat_outputDebugString(outbuf, 1);
}

void MigUtil::warn(const char* msg, ...)
{
	char* outbuf = getLoggingBuf();

	va_list args;
	va_start(args, msg);
	vsprintf(outbuf, msg, args);
	va_end(args);

	if (plat_isDebuggerPresent())
		plat_outputDebugString(outbuf, 2);
}

void MigUtil::error(const char* msg, ...)
{
	char* outbuf = getLoggingBuf();

	va_list args;
	va_start(args, msg);
	vsprintf(outbuf, msg, args);
	va_end(args);

	if (plat_isDebuggerPresent())
		plat_outputDebugString(outbuf, 3);
}

void MigUtil::fatal(const char* msg, ...)
{
	char* outbuf = getLoggingBuf();

	va_list args;
	va_start(args, msg);
	vsprintf(outbuf, msg, args);
	va_end(args);

	if (plat_isDebuggerPresent())
		plat_outputDebugString(outbuf, 4);
}

static std::string composeLogDumpFilePath()
{
	std::string path = plat_getFilesDir();
	path += "/";
	path += LOG_CRASH_DUMP_FILE;
	return path;
}

bool MigUtil::dumpLogToFile(const char* msg)
{
#ifdef LOG_TRACKING
	if (msg != nullptr)
		error(msg);

	std::string path = composeLogDumpFilePath();
	FILE* pf = fopen(path.c_str(), "w");
	if (pf == nullptr)
	{
		LOGWARN("(MigUtil::dumpLogsToFile) Could not open log file %s", path.c_str());
		return false;
	}

	for (int i = 0; i < LOG_NUM_LINES; i++)
	{
		int index = (lastLine + i) % LOG_NUM_LINES;
		if (logBufs[index][0] != 0)
		{
			fputs(logBufs[index], pf);
			fputs("\n", pf);
		}
	}

	fclose(pf);
	return true;
#else
	return false;
#endif // LOG_TRACKING
}

bool MigUtil::dumpLogFileToDebugger()
{
#ifdef LOG_TRACKING
	std::string path = composeLogDumpFilePath();
	FILE* pf = fopen(path.c_str(), "r");
	if (pf == nullptr)
	{
		LOGWARN("(MigUtil::dumpLogFileToDebugger) Could not open log file (%s)", path.c_str());
		return false;
	}
	LOGDBG("----- LOG DUMP FOLLOWS -----");

	char line[LOG_LINE_LENGTH+1];
	while (fgets(line, LOG_LINE_LENGTH, pf) != nullptr)
	{
		int len = strlen(line);
		if (len > 0)
		{
			if (line[len - 1] == '\n')
				line[len - 1] = 0;
			LOGDBG(line);
		}
	}
	fclose(pf);

	LOGDBG("----- LOG DUMP COMPLETE -----");
	return true;
#else
	return false;
#endif // LOG_TRACKING
}

bool MigUtil::dumpLogFileToExternalStorage()
{
#ifdef LOG_TRACKING
	std::string path = composeLogDumpFilePath();
	FILE* pf = fopen(path.c_str(), "r");
	if (pf == nullptr)
	{
		LOGWARN("(MigUtil::dumpLogFileToExternalStorage) Could not open log file (%s)", path.c_str());
		return false;
	}

	std::string opath = plat_getExternalFilesDir();
	if (opath.length() == 0)
	{
		LOGWARN("(MigUtil::dumpLogFileToExternalStorage) External storage not available");
		return false;
	}
	opath += "/";
	opath += LOG_CRASH_DUMP_FILE;

	FILE* pof = fopen(opath.c_str(), "w");
	if (pof == nullptr)
	{
		LOGWARN("(MigUtil::dumpLogFileToExternalStorage) Could not open external log file (%s)", opath.c_str());
		return false;
	}

	char line[LOG_LINE_LENGTH + 1];
	while (fgets(line, LOG_LINE_LENGTH, pf) != nullptr)
		fputs(line, pof);
	fclose(pof);
	fclose(pf);
	return true;
#else
	return false;
#endif // LOG_TRACKING
}

bool MigUtil::dumpLogFileToStrings(std::vector<std::string>& log)
{
#ifdef LOG_TRACKING
	std::string path = composeLogDumpFilePath();
	FILE* pf = fopen(path.c_str(), "r");
	if (pf == nullptr)
	{
		LOGWARN("(MigUtil::dumpLogFileToStrings) Could not open log file (%s)", path.c_str());
		return false;
	}

	char line[LOG_LINE_LENGTH + 1];
	while (fgets(line, LOG_LINE_LENGTH, pf) != nullptr)
		log.push_back(line);
	fclose(pf);
	return true;
#else
	return false;
#endif // LOG_TRACKING
}

bool MigUtil::dumpLogFileExists()
{
#ifdef LOG_TRACKING
	std::string path = composeLogDumpFilePath();
	FILE* pf = fopen(path.c_str(), "r");
	if (pf == nullptr)
		return false;
	fclose(pf);
	return true;
#else
	return false;
#endif // LOG_TRACKING
}

bool MigUtil::deleteLogFile()
{
#ifdef LOG_TRACKING
	std::string path = composeLogDumpFilePath();
	int ret = remove(path.c_str());
	return (ret == 0);
#endif // LOG_TRACKING
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// watchdog

// maximum amount of time that can elapse
static long maxWatchdogDelay = 1000;
static int watchdogPeriod = -1;
static uint64 lastWatchdogUpdate = 0;

void MigUtil::setWatchdog(int periodInSeconds, int maxDelayInSeconds)
{
	watchdogPeriod = periodInSeconds;
	maxWatchdogDelay = 1000 * maxDelayInSeconds;
}

int MigUtil::getWatchdogPeriod()
{
	return watchdogPeriod;
}

void MigUtil::petWatchdog()
{
	// i believe this is an atomic operation and thus doesn't need a lock
	lastWatchdogUpdate = plat_getRawTicks();
}

bool MigUtil::checkWatchdog()
{
	// don't start checking if the watchdog is suspended
	if (lastWatchdogUpdate == 0)
		return true;

	// need to call the platform current ticks directly
	uint64 diffTime = plat_getRawTicks() - lastWatchdogUpdate;
	return (Timer::ticksToMilliSeconds(diffTime) <= maxWatchdogDelay);
}

void MigUtil::suspendWatchdog()
{
	lastWatchdogUpdate = 0;
}

///////////////////////////////////////////////////////////////////////////
// conversion

float MigUtil::convertToRadians(float degrees)
{
	return (float) (degrees * MigTech::PI / 180);
}

float MigUtil::convertToDegrees(float radians)
{
	return (float) (180 * radians / MigTech::PI);
}

Vector3 MigUtil::screenPercentToCameraPlane(float u, float v)
{
	return Vector3(-1 + 2 * u, -1 + 2 * (1 - v), 0);
}

float MigUtil::screenPercentWidthToCameraPlane(float w)
{
	return 2 * w;
}

float MigUtil::screenPercentHeightToCameraPlane(float h)
{
	return 2 * h;
}

float MigUtil::cameraPlaneWidthToScreenPercent(float w)
{
	return w / 2.0f;
}

float MigUtil::cameraPlaneHeightToScreenPercent(float h)
{
	return h / 2.0f;
}

Vector2 MigUtil::cameraPlaneToScreenPercent(const Vector3& pt)
{
	return Vector2((pt.x + 1) / 2.0f, (-pt.y + 1) / 2.0f);
}

// assumes the u/v is the center of the rectangle
Rect MigUtil::screenPercentToRectangle(float u, float v, float w, float h)
{
	return Rect(u - w / 2, v - h / 2, w, h);
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

const char* MigUtil::intToString(int val)
{
	static char buf[16];
	sprintf(buf, "%d", val);
	return buf;
}

const char* MigUtil::intToPaddedString(int val, int padding)
{
	static char buf[16], bufIntOnly[16];
	sprintf(bufIntOnly, "%d", val);
	int len = strlen(bufIntOnly);
	int i;
	for (i = 0; i < (padding - len); i++)
		buf[i] = '0';
	buf[i] = 0;
	strcat(buf, bufIntOnly);
	return buf;
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// parsing

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

int MigUtil::parseInt(const char* intStr, int def)
{
	return (int)(intStr != nullptr && intStr[0] ? atoi(intStr) : def);
}

float MigUtil::parseFloat(const char* floatStr, float def)
{
	return (float)(floatStr != nullptr && floatStr[0] ? atof(floatStr) : def);
}

bool MigUtil::parseBool(const char* boolStr, bool def)
{
	if (boolStr != nullptr)
	{
		if (!strcmp(boolStr, "true"))
			return true;
		else if (!strcmp(boolStr, "false"))
			return false;
	}
	return def;
}

int MigUtil::parseFloats(const char* floatStr, float* fArray, int len)
{
	static char tmp[256];

	int count = 0;
	if (floatStr != nullptr)
	{
		strcpy(tmp, floatStr);

		for (int i = 0; i < len; i++)
		{
			const char* pstr = strtok((i == 0 ? tmp : nullptr), ", ");
			fArray[i] = (float)(pstr != nullptr ? atof(pstr) : 0);
			if (pstr != nullptr)
				count++;
		}
	}
	else
		memset(fArray, 0, len*sizeof(float));
	return count;
}

Color MigUtil::parseColorString(const char* colorStr, const Color& def)
{
	float vals[4];
	int found = parseFloats(colorStr, vals, 4);
	return (found > 0 ? Color(vals[0], vals[1], vals[2], (found == 4 ? vals[3] : 1)) : def);
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// localization and string handling

// holds all localized strings
static std::map<std::string, std::string> _stringsMap;

std::string MigUtil::getString(const std::string& name, const std::string& def)
{
	if (_stringsMap.empty())
	{
		// TODO: when appropriate, need to support multiple languages
		std::string stringsDocPath = "strings.xml";

		tinyxml2::XMLDocument* stringsDoc = XMLDocFactory::loadDocument(stringsDocPath);
		if (stringsDoc != nullptr)
		{
			XMLElement* elem = stringsDoc->FirstChildElement("resources");
			if (elem != nullptr)
			{
				XMLElement* item = elem->FirstChildElement("string");
				while (item != nullptr)
				{
					std::string sname = item->Attribute("name");
					std::string value = item->GetText();
					if (sname.length() > 0 && value.length() > 0)
						_stringsMap[sname] = value;

					item = item->NextSiblingElement("string");
				}
			}
			else
				throw std::runtime_error("(MigUtil::getString) Resources tree not found");

			delete stringsDoc;
		}
		else
			throw std::runtime_error("(MigUtil::getString) Failed to load strings.xml");
	}

	if (name.empty())
		return def;

	std::map<std::string, std::string>::const_iterator item = _stringsMap.find(name);
	if (item == _stringsMap.end())
		return def;
	return item->second;
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

std::string MigUtil::toUpper(const std::string& input)
{
	static char buf[1024];
	strcpy(buf, input.c_str());

	for (char* tmp = buf; *tmp; ++tmp)
		*tmp = toupper((unsigned char)*tmp);
	return buf;
}

#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// other

bool MigUtil::rollAgainstPercent(int percent)
{
	return (pickRandom(100) < percent);
}

int MigUtil::pickRandom(int max)
{
	if (max == 0)
		return 0;

	static bool randInit = false;
	if (!randInit)
	{
		srand((unsigned)time(nullptr));
		randInit = true;
	}
	return rand() % max;
}

float MigUtil::pickFloat()
{
	return (pickRandom(10000) / 10000.0f);
}

Color MigUtil::blendColors(const Color& col1, const Color& col2, float blend)
{
	Color blendColor;
	blendColor.r = col1.r + (col2.r - col1.r)*blend;
	blendColor.g = col1.g + (col2.g - col1.g)*blend;
	blendColor.b = col1.b + (col2.b - col1.b)*blend;
	blendColor.a = col1.a + (col2.a - col1.a)*blend;
	return blendColor;
}
