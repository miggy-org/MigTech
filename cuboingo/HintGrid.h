#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"
#include "CubeConst.h"
#include "GridBase.h"
#include "GameCube.h"
#include "LightBeam.h"

using namespace MigTech;

namespace Cuboingo
{
	class HintGrid : public GridBase
	{
	public:
		HintGrid(const GameCube& cube, const LightBeam& beam, const GridInfo& info);
		~HintGrid();

		bool init();
		void updateGridInfo(const GridInfo& info);

		// causes the hint pieces to glow, newVal should be between 0 and 1
		bool setGlowParam(float newVal, bool isGrowing);
		void setFallParam(float newVal);

		// returns the filled color for this hint grid (not the white, but the light shaft)
		Color getFilledColor() const { return _origInfo.fillCol; }
		bool getCancel() const { return _cancel; }
		void setCancel(bool val) { _cancel = val; }
		const GridInfo& getOrigGrid() const { return _origInfo; }

		void draw(const Matrix& mat, bool showLightBeams) const;

	protected:

	protected:
		const GameCube& _gameCube;
		const LightBeam& _lightBeam;
		GridInfo _origInfo;

		bool _cancel;
		float _animGlowParam;
		bool _animGrowing;
		int* _sortedRenderingOrder;
	};
}