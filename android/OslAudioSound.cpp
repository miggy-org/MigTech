#include "pch.h"
#include "OslAudioSound.h"
#include "../core/MigUtil.h"

///////////////////////////////////////////////////////////////////////////
// platform specific

using namespace MigTech;

OslAudioSound::OslAudioSound(const std::string& name)
	: SoundEffect(name), _fdPlayerObject(nullptr), _volume(1)
{
}

OslAudioSound::~OslAudioSound()
{
    // destroy file descriptor audio player object, and invalidate all associated interfaces
	if (_fdPlayerObject != nullptr)
	{
        (*_fdPlayerObject)->Destroy(_fdPlayerObject);
		_fdPlayerObject = nullptr;
		_fdPlayerPlay = nullptr;
		_fdPlayerSeek = nullptr;
        //_fdPlayerMuteSolo = nullptr;
		_fdPlayerVolume = nullptr;
    }
}

bool OslAudioSound::Initialize(SLEngineItf engine, SLObjectItf mixObject, int fd, off_t start, off_t length)
{
	SLresult result;

    // configure audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
	SLDataFormat_MIME format_mime = { SL_DATAFORMAT_MIME, nullptr, SL_CONTAINERTYPE_UNSPECIFIED };
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, mixObject};
	SLDataSink audioSnk = { &loc_outmix, nullptr };

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_SEEK, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_FALSE, SL_BOOLEAN_TRUE};
    result = (*engine)->CreateAudioPlayer(engine, &_fdPlayerObject, &audioSrc, &audioSnk,
            2, ids, req);
    if (SL_RESULT_SUCCESS != result)
	{
		LOGWARN("(OslAudioSound::Initialize) CreateAudioPlayer failed");
		return nullptr;
	}

    // realize the player
    result = (*_fdPlayerObject)->Realize(_fdPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result)
	{
		LOGWARN("(OslAudioSound::Initialize) Realize failed");
		return nullptr;
	}

    // get the play interface
    result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_PLAY, &_fdPlayerPlay);
    if (SL_RESULT_SUCCESS != result)
	{
		LOGWARN("(OslAudioSound::Initialize) Failed to get player interface");
		return false;
	}

    // get the seek interface
    result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_SEEK, &_fdPlayerSeek);
    //if (SL_RESULT_SUCCESS != result)
	//	return false;

    // get the mute/solo interface
    //result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_MUTESOLO, &_fdPlayerMuteSolo);
    //if (SL_RESULT_SUCCESS != result)
	//	return false;

    // get the volume interface
    result = (*_fdPlayerObject)->GetInterface(_fdPlayerObject, SL_IID_VOLUME, &_fdPlayerVolume);
    if (SL_RESULT_SUCCESS != result)
	{
		LOGWARN("(OslAudioSound::Initialize) Failed to get volume interface");
		return false;
	}

    result = (*_fdPlayerVolume)->GetMaxVolumeLevel(_fdPlayerVolume, &_maxVolume);
    //if (SL_RESULT_SUCCESS != result)
	//	return false;

	return true;
}

void OslAudioSound::playSound(bool loop)
{
	SLresult result;

	stopSound();

	if (nullptr != _fdPlayerSeek)
	{
		// reset position to the beginning
		//result = (*_fdPlayerSeek)->SetPosition(_fdPlayerSeek, 0, SL_SEEKMODE_ACCURATE);

		// enable whole file looping
		result = (*_fdPlayerSeek)->SetLoop(_fdPlayerSeek, (loop ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE), 0, SL_TIME_UNKNOWN);
	}

	//if (nullptr != _fdPlayerVolume)
	//{
		// set volume
	//	result = (*_fdPlayerVolume)->SetVolumeLevel(_fdPlayerVolume, volume*_maxVolume);
	//}

	if (nullptr != _fdPlayerPlay)
	{
        // set the player's state
        result = (*_fdPlayerPlay)->SetPlayState(_fdPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void OslAudioSound::pauseSound()
{
	if (nullptr != _fdPlayerPlay)
	{
        // set the player's state to pause
        SLresult result = (*_fdPlayerPlay)->SetPlayState(_fdPlayerPlay, SL_PLAYSTATE_PAUSED);
	}
}

void OslAudioSound::resumeSound()
{
	if (nullptr != _fdPlayerPlay)
	{
        // set the player's state to resume
        SLresult result = (*_fdPlayerPlay)->SetPlayState(_fdPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void OslAudioSound::stopSound()
{
	if (nullptr != _fdPlayerPlay)
	{
		SLuint32 state = SL_PLAYSTATE_STOPPED;
		SLresult result = (*_fdPlayerPlay)->GetPlayState(_fdPlayerPlay, &state);
		if (SL_RESULT_SUCCESS == result && SL_PLAYSTATE_STOPPED != state)
		{
			result = (*_fdPlayerPlay)->SetPlayState(_fdPlayerPlay, SL_PLAYSTATE_STOPPED);
		}
	}
}

bool OslAudioSound::isPlaying()
{
	if (nullptr != _fdPlayerPlay)
	{
		SLuint32 state = SL_PLAYSTATE_STOPPED;
		SLresult result = (*_fdPlayerPlay)->GetPlayState(_fdPlayerPlay, &state);
		return (SL_RESULT_SUCCESS == result && SL_PLAYSTATE_PLAYING == state);
	}

	return false;
}

void OslAudioSound::setVolume(float volume)
{
	if (nullptr != _fdPlayerVolume)
	{
		// set volume
		SLmillibel newVolume = (volume > 0 ? 2000 * log10(volume) : SL_MILLIBEL_MIN);
		SLresult result = (*_fdPlayerVolume)->SetVolumeLevel(_fdPlayerVolume, newVolume);
	}

	_volume = volume;
}

void OslAudioSound::fadeVolume(float fade)
{
	if (nullptr != _fdPlayerVolume)
	{
		// set volume
		SLmillibel newVolume = (fade*_volume > 0 ? 2000 * log10(fade*_volume) : SL_MILLIBEL_MIN);
		SLresult result = (*_fdPlayerVolume)->SetVolumeLevel(_fdPlayerVolume, newVolume);
	}
}

float OslAudioSound::getVolume()
{
	return _volume;
}
