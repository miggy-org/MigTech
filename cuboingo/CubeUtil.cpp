#include "pch.h"
#include "CubeUtil.h"
#include "CubeConst.h"
#include "../core/PerfMon.h"

using namespace MigTech;
using namespace Cuboingo;

// static init of quality controls
bool CubeUtil::useReflections = true;
bool CubeUtil::useShadows = true;
bool CubeUtil::useAntiAliasing = true;
bool CubeUtil::playMusic = true;
bool CubeUtil::playSounds = true;
bool CubeUtil::showPopupScores = true;
CuboingoRenderPass CubeUtil::renderPass = RENDER_PASS_FINAL;
PowerUp CubeUtil::currPowerUp = POWERUPTYPE_NONE;

void CubeUtil::loadPersistentSettings()
{
	if (MigUtil::thePersist != nullptr)
	{
		useReflections = (MigUtil::thePersist->getValue(KEY_REFLECTIONS, (useReflections ? 1 : 0)) ? true : false);
		useShadows = (MigUtil::thePersist->getValue(KEY_SHADOWS, (useShadows ? 1 : 0)) ? true : false);
		useAntiAliasing = (MigUtil::thePersist->getValue(KEY_ANTIALIASING, (useAntiAliasing ? 1 : 0)) ? true : false);

		if (MigUtil::theAudio != nullptr)
		{
			MigUtil::theAudio->setChannelVolume(AudioBase::AUDIO_CHANNEL_MUSIC, MigUtil::thePersist->getValue(KEY_MUSIC_VOLUME, 1.0f));
			MigUtil::theAudio->setChannelVolume(AudioBase::AUDIO_CHANNEL_SOUND, MigUtil::thePersist->getValue(KEY_SOUND_VOLUME, 1.0f));
		}
		else
			LOGWARN("(CubeUtil::loadPersistentSettings) Audio manager is offline");

		PerfMon::showFPS(MigUtil::thePersist->getValue(KEY_PERF_MON, 0) ? true : false);

		LOGINFO("(CubeUtil::loadPersistentSettings) Reflections are %s, shadows are %s",
			(useReflections ? "on" : "off"), (useShadows ? "on" : "off"));
	}
	else
		LOGWARN("(CubeUtil::loadPersistentSettings) Unable to load setting, persistence database is offline");
}

void CubeUtil::savePersistentSettings()
{
	if (MigUtil::thePersist != nullptr)
	{
		MigUtil::thePersist->putValue(KEY_REFLECTIONS, useReflections);
		MigUtil::thePersist->putValue(KEY_SHADOWS, useShadows);
		MigUtil::thePersist->putValue(KEY_ANTIALIASING, useAntiAliasing);

		if (MigUtil::theAudio != nullptr)
		{
			MigUtil::thePersist->putValue(KEY_MUSIC_VOLUME, MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_MUSIC));
			MigUtil::thePersist->putValue(KEY_SOUND_VOLUME, MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_SOUND));
		}

		MigUtil::thePersist->putValue(KEY_PERF_MON, PerfMon::isFPSOn());

		MigUtil::thePersist->commit();
	}
}

void CubeUtil::loadDefaultPerspectiveMatrix(Matrix& mat)
{
	// compute aspect ratio
	MigTech::Size outputSize = MigUtil::theRend->getOutputSize();
	float aspectRatio = outputSize.width / outputSize.height;
	float fovAngleY = MigUtil::convertToRadians(Cuboingo::defFOVAngleY);

	// correct for portrait view
	if (aspectRatio < 1.0f)
		fovAngleY *= 2.0f;

	mat.loadPerspectiveFovRH(fovAngleY, aspectRatio, Cuboingo::defNearPlane, Cuboingo::defFarPlane);
}

void CubeUtil::loadDefaultViewMatrix(Matrix& mat, float rotateY)
{
	if (rotateY != 0)
	{
		Matrix tmpMatrix;
		tmpMatrix.loadLookAtRH(Cuboingo::defEye, Cuboingo::defAt, Cuboingo::defUp);
		mat.identity();
		mat.rotateY(rotateY);
		mat.multiply(tmpMatrix);
	}
	else
		mat.loadLookAtRH(Cuboingo::defEye, Cuboingo::defAt, Cuboingo::defUp);
}

std::string CubeUtil::getUpperSubString(const std::string& str, int len)
{
	return MigUtil::toUpper(str).substr(0, len);
}

const char* CubeUtil::axisToString(AxisOrient axis)
{
	if (axis == AXISORIENT_X)
		return "X";
	else if (axis == AXISORIENT_Y)
		return "Y";
	return "Z";
}
