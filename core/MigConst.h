#pragma once

#include "MigDefines.h"

namespace MigTech
{
	const float PI = 3.141592654f;

	// common angles in radians
	const float rad45 = 45 * PI / 180.0f;
	const float rad90 = 90 * PI / 180.0f;
	const float rad180 = 180 * PI / 180.0f;

	// default duration for all screen fades (both in and out)
	const long defFadeDuration = 1000;
	const long defFastFadeDuration = 500;

	// default fade level for screens behind overlays
	const float defOverlayFade = 0.8f;

	// default common colors
	const Color colBlack(0, 0, 0, 1);
	const Color colWhite(1, 1, 1, 1);

	// default common vectors
	const Vector3 ptOrigin(0, 0, 0);
	const Vector3 unitX(1, 0, 0);
	const Vector3 unitY(0, 1, 0);
	const Vector3 unitZ(0, 0, 1);

	// default watchdog period
	const int defWatchdogPeriod = 5;
}