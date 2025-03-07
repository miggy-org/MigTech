#pragma once

#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"

namespace TestGame
{
	class Cube : public MigTech::IAnimTarget
	{
	public:
		Cube();
		~Cube();

		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		void Translate(float x, float y);
		void RotateDir(float r);
		void Render();

	private:
		// Resources for cube geometry
		MigTech::Object* m_cube;
		MigTech::Matrix m_matrix;

		// animation IDs
		int m_idAnim1;
		int m_idAnim2;

		// animation values
		float m_rotateY;
		float m_rotateDir;
		float m_translateX;
		float m_translateY;
		float m_alpha;
	};
}
