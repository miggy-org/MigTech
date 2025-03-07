/////////////////////////////////////////////////////////////////////////////
//  Music and sound playback routines
//
//   Adapted from Ahmed Abbas's excellent code in the original Cuboingo
//   This should only be used for Windows desktop support to support pre-Windows 8
//

#include "pch.h"
#include "../core/MigUtil.h"
#include "LegacySound.h"
//#include <dsound.h>

using namespace MigTech;

// defined in Platform.cpp
extern const std::string& plat_getContentDir(int index);

/////////////////////////////////////////////////////////////////////////////
//  Sound playback routines (uses DirectSound)

static SOUND_MACHINE sndMach;

// Sound effects volume
static float fSoundVolume = 1.0f;

BOOL MigTech::InitSound(HINSTANCE hInst, HWND hWnd)
{
	BOOL bret = FALSE;
	HRESULT hr;

	ZeroMemory(&sndMach, sizeof(sndMach));

	hr = DirectSoundCreate(NULL, &sndMach.lpds, NULL);
	if(hr != DS_OK)
	{
		// This is not necessarily an error, it just means you don't have DirectSound
		LOGWARN("(MigTech::InitSound) DirectSoundCreate() returned %d", hr);
		goto done;
	}

	// Set the coop level
	hr = sndMach.lpds->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE);
	if(hr != DS_OK)
	{
		LOGWARN("(MigTech::InitSound) SetCooperativeLevel() returned %d", hr);
		goto done;
	}

	// Load the music volume from the registry
	fSoundVolume = 1;
	bret = TRUE;

done:
	return bret;
}

void MigTech::DestroySound(void)
{
	if (sndMach.lpds)
		sndMach.lpds->Release();
	sndMach.lpds = NULL;
}

static void SetSampleVolume(PSAMPLE psamp)
{
	float fScale = (fSoundVolume == 0 ? 0 : 0.6f + 0.4f*fSoundVolume);
	long lNew = (long) (10000*fScale*psamp->vol - 10000);
	psamp->lpdsb->SetVolume(lNew);
}

void MigTech::SetSoundVolume(float fScale)
{
	fSoundVolume = fScale;
}

float MigTech::GetSoundVolume(void)
{
	return fSoundVolume;
}

static BOOL SeekToData(HANDLE h, DWORD* dataLen)
{
	WAVEDATAHEADER	wdh;
	DWORD			xfer;
	char			dataHeader[4] = {'d', 'a', 't', 'a'};
	char*			tempBuf;

	//	seek until we are out of file
	while(TRUE)
	{
		if(!ReadFile(h, &wdh, sizeof(wdh), &xfer, NULL) || xfer == 0)
		{
			return FALSE;
		}

		if(!memcmp(wdh.data, dataHeader, 4))
		{
			*dataLen = wdh.dataLen;
			return TRUE;
		}

		tempBuf = (char*) LocalAlloc(LPTR, wdh.dataLen);
		if(!tempBuf)
		{
			return FALSE;
		}
		
		if(!ReadFile(h, tempBuf, wdh.dataLen, &xfer, NULL))
		{
			LocalFree(tempBuf);
			return FALSE;
		}

		LocalFree(tempBuf);
	}
}

