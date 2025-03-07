#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/MovieClip.h"
#include "CubeConst.h"

using namespace MigTech;

namespace Cuboingo
{
	class ShadowPass : public RenderPass
	{
	public:
		ShadowPass();

		bool init();

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual bool isValid() const;
		virtual bool preRender();
		virtual void postRender();

		void draw();

	protected:
		Object* _shadowPoly;
		Color _shadowObjColor;
	};
}
