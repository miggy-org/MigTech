#include "pch.h"
#include "AudioBase.h"
#include "MigUtil.h"

using namespace MigTech;

SoundCache::SoundCache()
{
}

SoundCache::~SoundCache()
{
	if (MigUtil::theAudio != nullptr)
	{
		std::map<std::string, SoundEffect*>::iterator iter = _sounds.begin();
		while (iter != _sounds.end())
		{
			if (iter->second != nullptr)
				MigUtil::theAudio->deleteMedia(iter->second);
			iter++;
		}
	}
}

bool SoundCache::loadSound(const std::string& name)
{
	if (_sounds.find(name) != _sounds.end())
		return true;

	if (MigUtil::theAudio != nullptr)
	{
		std::string lname = name;
		if (lname.find(".wav") == std::string::npos)
			lname += ".wav";
		SoundEffect* peff = MigUtil::theAudio->loadMedia(lname, AudioBase::AUDIO_CHANNEL_SOUND);
		if (peff != nullptr)
		{
			_sounds[name] = peff;
			return true;
		}
		else
			LOGWARN("(SoundCache::loadSound) Couldn't load sound effect '%s'", lname.c_str());
	}
	
	return false;
}

bool SoundCache::playSound(const std::string& name)
{
	std::map<std::string, SoundEffect*>::iterator iter = _sounds.find(name);
	if (iter != _sounds.end() && iter->second != nullptr)
	{
		iter->second->playSound(false);
		return true;
	}

	if (loadSound(name))
	{
		iter = _sounds.find(name);
		if (iter != _sounds.end() && iter->second != nullptr)
		{
			iter->second->playSound(false);
			return true;
		}
	}

	return false;
}
