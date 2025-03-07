#pragma once

#include <dsound.h>
#include "../core/AudioBase.h"

// Music callback message
#define  WM_MUSICEVENT		WM_USER + 1

namespace MigTech
{
	typedef struct _WAVEHEADER
	{
		char	RIFF[4];
		DWORD	fileLen;
		char	WAVE[4];
		char	fmt[4];
		DWORD	fmtLen;
		WORD	tagFormat;
		WORD	channels;
		DWORD	sampRate;
		DWORD	bps;
		WORD	blockAlign;
		WORD	bits;
		//char	data[4];
		//DWORD	dataLen;

	} WAVEHEADER, *PWAVEHEADER;

	typedef struct _WAVEDATAHEADER
	{
		char	data[4];
		DWORD	dataLen;

	} WAVEDATAHEADER, *PWAVEDATAHEADER;

	typedef struct _SAMPLE
	{
		LIST_ENTRY			link;
		LPDIRECTSOUNDBUFFER lpdsb;
		HANDLE				hsnd;
		char				szFile[MAX_PATH];

		LPBYTE				pdata;		// Data for restoring the buffer
		DWORD				size;		// Size of the data
		DWORD				len;		// Buffer length (milliseconds)
		BOOL				bdelete;	// Buffer must be deleted
		float				vol;		// Sound volume
	} SAMPLE, *PSAMPLE;

	typedef struct _SOUND_MACHINE
	{
		LPDIRECTSOUND		lpds;
		LPDIRECTSOUNDBUFFER lpdsb;
		LPDIRECTSOUNDNOTIFY lpdsn;
		HANDLE				notifyEvent[2];
		WAVEFORMATEX		wfex;
		HANDLE				hsnd;

	} SOUND_MACHINE, *PSOUND_MACHINE;

	//  Sound playback routines
	BOOL InitSound(HINSTANCE hInst, HWND hWnd);
	void DestroySound(void);
	void SetSoundVolume(float fScale);
	float GetSoundVolume(void);
	PSAMPLE LoadSoundFile(LPCSTR szFile);
	void DeleteTheSound(PSAMPLE die);
	BOOL PlayTheSound(PSAMPLE psamp, BOOL bLoop = FALSE);
	void StopTheSound(PSAMPLE psamp);
	void SetSampleVolume(PSAMPLE psamp, float fVol);
	BOOL IsSoundPlaying(PSAMPLE psamp);
	DWORD GetSoundLength(PSAMPLE psamp);

	//  Music playback routines
	BOOL InitMusic(HINSTANCE hInst, HWND hWnd);
	void TermMusic(void);
	void SetMusicVolume(float fScale);
	float GetMusicVolume(void);
	BOOL PlayMusic(LPCSTR pszSong, BOOL bLoop = TRUE);
	void PauseMusic(void);
	void ResumeMusic(void);
	void StopMusic(void);
	void ProcessMusicEvent(void);

	// Windows XAudio2 version of MigTech audio
	class LegacySound : public SoundEffect
	{
	public:
		LegacySound(const std::string& name, AudioBase::Channel channel);
		virtual ~LegacySound();

		virtual void playSound(bool loop);
		virtual void pauseSound();
		virtual void resumeSound();
		virtual void stopSound();
		virtual bool isPlaying();

		virtual void setVolume(float volume);
		virtual void fadeVolume(float fade);
		virtual float getVolume();

	public:
		bool initialize();

	protected:
		AudioBase::Channel _channel;
		PSAMPLE _sample;
		float _volume;
	};

	// legacy version of MigTech audio
	class LegacyAudioManager : public AudioBase
	{
	public:
		LegacyAudioManager(HINSTANCE hInst, HWND hWnd);
		virtual ~LegacyAudioManager();

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
		// Windows specific
		void CreateDeviceIndependentResources();

	protected:
		SoundEffect* loadMediaCallback(const std::string& name, Channel channel, bool addCallback);

	protected:
		HINSTANCE _hInst;
		HWND _hWnd;
		float _musicVolume;
		float _soundVolume;
		SoundEffect* _pEffectToDestroy;
	};
}