static BOOL LoadSoundBuffer(PSAMPLE psamp, LPBYTE pData, DWORD dwBytes)
{
	LPVOID lpData;
	DWORD xfer;
	HRESULT hr = psamp->lpdsb->Lock(0, dwBytes, &lpData, &xfer, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (hr != DS_OK)
	{
		//dxError(hr);
		return FALSE;
	}

	BOOL bRet = FALSE;
	if (xfer == dwBytes)
	{
		memcpy(lpData, pData, dwBytes);
		bRet = TRUE;
	}
	//else
	//	Err("Locked buffer not sufficient");

	psamp->lpdsb->Unlock(lpData, dwBytes, NULL, 0);
	return bRet;
}

PSAMPLE MigTech::LoadSoundFile(LPCSTR szFile)
{
	static BOOL bLoadNoMore = FALSE;
	if (bLoadNoMore)
		return NULL;

	BOOL			bret = FALSE, bLocked = FALSE;
	WAVEHEADER		whdr;
	DSBUFFERDESC	dsbd;
	WAVEFORMATEX	wfex;
	DWORD			xfer;
	HRESULT			hr;
	LPVOID			lpData;
	PSAMPLE			psamp = NULL;
	DWORD			dataLen = 0;

	//	no sound to load
	if (sndMach.lpds == NULL || !szFile || (strlen(szFile) == 0))
		return NULL;
	
	ZeroMemory(&whdr, sizeof(whdr));
	ZeroMemory(&dsbd, sizeof(dsbd));
	ZeroMemory(&wfex, sizeof(wfex));

	psamp = (PSAMPLE) LocalAlloc(LPTR, sizeof(SAMPLE));
	if(!psamp)
	{
		//Err("Could not allocate sound buffer memory");
		goto done;
	}

	psamp->hsnd = CreateFileA(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(psamp->hsnd == INVALID_HANDLE_VALUE)
	{
		//char buf[1024];
		
		//sprintf(buf, "Could not open file [%s]", szFile);
		//Err(buf);
		goto done;
	}

	if(!ReadFile(psamp->hsnd, &whdr, sizeof(whdr), &xfer, NULL))
	{
		//Err("Could not read header");
		goto done;
	}

	if(!SeekToData(psamp->hsnd, &dataLen))
	{
		//Err("Could not find data segment");
		goto done;
	}
	psamp->len = (DWORD) (1000*(dataLen / (double) whdr.bps));
	psamp->size = dataLen;
	strcpy_s(psamp->szFile, MAX_PATH, szFile);
	psamp->vol = 1;

	wfex.wFormatTag			= WAVE_FORMAT_PCM;
	wfex.nChannels			= whdr.channels;
	wfex.nSamplesPerSec		= whdr.sampRate;
	wfex.nAvgBytesPerSec	= whdr.bps;
	wfex.nBlockAlign		= whdr.blockAlign;
	wfex.wBitsPerSample		= whdr.bits;
	wfex.cbSize				= 0;

	dsbd.dwSize			= sizeof(dsbd);
	dsbd.dwFlags		= DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
	dsbd.dwBufferBytes	= dataLen;
	dsbd.lpwfxFormat	= &wfex;

 	hr = sndMach.lpds->CreateSoundBuffer(&dsbd, &psamp->lpdsb, NULL);
	if(hr != DS_OK)
	{
		//dxError(hr);

		// If the DirectX version is too old, this is where it's likely to fail...
		bLoadNoMore = TRUE;
		goto done;
	}

	hr = psamp->lpdsb->Lock(0, dsbd.dwBufferBytes, &lpData, &xfer, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if(hr != DS_OK)
	{
		//dxError(hr);
		goto done;
	}
	bLocked = TRUE;

	if(xfer < dsbd.dwBufferBytes)
	{
		//Err("Locked buffer not sufficient");
		goto done;
	}
	
	if(!ReadFile(psamp->hsnd, lpData, dsbd.dwBufferBytes, &xfer, NULL))
	{
		//Err("Could not find data segment on at least one sound effect!");
		goto done;
	}

	// Create a copy of the sound in memory for restoring purposes
	psamp->pdata = (LPBYTE) LocalAlloc(LPTR, psamp->size);
	if (psamp->pdata)
	{
		memcpy(psamp->pdata, lpData, psamp->size);
		psamp->bdelete = TRUE;
	}

	bret = TRUE;

done:
	if (bLocked)
		psamp->lpdsb->Unlock(lpData, dsbd.dwBufferBytes, NULL, 0);

	if (psamp)
	{
		if (!bret)
		{
			if (psamp->hsnd != INVALID_HANDLE_VALUE)
				CloseHandle(psamp->hsnd);
			if (psamp->lpdsb)
				psamp->lpdsb->Release();
			
			LocalFree(psamp);
			return NULL;
		}
		else
			::SetSampleVolume(psamp);
	}

	return psamp;
}

static BOOL SeekToDataResource(LPBYTE pData, DWORD dwSize, DWORD* dataOffset, DWORD* dataLen)
{
	WAVEDATAHEADER	wdh;
	char			dataHeader[4] = {'d', 'a', 't', 'a'};

	//	seek until we are out of file
	dwSize -= *dataOffset;
	while(TRUE)
	{
		memcpy(&wdh, pData + *dataOffset, sizeof(wdh));
		*dataOffset += sizeof(wdh);
		if (!memcmp(wdh.data, dataHeader, 4))
		{
			*dataLen = wdh.dataLen;
			return TRUE;
		}

		if (wdh.dataLen <= dwSize)
		{
			dwSize -= wdh.dataLen;
			*dataOffset += wdh.dataLen;
		}
		else
			return FALSE;
	}
}

static void ClearSound(PSAMPLE die)
{
	// Free the sound resources
	if (die->hsnd && die->hsnd != INVALID_HANDLE_VALUE)
		CloseHandle(die->hsnd);
	die->hsnd = NULL;
	if (die->pdata != NULL && die->bdelete)
		LocalFree(die->pdata);
	die->pdata = NULL;
	if (die->lpdsb)
		die->lpdsb->Release();
	die->lpdsb = NULL;
}

void MigTech::DeleteTheSound(PSAMPLE die)
{
	ClearSound(die);
	LocalFree(die);
}

BOOL MigTech::PlayTheSound(PSAMPLE psamp, BOOL bLoop)
{
	if (!psamp)
		return FALSE;

	// Determine if the sound buffer has been lost
	DWORD dwStatus = 0;
	if (psamp->lpdsb->GetStatus(&dwStatus) == DS_OK && (dwStatus & DSBSTATUS_BUFFERLOST))
		LoadSoundBuffer(psamp, psamp->pdata, psamp->size);

	// Stop the buffer, reset the position, and play again
	if (dwStatus & DSBSTATUS_PLAYING)
		psamp->lpdsb->Stop();
	psamp->lpdsb->SetCurrentPosition(0);
	//if (!(dwStatus & DSBSTATUS_PLAYING))
	//{
		if (psamp->lpdsb->Play(0, 0, (bLoop ? DSBPLAY_LOOPING : 0)) != DS_OK)
		{
			//Err("Could not play again");
			return FALSE;
		}
	//}

	return TRUE;
}

void MigTech::StopTheSound(PSAMPLE psamp)
{
	if (psamp)
		psamp->lpdsb->Stop();
}

void MigTech::SetSampleVolume(PSAMPLE psamp, float fVol)
{
	if (psamp)
	{
		psamp->vol = (0.6f + 0.4f*fVol);
		::SetSampleVolume(psamp);
	}
}

BOOL MigTech::IsSoundPlaying(PSAMPLE psamp)
{
	DWORD dwStatus = 0;
	if (!psamp || psamp->lpdsb->GetStatus(&dwStatus) != DS_OK)
		return FALSE;
	return (dwStatus & DSBSTATUS_PLAYING);
}

DWORD MigTech::GetSoundLength(PSAMPLE psamp)
{
	if (!psamp)
		return 0;
	return psamp->len;
}

/////////////////////////////////////////////////////////////////////////////
//  Music playback routines (uses DirectShow 8.0)

#include <dshow.h>

// DirectShow interfaces
static IGraphBuilder *g_pGraphBuilder = NULL;
static IMediaControl *g_pMediaControl = NULL;
static IMediaSeeking *g_pMediaSeeking = NULL;
static IMediaEventEx *g_pMediaEvent = NULL;
static IBasicAudio   *g_pBasicAudio = NULL;

// Playback variables
static char szCurSong[MAX_PATH];
static BOOL bLoopSong = TRUE;

// Music volume
static float fMusicVolume = 0.9f;

// Returns TRUE if a song is currently playing
static BOOL IsPlaying(void)
{
	return (szCurSong[0] != '\0');
}

// Sets the volume level
static void SetVolume(long lVol)
{
	if (g_pBasicAudio)
	{
		float fScale = (fMusicVolume == 0 ? 0 : 0.5f + 0.5f*fMusicVolume);
		long lNew = (long)((lVol + 10000)*fScale - 10000);
		g_pBasicAudio->put_Volume(lNew);
	}
}

// Starts immediate playback of a song
static BOOL StartPlayback(LPCSTR pszSong)
{
	if (!IsPlaying() && g_pGraphBuilder && g_pMediaControl)
	{
		HRESULT hr = S_OK;

		// Render the file (creates the appropriate filter)
		WCHAR wFileName[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, pszSong, -1, wFileName, MAX_PATH);
		hr = g_pGraphBuilder->RenderFile(wFileName, NULL);
		if (FAILED(hr))
			return FALSE;

		// Run the player
		hr = g_pMediaControl->Run();
		if (SUCCEEDED(hr))
		{
			strcpy_s(szCurSong, MAX_PATH, pszSong);
			return TRUE;
		}
	}

	return FALSE;
}

// Immediately stops playback of the current song
static void StopPlayback(void)
{
	if (IsPlaying() && g_pGraphBuilder)
	{
		HRESULT hr;

		// Stop playback immediately
		if (g_pMediaControl)
			g_pMediaControl->Stop();

		// Remove the old filters
		IEnumFilters *pFilterEnum = NULL;
		if (SUCCEEDED(hr = g_pGraphBuilder->EnumFilters(&pFilterEnum)))
		{
			// Need a loop, since RenderFilter() may create more filters
			IBaseFilter *pFilter = NULL;
			while (pFilterEnum->Next(1, &pFilter, NULL) == S_OK)
			{
				if (pFilter)
				{
					g_pGraphBuilder->RemoveFilter(pFilter);
					pFilter->Release();

					// Reset the enumerator since it was invalidated by RemoveFilter()
					pFilterEnum->Reset();
				}
			}
			pFilterEnum->Release();
		}

		// Terminate the music interfaces (see BUG note above)
		szCurSong[0] = '\0';
	}
}

// Initializes the music playback module
BOOL MigTech::InitMusic(HINSTANCE hInst, HWND hWnd)
{
	if (g_pGraphBuilder == NULL)
	{
		// Create DirectShow Graph
		HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL,
			CLSCTX_INPROC, IID_IGraphBuilder,
			reinterpret_cast<void **>(&g_pGraphBuilder));
		if (FAILED(hr))
		{
			LOGWARN("(MigTech::InitMusic) CoCreateInstance() returned %d", hr);
			return FALSE;
		}

		// Get the IMediaControl interface
		hr = g_pGraphBuilder->QueryInterface(IID_IMediaControl,
			reinterpret_cast<void **>(&g_pMediaControl));
		if (FAILED(hr))
		{
			LOGWARN("(MigTech::InitMusic) IID_IMediaControl returned %d", hr);
			return FALSE;
		}

		// Get the IMediaSeeking interface
		hr = g_pGraphBuilder->QueryInterface(IID_IMediaSeeking,
			reinterpret_cast<void **>(&g_pMediaSeeking));
		if (FAILED(hr))
		{
			LOGWARN("(MigTech::InitMusic) IID_IMediaSeeking returned %d", hr);
			return FALSE;
		}

		// Get the IMediaEvent interface
		hr = g_pGraphBuilder->QueryInterface(IID_IMediaEventEx,
			reinterpret_cast<void **>(&g_pMediaEvent));
		if (FAILED(hr))
		{
			LOGWARN("(MigTech::InitMusic) IID_IMediaEventEx returned %d", hr);
			return FALSE;
		}

		// Get the IBasicAudio interface
		hr = g_pGraphBuilder->QueryInterface(IID_IBasicAudio,
			reinterpret_cast<void **>(&g_pBasicAudio));
		if (FAILED(hr))
		{
			LOGWARN("(MigTech::InitMusic) IID_IBasicAudio returned %d", hr);
			return FALSE;
		}
	}

	// Create the event window
	if (hWnd != NULL)
		g_pMediaEvent->SetNotifyWindow((OAHWND)hWnd, WM_MUSICEVENT, 0);

	// Load the music volume from the registry
	fMusicVolume = 1;

	szCurSong[0] = '\0';
	return TRUE;
}

// Terminates the music playback module
void MigTech::TermMusic(void)
{
	StopMusic();

	// Release all remaining pointers
	if (g_pMediaControl)
		g_pMediaControl->Release();
	g_pMediaControl = NULL;
	if (g_pMediaSeeking)
		g_pMediaSeeking->Release();
	g_pMediaSeeking = NULL;
	if (g_pMediaEvent)
	{
		g_pMediaEvent->SetNotifyWindow(NULL, 0, 0);
		g_pMediaEvent->Release();
	}
	g_pMediaEvent = NULL;
	if (g_pBasicAudio)
		g_pBasicAudio->Release();
	g_pBasicAudio = NULL;
	if (g_pGraphBuilder)
		g_pGraphBuilder->Release();
	g_pGraphBuilder = NULL;
}

// Sets the music volume
void MigTech::SetMusicVolume(float fScale)
{
	fMusicVolume = fScale;
	SetVolume(0);
}

// Returns the music volume level
float MigTech::GetMusicVolume(void)
{
	return fMusicVolume;
}

// Plays the given song, fading it in if necessary
BOOL MigTech::PlayMusic(LPCSTR pszSong, BOOL bLoop)
{
	// If the song is already playing, do nothing
	if (_stricmp(pszSong, szCurSong))
	{
		// Attempt to stop the current song
		if (IsPlaying())
			StopMusic();

		// Start the new song
		if (StartPlayback(pszSong))
		{
			// Reset the volume
			SetVolume(0);
		}
	}

	bLoopSong = bLoop;
	return TRUE;
}

// Pauses the music that's playing
void MigTech::PauseMusic(void)
{
	if (IsPlaying() && g_pMediaControl)
		g_pMediaControl->Pause();
}

// Resumes the music that's playing
void MigTech::ResumeMusic(void)
{
	if (IsPlaying() && g_pMediaControl)
		g_pMediaControl->Run();
}

// Stops the current song, fading it out if necessary
void MigTech::StopMusic(void)
{
	if (IsPlaying())
	{
		// Stop playback immediately
		StopPlayback();
	}
}

// ProcessMusicEvent - call this when processing WM_MUSICEVENT
void MigTech::ProcessMusicEvent()
{
	if (g_pMediaControl && g_pMediaEvent)
	{
		// Get the event
		long lEvent, lParam1, lParam2;
		if (SUCCEEDED(g_pMediaEvent->GetEvent(&lEvent, &lParam1, &lParam2, 1000)))
		{
			// Looking for the media complete event only
			if (lEvent == EC_COMPLETE && bLoopSong)
			{
				// Set the position to the beginning
				LONGLONG llPos = 0;
				if (SUCCEEDED(g_pMediaSeeking->SetPositions(
					&llPos, AM_SEEKING_AbsolutePositioning,
					NULL, AM_SEEKING_NoPositioning)))
				{
					// Start her up again
					g_pMediaControl->Run();
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//  LegacySound

LegacySound::LegacySound(const std::string& name, AudioBase::Channel channel)
	: SoundEffect(name), _channel(channel), _sample(nullptr), _volume(1)
{
}

LegacySound::~LegacySound()
{
}

bool LegacySound::initialize()
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND)
	{
		int currIndex = 0;
		std::string path = plat_getContentDir(currIndex++);
		while (path.length() > 0 && _sample == nullptr)
		{
			_sample = MigTech::LoadSoundFile((path + _name).c_str());
			if (_sample == nullptr)
				path = plat_getContentDir(currIndex++);
		}
		if (_sample == nullptr)
			LOGWARN("(LegacySound::LegacySound) Unable to load sound effect '%s'", _name.c_str());
		return (_sample != nullptr);
	}
	return true;
}

void LegacySound::playSound(bool loop)
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND)
	{
		if (_sample == nullptr || !MigTech::PlayTheSound(_sample, loop))
			LOGWARN("(LegacySound::playSound) Unalbe to play sound '%s'", _name.c_str());
	}
	else if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
	{
		int currIndex = 0;
		std::string path = plat_getContentDir(currIndex++);
		while (path.length() > 0)
		{
			if (MigTech::PlayMusic((path + _name).c_str(), loop))
				break;
			else
				path = plat_getContentDir(currIndex++);
		}
		if (path.length() == 0)
			LOGWARN("(LegacySound::playSound) Unable to play music '%s'", _name.c_str());
	}
}

void LegacySound::pauseSound()
{
	if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		MigTech::PauseMusic();
}

void LegacySound::resumeSound()
{
	if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		MigTech::ResumeMusic();
}

void LegacySound::stopSound()
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND)
	{
		if (_sample != nullptr)
			MigTech::StopTheSound(_sample);
	}
	else if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		MigTech::StopMusic();
}

bool LegacySound::isPlaying()
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND && _sample != nullptr)
		return (MigTech::IsSoundPlaying(_sample) ? true : false);
	else if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		return (::IsPlaying() ? true : false);
	return false;
}

