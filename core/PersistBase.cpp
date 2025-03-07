#include "pch.h"
#include "MigUtil.h"
#include "PersistBase.h"

using namespace tinyxml2;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// platform specific

extern byte* plat_loadFileBuffer(const char* filePath, int& length);

///////////////////////////////////////////////////////////////////////////
// XML document factory

tinyxml2::XMLDocument* XMLDocFactory::loadDocument(const std::string& docPath)
{
	tinyxml2::XMLDocument* pdoc = nullptr;

	int len = 0;
	byte* pFile = plat_loadFileBuffer(docPath.c_str(), len);
	if (pFile != nullptr && len > 0)
	{
		LOGDBG("(XMLDocFactory::loadDocument) File %s size is %d", docPath.c_str(), len);

		pdoc = new tinyxml2::XMLDocument();
		XMLError err = pdoc->Parse((const char*)pFile, len);
		delete pFile;

		if (err != XML_SUCCESS)
		{
			LOGWARN("(XMLDocFactory::loadDocument) File %s could not be parsed", docPath.c_str());
			delete pdoc;
			pdoc = nullptr;
		}
	}

	return pdoc;
}

///////////////////////////////////////////////////////////////////////////
// SimplePersist

#define MAX_KEY_LENGTH			128
#define MAX_VAL_LENGTH			1024

#define DATA_FILE_NAME			"mtdata.txt"
#define DATA_FILE_HDR			"MIG_TECH_DATA_1\n"
#define DATA_FILE_ENTRY_LEN		(MAX_KEY_LENGTH + MAX_VAL_LENGTH + 8)

extern const std::string& plat_getFilesDir();

static std::string composeDataFilePath()
{
	std::string path = plat_getFilesDir();
	path += "/";
	path += DATA_FILE_NAME;
	return path;
}

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

SimplePersist::SimplePersist()
{
}

bool SimplePersist::open()
{
	//LOGDBG("(SimplePersist::open) Starting");

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
		//LOGDBG("(SimplePersist::open) Key = '%s'", key);
		const char* type = strtok(nullptr, "\t\n");
		//LOGDBG("(SimplePersist::open) Type = '%s'", type);
		const char* val = strtok(nullptr, "\t\n");
		//LOGDBG("(SimplePersist::open) Val = '%s'", val);

		if (key != nullptr && type != nullptr && val != nullptr)
		{
			int itype = atoi(type);
			if (itype == (int)SIMPLE_VALUE_INT)
				putValue(key, (int)atoi(val));
			else if (itype == (int)SIMPLE_VALUE_FLOAT)
				putValue(key, (float)atof(val));
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
	//LOGDBG("(SimplePersist::putValue) Adding %s : %d", key.c_str(), value);
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
	//LOGDBG("(SimplePersist::putValue) Adding %s : %f", key.c_str(), value);
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
	//LOGDBG("(SimplePersist::putValue) Adding %s : %s", key.c_str(), value.c_str());
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
	//LOGDBG("(SimplePersist::getValue) Getting %s : %d", key.c_str(), iter->second.intValue);
	return iter->second.intValue;
}

float SimplePersist::getValue(const std::string& key, float def) const
{
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.find(key);
	if (iter == _mapValues.end())
		return def;
	//LOGDBG("(SimplePersist::getValue) Getting %s : %f", key.c_str(), iter->second.floatValue);
	return iter->second.floatValue;
}

std::string SimplePersist::getValue(const std::string& key, const std::string& def) const
{
	std::map<std::string, SimpleValue>::const_iterator iter = _mapValues.find(key);
	if (iter == _mapValues.end())
		return def;
	//LOGDBG("(SimplePersist::getValue) Getting %s : %f", key.c_str(), iter->second.stringValue.c_str());
	return iter->second.stringValue;
}

bool SimplePersist::deleteValue(const std::string& key)
{
	_mapValues.erase(key);
	return true;
}

bool SimplePersist::commit()
{
	//LOGDBG("(SimplePersist::commit) Starting");

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

#pragma warning(pop)
