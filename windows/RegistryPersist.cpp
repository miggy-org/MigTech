///////////////////////////////////////////////////////////////////////////
// platform specific

#include "pch.h"
#include "../core/MigUtil.h"
#include "RegistryPersist.h"

using namespace MigTech;

static HKEY OpenPersistKey(bool read)
{
	HKEY hKey;
	LONG ret = ::RegCreateKeyEx(
		HKEY_CURRENT_USER,
		L"Software\\Jordan Enterprises\\MigTech\\Persist",
		0, NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_QUERY_VALUE | KEY_SET_VALUE,
		NULL, &hKey, NULL);
	return hKey;
}

RegistryPersist::RegistryPersist() : _hKey(NULL)
{
}

bool RegistryPersist::open()
{
	_hKey = ::OpenPersistKey(true);
	return (_hKey != NULL);
}

void RegistryPersist::close()
{
	::RegCloseKey(_hKey);
	_hKey = NULL;
}

bool RegistryPersist::putValue(const std::string& key, int value)
{
	if (key.length() == 0)
		return false;

	if (::RegSetValueExA(_hKey, key.c_str(), 0, REG_DWORD, (LPBYTE)&value, sizeof(DWORD)) != ERROR_SUCCESS)
		LOGWARN("(RegistryPersist::putValue) Unable to write value %s", key.c_str());
	return true;
}

bool RegistryPersist::putValue(const std::string& key, float value)
{
	if (key.length() == 0)
		return false;

	if (::RegSetValueExA(_hKey, key.c_str(), 0, REG_BINARY, (LPBYTE)&value, sizeof(float)) != ERROR_SUCCESS)
		LOGWARN("(RegistryPersist::putValue) Unable to write value %s", key.c_str());
	return true;
}

bool RegistryPersist::putValue(const std::string& key, const std::string& value)
{
	if (key.length() == 0)
		return false;

	if (::RegSetValueExA(_hKey, key.c_str(), 0, REG_SZ, (LPBYTE)value.c_str(), (value.length() + 1)) != ERROR_SUCCESS)
		LOGWARN("(RegistryPersist::putValue) Unable to write value %s", key.c_str());
	return true;
}

int RegistryPersist::getValue(const std::string& key, int def) const
{
	if (key.length() == 0)
		return def;

	int retVal = def;
	DWORD type = REG_NONE;
	DWORD size = 0;
	::RegQueryValueExA(_hKey, key.c_str(), NULL, &type, NULL, &size);
	if (type == REG_DWORD && size == sizeof(DWORD))
		::RegQueryValueExA(_hKey, key.c_str(), NULL, NULL, (LPBYTE)&retVal, &size);

	return retVal;
}

float RegistryPersist::getValue(const std::string& key, float def) const
{
	if (key.length() == 0)
		return def;

	float retVal = def;
	DWORD type = REG_NONE;
	DWORD size = 0;
	::RegQueryValueExA(_hKey, key.c_str(), NULL, &type, NULL, &size);
	if (type == REG_BINARY)
		::RegQueryValueExA(_hKey, key.c_str(), NULL, NULL, (LPBYTE)&retVal, &size);

	return retVal;
}

std::string RegistryPersist::getValue(const std::string& key, const std::string& def) const
{
	if (key.length() == 0)
		return def;

	std::string retVal = def;
	DWORD type = REG_NONE;
	DWORD size = 0;
	::RegQueryValueExA(_hKey, key.c_str(), NULL, &type, NULL, &size);
	if (type == REG_SZ)
	{
		char* buf = new char[size];
		if (::RegQueryValueExA(_hKey, key.c_str(), NULL, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
			retVal = buf;
		delete[] buf;
	}

	return retVal;
}

bool RegistryPersist::deleteValue(const std::string& key)
{
	if (key.length() == 0)
		return false;

	::RegDeleteValueA(_hKey, key.c_str());
	return true;
}

bool RegistryPersist::commit()
{
	::RegFlushKey(_hKey);
	return true;
}
