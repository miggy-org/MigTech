#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/SoundEffect.h"

namespace MigTech
{
	// Windows XAudio2 version of MigTech audio
	class XAudioSound : public SoundEffect
	{
	public:
		XAudioSound(const std::string& name);
		virtual ~XAudioSound();

		virtual void playSound(bool loop);
		virtual void pauseSound();
		virtual void resumeSound();
		virtual void stopSound();
		virtual bool isPlaying();

		virtual void setVolume(float volume);
		virtual void fadeVolume(float fade);
		virtual float getVolume();

	public:
		// Windows specific
		void Initialize(
			IXAudio2*              masteringEngine,
			WAVEFORMATEX*          sourceFormat,
			std::vector<byte>*     soundData,
			IXAudio2VoiceCallback* callBack
			);

	protected:
		bool                    _audioAvailable;
		IXAudio2SourceVoice*    _sourceVoice;
		std::vector<byte>*      _soundData;
		float					_volume;
	};
}