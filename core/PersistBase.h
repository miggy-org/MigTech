#pragma once

#include <map>

#include "tinyxml/tinyxml2.h"

#include "MigDefines.h"
#include "SoundEffect.h"

namespace MigTech
{
	///////////////////////////////////////////////////////////////////////////
	// base class for persistence support

	class PersistBase
	{
	public:
		PersistBase() { }
		virtual ~PersistBase() { }

		virtual bool open() = 0;
		virtual void close() = 0;

		virtual bool putValue(const std::string& key, int value) = 0;
		virtual bool putValue(const std::string& key, float value) = 0;
		virtual bool putValue(const std::string& key, const std::string& value) = 0;

		virtual int getValue(const std::string& key, int def) const = 0;
		virtual float getValue(const std::string& key, float def) const = 0;
		virtual std::string getValue(const std::string& key, const std::string& def) const = 0;

		virtual bool deleteValue(const std::string& key) = 0;

		virtual bool commit() = 0;
	};

	///////////////////////////////////////////////////////////////////////////
	// simple file based implementation

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

	///////////////////////////////////////////////////////////////////////////
	// XML document factory

	class XMLDocFactory
	{
	public:
		static tinyxml2::XMLDocument* loadDocument(const std::string& docPath);
	};
}