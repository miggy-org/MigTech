#pragma once

namespace MigTech
{
	class Timer
	{
	///////////////////////////////////////////////////////////////////////////
	// utils

	public:
		static const uint64 ticksPerSecond = 10000000;

		static double ticksToSeconds(uint64 ticks)			{ return (double) (ticks) / ticksPerSecond; }
		static uint64 secondsToTicks(double seconds)		{ return (uint64) (seconds * ticksPerSecond); }
		static long ticksToMilliSeconds(uint64 ticks)		{ return (long) (1000*(ticks / (double) ticksPerSecond)); }
		static uint64 milliSecondsToTicks(double mseconds)	{ return (uint64) (mseconds * ticksPerSecond / 1000); }

	///////////////////////////////////////////////////////////////////////////
	// these are for the global game timer

	private:
		// current time in milliseconds
		static uint64 currTimeTicks;
		static uint64 systemTimeTicks;

		// global game timer
		static Timer gameTimer;

	public:
		// initialize the global game timer
		static bool init();

		// update the global game timer
		static void updateGameTime();

		// pause the game timer
		static void pauseGameTime();

		// resume the game timer
		static void resumeGameTime();

		// is the global game timer paused
		static bool isGameTimePaused();

		// get the global game time
		static uint64 gameTime();
		static long gameTimeMillis();

		// get the global elapsed time (doesn't take game pause into effect)
		static uint64 systemTime();
		static long systemTimeMillis();

	///////////////////////////////////////////////////////////////////////////
	// the rest of these are for private timers

	private:
		uint64 _pauseTimeTicks;
		uint64 _pauseStart;

	public:
		Timer();

		// pauses the timer
		void pauseTime();

		// resumes the timer
		void resumeTime();

		// determines if the timer is paused
		bool isPaused() const;

		// returns the current time
		uint64 getElapsedTime() const;
		long getElapsedTimeMillis() const;
	};
}