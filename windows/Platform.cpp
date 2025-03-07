#include "pch.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../core/MigUtil.h"
#include "../core/Timer.h"

using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// platform specific configuration bits

unsigned int plat_getBits()
{
#ifdef _WINDOWS
	// _WINDOWS actually refers to desktop Windows, confusing I know...
	return PLAT_DESKTOP;
#else
	return PLAT_WINDOWS;
#endif // _WINDOWS
}

///////////////////////////////////////////////////////////////////////////
// platform specific timer functions

static LARGE_INTEGER qpcFrequency;
static LARGE_INTEGER qpcStartTime;
static LARGE_INTEGER qpcLastTime;
static uint64 qpcMaxDelta;
static uint64 elapsedTicks;
static uint64 totalTicks;

bool plat_initTimer()
{
	QueryPerformanceFrequency(&qpcFrequency);
	QueryPerformanceCounter(&qpcStartTime);
	qpcLastTime = qpcStartTime;

	// initialize max delta to 1/10 of a second.
	qpcMaxDelta = qpcFrequency.QuadPart / 10;

	return true;
}

uint64 plat_getRawTicks()
{
	// query the current time
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	// compute elapsed time since last update
	uint64 timeDelta = currentTime.QuadPart - qpcStartTime.QuadPart;

	// convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
	timeDelta *= Timer::ticksPerSecond;
	timeDelta /= qpcFrequency.QuadPart;
	return timeDelta;
}

uint64 plat_getCurrentTicks()
{
	// query the current time
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	// compute elapsed time since last update
	uint64 timeDelta = currentTime.QuadPart - qpcLastTime.QuadPart;
	qpcLastTime = currentTime;

	// clamp excessively large time deltas (e.g. after paused in the debugger)
	if (timeDelta > qpcMaxDelta)
		timeDelta = qpcMaxDelta;

	// convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
	timeDelta *= Timer::ticksPerSecond;
	timeDelta /= qpcFrequency.QuadPart;

	elapsedTicks = timeDelta;
	totalTicks += timeDelta;
	return totalTicks;
}

///////////////////////////////////////////////////////////////////////////
// platform specific logging functions

bool plat_isDebuggerPresent()
{
	return (IsDebuggerPresent() ? true : false);
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS
void plat_outputDebugString(const char* msg, int level)
{
	static char prefix[8] = { 0 };

	if (prefix[0] == 0)
	{
		prefix[0] = '[';
		//prefix[1] = ((level == 0 ? 'd' : (level == 3 ? 'e' : 'w')));
		prefix[2] = ']';
		prefix[3] = ' ';
		prefix[4] = 0;
	}
	switch (level)
	{
	case 0: prefix[1] = 'd'; break;
	case 1: prefix[1] = 'i'; break;
	case 2: prefix[1] = 'w'; break;
	case 3: prefix[1] = 'e'; break;
	case 4: prefix[1] = 'f'; break;
	default: prefix[1] = '?'; break;
	}

	OutputDebugStringA(prefix);
	OutputDebugStringA(msg);
	OutputDebugStringA("\n");
}
#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////
// platform specific string utilities

// wstring conversion
bool toWString(const std::string& inStr, std::wstring& outStr)
{
	int reqSize = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, inStr.c_str(), -1, nullptr, 0);
	if (reqSize > 0)
	{
		wchar_t* temp = new wchar_t[reqSize];
		if (MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, inStr.c_str(), -1, temp, reqSize) == reqSize)
		{
			outStr.assign(temp);
			delete temp;
			return true;
		}
	}

	return false;
}

// string conversion
bool toString(const std::wstring& inStr, std::string& outStr)
{
	int reqSize = WideCharToMultiByte(CP_ACP, 0, inStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (reqSize > 0)
	{
		char* temp = new char[reqSize];
		if (WideCharToMultiByte(CP_ACP, 0, inStr.c_str(), -1, temp, reqSize, nullptr, nullptr) == reqSize)
		{
			outStr.assign(temp);
			delete temp;
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////
// platform specific file IO utilities

std::vector<std::string> dtContentFolders;
std::vector<std::string> dtShaderFolders;
static std::string dtEmpty;

const std::string& plat_getContentDir(int index)
{
	if (index < (int)dtContentFolders.size())
		return dtContentFolders[index];
	return dtEmpty;
}

const std::string& plat_getShaderDir(int index)
{
	if (index < (int)dtShaderFolders.size())
		return dtShaderFolders[index];
	return dtEmpty;
}

const std::string& plat_getFilesDir()
{
	static std::string localDir;
	if (localDir.length() == 0)
	{
#ifdef _WINDOWS
		char localPath[MAX_PATH];
		::GetCurrentDirectoryA(MAX_PATH, localPath);
		localDir = localPath;
#else
		Platform::String^ localPath = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
		toString(localPath->Data(), localDir);
#endif // _WINDOWS
	}
	return localDir;
}

const std::string& plat_getExternalFilesDir()
{
	static std::string externalDir;
	// there is no external files directory (SD card) on desktop
#ifndef _WINDOWS
	if (externalDir.length() == 0)
	{
		try
		{
			// this will throw an exception if the capabilities aren't present
			Platform::String^ extPath = Windows::Storage::KnownFolders::RemovableDevices->Path;
			if (extPath != nullptr)
				toString(extPath->Data(), externalDir);
		}
		catch (...)
		{
			// removable storage isn't accessible
		}
	}
#endif  // !_WINDOWS
	return externalDir;
}

byte* plat_loadFileBuffer(const char* filePath, int& length)
{
	byte* pFile = nullptr;
	length = 0;

	// compose the full path name
	std::string fullPath = plat_getContentDir(0) + filePath;

	// retrieve the file size
	struct _stat st;
	if (_stat(fullPath.c_str(), &st) == 0)
	{
		//LOGDBG("(::plat_loadFileBuffer) File %s size is %d", filePath, st.st_size);
		length = st.st_size;

		// allocate a buffer
		pFile = new byte[length];
		if (pFile != nullptr)
		{
			FILE* pf = nullptr;
			if (fopen_s(&pf, fullPath.c_str(), "rb") == 0)
			{
				if (fread_s(pFile, length, 1, length, pf) < (size_t)length)
				{
					LOGWARN("(::plat_loadFileBuffer) Could not read file %s", filePath);
					delete pFile;
					pFile = nullptr;
				}

				fclose(pf);
			}
			else
			{
				LOGWARN("(::plat_loadFileBuffer) Could not open file %s", filePath);
				delete pFile;
				pFile = nullptr;
			}
		}
		else
		{
			LOGWARN("(::plat_loadFileBuffer) Could not allocate a buffer for %s", filePath);
		}
	}
	else
	{
		LOGWARN("(::plat_loadFileBuffer) Unable to determine size of %s", filePath);
	}

	return pFile;
}
