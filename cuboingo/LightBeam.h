#pragma once

#include "../core/MigInclude.h"
#include "../core/Object.h"
#include "GridBase.h"

using namespace MigTech;

namespace Cuboingo
{
	class LightBeam
	{
	public:
		LightBeam();
		~LightBeam();

		void createGraphics();
		void destroyGraphics();

		void draw(const GridInfo& gridInfo, float xCenter, float yCenter, float glowParam, bool isGrowing, const Matrix& worldMatrix) const;

	protected:
		Object* _objBeam;
	};
}
