#pragma once

#include "MigDefines.h"
#include "SoundEffect.h"

namespace MigTech
{
	class AudioBase
	{
	public:
		enum Channel { AUDIO_CHANNEL_NONE, AUDIO_CHANNEL_SOUND, AUDIO_CHANNEL_MUSIC };

	public:
		AudioBase() { }
		virtual ~AudioBase() { }

		virtual bool initAudio() = 0;
		virtual void termAudio() = 0;

		virtual void onSuspending() = 0;
		virtual void onResuming() = 0;

		virtual SoundEffect* loadMedia(const std::string& name, Channel channel) = 0;
		virtual bool playMedia(const std::string& name, Channel channel) = 0;
		virtual void deleteMedia(SoundEffect* pMedia) = 0;

		virtual float getChannelVolume(Channel channel) = 0;
		virtual void setChannelVolume(Channel channel, float volume) = 0;
	};

	// sound cache, assumes sound channel only
	class SoundCache
	{
	public:
		SoundCache();
		~SoundCache();

		bool loadSound(const std::string& name);
		bool playSound(const std::string& name);

	protected:
		std::map<std::string, SoundEffect*> _sounds;
	};
}