#pragma once

#include "pch.h"
#include "../core/PersistBase.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

namespace MigTech
{
	class RegistryPersist sealed : public MigTech::PersistBase
	{
	public:
		RegistryPersist();

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
		HKEY _hKey;
	};
}
