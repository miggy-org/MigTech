#include "pch.h"
#include "XAudio.h"
#include "XaudioSound.h"
#include "DxDefines.h"
#include "../core/MigUtil.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace Microsoft::WRL;

using namespace MigTech;

// defined in Platform.cpp
extern bool toWString(const std::string& inStr, std::wstring& outStr);
extern const std::string& plat_getContentDir(int index);

XAudioManager::XAudioManager() :
	_audioAvailable(false),
	_musicMasteringVoice(nullptr),
	_soundEffectMasteringVoice(nullptr),
	_mediaReader(nullptr),
	_musicVolume(1),
	_soundVolume(1),
	_pEffectToDestroy(nullptr)
{
}

XAudioManager::~XAudioManager()
{
}

bool XAudioManager::initAudio()
{
	CreateDeviceIndependentResources();
	return _audioAvailable;
}

void XAudioManager::termAudio()
{
	delete _mediaReader;
}

void XAudioManager::CreateDeviceIndependentResources()
{
	UINT32 flags = 0;

	HRESULT hr = XAudio2Create(&_musicEngine, flags);
	if (FAILED(hr))
		throw hres_error("(XAudioManager::CreateDeviceIndependentResources) XAudio2Create failed", hr);

#if defined(_DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION debugConfiguration = { 0 };
	debugConfiguration.BreakMask = XAUDIO2_LOG_ERRORS;
	debugConfiguration.TraceMask = XAUDIO2_LOG_ERRORS;
	_musicEngine->SetDebugConfiguration(&debugConfiguration);
#endif

	hr = _musicEngine->CreateMasteringVoice(&_musicMasteringVoice);
	if (FAILED(hr))
	{
		LOGWARN("(XAudioManager::CreateDeviceIndependentResources) Unable to create mastering voice (%d)", hr);

		// Unable to create an audio device
		_audioAvailable = false;
		return;
	}

	hr = XAudio2Create(&_soundEffectEngine, flags);
	if (FAILED(hr))
		throw hres_error("(XAudioManager::CreateDeviceIndependentResources) XAudio2Create failed", hr);

#if defined(_DEBUG)
	_soundEffectEngine->SetDebugConfiguration(&debugConfiguration);
#endif

	hr = _soundEffectEngine->CreateMasteringVoice(&_soundEffectMasteringVoice);
	if (FAILED(hr))
		throw hres_error("(XAudioManager::CreateDeviceIndependentResources) CreateMasteringVoice failed", hr);

	_mediaReader = new MediaReader();
	_audioAvailable = true;
}

void XAudioManager::onSuspending()
{
	if (_audioAvailable)
	{
		_musicEngine->StopEngine();
		_soundEffectEngine->StopEngine();
	}
}

void XAudioManager::onResuming()
{
	if (_audioAvailable)
	{
		HRESULT hr = _musicEngine->StartEngine();
		if (FAILED(hr))
			throw hres_error("(XAudioManager::onResuming) StartEngine failed", hr);
		hr = _soundEffectEngine->StartEngine();
		if (FAILED(hr))
			throw hres_error("(XAudioManager::onResuming) StartEngine failed", hr);
	}
}

SoundEffect* XAudioManager::loadMediaCallback(const std::string& name, Channel channel, bool addCallback)
{
	IXAudio2* pEngine = nullptr;
	if (channel == AUDIO_CHANNEL_MUSIC)
		pEngine = _musicEngine.Get();
	else if (channel == AUDIO_CHANNEL_SOUND)
		pEngine = _soundEffectEngine.Get();

	XAudioSound* pSound = nullptr;
	if (pEngine != nullptr)
	{
		auto soundData = _mediaReader->LoadMedia(name);

		pSound = new XAudioSound(name);
		pSound->Initialize(pEngine, _mediaReader->GetOutputWaveFormatEx(), soundData, (addCallback ? this : nullptr));
		pSound->setVolume(getChannelVolume(channel));
	}
	else
		LOGWARN("(XAudioManager::loadMediaCallback) No sound channel chosen!");

	return pSound;
}

