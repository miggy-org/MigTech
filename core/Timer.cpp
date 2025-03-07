#include "pch.h"
#include "Timer.h"

using namespace MigTech;

// current time in milliseconds
uint64 Timer::currTimeTicks;
uint64 Timer::systemTimeTicks;

// global game timer
Timer Timer::gameTimer;

///////////////////////////////////////////////////////////////////////////
// platform specific

extern bool plat_initTimer();
extern uint64 plat_getCurrentTicks();

///////////////////////////////////////////////////////////////////////////
// global game timer

// initialize the global game timer
bool Timer::init()
{
	currTimeTicks = 0;
	systemTimeTicks = 0;

	return plat_initTimer();
}

// update the global game timer
void Timer::updateGameTime()
{
	uint64 newTicks = plat_getCurrentTicks();

	systemTimeTicks = newTicks;
	if (!gameTimer.isPaused() && newTicks > currTimeTicks)
		currTimeTicks = newTicks;
}

// pause the game timer
void Timer::pauseGameTime()
{
	gameTimer.pauseTime();
}

// resume the game timer
void Timer::resumeGameTime()
{
	gameTimer.resumeTime();
}

// is the global game timer paused
bool Timer::isGameTimePaused()
{
	return gameTimer.isPaused();
}

// get the global game time
uint64 Timer::gameTime()
{
	return currTimeTicks - gameTimer._pauseTimeTicks;
}

// get the global game time (ms)
long Timer::gameTimeMillis()
{
	return ticksToMilliSeconds(gameTime());
}

// get the global elapsed time (doesn't take game pause into effect)
uint64 Timer::systemTime()
{
	return systemTimeTicks;
}

// get the global elapsed time (ms)
long Timer::systemTimeMillis()
{
	return ticksToMilliSeconds(systemTime());
}

///////////////////////////////////////////////////////////////////////////
// private timers

Timer::Timer()
	: _pauseTimeTicks(0), _pauseStart(0)
{
}

// pauses the timer
void Timer::pauseTime()
{
	if (_pauseStart == 0)
		_pauseStart = systemTime();
}

// resumes the timer
void Timer::resumeTime()
{
	if (_pauseStart > 0)
	{
		// update the timer w/ the time the game was paused
		_pauseTimeTicks += (systemTime() - _pauseStart);
		_pauseStart = 0;
	}
}

// determines if the timer is paused
bool Timer::isPaused() const
{
	return (_pauseStart > 0);
}

// returns the current time
uint64 Timer::getElapsedTime() const
{
	return (!isPaused() ? gameTime() : _pauseStart) - _pauseTimeTicks;
}

// returns the current time (ms)
long Timer::getElapsedTimeMillis() const
{
	return Timer::ticksToMilliSeconds(getElapsedTime());
}
