#pragma once

#include "CubeBase.h"

using namespace MigTech;

namespace Cuboingo
{
	class SplashGrid : public GridBase, public IAnimTarget
	{
	public:
		SplashGrid();

		bool startFlashingAnimation();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);
	};

	class SplashCube : public CubeGrid
	{
	protected:
		static const int EXIT_ANIM_LENGTH = 1000;

	public:
		SplashCube();

		void init();
		void startRotation();
		int startExitAnimation();

		bool newFallingGridCandidate(GridInfo& info, int holeLimit) const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);

	protected:
		AnimID _idTimer;
		int _nextGridToAnimate;
	};
}