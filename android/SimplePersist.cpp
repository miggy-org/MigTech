///////////////////////////////////////////////////////////////////////////
// platform specific

#include "pch.h"
#include "../core/MigUtil.h"
#include "AndroidApp.h"
#include "SimplePersist.h"

using namespace MigTech;

#define MAX_KEY_LENGTH			128
#define MAX_VAL_LENGTH			1024

#define DATA_FILE_NAME			"mtData.txt"
#define DATA_FILE_HDR			"MIG_TECH_DATA_1\n"
#define DATA_FILE_ENTRY_LEN		(MAX_KEY_LENGTH + MAX_VAL_LENGTH + 8)

static std::string composeDataFilePath()
{
	std::string path = AndroidUtil_getFilesDir();
	path += "/";
	path += DATA_FILE_NAME;
	return path;
}

SimplePersist::SimplePersist()
{
}

bool SimplePersist::open()
{
	//MigUtil::debug("(SimplePersist::open) Starting");

	std::string path = composeDataFilePath();
	FILE* pf = fopen(path.c_str(), "r");
	if (pf == nullptr)
	{
		// this is ok on first run
		LOGWARN("(SimplePersist::open) Could not open data file (%s)", path.c_str());
		return true;
	}
	LOGINFO("(SimplePersist::open) Data file '%s' opened", path.c_str());

	char buf[DATA_FILE_ENTRY_LEN];
	if (fgets(buf, DATA_FILE_ENTRY_LEN, pf) == nullptr)
	{
		LOGWARN("(SimplePersist::open) Unable to read header");
		fclose(pf);
		return true;
	}
	if (strcmp(buf, DATA_FILE_HDR))
	{
		LOGWARN("(SimplePersist::open) Could not recognize header");
		fclose(pf);
		return true;
	}

	while (fgets(buf, DATA_FILE_ENTRY_LEN, pf) != nullptr)
	{
		const char* key = strtok(buf, "\t\n");
		//MigUtil::debug("(SimplePersist::open) Key = '%s'", key);
		const char* type = strtok(nullptr, "\t\n");
		//MigUtil::debug("(SimplePersist::open) Type = '%s'", type);
		const char* val = strtok(nullptr, "\t\n");
		//MigUtil::debug("(SimplePersist::open) Val = '%s'", val);

		if (key != nullptr && type != nullptr && val != nullptr)
		{
			int itype = atoi(type);
			if (itype == (int)SIMPLE_VALUE_INT)
				putValue(key, (int) atoi(val));
			else if (itype == (int)SIMPLE_VALUE_FLOAT)
				putValue(key, (float) atof(val));
			else if (itype == (int)SIMPLE_VALUE_STRING)
				putValue(key, val);
			else
				LOGWARN("(SimplePersist::open) Unrecognized value type found (%s, %d)", key, itype);
		}
	}

	fclose(pf);
	LOGDBG("(SimplePersist::open) Complete");
	return true;
}

void SimplePersist::close()
{
	// just in case
	commit();
}

bool SimplePersist::putValue(const std::string& key, int value)
{
	//MigUtil::debug("(SimplePersist::putValue) Adding %s : %d", key.c_str(), value);
	if (key.length() == 0 || key.length() > MAX_KEY_LENGTH)
		return false;

	SimpleValue& val = _mapValues[key];
	val.type = SIMPLE_VALUE_INT;
	val.intValue = value;
	val.floatValue = 0;
	val.stringValue.clear();
	return true;
}

bool SimplePersist::putValue(const std::string& key, float value)
{
	//MigUtil::debug("(SimplePersist::putValue) Adding %s : %f", key.c_str(), value);
	if (key.length() == 0 || key.length() > MAX_KEY_LENGTH)
		return false;

	SimpleValue& val = _mapValues[key];
	val.type = SIMPLE_VALUE_FLOAT;
	val.intValue = 0;
	val.floatValue = value;
	val.stringValue.clear();
	return true;
}

bool SimplePersist::putValue(const std::string& key, const std::string& value)
{
	//MigUtil::debug("(SimplePersist::putValue) Adding %s : %s", key.c_str(), value.c_str());
	if (key.length() == 0 || key.length() > MAX_KEY_LENGTH)
		return false;
	if (value.length() > MAX_VAL_LENGTH)
		return false;

	SimpleValue& val = _mapValues[key];
	val.type = SIMPLE_VALUE_INT;
	val.intValue = 0;
	val.floatValue = 0;
	val.stringValue = value;
	return true;
}

int SimplePersist::getValue(const std::string& key, int def) const
{
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.find(key);
	if (iter == _mapValues.end())
		return def;
	//MigUtil::debug("(SimplePersist::getValue) Getting %s : %d", key.c_str(), iter->second.intValue);
	return iter->second.intValue;
}

float SimplePersist::getValue(const std::string& key, float def) const
{
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.find(key);
	if (iter == _mapValues.end())
		return def;
	//MigUtil::debug("(SimplePersist::getValue) Getting %s : %f", key.c_str(), iter->second.floatValue);
	return iter->second.floatValue;
}

std::string SimplePersist::getValue(const std::string& key, const std::string& def) const
{
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.find(key);
	if (iter == _mapValues.end())
		return def;
	//MigUtil::debug("(SimplePersist::getValue) Getting %s : %f", key.c_str(), iter->second.stringValue.c_str());
	return iter->second.stringValue;
}

bool SimplePersist::deleteValue(const std::string& key)
{
	_mapValues.erase(key);
	return true;
}

bool SimplePersist::commit()
{
	//MigUtil::debug("(SimplePersist::commit) Starting");

	std::string path = composeDataFilePath();
	FILE* pf = fopen(path.c_str(), "w");
	if (pf == nullptr)
	{
		LOGWARN("(SimplePersist::commit) Could not open data file %s", path.c_str());
		return false;
	}
	LOGINFO("(SimplePersist::commit) Data file '%s' opened", path.c_str());
	fputs(DATA_FILE_HDR, pf);

	char buf[DATA_FILE_ENTRY_LEN];
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.begin();
	while (iter != _mapValues.end())
	{
		bool ok = true;
		if (iter->second.type == SIMPLE_VALUE_INT)
			sprintf(buf, "%s\t%d\t%d\n", iter->first.c_str(), iter->second.type, iter->second.intValue);
		else if (iter->second.type == SIMPLE_VALUE_FLOAT)
			sprintf(buf, "%s\t%d\t%f\n", iter->first.c_str(), iter->second.type, iter->second.floatValue);
		else if (iter->second.type == SIMPLE_VALUE_STRING)
			sprintf(buf, "%s\t%d\t%s\n", iter->first.c_str(), iter->second.type, iter->second.stringValue.c_str());
		else
			ok = false;

		if (ok)
			fputs(buf, pf);
		iter++;
	}

	fclose(pf);
	LOGDBG("(SimplePersist::commit) Complete");
	return true;
}
