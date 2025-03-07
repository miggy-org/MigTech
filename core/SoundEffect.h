#pragma once

#include "MigDefines.h"

namespace MigTech
{
	class SoundEffect
	{
	public:
		SoundEffect(const std::string& name) { _name = name; }
		const std::string& getName() const { return _name; }

		virtual void playSound(bool loop) = 0;
		virtual void pauseSound() = 0;
		virtual void resumeSound() = 0;
		virtual void stopSound() = 0;
		virtual bool isPlaying() = 0;

		virtual void setVolume(float volume) = 0;
		virtual void fadeVolume(float fade) = 0;
		virtual float getVolume() = 0;

	protected:
		std::string _name;
	};
}