void LegacySound::setVolume(float volume)
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND)
		MigTech::SetSoundVolume(volume);
	else if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		MigTech::SetMusicVolume(volume);
	_volume = volume;
}

void LegacySound::fadeVolume(float fade)
{
	if (_channel == AudioBase::AUDIO_CHANNEL_SOUND)
		MigTech::SetSoundVolume(fade*_volume);
	else if (_channel == AudioBase::AUDIO_CHANNEL_MUSIC)
		MigTech::SetMusicVolume(fade*_volume);
}

float LegacySound::getVolume()
{
	return _volume;
}

/////////////////////////////////////////////////////////////////////////////
//  LegacyAudioManager

LegacyAudioManager::LegacyAudioManager(HINSTANCE hInst, HWND hWnd) :
	_hInst(hInst), _hWnd(hWnd), _pEffectToDestroy(nullptr), _musicVolume(1), _soundVolume(1)
{
}

LegacyAudioManager::~LegacyAudioManager()
{
}

bool LegacyAudioManager::initAudio()
{
	if (!MigTech::InitSound(_hInst, _hWnd))
		return false;
	if (!MigTech::InitMusic(_hInst, _hWnd))
		return false;
	return true;
}

void LegacyAudioManager::termAudio()
{
	MigTech::DestroySound();
	MigTech::TermMusic();
}

