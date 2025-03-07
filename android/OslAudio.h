#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/AudioBase.h"

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace MigTech
{
	// OpenSL version of MigTech audio
	class OslAudioManager : public AudioBase
	{
	public:
		OslAudioManager();
		virtual ~OslAudioManager();

		virtual bool initAudio();
		virtual void termAudio();

		virtual void onSuspending();
		virtual void onResuming();

		virtual SoundEffect* loadMedia(const std::string& name, Channel channel);
		virtual bool playMedia(const std::string& name, Channel channel);
		virtual void deleteMedia(SoundEffect* pMedia);

		virtual float getChannelVolume(Channel channel);
		virtual void setChannelVolume(Channel channel, float volume);

	public:
		// OpenSL specific

	protected:
		// engine interfaces
		SLObjectItf _engineObject;
		SLEngineItf _engineEngine;

		// output mix interfaces
		SLObjectItf _outputMixObject;

		// volumes
		float _musicVolume;
		float _soundVolume;

		// cached sound effect in playMedia() to destroy later
		SoundEffect* _pEffectToDestroy;
	};
}
