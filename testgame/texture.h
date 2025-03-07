#pragma once

#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"

namespace TestGame
{
	class Texture
	{
	public:
		Texture();
		~Texture();

		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();

		void Render();

	private:
		// Resources for texture geometry
		MigTech::Object* m_txtObj;
	};
}
