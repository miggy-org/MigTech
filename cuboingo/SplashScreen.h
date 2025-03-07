#pragma once

#include "../core/MigInclude.h"
#include "../core/ScreenBase.h"
#include "../core/MigUtil.h"
#include "../core/Font.h"
#include "SplashCube.h"
#include "FallingGrid.h"
#include "ShadowPass.h"

using namespace MigTech;

namespace Cuboingo
{
	class SplashLauncher : public MigBase, public IAnimTarget
	{
	public:
		SplashLauncher();
		~SplashLauncher();

		void init();
		void startGridLaunch(GridInfo& newGrid);
		void draw() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		std::list<FallingGrid*> _fallList;
		AxisOrient _lastOrient;
	};

	class SplashScreen : public MigTech::ScreenBase
	{
	public:
		SplashScreen();

		virtual ScreenBase* getNextScreen();

		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();

		virtual bool render();
		virtual bool renderPass(int index, RenderPass* pass);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IOverlayCallback
		virtual void onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration);
		virtual void onOverlayCustom(int id, void* data);

	protected:
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		void startExitAnimation();
		void doLaunch();

	private:
		Matrix _viewMatrix;
		Matrix _projMatrix;
		SplashCube _cube;
		SplashLauncher _launcher;
		AnimID _idTimer;
		AnimID _idLaunch;

		ShadowPass _shadowPass;
	};
}
