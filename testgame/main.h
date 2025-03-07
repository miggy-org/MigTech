#pragma once

#include "../core/MigGame.h"
#include "../core/SoundEffect.h"
#include "../core/BgBase.h"
#include "cube.h"
#include "texture.h"

// Renders Direct2D and 3D content on the screen.
namespace TestGame
{
	class TestGameMain : public MigTech::MigGame
	{
	public:
		TestGameMain();

		virtual void onCreate();
		virtual void onCreateGraphics();
		virtual void onWindowSizeChanged();
		virtual void onSuspending();
		virtual void onResuming();
		virtual void onDestroyGraphics();
		virtual void onDestroy();

		virtual void onPointerPressed(float x, float y);
		virtual void onPointerReleased(float x, float y);
		virtual void onPointerMoved(float x, float y, bool isInContact);
		virtual void onKeyDown(MigTech::VIRTUAL_KEY key);
		virtual void onKeyUp(MigTech::VIRTUAL_KEY key);

		virtual MigTech::ScreenBase* createStartupScreen();

	private:
	};
}