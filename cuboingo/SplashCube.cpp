#include "pch.h"
#include "CubeConst.h"
#include "SplashCube.h"
#include "../core/AnimList.h"

using namespace MigTech;
using namespace Cuboingo;

///////////////////////////////////////////////////////////////////////////
// SplashGrid

SplashGrid::SplashGrid()
{
}

bool SplashGrid::startFlashingAnimation()
{
	// pick a random slot in the grid to flash
	int slotIndex = MigUtil::pickRandom(4);

	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, 2000, AnimItem::ANIM_TYPE_LINEAR_BOUNCE, reinterpret_cast<void*>(slotIndex));
	return (MigUtil::theAnimList->addItem(animItem) > 0);
}

bool SplashGrid::doFrame(int id, float newVal, void* optData)
{
	float red = _emptyCol.r + (_fillCol.r - _emptyCol.r) * newVal;
	float grn = _emptyCol.g + (_fillCol.g - _emptyCol.g) * newVal;
	float blu = _emptyCol.b + (_fillCol.b - _emptyCol.b) * newVal;

	auto slotIndex = reinterpret_cast<std::uintptr_t>(optData);
	_theSlots[slotIndex].setColor(red, grn, blu);
	return true;
}

void SplashGrid::animComplete(int id, void* optData)
{
}

///////////////////////////////////////////////////////////////////////////
// SplashCube

SplashCube::SplashCube() : CubeGrid(Cuboingo::defCubeRadius)
{
	_nextGridToAnimate = 0;

	// supports shadows
	_useShadows = true;
}

void SplashCube::init()
{
	// set up the timer that will create flashing slot animations
	AnimItem animItem(this);
	animItem.configTimer(350, true);
	_idTimer = MigUtil::theAnimList->addItem(animItem);

	// set up the grids on the sides of the cube
	_theGrids[FACE_FRONT] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_FRONT])->init(2, 0, AXISORIENT_Z, defCubeRadius + 0.02f);
	((SplashGrid*)_theGrids[FACE_FRONT])->setAllColors(0.41f, 0.98f, 0.00f, 1.00f, 1.00f, 1.00f);
	_theGrids[FACE_RIGHT] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_RIGHT])->init(2, 1, AXISORIENT_X, defCubeRadius + 0.02f);
	((SplashGrid*)_theGrids[FACE_RIGHT])->setAllColors(0.98f, 0.78f, 0.00f, 1.00f, 1.00f, 1.00f);
	_theGrids[FACE_TOP] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_TOP])->init(2, 2, AXISORIENT_Y, defCubeRadius + 0.02f);
	((SplashGrid*)_theGrids[FACE_TOP])->setAllColors(1.00f, 0.00f, 0.00f, 1.00f, 1.00f, 1.00f);
	_theGrids[FACE_BACK] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_BACK])->init(2, 3, AXISORIENT_Z, -(defCubeRadius + 0.02f));
	((SplashGrid*)_theGrids[FACE_BACK])->setAllColors(0.63f, 0.24f, 1.00f, 1.00f, 1.00f, 1.00f);
	_theGrids[FACE_LEFT] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_LEFT])->init(2, 4, AXISORIENT_X, -(defCubeRadius + 0.02f));
	((SplashGrid*)_theGrids[FACE_LEFT])->setAllColors(0.20f, 0.63f, 1.00f, 1.00f, 1.00f, 1.00f);
	_theGrids[FACE_BOTTOM] = new SplashGrid();
	((SplashGrid*)_theGrids[FACE_BOTTOM])->init(2, 5, AXISORIENT_Y, -(defCubeRadius + 0.02f));
	((SplashGrid*)_theGrids[FACE_BOTTOM])->setAllColors(0.00f, 0.00f, 1.00f, 1.00f, 1.00f, 1.00f);
}

void SplashCube::startRotation()
{
	// set up the cube rotation animation
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, MigUtil::convertToRadians(90), 5000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
	_idRotY = MigUtil::theAnimList->addItem(animItem);
}

int SplashCube::startExitAnimation()
{
	_idRotY.clearAnim();
	_idTimer.clearAnim();

	// set up the exit animations
	AnimItem animItemX(this);
	animItemX.configSimpleAnim(0, MigUtil::convertToRadians(25), EXIT_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotX = MigUtil::theAnimList->addItem(animItemX);
	AnimItem animItemY(this);
	animItemY.configSimpleAnim(_rotY, _rotY + MigUtil::convertToRadians(200), EXIT_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotY = MigUtil::theAnimList->addItem(animItemY);
	AnimItem animItemZ(this);
	animItemZ.configSimpleAnim(0, MigUtil::convertToRadians(20), EXIT_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotZ = MigUtil::theAnimList->addItem(animItemZ);

	// the scale exit animation is a little more complicated
	float fy[] = { 1.0f, 1.12f, 1.2f, 1.25f, 1.2f, 1.15f, 1.0f, 0.85f, 0.65f, 0.35f, 0.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, 1, EXIT_ANIM_LENGTH, fy, ARRAYSIZE(fy));
	_idScale = MigUtil::theAnimList->addItem(animItemS);

	return EXIT_ANIM_LENGTH;
}

bool SplashCube::newFallingGridCandidate(GridInfo& info, int holeLimit) const
{
	// pick a random one grid
	int chosenIndex = (int)MigUtil::pickRandom(NUM_GRIDS);
	const GridBase* chosenGrid = _theGrids[chosenIndex];

	// invert the filled bit, and then randomly turn some off
	info.initFromGrid(*chosenGrid, false);
	info.invertFillList();
	info.randomizeFillList(3);

	return true;
}

bool SplashCube::doFrame(int id, float newVal, void* optData)
{
	if (_idTimer == id)
	{
		// set up an animation to flash a slot
		int gridIndex = (_nextGridToAnimate++) % NUM_GRIDS;
		((SplashGrid*) (_theGrids[gridIndex]))->startFlashingAnimation();
		return true;
	}

	return CubeGrid::doFrame(id, newVal, optData);
}
