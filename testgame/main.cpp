#include "pch.h"
#include "../core/MigUtil.h"
#include "../core/MigConst.h"
#include "main.h"
#include "screen.h"

using namespace TestGame;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// allocator for the MigGame singleton

MigTech::MigGame* allocGame()
{
	return new TestGameMain();
}

TestGameMain::TestGameMain() : MigGame("testgame")
{
}

void TestGameMain::onCreate()
{
	MigGame::onCreate();
}

void TestGameMain::onCreateGraphics()
{
	MigGame::onCreateGraphics();
}

void TestGameMain::onWindowSizeChanged()
{
	MigGame::onWindowSizeChanged();
}

void TestGameMain::onSuspending()
{
	MigGame::onSuspending();
}

void TestGameMain::onResuming()
{
	MigGame::onResuming();
}

void TestGameMain::onDestroyGraphics()
{
	MigGame::onDestroyGraphics();
}

void TestGameMain::onDestroy()
{
	MigGame::onDestroy();
}

void TestGameMain::onPointerPressed(float x, float y)
{
	MigGame::onPointerPressed(x, y);
}

void TestGameMain::onPointerReleased(float x, float y)
{
	MigGame::onPointerReleased(x, y);
}

void TestGameMain::onPointerMoved(float x, float y, bool isInContact)
{
	MigGame::onPointerMoved(x, y, isInContact);
}

void TestGameMain::onKeyDown(VIRTUAL_KEY key)
{
	MigGame::onKeyDown(key);
}

void TestGameMain::onKeyUp(VIRTUAL_KEY key)
{
	MigGame::onKeyUp(key);
}

ScreenBase* TestGameMain::createStartupScreen()
{
	return new TestScreen();
}
