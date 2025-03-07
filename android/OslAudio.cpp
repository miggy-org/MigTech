#include "pch.h"
#include "OslAudio.h"
#include "OslAudioSound.h"
#include "AndroidApp.h"
#include "../core/MigUtil.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OslAudioManager::OslAudioManager()
	: _engineObject(nullptr), _outputMixObject(nullptr), _musicVolume(1), _soundVolume(1), _pEffectToDestroy(nullptr)
{
}

OslAudioManager::~OslAudioManager()
{
}

bool OslAudioManager::initAudio()
{
    SLresult result;

    // create engine
	result = slCreateEngine(&_engineObject, 0, nullptr, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result)
		return false;

    // realize the engine
    result = (*_engineObject)->Realize(_engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result)
		return false;

    // get the engine interface, which is needed in order to create other objects
    result = (*_engineObject)->GetInterface(_engineObject, SL_IID_ENGINE, &_engineEngine);
    if (SL_RESULT_SUCCESS != result)
		return false;

    // create output mix
    //const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    //const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = (*_engineEngine)->CreateOutputMix(_engineEngine, &_outputMixObject, 0, nullptr, nullptr);
    if (SL_RESULT_SUCCESS != result)
		return false;

    // realize the output mix
    result = (*_outputMixObject)->Realize(_outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result)
		return false;

	LOGINFO("(OslAudioManager::initAudio) OpenSLES audio engine started");
	return true;
}

void OslAudioManager::termAudio()
{
	if (_pEffectToDestroy != nullptr)
	{
		deleteMedia(_pEffectToDestroy);
		_pEffectToDestroy = nullptr;
	}

    // destroy output mix object, and invalidate all associated interfaces
	if (_outputMixObject != nullptr)
	{
        (*_outputMixObject)->Destroy(_outputMixObject);
		_outputMixObject = nullptr;
    }

    // destroy engine object, and invalidate all associated interfaces
	if (_engineObject != nullptr)
	{
        (*_engineObject)->Destroy(_engineObject);
		_engineObject = nullptr;
		_engineEngine = nullptr;
    }
}

void OslAudioManager::onSuspending()
{
}

void OslAudioManager::onResuming()
{
}

// TODO: proper channel support
SoundEffect* OslAudioManager::loadMedia(const std::string& name, Channel channel)
{
	LOGINFO("(OslAudioManager::loadMedia) Loading sound %s", name.c_str());
	SLresult result;

    // use asset manager to open asset by filename
    AAssetManager* mgr = AndroidUtil_getAssetManager();
    AAsset* asset = AAssetManager_open(mgr, name.c_str(), AASSET_MODE_UNKNOWN);
	if (nullptr == asset)
	{
		LOGWARN("(OslAudioManager::loadMedia) Failed to open asset %s", name.c_str());
		return nullptr;
	}

    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
	if (0 == fd)
	{
		LOGWARN("(OslAudioManager::loadMedia) Failed to open file descriptor for %s", name.c_str());
		return nullptr;
	}
    AAsset_close(asset);

	// create the sound object
	OslAudioSound* pSound = new OslAudioSound(name);
	if (pSound->Initialize(_engineEngine, _outputMixObject, fd, start, length))
	{
		// set the initial volume
		pSound->setVolume(getChannelVolume(channel));
	}
	else
	{
		delete pSound;
		pSound = nullptr;
	}

	return pSound;
}

bool OslAudioManager::playMedia(const std::string& name, Channel channel)
{
	// if there's an existing sound played using this API, stop it first
	if (_pEffectToDestroy != nullptr)
		deleteMedia(_pEffectToDestroy);

	_pEffectToDestroy = loadMedia(name, channel);
	if (_pEffectToDestroy != nullptr)
		_pEffectToDestroy->playSound(false);

	return (_pEffectToDestroy != nullptr);
}

void OslAudioManager::deleteMedia(SoundEffect* pMedia)
{
	delete (OslAudioSound*) pMedia;
}

float OslAudioManager::getChannelVolume(Channel channel)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		return _musicVolume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		return _soundVolume;
	return 0;
}

void OslAudioManager::setChannelVolume(Channel channel, float volume)
{
	if (channel == AUDIO_CHANNEL_MUSIC)
		_musicVolume = volume;
	else if (channel == AUDIO_CHANNEL_SOUND)
		_soundVolume = volume;
}