void LegacyAudioManager::onSuspending()
{
}

void LegacyAudioManager::onResuming()
{
}

SoundEffect* LegacyAudioManager::loadMedia(const std::string& name, Channel channel)
{
	LegacySound* pSound = new LegacySound(name, channel);
	if (!pSound->initialize())
	{
		delete pSound;
		return nullptr;
	}
	pSound->setVolume(getChannelVolume(channel));
	return pSound;
}

bool LegacyAudioManager::playMedia(const std::string& name, Channel channel)
{
	// if there's an existing sound played using this API, stop it first
	if (_pEffectToDestroy != nullptr)
		deleteMedia(_pEffectToDestroy);

	// load and play the sound - this object will receive callback events
	_pEffectToDestroy = loadMedia(name, channel);
	if (_pEffectToDestroy != nullptr)
		_pEffectToDestroy->playSound(false);
	return (_pEffectToDestroy != nullptr);
}

void LegacyAudioManager::deleteMedia(SoundEffect* pMedia)
{
	delete (LegacySound*)pMedia;
}

float LegacyAudioManager::getChannelVolume(Channel channel)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		return _musicVolume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		return _soundVolume;
	return 0;
}

void LegacyAudioManager::setChannelVolume(Channel channel, float volume)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		_musicVolume = volume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		_soundVolume = volume;
}
