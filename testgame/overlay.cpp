#include "pch.h"
#include "overlay.h"

using namespace TestGame;
using namespace MigTech;

bool TestOverlay::onBackKey()
{
	startExitAnimation();
	return true;
}
