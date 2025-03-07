#pragma once

#include "../core/MigDefines.h"

using namespace MigTech;

namespace Cuboingo
{
	// default cube radius
	const float defCubeRadius = 1;

	// default cube rounded edge deltah
	const float defCubeRoundedDelta = 0.05f;

	// default grid depth
	const float defGridDepth = 0.1f;

	// default time to animate a slot (but this happens twice, once to shrink and once to grow)
	const int defSlotAnimDuration = 250;

	// default starting distance for falling pieces
	const float defFallStartPos = 4;

	// default time for the cube hit and miss animations
	const int defCubeHitAnimDuration = 360;
	const int defCubeMissAnimDuration = 480;

	// default time for the tap acceleration animations
	const int defTapAccelAnimDuration = 650;

	// default time for the rejection animation
	const int defRejectAnimDuration = 500;

	// default stamp radius
	const float defStampRadius = 1.0f;
	const float defStampDepth = 0.1f;

	// default distance from origin to the stamps
	const float defStampDistXZ = 5;
	const float defStampDistY = 2.8f;
	const float defStampRecoil = 2;

	// default view / projection settings
	const Vector3 defEye = Vector3(0.0f, 3.0f, 5.2f);
	const Vector3 defAt  = Vector3(0.0f, 0.8f, 0.0f);
	const Vector3 defUp  = Vector3(0.0f, 1.0f, 0.0f);
	const float defFOVAngleY = 70.0f;
	const float defNearPlane = 1.0f;
	const float defFarPlane = 12.0f;

	// names of values that will be persisted
	const std::string KEY_LAST_USED_SCRIPT_INDEX = "LastUsedScriptIndex";
	const std::string KEY_SHADOWS = "Shadows";
	const std::string KEY_REFLECTIONS = "Reflections";
	const std::string KEY_ANTIALIASING = "Antialiasing";
	const std::string KEY_PARTICLES = "Particles";
	const std::string KEY_PERF_MON = "PerfMon";
	const std::string KEY_MUSIC_VOLUME = "MusicVolume";
	const std::string KEY_SOUND_VOLUME = "SoundVolume";

	// media files
	const std::string WAV_START = "start.wav";
	const std::string WAV_SPLASH_EXIT = "splash_exit.wav";
	const std::string WAV_ROTATE = "rotate.wav";
	const std::string WAV_STAMP = "stamp.wav";
	const std::string WAV_MATCH = "match.wav";
	const std::string WAV_SAME = "same.wav";
	const std::string WAV_ERROR = "error.wav";
	const std::string WAV_TAP = "tap.wav";
	const std::string WAV_PAUSE = "pause.wav";
	const std::string WAV_FILL1 = "fill1.wav";
	const std::string WAV_FILL2 = "fill2.wav";
	const std::string WAV_FILL3 = "fill3.wav";
	const std::string WAV_FILL4 = "fill4.wav";
	const std::string WAV_FILL5 = "fill5.wav";
	const std::string WAV_FILL6 = "fill6.wav";
	const std::string WAV_WIN = "win.wav";
	const std::string WAV_DEATH = "death.wav";

	// grid locking
	enum GridLock
	{
		GRIDLOCK_NONE,	    // none
		GRIDLOCK_WAIT,	    // falling pieces are locked to a given face, pause until that face is complete
		GRIDLOCK_NO_WAIT	// falling pieces are locked to a given face, no pause (evil)
	};

	// launch axis styles
	enum LaunchStyle
	{
		LAUNCHSTYLE_NORMAL,     // falling pieces are launched using random axis
		LAUNCHSTYLE_SEQUENTIAL, // falling pieces are launched using sequential axis
		LAUNCHSTYLE_BY_COLOR    // falling pieces are launched using sequential axis, but changes when a face is exhausted
	};

	// axis locking (used by certain falling grid types)
	enum AxisLock
	{
		AXISLOCK_NONE,	    // none
		AXISLOCK_SINGLE, 	// locks the used axis only
		AXISLOCK_ALL	    // locks all axes
	};

	// 3D axis orientations
	enum AxisOrient
	{
		AXISORIENT_NONE,
		AXISORIENT_Z,
		AXISORIENT_X,
		AXISORIENT_Y
	};

	// stype of stamping
	enum StampStyle
	{
		STAMPSTYLE_BONUS_RANDOM,          // will appear to match a complete face on the grid, for bonus points only
		STAMPSTYLE_BONUS_ALWAYS,          // always available for bonuses that can be chained in sequence
		STAMPSTYLE_COMPLETES_FACE,        // will complement an incomplete face on the grid
		STAMPSTYLE_POWERUP_COLOR,         // grid that cycles through filled cube colors, awards a power-up
		STAMPSTYLE_POWERUP_FILLED_COLOR,  // grid that cycles through all cube colors, awards a power-up
		STAMPSTYLE_POWERUP_FACE           // grid that cycles through all cube faces, awards a power-up
	};

	// power ups
	enum PowerUpType
	{
		POWERUPTYPE_NONE,
		POWERUPTYPE_DROP_MISS,     // one miss is dropped (not in use at this time)
		POWERUPTYPE_INVULNERABLE,  // cube is temporarily invulnerable to slips
		POWERUPTYPE_MULTIPLIER     // temporary point multiplier is awarded
	};

	// style of evil falling grids
	enum EvilGridStyle
	{
		EVILGRIDSTYLE_COLOR_ONLY,	// color only, the entire grid is filled in
		EVILGRIDSTYLE_COMPLETE,		// the grids complement unfilled sides of the cube
		EVILGRIDSTYLE_RANDOMIZED	// the grids are randomized
	};

	// style of wild falling grids
	enum WildGridStyle
	{
		WILDGRIDSTYLE_NORMAL		// only style available so far
	};

	// render passes
	enum CuboingoRenderPass
	{
		RENDER_PASS_FINAL,
		RENDER_PASS_SHADOW
	};
}