SoundEffect* XAudioManager::loadMedia(const std::string& name, Channel channel)
{
	LOGINFO("(XAudioManager::loadMedia) Loading sound %s", name.c_str());
	return loadMediaCallback(name, channel, false);
}

bool XAudioManager::playMedia(const std::string& name, Channel channel)
{
	// if there's an existing sound played using this API, stop it first
	if (_pEffectToDestroy != nullptr)
		deleteMedia(_pEffectToDestroy);

	// load and play the sound - this object will receive callback events
	_pEffectToDestroy = loadMediaCallback(name, channel, true);
	if (_pEffectToDestroy != nullptr)
		_pEffectToDestroy->playSound(false);
	return (_pEffectToDestroy != nullptr);
}

void XAudioManager::deleteMedia(SoundEffect* pMedia)
{
	delete (XAudioSound*) pMedia;
}

float XAudioManager::getChannelVolume(Channel channel)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		return _musicVolume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		return _soundVolume;
	return 0;
}

void XAudioManager::setChannelVolume(Channel channel, float volume)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		_musicVolume = volume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		_soundVolume = volume;
}

void XAudioManager::OnVoiceProcessingPassStart(UINT32 BytesRequired)
{
}

void XAudioManager::OnVoiceProcessingPassEnd()
{
}

void XAudioManager::OnStreamEnd()
{
}

void XAudioManager::OnBufferStart(void* pBufferContext)
{
}

void XAudioManager::OnBufferEnd(void* pBufferContext)
{
}

void XAudioManager::OnLoopEnd(void* pBufferContext)
{
	// destroy the sound effect
	if (_pEffectToDestroy != nullptr)
		deleteMedia(_pEffectToDestroy);
	_pEffectToDestroy = nullptr;
}

void XAudioManager::OnVoiceError(void* pBufferContext, HRESULT Error)
{
}

///////////////////////////////////////////////////////////////////////////
// MediaReader helper class

MediaReader::MediaReader()
{
	ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));
//#ifdef _WINDOWS
	toWString(plat_getContentDir(0), m_installedLocationPath);
//#else
//	Windows::Storage::StorageFolder^ installedLocation = Windows::ApplicationModel::Package::Current->InstalledLocation;
//	m_installedLocationPath = std::wstring(installedLocation->Path->Data()) + L"\\";
//#endif // _WINDOWS
}

WAVEFORMATEX *MediaReader::GetOutputWaveFormatEx()
{
	return &m_waveFormat;
}

