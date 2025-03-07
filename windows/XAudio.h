#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/AudioBase.h"

namespace MigTech
{
	class MediaReader
	{
	public:
		MediaReader();

		std::vector<byte>* LoadMedia(const std::string& filename);
		WAVEFORMATEX*      GetOutputWaveFormatEx();

	protected:
		std::wstring   m_installedLocationPath;
		WAVEFORMATEX   m_waveFormat;
	};

	// Windows XAudio2 version of MigTech audio
	class XAudioManager : public AudioBase, IXAudio2VoiceCallback
	{
	public:
		XAudioManager();
		virtual ~XAudioManager();

		virtual bool initAudio();
		virtual void termAudio();

		virtual void onSuspending();
		virtual void onResuming();

		virtual SoundEffect* loadMedia(const std::string& name, Channel channel);
		virtual bool playMedia(const std::string& name, Channel channel);
		virtual void deleteMedia(SoundEffect* pMedia);

		virtual float getChannelVolume(Channel channel);
		virtual void setChannelVolume(Channel channel, float volume);

		// IXAudio2VoiceCallback
		STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired);
		STDMETHOD_(void, OnVoiceProcessingPassEnd) ();
		STDMETHOD_(void, OnStreamEnd) ();
		STDMETHOD_(void, OnBufferStart) (void* pBufferContext);
		STDMETHOD_(void, OnBufferEnd) (void* pBufferContext);
		STDMETHOD_(void, OnLoopEnd) (void* pBufferContext);
		STDMETHOD_(void, OnVoiceError) (void* pBufferContext, HRESULT Error);

	public:
		// Windows specific
		void CreateDeviceIndependentResources();

	protected:
		SoundEffect* loadMediaCallback(const std::string& name, Channel channel, bool addCallback);

	protected:
		bool                                _audioAvailable;
		Microsoft::WRL::ComPtr<IXAudio2>    _musicEngine;
		Microsoft::WRL::ComPtr<IXAudio2>    _soundEffectEngine;
		IXAudio2MasteringVoice*             _musicMasteringVoice;
		IXAudio2MasteringVoice*             _soundEffectMasteringVoice;
		MediaReader*						_mediaReader;
		float								_musicVolume;
		float								_soundVolume;

		SoundEffect*						_pEffectToDestroy;
	};
}