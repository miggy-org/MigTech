#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"
#include "GridBase.h"

using namespace MigTech;

namespace Cuboingo
{
	// the basic white cube
	class CubeBase : public MigBase, public IAnimTarget
	{
	public:
		static const int FACE_FRONT = 0;
		static const int FACE_RIGHT = 1;
		static const int FACE_TOP = 2;
		static const int FACE_BACK = 3;
		static const int FACE_LEFT = 4;
		static const int FACE_BOTTOM = 5;

	public:
		CubeBase(float radX, float radY, float radZ);

		//virtual void init(bool useReflections, bool useRoundedEdges);
		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void draw(Matrix& mat, const Color& col) const;
		virtual void draw(Matrix& mat) const;
		virtual void draw() const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		virtual void applyTransform(Matrix& worldMatrix) const;

	protected:
		VertexPositionNormalTexture* createVertices(bool useRoundedEdges, unsigned int& arraySize);
		unsigned short* createIndices(bool useRoundedEdges, unsigned int& arraySize);

	protected:
		float _radiusX, _radiusY, _radiusZ;
		bool _isRounded;
		Object* _cubeObj;

		// rotation, scaling, translation, etc
		Vector3 _translate;
		float _rotX, _rotY, _rotZ, _scale;
		Color _color;

		// reserved for the animations
		AnimID _idRotX, _idRotY, _idRotZ, _idScale;
		AnimID _idTransX, _idTransY, _idTransZ;
		AnimID _idOpacity;

		// shaders
		std::string _vertexShader;
		std::string _pixelShader;
		bool _useShadows;
		bool _writeDepth;

		// visual control
		std::string _textureName;
		std::string _reflectName;
		float _reflectIntensity;
	};

	// white cube w/ support for 6 grids, one on each side
	class CubeGrid : public CubeBase
	{
	public:
		static const int NUM_GRIDS = 6;

	public:
		CubeGrid(float radius);
		virtual ~CubeGrid();

		virtual void draw(Matrix& mat, const Color& col) const;
		virtual void draw(Matrix& mat) const;
		virtual void draw() const;

	protected:
		void deleteAllGrids();

	protected:
		GridBase* _theGrids[NUM_GRIDS];
	};
}