std::vector<byte>* MediaReader::LoadMedia(const std::string& filename)
{
	HRESULT hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
		throw hres_error("MediaReader: MFStartup failed", hr);

	std::wstring wfilename;
	toWString(filename, wfilename);

	ComPtr<IMFSourceReader> reader;
	hr = MFCreateSourceReaderFromURL(
		(m_installedLocationPath + wfilename).c_str(),
		nullptr,
		&reader
		);
	if (FAILED(hr))
		throw hres_error("MediaReader: MFCreateSourceReaderFromURL failed", hr);

	// Set the decoded output format as PCM.
	// XAudio2 on Windows can process PCM and ADPCM-encoded buffers.
	// When using MediaFoundation, this sample always decodes into PCM.
	Microsoft::WRL::ComPtr<IMFMediaType> mediaType;
	hr = MFCreateMediaType(&mediaType);
	if (FAILED(hr))
		throw hres_error("MediaReader: MFCreateMediaType failed", hr);

	hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if (FAILED(hr))
		throw hres_error("MediaReader: SetGUID failed", hr);

	hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	if (FAILED(hr))
		throw hres_error("MediaReader: SetGUID failed", hr);

	hr = reader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, mediaType.Get());
	if (FAILED(hr))
		throw hres_error("MediaReader: SetCurrentMediaType failed", hr);

	// Get the complete WAVEFORMAT from the Media Type.
	Microsoft::WRL::ComPtr<IMFMediaType> outputMediaType;
	hr = reader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), &outputMediaType);
	if (FAILED(hr))
		throw hres_error("MediaReader: GetCurrentMediaType failed", hr);

	UINT32 size = 0;
	WAVEFORMATEX* waveFormat;
	hr = MFCreateWaveFormatExFromMFMediaType(outputMediaType.Get(), &waveFormat, &size);
	if (FAILED(hr))
		throw hres_error("MediaReader: MFCreateWaveFormatExFromMFMediaType failed", hr);

	CopyMemory(&m_waveFormat, waveFormat, sizeof(m_waveFormat));
	CoTaskMemFree(waveFormat);

	PROPVARIANT propVariant;
	hr = reader->GetPresentationAttribute(static_cast<DWORD>(MF_SOURCE_READER_MEDIASOURCE), MF_PD_DURATION, &propVariant);
	if (FAILED(hr))
		throw hres_error("MediaReader: GetPresentationAttribute failed", hr);

	// 'duration' is in 100ns units; convert to seconds, and round up
	// to the nearest whole byte.
	LONGLONG duration = propVariant.uhVal.QuadPart;
	unsigned int maxStreamLengthInBytes =
		static_cast<unsigned int>(
		((duration * static_cast<ULONGLONG>(m_waveFormat.nAvgBytesPerSec)) + 10000000) /
		10000000
		);

	// some media files don't seem to have the total size block aligned
	if (maxStreamLengthInBytes%m_waveFormat.nBlockAlign != 0)
	{
		LOGWARN("(MediaReader::LoadMedia) Buffer size (%d) doesn't align w/ block size (%d), adjusting", maxStreamLengthInBytes, m_waveFormat.nBlockAlign);
		maxStreamLengthInBytes += (m_waveFormat.nBlockAlign - maxStreamLengthInBytes%m_waveFormat.nBlockAlign);
	}
	//if (filename == "splash.mp3")
	//	maxStreamLengthInBytes = 11953156;

	std::vector<byte>* fileData = new std::vector<byte>(maxStreamLengthInBytes);

	ComPtr<IMFSample> sample;
	ComPtr<IMFMediaBuffer> mediaBuffer;
	DWORD flags = 0;

	int positionInData = 0;
	bool done = false;
	while (!done)
	{
		hr = reader->ReadSample(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0, nullptr, &flags, nullptr, &sample);
		if (FAILED(hr))
			throw hres_error("MediaReader: ReadSample failed", hr);

		if (sample != nullptr)
		{
			hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
			if (FAILED(hr))
				throw hres_error("MediaReader: ConvertToContiguousBuffer failed", hr);

			BYTE *audioData = nullptr;
			DWORD sampleBufferLength = 0;
			hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);
			if (FAILED(hr))
				throw hres_error("MediaReader: Lock failed", hr);

			// sometimes the computed buffer size isn't right, not sure why
			if (maxStreamLengthInBytes - positionInData < sampleBufferLength)
			{
				unsigned int newMaxStreamLengthInBytes = positionInData + sampleBufferLength;
				if (newMaxStreamLengthInBytes%m_waveFormat.nBlockAlign != 0)
					newMaxStreamLengthInBytes += (m_waveFormat.nBlockAlign - newMaxStreamLengthInBytes%m_waveFormat.nBlockAlign);
				LOGWARN("(MediaReader::LoadMedia) Buffer size (%d) incorrect, resizing to %d", maxStreamLengthInBytes, newMaxStreamLengthInBytes);

				std::vector<byte>* newFileData = new std::vector<byte>(newMaxStreamLengthInBytes);

				// wish there were a more efficient way to do this
				for (int i = 0; i < positionInData; i++)
				{
					(*newFileData)[i] = (*fileData)[i];
				}
				delete fileData;
				fileData = newFileData;
				maxStreamLengthInBytes = newMaxStreamLengthInBytes;
			}

			for (DWORD i = 0; i < sampleBufferLength; i++)
			{
				(*fileData)[positionInData++] = audioData[i];
			}
		}
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			done = true;
		}
	}

	return fileData;
}
