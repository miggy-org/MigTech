#include "pch.h"
#include "XAudioSound.h"
#include "DxDefines.h"
#include "../core/MigUtil.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

XAudioSound::XAudioSound(const std::string& name) : SoundEffect(name),
	_audioAvailable(false),
	_sourceVoice(nullptr),
	_volume(1)
{
}

XAudioSound::~XAudioSound()
{
	// TODO: need to release _sourceVoice, and ensure playMedia() still works
	if (_sourceVoice != nullptr)
		_sourceVoice->DestroyVoice();
	if (_soundData != nullptr)
		delete _soundData;
}

void XAudioSound::Initialize(
	IXAudio2 *masteringEngine,
	WAVEFORMATEX *sourceFormat,
	std::vector<byte>* soundData,
	IXAudio2VoiceCallback* callBack)
{
	_soundData = soundData;

	if (masteringEngine == nullptr)
	{
		// Audio is not available so just return.
		_audioAvailable = false;
		return;
	}

	// Create a source voice for this sound effect.
	HRESULT hr = masteringEngine->CreateSourceVoice(
		&_sourceVoice,
		sourceFormat,
		0,
		XAUDIO2_DEFAULT_FREQ_RATIO,
		callBack);
	if (FAILED(hr))
		throw hres_error("XAudioSound: CreateSourceVoice failed", hr);
	_audioAvailable = true;
}

void XAudioSound::playSound(bool loop)
{
	XAUDIO2_BUFFER buffer = { 0 };

	if (!_audioAvailable)
	{
		// Audio is not available so just return.
		return;
	}

	// Interrupt sound effect if it is currently playing.
	stopSound();

	// Queue the memory buffer for playback and start the voice.
	buffer.AudioBytes = _soundData->size();
	buffer.pAudioData = _soundData->data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = (loop ? XAUDIO2_LOOP_INFINITE : 0);
	buffer.pContext = this;

	HRESULT hr = _sourceVoice->SetVolume(_volume);
	if (FAILED(hr))
		throw hres_error("XAudioSound: SetVolume failed", hr);
	hr = _sourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
		throw hres_error("XAudioSound: SubmitSourceBuffer failed", hr);
	hr = _sourceVoice->Start();
	if (FAILED(hr))
		throw hres_error("XAudioSound: Start failed", hr);
}

void XAudioSound::pauseSound()
{
	HRESULT hr = _sourceVoice->Stop();
	if (FAILED(hr))
		throw hres_error("(XAudioSound::pauseSound) Stop failed", hr);
}

void XAudioSound::resumeSound()
{
	HRESULT hr = _sourceVoice->Start();
	if (FAILED(hr))
		throw hres_error("(XAudioSound::resumeSound) Start failed", hr);
}

void XAudioSound::stopSound()
{
	HRESULT hr = _sourceVoice->Stop();
	if (FAILED(hr))
		throw hres_error("XAudioSound: Stop failed", hr);
	hr = _sourceVoice->FlushSourceBuffers();
	if (FAILED(hr))
		throw hres_error("XAudioSound: FlushSourceBuffers failed", hr);
}

bool XAudioSound::isPlaying()
{
	XAUDIO2_VOICE_STATE state;
	_sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return (state.pCurrentBufferContext != nullptr ? true : false);
}

void XAudioSound::setVolume(float volume)
{
	HRESULT hr = _sourceVoice->SetVolume(volume);
	if (FAILED(hr))
		throw hres_error("XAudioSound: SetVolume failed", hr);
	_volume = volume;
}

void XAudioSound::fadeVolume(float fade)
{
	HRESULT hr = _sourceVoice->SetVolume(fade*_volume);
	if (FAILED(hr))
		throw hres_error("XAudioSound: SetVolume failed", hr);
}

float XAudioSound::getVolume()
{
	return _volume;
}
