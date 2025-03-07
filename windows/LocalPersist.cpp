///////////////////////////////////////////////////////////////////////////
// platform specific

#include "pch.h"
#include "LocalPersist.h"

using namespace Windows::Storage;
using namespace Windows::Foundation;

using namespace MigTech;

// defined in Platform.cpp
extern bool toWString(const std::string& inStr, std::wstring& outStr);
extern bool toString(const std::wstring& inStr, std::string& outStr);

LocalPersist::LocalPersist()
{
}

bool LocalPersist::open()
{
	return true;
}

void LocalPersist::close()
{
}

// local utility to convert a normal string to a managed string
static Platform::String^ convertKey(const std::string& key)
{
	std::wstring wkey;
	toWString(key, wkey);
	return Platform::StringReference(wkey.c_str());
}

bool LocalPersist::putValue(const std::string& key, int value)
{
	if (key.length() == 0)
		return false;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	return values->Insert(convertKey(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(value)));
}

bool LocalPersist::putValue(const std::string& key, float value)
{
	if (key.length() == 0)
		return false;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	return values->Insert(convertKey(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
}

bool LocalPersist::putValue(const std::string& key, const std::string& value)
{
	if (key.length() == 0)
		return false;

	std::wstring wvalue;
	toWString(value, wvalue);

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	return values->Insert(convertKey(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateString(Platform::StringReference(wvalue.c_str()))));
}

int LocalPersist::getValue(const std::string& key, int def) const
{
	if (key.length() == 0)
		return def;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	Platform::Object^ refValue = localSettings->Values->Lookup(convertKey(key));
	if (refValue)
		return safe_cast<int>(refValue);
	return def;
}

float LocalPersist::getValue(const std::string& key, float def) const
{
	if (key.length() == 0)
		return def;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	Platform::Object^ refValue = localSettings->Values->Lookup(convertKey(key));
	if (refValue)
		return (float) safe_cast<double>(refValue);
	return def;
}

std::string LocalPersist::getValue(const std::string& key, const std::string& def) const
{
	if (key.length() == 0)
		return def;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	Platform::Object^ refValue = localSettings->Values->Lookup(convertKey(key));
	if (refValue)
	{
		std::string outStr;
		Platform::String^ refStr = safe_cast<Platform::String^>(refValue);
		toString(refStr->Data(), outStr);
		return outStr;
	}
	return def;
}

bool LocalPersist::deleteValue(const std::string& key)
{
	if (key.length() == 0)
		return false;

	ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
	auto values = localSettings->Values;
	values->Remove(convertKey(key));

	return true;
}

bool LocalPersist::commit()
{
	// nothing to do
	return true;
}
