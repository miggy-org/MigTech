#pragma once

#include <map>

#include "pch.h"
#include "../core/PersistBase.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

namespace MigTech
{
	enum SIMPLE_VALUE_TYPE
	{
		SIMPLE_VALUE_NONE,
		SIMPLE_VALUE_INT,
		SIMPLE_VALUE_FLOAT,
		SIMPLE_VALUE_STRING
	};

	struct SimpleValue
	{
		SIMPLE_VALUE_TYPE type;
		int intValue;
		float floatValue;
		std::string stringValue;
	};

	class SimplePersist : public MigTech::PersistBase
	{
	public:
		SimplePersist();

		virtual bool open();
		virtual void close();

		virtual bool putValue(const std::string& key, int value);
		virtual bool putValue(const std::string& key, float value);
		virtual bool putValue(const std::string& key, const std::string& value);

		virtual int getValue(const std::string& key, int def) const;
		virtual float getValue(const std::string& key, float def) const;
		virtual std::string getValue(const std::string& key, const std::string& def) const;

		virtual bool deleteValue(const std::string& key);

		virtual bool commit();

	private:
		std::map<std::string, SimpleValue> _mapValues;
	};
}
