#include "pch.h"
#include "../core/MigUtil.h"
#include "../core/MigConst.h"
#include "CuboingoGame.h"
#include "CubeUtil.h"
#include "GameScripts.h"
#include "SplashScreen.h"
#include "GameScreen.h"

using namespace MigTech;
using namespace Cuboingo;

///////////////////////////////////////////////////////////////////////////
// allocator for the MigGame singleton

MigTech::MigGame* allocGame()
{
	return new CuboingoGame();
}

CuboingoGame::CuboingoGame() : MigGame("cuboingo")
{
}

void CuboingoGame::onCreate()
{
	MigGame::onCreate();

	// load persistent settings
	CubeUtil::loadPersistentSettings();

	// init the game scripts
	GameScripts::init(_cfgRoot);
}

void CuboingoGame::onCreateGraphics()
{
	MigGame::onCreateGraphics();

	// static init of grid shared geometry
	GridBase::createGraphics();
}

void CuboingoGame::onWindowSizeChanged()
{
	MigGame::onWindowSizeChanged();
}

void CuboingoGame::onSuspending()
{
	MigGame::onSuspending();
}

void CuboingoGame::onResuming()
{
	MigGame::onResuming();
}

void CuboingoGame::onDestroyGraphics()
{
	MigGame::onDestroyGraphics();

	// static init of grid shared geometry
	GridBase::destroyGraphics();
}

void CuboingoGame::onDestroy()
{
	MigGame::onDestroy();

	GameScripts::clear();
}

void CuboingoGame::onPointerPressed(float x, float y)
{
	MigGame::onPointerPressed(x, y);
}

void CuboingoGame::onPointerReleased(float x, float y)
{
	MigGame::onPointerReleased(x, y);
}

void CuboingoGame::onPointerMoved(float x, float y, bool isInContact)
{
	MigGame::onPointerMoved(x, y, isInContact);
}

void CuboingoGame::onKeyDown(VIRTUAL_KEY key)
{
	MigGame::onKeyDown(key);
}

void CuboingoGame::onKeyUp(VIRTUAL_KEY key)
{
	MigGame::onKeyUp(key);
}

ScreenBase* CuboingoGame::createStartupScreen()
{
	return new SplashScreen();
	//return new GameScreen();
}
