#pragma once

#include "../core/MigInclude.h"
#include "../core/OverlayBase.h"
#include "../core/MigUtil.h"
#include "../core/Font.h"

using namespace MigTech;

namespace TestGame
{
	class TestOverlay : public OverlayBase
	{
	public:
		TestOverlay() : OverlayBase("TestOverlay") { }

	protected:
		virtual bool onBackKey();
	};
}
