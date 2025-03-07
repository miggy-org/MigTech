#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/SoundEffect.h"

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace MigTech
{
	// OpenSL version of MigTech audio
	class OslAudioSound : public SoundEffect
	{
	public:
		OslAudioSound(const std::string& name);
		virtual ~OslAudioSound();

		virtual void playSound(bool loop);
		virtual void pauseSound();
		virtual void resumeSound();
		virtual void stopSound();
		virtual bool isPlaying();

		virtual void setVolume(float volume);
		virtual void fadeVolume(float fade);
		virtual float getVolume();

	public:
		// OpenSL specific
		bool Initialize(SLEngineItf engine, SLObjectItf mixObject, int fd, off_t start, off_t length);

	protected:
		// file descriptor player interfaces
		SLObjectItf _fdPlayerObject;
		SLPlayItf _fdPlayerPlay;
		SLSeekItf _fdPlayerSeek;
		//SLMuteSoloItf _fdPlayerMuteSolo;
		SLVolumeItf _fdPlayerVolume;
		SLmillibel _maxVolume;
		float _volume;
	};
}