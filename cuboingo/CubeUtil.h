#pragma once

#include "../core/MigInclude.h"
#include "../core/Matrix.h"
#include "CubeConst.h"
#include "PowerUp.h"

using namespace MigTech;

namespace Cuboingo
{
	class CubeUtil
	{
	public:
		// controls
		static bool useReflections;
		static bool useShadows;
		static bool useAntiAliasing;
		static bool playMusic;
		static bool playSounds;
		static bool showPopupScores;

		// current render pass
		static CuboingoRenderPass renderPass;

		// current power up
		static PowerUp currPowerUp;

	public:
		static void loadPersistentSettings();
		static void savePersistentSettings();

		static void loadDefaultPerspectiveMatrix(Matrix& mat);
		static void loadDefaultViewMatrix(Matrix& mat, float rotateY);

		static std::string getUpperSubString(const std::string& str, int len);

		static const char* axisToString(AxisOrient axis);
	};
}