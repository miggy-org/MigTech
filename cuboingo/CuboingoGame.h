#pragma once

#include "../core/MigGame.h"

namespace Cuboingo
{
	class CuboingoGame : public MigTech::MigGame
	{
	public:
		CuboingoGame();

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