#include "pch.h"
#include "CubeConst.h"
#include "CubeUtil.h"
#include "GameCube.h"
#include "GameGrid.h"
#include "GameScripts.h"
#include "../core/AnimList.h"
#include "../core/Timer.h"

using namespace MigTech;
using namespace Cuboingo;

///////////////////////////////////////////////////////////////////////////
// GameCubeShield

// shield globals
static const float defCubeShieldRadius = defCubeRadius + 0.1f;
static const Color defFlashShieldColor = colWhite;
static const Color defFullShieldColor = Color(0.0f, 0.5f, 1.0f, 0.5f);
static const Color defEmptyShieldColor = Color(1.0f, 0.0f, 0.0f, 0.5f);
static const int defFlashAnimDuration = 500;

GameCubeShield::GameCubeShield() : CubeBase(defCubeShieldRadius, defCubeShieldRadius, defCubeShieldRadius)
{
	_vertexShader = "cvs_Shield";
	_pixelShader = "cps_Shield";
	_writeDepth = false;
}

void GameCubeShield::doHitAnimation()
{
	AnimItem animItem(this);
	animItem.configSimpleAnim(1, 0, defFlashAnimDuration, AnimItem::ANIM_TYPE_LINEAR);
	_idHitAnim = MigUtil::theAnimList->addItem(animItem);
	_flashParam = 1;
}

bool GameCubeShield::isVisible() const
{
	return (CubeUtil::currPowerUp == POWERUPTYPE_INVULNERABLE || _idHitAnim.isActive());
}

bool GameCubeShield::doFrame(int id, float newVal, void* optData)
{
	if (_idHitAnim == id)
		_flashParam = newVal;
	return CubeBase::doFrame(id, newVal, optData);
}

void GameCubeShield::animComplete(int id, void* optData)
{
	if (_idHitAnim == id)
		_idHitAnim = 0;
	CubeBase::animComplete(id, optData);
}

void GameCubeShield::draw(Matrix& mat) const
{
	// this will draw the correct color if the shield is timed
	Color drawColor;
	if (CubeUtil::currPowerUp == POWERUPTYPE_INVULNERABLE)
		drawColor = MigUtil::blendColors(defEmptyShieldColor, defFullShieldColor, CubeUtil::currPowerUp.getInvStrength());
	else
		drawColor = Color(colBlack, 0.0f);
	if (_idHitAnim.isActive())
		drawColor = MigUtil::blendColors(drawColor, defFlashShieldColor, _flashParam);
	CubeBase::draw(mat, drawColor);
}

///////////////////////////////////////////////////////////////////////////
// GameCube

const int INTRO_ANIM_LENGTH = 750;
const int WIN_ANIM_LENGTH = 3000;
const int LOSE_ANIM_LENGTH = 2000;

GameCube::GameCube(IGameCubeCallback* cb) : CubeGrid(Cuboingo::defCubeRadius)
{
	_rotAxis = ROTATE_AXIS_NONE;
	_rotDir = 0;
	_shuffleComplete = false;
	_nextFallingGridID = 1;
	_lastChosenGrid = nullptr;
	_callback = cb;
	_idStampAnimation = 0;

	// invisible to start
	_scale = 0;

	// supports shadows
	_useShadows = true;
}

// produces an array of ints based upon the number of colors allowed
static const int* getAllowedColorIndexArray(int numColors, int maxColors)
{
	static int indexArray[6];

	std::vector<int> candidates;
	for (int i = 0; i < maxColors; i++)
		candidates.push_back(i);

	for (int i = 0; i < numColors; i++)
	{
		int index = MigUtil::pickRandom(candidates.size());
		indexArray[i] = candidates[index];

		std::vector<int>::iterator iter = candidates.begin();
		for (int j = 0; j < index; j++)
			iter++;
		candidates.erase(iter);
	}

	return indexArray;
}

// creates a single grid for the cube
static GameGrid* makeGrid(int gridLevel, int index, AxisOrient orient, float dist, const Color& dim, const Color& max)
{
	GameGrid* theGrid = new GameGrid();

	theGrid->init(gridLevel, index, orient, dist);
	theGrid->setAllColors(dim, max);
	return theGrid;
}

// cube grid initialization - this version allows us to configure the color of each face directly
void GameCube::initCubeGrids(int gridLevel, int numColors, const Color* dim, const Color* max, const int* indArray)
{
	// clear resources for the existing faces
	//destroyGraphics();

	// delete the existing grids
	deleteAllGrids();
	_lastChosenGrid = nullptr;

	// cache game level settings
	float cubeRadius = defCubeRadius + 0.02f;

	// set up the grids on the sides of the cube
	int index = indArray[0];
	_theGrids[FACE_FRONT] = makeGrid(gridLevel, index, AXISORIENT_Z, cubeRadius, dim[index], max[index]);
	index = indArray[1 % numColors];
	_theGrids[FACE_RIGHT] = makeGrid(gridLevel, index, AXISORIENT_X, cubeRadius, dim[index], max[index]);
	index = indArray[2 % numColors];
	_theGrids[FACE_TOP] = makeGrid(gridLevel, index, AXISORIENT_Y, cubeRadius, dim[index], max[index]);
	index = indArray[3 % numColors];
	_theGrids[FACE_BACK] = makeGrid(gridLevel, index, AXISORIENT_Z, -cubeRadius, dim[index], max[index]);
	index = indArray[4 % numColors];
	_theGrids[FACE_LEFT] = makeGrid(gridLevel, index, AXISORIENT_X, -cubeRadius, dim[index], max[index]);
	index = indArray[5 % numColors];
	_theGrids[FACE_BOTTOM] = makeGrid(gridLevel, index, AXISORIENT_Y, -cubeRadius, dim[index], max[index]);

	// create new graphics
	//createGraphics();
}

// cube grid initialization - creates a random set of faces
void GameCube::initCubeGrids(int gridLevel, int numColors, const Color* dim, const Color* max)
{
	// cache game level settings
	const int* indArray = getAllowedColorIndexArray(numColors, 6);
	initCubeGrids(gridLevel, numColors, dim, max, indArray);
}

// cube grid initialization - uses the current game script for all the settings
void GameCube::initCubeGrids()
{
	const Level& lvl = GameScripts::getCurrLevel();
	initCubeGrids(lvl.gridLevel, lvl.numColors, lvl.dimColors, lvl.maxColors);
}

void GameCube::create()
{
	// load script visual controls
	const Script& script = GameScripts::getCurrScript();
	if (!script.textureFile.empty())
		_textureName = script.textureFile;
	if (!script.reflectFile.empty())
		_reflectName = script.reflectFile;
	_color = script.colCube;
	_reflectIntensity = script.reflectIntensity;
}

void GameCube::createGraphics()
{
	CubeGrid::createGraphics();
	_shield.createGraphics();
}

void GameCube::destroyGraphics()
{
	CubeGrid::destroyGraphics();
	_shield.destroyGraphics();
}

// shuffles 4 grids
static void shuffleGrids(GridBase* grids[], int g1, int g2, int g3, int g4)
{
	GridBase* tmp = grids[g4];
	grids[g4] = grids[g3];
	grids[g3] = grids[g2];
	grids[g2] = grids[g1];
	grids[g1] = tmp;
}

// just shuffles slots on the grid
static void shuffleSlots(GridBase* grids[], int g1, bool cw)
{
	((GameGrid*) grids[g1])->shuffleSlots(cw);
}

// just inverts slots on the grid
static void invertSlots(GridBase* grids[], int g1)
{
	((GameGrid*) grids[g1])->invertSlots();
}

void GameCube::handleShuffling()
{
	// shuffle the grids and slots in the grids
	if (_rotAxis == ROTATE_AXIS_X)
	{
		if (_rotDir == ROTATE_CCW)
		{
			invertSlots(_theGrids, FACE_BOTTOM);
			invertSlots(_theGrids, FACE_BACK);
			shuffleGrids(_theGrids, FACE_FRONT, FACE_BOTTOM, FACE_BACK, FACE_TOP);
			shuffleSlots(_theGrids, FACE_LEFT, false);
			shuffleSlots(_theGrids, FACE_RIGHT, true);
		}
		else
		{
			invertSlots(_theGrids, FACE_TOP);
			invertSlots(_theGrids, FACE_BACK);
			shuffleGrids(_theGrids, FACE_FRONT, FACE_TOP, FACE_BACK, FACE_BOTTOM);
			shuffleSlots(_theGrids, FACE_LEFT, true);
			shuffleSlots(_theGrids, FACE_RIGHT, false);
		}
	}
	else if (_rotAxis == ROTATE_AXIS_Y)
	{
		if (_rotDir == ROTATE_CCW)
		{
			shuffleGrids(_theGrids, FACE_FRONT, FACE_RIGHT, FACE_BACK, FACE_LEFT);
			shuffleSlots(_theGrids, FACE_TOP, true);
			shuffleSlots(_theGrids, FACE_BOTTOM, false);
		}
		else
		{
			shuffleGrids(_theGrids, FACE_FRONT, FACE_LEFT, FACE_BACK, FACE_RIGHT);
			shuffleSlots(_theGrids, FACE_TOP, false);
			shuffleSlots(_theGrids, FACE_BOTTOM, true);
		}
	}
	else if (_rotAxis == ROTATE_AXIS_Z)
	{
		if (_rotDir == ROTATE_CCW)
		{
			shuffleSlots(_theGrids, FACE_BOTTOM, true);
			shuffleSlots(_theGrids, FACE_TOP, true);
			shuffleSlots(_theGrids, FACE_LEFT, true);
			shuffleSlots(_theGrids, FACE_RIGHT, true);
			shuffleGrids(_theGrids, FACE_RIGHT, FACE_TOP, FACE_LEFT, FACE_BOTTOM);
			shuffleSlots(_theGrids, FACE_FRONT, true);
			shuffleSlots(_theGrids, FACE_BACK, false);
		}
		else
		{
			shuffleSlots(_theGrids, FACE_BOTTOM, false);
			shuffleSlots(_theGrids, FACE_TOP, false);
			shuffleSlots(_theGrids, FACE_LEFT, false);
			shuffleSlots(_theGrids, FACE_RIGHT, false);
			shuffleGrids(_theGrids, FACE_RIGHT, FACE_BOTTOM, FACE_LEFT, FACE_TOP);
			shuffleSlots(_theGrids, FACE_FRONT, false);
			shuffleSlots(_theGrids, FACE_BACK, true);
		}
	}

	// update orientations
	((GameGrid*)_theGrids[FACE_FRONT])->updateConfig(AXISORIENT_Z, 1);
	((GameGrid*)_theGrids[FACE_RIGHT])->updateConfig(AXISORIENT_X, 1);
	((GameGrid*)_theGrids[FACE_TOP])->updateConfig(AXISORIENT_Y, 1);
	((GameGrid*)_theGrids[FACE_BACK])->updateConfig(AXISORIENT_Z, -1);
	((GameGrid*)_theGrids[FACE_LEFT])->updateConfig(AXISORIENT_X, -1);
	((GameGrid*)_theGrids[FACE_BOTTOM])->updateConfig(AXISORIENT_Y, -1);

	_shuffleComplete = true;
}

bool GameCube::doFrame(int id, float newVal, void* optData)
{
	bool retVal = CubeGrid::doFrame(id, newVal, optData);

	if (_rotAxis == ROTATE_AXIS_X)
	{
		if (fabs(_rotX) >= rad45)
		{
			_rotX += -_rotDir * rad90;
			if (!_shuffleComplete)
				handleShuffling();
		}
	}
	else if (_rotAxis == ROTATE_AXIS_Y)
	{
		if (fabs(_rotY) >= rad45)
		{
			_rotY += -_rotDir * rad90;
			if (!_shuffleComplete)
				handleShuffling();
		}
	}
	else if (_rotAxis == ROTATE_AXIS_Z)
	{
		if (fabs(_rotZ) >= rad45)
		{
			_rotZ += -_rotDir * rad90;
			if (!_shuffleComplete)
				handleShuffling();
		}
	}
	else
	{
		// handle the cube hit jiggle animations
		if (_idJiggleRotX == id)
		{
			_rotX = newVal;
		}
		else if (_idJiggleRotY == id)
		{
			_rotY = newVal;
		}
		else if (_idJiggleScaleAnim == id)
		{
			_rotX *= newVal;
			_rotY *= newVal;
			_rotZ *= newVal;
		}
	}

	return retVal;
}

void GameCube::startRealExitAnimation(int length)
{
	// set up the real exit animations
	AnimItem animItemX(this);
	animItemX.configSimpleAnim(0, MigUtil::convertToRadians(20), length, AnimItem::ANIM_TYPE_LINEAR);
	_idRotX = MigUtil::theAnimList->addItem(animItemX);
	AnimItem animItemY(this);
	animItemY.configSimpleAnim(0, MigUtil::convertToRadians(150), length, AnimItem::ANIM_TYPE_LINEAR);
	_idRotY = MigUtil::theAnimList->addItem(animItemY);
	AnimItem animItemZ(this);
	animItemZ.configSimpleAnim(0, MigUtil::convertToRadians(15), length, AnimItem::ANIM_TYPE_LINEAR);
	_idRotZ = MigUtil::theAnimList->addItem(animItemZ);

	// the scale exit animation is a little more complicated
	float fy[] = { 1.0f, 1.12f, 1.2f, 1.25f, 1.2f, 1.15f, 1.0f, 0.65f, 0.35f, 0.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, 1, length, fy, ARRAYSIZE(fy));
	_idScale = MigUtil::theAnimList->addItem(animItemS);
}

void GameCube::animComplete(int id, void* optData)
{
	if (_idRotX == id || _idRotY == id || _idRotZ == id)
	{
		// this indicates that the cube can be rotated again
		//CubeUtil.info("animComplete() called, cube can be rotated again");
		_rotAxis = ROTATE_AXIS_NONE;
	}
	else if (_idTransX == id || _idTransY == id || _idTransZ == id)
	{
		if (_idStampAnimation == id)
		{
			_idStampAnimation = 0;

			// load rotation time from script settings, which we'll use for stamping
			int rotDuration = (GameScripts::getCurrScript().isLoaded() ? GameScripts::getCurrLevel().rotTime : 500);

			// create an animation for the given axis to return the cube
			AnimItem animItem(this);
			AxisOrient axis = AXISORIENT_NONE;
			if (_idTransX == id)
			{
				animItem.configSimpleAnim(_translate.x, 0, rotDuration / 2, AnimItem::ANIM_TYPE_LINEAR);
				_idTransX = MigUtil::theAnimList->addItem(animItem);
				axis = AXISORIENT_X;
			}
			else if (_idTransY == id)
			{
				animItem.configSimpleAnim(_translate.y, 0, rotDuration / 2, AnimItem::ANIM_TYPE_LINEAR);
				_idTransY = MigUtil::theAnimList->addItem(animItem);
				axis = AXISORIENT_Y;
			}
			else if (_idTransZ == id)
			{
				animItem.configSimpleAnim(_translate.z, 0, rotDuration / 2, AnimItem::ANIM_TYPE_LINEAR);
				_idTransZ = MigUtil::theAnimList->addItem(animItem);
				axis = AXISORIENT_Z;
			}

			// signal that the stamp is complete
			if (_callback != nullptr)
				_callback->onCubeStampComplete(axis);
		}
	}
	else if (_idScale == id)
	{
		// the scale animation interface is only used for intro and exit animations
		if (_callback != nullptr && !_idLoseAnim.isActive())
			_callback->onCubeAnimComplete();
	}
	else if (_idExitDelay == id)
	{
		_idExitDelay = 0;

		// set up the real exit animations
		startRealExitAnimation(INTRO_ANIM_LENGTH);
	}
	else if (_idWinAnim == id)
	{
		_idWinAnim = 0;

		// game win animation complete
		if (_callback != nullptr)
			_callback->onCubeGameOverAnimComplete(true);
		_scale = 0;	// hide the cube
	}
	else if (_idLoseAnim == id)
	{
		_idLoseAnim = 0;

		// game win animation complete
		if (_callback != nullptr)
			_callback->onCubeGameOverAnimComplete(false);
	}
	else if (_idJiggleRotX == id)
		_idJiggleRotX = 0;
	else if (_idJiggleRotY == id)
		_idJiggleRotY = 0;
	else if (_idJiggleScaleAnim == id)
		_idJiggleScaleAnim = 0;

	CubeGrid::animComplete(id, optData);
}

void GameCube::draw(Matrix& mat) const
{
	if (CubeUtil::currPowerUp == POWERUPTYPE_INVULNERABLE && CubeUtil::renderPass == RENDER_PASS_FINAL)
	{
		float alpha = Timer::gameTimeMillis() / 1000.0f;
		alpha = (alpha - (int)alpha);
		if (alpha > 0.5f)
			alpha = 1.0f - alpha;
		CubeGrid::draw(mat, Color(_color, 0.5f + alpha));
	}
	else
		CubeGrid::draw(mat);

	if (CubeUtil::renderPass == RENDER_PASS_FINAL && _shield.isVisible() && _scale > 0)
		_shield.draw(mat);
}

void GameCube::draw() const
{
	CubeGrid::draw();
}

int GameCube::startIntroAnimation()
{
	// set up the intro animations
	AnimItem animItemX(this);
	animItemX.configSimpleAnim(MigUtil::convertToRadians(20), 0, INTRO_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotX = MigUtil::theAnimList->addItem(animItemX);
	AnimItem animItemY(this);
	animItemY.configSimpleAnim(MigUtil::convertToRadians(150), 0, INTRO_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotY = MigUtil::theAnimList->addItem(animItemY);
	AnimItem animItemZ(this);
	animItemZ.configSimpleAnim(MigUtil::convertToRadians(15), 0, INTRO_ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idRotZ = MigUtil::theAnimList->addItem(animItemZ);

	// the scale intro animation is a little more complicated
	float fy[] = { 0.0f, 0.35f, 0.65f, 1.0f, 1.15f, 1.2f, 1.25f, 1.2f, 1.12f, 1.0f };
	AnimItem animItemS(this);
	animItemS.configParametricAnim(0, 1, INTRO_ANIM_LENGTH, fy, ARRAYSIZE(fy));
	_idScale = MigUtil::theAnimList->addItem(animItemS);

	return INTRO_ANIM_LENGTH;
}

int GameCube::startExitAnimation()
{
	// start w/ a delay to allow the cube hit animation to complete
	AnimItem animItem(this);
	animItem.configTimer(defCubeHitAnimDuration, false);
	_idExitDelay = MigUtil::theAnimList->addItem(animItem);

	return INTRO_ANIM_LENGTH;  // why not defCubeHitAnimDuration?
}

int GameCube::startWinAnimation()
{
	// timer
	AnimItem animItemW(this);
	animItemW.configTimer(WIN_ANIM_LENGTH, false);
	_idWinAnim = MigUtil::theAnimList->addItem(animItemW);

	// rotation
	float fy[] = {
		-10, -18, -23, -25, -20, -12, -2, 8, 20, 38,
		54, 67, 78, 88, 97, 104, 109, 115, 119, 123,
		127, 131, 134, 137, 139, 141, 143, 144, 145, 146,
		147, 148, 149, 150, 151 };
	AnimItem animItemY(this);
	animItemY.configParametricAnim(0, MigUtil::convertToRadians(10), WIN_ANIM_LENGTH, fy, ARRAYSIZE(fy));
	_idRotY = MigUtil::theAnimList->addItem(animItemY);

	// translation
	float ft[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, -0.08f, -0.13f, -0.15f, -0.12f, -0.04f, 0.1f, 0.3f, 0.6f, 1 };
	AnimItem animItemT(this);
	animItemT.configParametricAnim(0, 3 * defFallStartPos, WIN_ANIM_LENGTH, ft, ARRAYSIZE(ft));
	_idTransY = MigUtil::theAnimList->addItem(animItemT);

	// cancel other possible animations
	_idTransX.clearAnim();
	_idTransZ.clearAnim();

	return WIN_ANIM_LENGTH;
}

int GameCube::startLoseAnimation()
{
	// timer
	AnimItem animItem(this);
	animItem.configTimer(LOSE_ANIM_LENGTH, false);
	_idLoseAnim = MigUtil::theAnimList->addItem(animItem);

	// the grids flee
	((GameGrid*)_theGrids[FACE_FRONT])->startFleeAnimation(INTRO_ANIM_LENGTH);
	((GameGrid*)_theGrids[FACE_RIGHT])->startFleeAnimation(INTRO_ANIM_LENGTH);
	((GameGrid*)_theGrids[FACE_TOP])->startFleeAnimation(INTRO_ANIM_LENGTH);
	((GameGrid*)_theGrids[FACE_BACK])->setVisibility(false);
	((GameGrid*)_theGrids[FACE_LEFT])->setVisibility(false);
	((GameGrid*)_theGrids[FACE_BOTTOM])->setVisibility(false);

	// start w/ a delay to allow the cube hit animation to complete
	AnimItem animItemD(this);
	animItemD.configTimer(LOSE_ANIM_LENGTH - INTRO_ANIM_LENGTH, false);
	_idExitDelay = MigUtil::theAnimList->addItem(animItemD);

	return LOSE_ANIM_LENGTH;
}

static const char* axisToString(GameCube::ROTATE_AXIS axis)
{
	if (axis == GameCube::ROTATE_AXIS_X)
		return "X";
	else if (axis == GameCube::ROTATE_AXIS_Y)
		return "Y";
	return "Z";
}

static const char* dirToString(int dir)
{
	return (dir == GameCube::ROTATE_CW ? "CW" : "CCW");
}

bool GameCube::startRotate(ROTATE_AXIS axis, int dir, int dur)
{
	LOGINFO("(GameCube::startRotate) Rotate invoked, axis=%s, dir=%s", axisToString(axis), dirToString(dir));

	// clear existing animations
	_rotX = _rotY = _rotZ = 0;
	_idRotX = 0; _idRotY = 0; _idRotZ = 0;

	// create a rotation animation for the given axis
	float fParams[] = { 0.0f, 0.35f, 0.6f, 0.8f, 0.95f, 1.05f, 1.12f, 1.15f, 1.15f, 1.1f, 1.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, MigUtil::convertToRadians(90) * dir, dur, fParams, ARRAYSIZE(fParams));
	if (axis == ROTATE_AXIS_X)
		_idRotX = MigUtil::theAnimList->addItem(animItem);
	else if (axis == ROTATE_AXIS_Y)
		_idRotY = MigUtil::theAnimList->addItem(animItem);
	else if (axis == ROTATE_AXIS_Z)
		_idRotZ = MigUtil::theAnimList->addItem(animItem);

	_rotAxis = axis;
	_rotDir = dir;
	_shuffleComplete = false;
	return true;
}

bool GameCube::startRotate(ROTATE_AXIS axis, int dir)
{
	// load rotation time from script settings
	return startRotate(axis, dir, GameScripts::getCurrLevel().rotTime);
}

bool GameCube::startStamp(AxisOrient axis, float dist, int dur)
{
	LOGINFO("(GameCube::startStamp) Stamp invoked, axis=%s", CubeUtil::axisToString(axis));

	// create a stamping animation for the given axis
	//float tParams[] = { 0.0f, 0.6f, 0.9f, 1.0f };
	float tParams[] = { 0.0f, 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.50f, 1.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0.0f, dist, dur / 2, tParams, ARRAYSIZE(tParams));
	if (axis == AXISORIENT_X)
		_idTransX = _idStampAnimation = MigUtil::theAnimList->addItem(animItem);
	else if (axis == AXISORIENT_Y)
		_idTransY = _idStampAnimation = MigUtil::theAnimList->addItem(animItem);
	else if (axis == AXISORIENT_Z)
		_idTransZ = _idStampAnimation = MigUtil::theAnimList->addItem(animItem);

	return true;
}

bool GameCube::startStamp(AxisOrient axis, float dist)
{
	// load rotation time from script settings
	return startStamp(axis, dist, GameScripts::getCurrLevel().rotTime);
}

static void getGridCandidates(std::vector<GridBase*>& candidates, GridBase* grids[], int numGrids, bool incRes, bool allowEmpty, bool allowFilled, bool allowPartial)
{
	for (int i = 0; i < numGrids; i++)
	{
		GridBase::FillState state = grids[i]->getFillState(incRes);

		// grids that are completely filled may not be eligible
		if (allowFilled || state != GridBase::FILLSTATE_FILLED)
		{
			// grids that are completely empty may not be eligible
			if (allowEmpty || state != GridBase::FILLSTATE_EMPTY)
			{
				// grids that are partially filled may not be eligible
				if (allowPartial || state != GridBase::FILLSTATE_PARTIAL)
					candidates.push_back(grids[i]);
			}
		}
	}
}

// shuffles the slots w/in a grid (2x2)
static void shuffleSlots(bool* slots, int s1, int s2, int s3, int s4)
{
	bool tmp = slots[s4];
	slots[s4] = slots[s3];
	slots[s3] = slots[s2];
	slots[s2] = slots[s1];
	slots[s1] = tmp;
}

// shuffles the slots w/in a grid (3x3)
static void shuffleSlots(bool* slots, int s1, int s2, int s3, int s4, int s5, int s6, int s7, int s8)
{
	shuffleSlots(slots, s1, s2, s3, s4);
	shuffleSlots(slots, s5, s6, s7, s8);
}

// shuffles the slots w/in a grid
static void shuffleSlots(bool* slots, int dimen, bool cw)
{
	if (dimen == 2)
	{
		if (cw)
			shuffleSlots(slots, 0, 2, 3, 1);
		else
			shuffleSlots(slots, 0, 1, 3, 2);
	}
	else if (dimen == 3)
	{
		if (cw)
			shuffleSlots(slots, 0, 6, 8, 2, 1, 3, 7, 5);
		else
			shuffleSlots(slots, 0, 2, 8, 6, 1, 5, 7, 3);
	}
}

// creates the basis for a single new falling grid
bool GameCube::newFallingGridCandidate(GridInfo& info, bool allowEmpty, bool incReserved)
{
	// determine if grid locking is on and pertains to this function call
	GridLock gridLock = GameScripts::getCurrLevel().gridLock;
	bool isGridLockingOn = (gridLock != GRIDLOCK_NONE);

	// if so, then use the most recent chosen grid instead of picking a new one
	GridBase* chosenGrid = (isGridLockingOn ? _lastChosenGrid : nullptr);

	// if the chosen grid is completely filled or reserved
	if (chosenGrid != nullptr && chosenGrid->isFilled(true))
	{
		// if the grid lock setting is to wait until the chosen grid is full, we cannot launch
		if (gridLock == GRIDLOCK_WAIT && !chosenGrid->isFilled(false))
			return false;

		// we can move on to a new grid
		chosenGrid = nullptr;
	}

	// if there's no existing grid that we will use then pick a new one
	if (chosenGrid == nullptr)
	{
		// first, produce a list of candidate grids
		std::vector<GridBase*> candidates;
		getGridCandidates(candidates, _theGrids, NUM_GRIDS, incReserved, allowEmpty, false, true);
		int numCandidates = candidates.size();
		if (numCandidates == 0)
			return false;

		// pick a random one
		int chosenIndex = (int)MigUtil::pickRandom(numCandidates);
		chosenGrid = candidates[chosenIndex];
	}

	// invert the filled bit, and then randomly turn some off
	info.initFromGrid(*chosenGrid, incReserved);
	info.invertFillList();
	info.randomizeFillList(GameScripts::getCurrLevel().fallingHoleLimit);

	if (incReserved)
	{
		// need to set the reserved bits so we don't pick them again
		info.uniqueID = _nextFallingGridID++;
		chosenGrid->reserveSlots(info);
	}

	// if required, randomize the orientation of the falling grid candidate
	const Level& lvl = GameScripts::getCurrLevel();
	if (GameScripts::getCurrScript().isLoaded() && lvl.randFallingPieces)
	{
		int nRotations = MigUtil::pickRandom(4);
		for (int i = 0; i < nRotations; i++)
			shuffleSlots(info.fillList, info.dimen, true);
	}

	// if grid locking is on then remember the last chosen grid for next time
	if (isGridLockingOn)
		_lastChosenGrid = chosenGrid;

	return true;
}

// creates a generic grid candidate
bool GameCube::newGridCandidate(GridInfo& info, bool allowEmpty, bool allowFilled, bool allowPartial)
{
	// first, produce a list of candidate grids
	std::vector<GridBase*> candidates;
	getGridCandidates(candidates, _theGrids, NUM_GRIDS, false, allowEmpty, allowFilled, allowPartial);
	int numCandidates = candidates.size();
	if (numCandidates == 0)
		return false;

	// pick a random one
	int chosenIndex = (int)MigUtil::pickRandom(numCandidates);
	info.initFromGrid(*candidates[chosenIndex], false);
	return true;
}

// creates the basis for any sort of falling grid that requires a complete list of available grids
GridInfo* GameCube::newGridCandidatesList(bool allowEmptyGrids, bool allowFilledGrids, bool allowPartialGrids, bool incReserved, int& numGrids)
{
	// first, produce a list of candidate grids
	std::vector<GridBase*> candidates;
	getGridCandidates(candidates, _theGrids, NUM_GRIDS, incReserved, allowEmptyGrids, allowFilledGrids, allowPartialGrids);
	numGrids = candidates.size();
	if (numGrids == 0)
		return nullptr;

	// create an array of grids
	GridInfo* grids = new GridInfo[numGrids];
	for (int i = 0; i < numGrids; i++)
		grids[i].initFromGrid(*candidates[i], false);

	return grids;
}

// creates the basis for a burst launch of falling pieces
GridInfo* GameCube::newBurstCandidatesList(bool allowEmptyGrids, int& numGrids)
{
	const Level& lvl = GameScripts::getCurrLevel();

	// cannot burst on grid level 1 and grid locking on
	if (lvl.gridLock != GRIDLOCK_NONE && lvl.gridLevel == 1)
		return nullptr;

	// first, produce a list of candidate grids
	std::vector<GridBase*> candidates;
	getGridCandidates(candidates, _theGrids, NUM_GRIDS, false, allowEmptyGrids, false, (lvl.gridLock == GRIDLOCK_NONE));
	numGrids = candidates.size();
	if (numGrids == 0)
		return nullptr;

	// cannot burst if grid locking is off and we don't have at least 3 to choose from
	if (lvl.gridLock == GRIDLOCK_NONE && numGrids < 3)
		return nullptr;

	// create an array of grids
	GridInfo* grids = new GridInfo[numGrids];
	for (int i = 0; i < numGrids; i++)
		grids[i].initFromGrid(*candidates[i], false);

	return grids;
}

// creates the basis for a burst launch of evil falling pieces
GridInfo* GameCube::newEvilBurstCandidatesList(int& numGrids)
{
	// first, produce a list of candidate grids
	std::vector<GridBase*> candidates;
	getGridCandidates(candidates, _theGrids, NUM_GRIDS, false, true, false, true);
	numGrids = candidates.size();
	if (numGrids < 3)
		return nullptr;

	// create an array of grids
	GridInfo* grids = new GridInfo[numGrids];
	for (int i = 0; i < numGrids; i++)
		grids[i].initFromGrid(*candidates[i], false);

	return grids;
}

// sets up the cube hit/miss animation when a falling piece impacts the cube
void GameCube::startCubeHitAnimation(bool isHit, AxisOrient orient)
{
	const int hitDur = defCubeHitAnimDuration;
	const int missDur = defCubeMissAnimDuration;

	// translation applies to both hit and miss
	float tParams[] = { 0.0f, 0.6f, 0.9f, 1.0f, 0.9f, 0.8f, 0.6f, 0.4f, 0.2f, 0.1f, 0.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, -0.1f * (isHit ? 1.3f : 3.0f), (isHit ? hitDur : missDur), tParams, ARRAYSIZE(tParams));
	int animID = MigUtil::theAnimList->addItem(animItem);

	// assign the animation ID to the correct axis
	if (orient == AXISORIENT_X)
		_idTransX = animID;
	else if (orient == AXISORIENT_Y)
		_idTransY = animID;
	else if (orient == AXISORIENT_Z)
		_idTransZ = animID;

	// in addition, a hit causes a jiggle
	if (isHit)
	{
		// add a parametric rotation
		float rParams[] = { 0.0f, -0.7f, -1.0f, -0.7f, 0.0f, 0.7f, 1.0f, 0.7f, 0.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, MigUtil::convertToRadians(5), hitDur, rParams, ARRAYSIZE(rParams));
		animItem.cycle = 3;		// this animation will cycle 3 times during the duration
		animID = MigUtil::theAnimList->addItem(animItem);

		// assign the animation ID to the correct axis
		if (orient == AXISORIENT_X)
			_idJiggleRotY = animID;
		else if (orient == AXISORIENT_Y)
			_idJiggleRotX = animID;
		else if (orient == AXISORIENT_Z)
			_idJiggleRotY = animID;

		// scale the rotation by a second animation that decreases the rotation over time
		AnimItem animItemS(this);
		animItemS.configSimpleAnim(1, 0, hitDur, AnimItem::ANIM_TYPE_LINEAR);
		_idJiggleScaleAnim = MigUtil::theAnimList->addItem(animItemS);
	}
}

// returns the number of filled grids on the cube (0-6)
static int countFilledGrids(const GridBase* const grids[], int numGrids)
{
	int cnt = 0;
	for (int i = 0; i < numGrids; i++)
	{
		if (grids[i] != nullptr && ((const GameGrid*)grids[i])->isFilled())
			cnt++;
	}
	return cnt;
}

// returns the ID of the correct sound to play when a grid is filled
static const std::string& getFilledGridSoundID(const GridBase* const grids[], int numGrids)
{
	int cnt = countFilledGrids(grids, numGrids);
	if (cnt == 1)
		return WAV_FILL1;
	else if (cnt == 2)
		return WAV_FILL2;
	else if (cnt == 3)
		return WAV_FILL3;
	else if (cnt == 4)
		return WAV_FILL4;
	else if (cnt == 5)
		return WAV_FILL5;
	return WAV_FILL6;
}

// clears any slot that has been reserved by the given ID
static void clearReservedSlots(GridBase* grids[], int numGrids, int uniqueID, int mapIndex)
{
	// check each grid on the cube
	for (int i = 0; i < numGrids; i++)
	{
		// the map index has to match, or it couldn't have been reserved by this ID
		if (grids[i]->getMapIndex() == mapIndex)
		{
			// map index was a match so check each slot
			grids[i]->clearReserveSlots(uniqueID);
		}
	}
}

// returns the grid face on the cube that faces a given orientation
GridBase* GameCube::gridFaceByOrientation(AxisOrient orient, bool frontFacing)
{
	GridBase* theGrid = nullptr;
	if (orient == AXISORIENT_Z)
		theGrid = (frontFacing ? _theGrids[FACE_FRONT] : _theGrids[FACE_BACK]);
	else if (orient == AXISORIENT_X)
		theGrid = (frontFacing ? _theGrids[FACE_RIGHT] : _theGrids[FACE_LEFT]);
	else if (orient == AXISORIENT_Y)
		theGrid = (frontFacing ? _theGrids[FACE_TOP] : _theGrids[FACE_BOTTOM]);
	return theGrid;
}

void GameCube::doHint(const GridInfo& gridInfo)
{
	GameGrid* hintGrid = (GameGrid*) gridFaceByOrientation(gridInfo.orient, true);
	if (hintGrid != nullptr)
		hintGrid->doHint(gridInfo);
}

bool GameCube::doCollision(CollisionResult& res, const GridInfo* pieces, int numPieces)
{
	if (pieces == nullptr || numPieces == 0)
		return false;

	// need to clear the reserved bits on the original grid that this piece was intended for
	for (int i = 0; i < numPieces; i++)
	{
		const GridInfo& piece = pieces[i];
		if (piece.uniqueID > 0)
			clearReservedSlots(_theGrids, NUM_GRIDS, piece.uniqueID, piece.mapIndex);
	}

	// determine which grid face was hit by the falling grid
	GridInfo sample = pieces[0];
	GridBase* hitGrid = gridFaceByOrientation(sample.orient, true);	// note that all pieces have the same orientation
	if (hitGrid != nullptr)
	{
		res.wasSideComplete = res.wasPieceFilled = false;

		// see if any of the given pieces matches the impacted grid
		for (int i = 0; i < numPieces; i++)
		{
			if (pieces[i].mapIndex == hitGrid->getMapIndex())
			{
				sample = pieces[i];
				break;
			}
		}

		// was it a match?
		res.wasRealHit = (hitGrid->getMapIndex() == sample.mapIndex);
		res.wasHit = (res.wasRealHit || CubeUtil::currPowerUp == POWERUPTYPE_INVULNERABLE);
		if (!res.wasHit)
			res.rejGrid = sample;
		res.mapIndexHit = hitGrid->getMapIndex();
		res.orientHit = hitGrid->getOrient();
		res.colHit = hitGrid->getFillColor();
		LOGINFO("(GameCube::doCollision) This was a %s", (res.wasHit ? "hit" : "miss"));

		// let the grid animate the impacted slots
		bool isAnimating = false;
		if (res.wasRealHit || CubeUtil::currPowerUp != POWERUPTYPE_INVULNERABLE)
			isAnimating = ((GameGrid*)hitGrid)->doCollision(&sample, res.wasHit);

		// play a sound
		if (res.wasHit)
		{
			// indicate if at least one piece was actually hit
			res.wasPieceFilled = isAnimating;

			if (((GameGrid*)hitGrid)->isFilled() && isAnimating)
			{
				// since this side has just been filled in, clear all reserved slots
				hitGrid->clearReserveSlots(-1);

				// indicate that a cube side was completed
				res.wasSideComplete = res.wasPieceFilled = true;

				// play the correct side complete sound
				res.soundToPlay = getFilledGridSoundID(_theGrids, NUM_GRIDS);
			}
			else
				res.soundToPlay = (isAnimating ? WAV_MATCH : WAV_SAME);
		}
		else
			res.soundToPlay = WAV_ERROR;

		// start the hit/miss animation
		startCubeHitAnimation(res.wasHit, sample.orient);

		// determine if the launcher will need to check for obsolete falling grids
		res.needObsoleteCheck = false;
		if (res.wasHit && ((GameGrid*)hitGrid)->isFilled())
		{
			res.needObsoleteCheck = true;
			for (int i = 0; i < NUM_GRIDS; i++)
			{
				if (_theGrids[i]->getMapIndex() == sample.mapIndex && !((GameGrid*)_theGrids[i])->isFilled())
					res.needObsoleteCheck = false;
			}
		}

		// if this wasn't a real hit, apply the miss to the power-up
		if (res.wasHit && !res.wasRealHit)
		{
			CubeUtil::currPowerUp.applyMiss();
			_shield.doHitAnimation();
		}

		return true;
	}

	return false;
}

bool GameCube::doStamp(StampResult& res, AxisOrient axis, GridInfo* stampGrid)
{
	// determine which grid face was hit by the falling grid
	GridBase* hitGrid = gridFaceByOrientation(axis, false);
	if (hitGrid != nullptr)
	{
		res.wasSideComplete = false;

		if (stampGrid != nullptr)
		{
			// was it a match?
			res.wasRealHit = (hitGrid->getMapIndex() == stampGrid->mapIndex);
			res.wasHit = (res.wasRealHit || CubeUtil::currPowerUp == POWERUPTYPE_INVULNERABLE);

			// let the grid animate the impacted slots
			bool isAnimating = false;
			if (res.wasRealHit || CubeUtil::currPowerUp != POWERUPTYPE_INVULNERABLE)
				isAnimating = ((GameGrid*)hitGrid)->doCollision(stampGrid, res.wasHit);

			// play a sound
			if (res.wasHit)
			{
				const Level& lvl = GameScripts::getCurrLevel();
				if (((GameGrid*)hitGrid)->isFilled() && (isAnimating || lvl.stampStyle == STAMPSTYLE_BONUS_RANDOM))
				{
					// indicate that a cube side was completed
					res.wasSideComplete = true;

					// play the correct side complete sound
					res.soundToPlay = getFilledGridSoundID(_theGrids, NUM_GRIDS);
				}
				else
					res.soundToPlay = (isAnimating ? WAV_MATCH : WAV_SAME);
			}
			else
				res.soundToPlay = WAV_ERROR;

			// determine if the launcher will need to check for obsolete falling grids
			if (res.wasHit && ((GameGrid*)hitGrid)->isFilled())
			{
				res.needObsoleteCheck = true;
				for (int i = 0; i < NUM_GRIDS; i++)
				{
					if (_theGrids[i]->getMapIndex() == stampGrid->mapIndex && !((GameGrid*)_theGrids[i])->isFilled())
						res.needObsoleteCheck = false;
				}
			}

			// if this wasn't a real hit, apply the miss to the power-up
			if (res.wasHit && !res.wasRealHit)
			{
				CubeUtil::currPowerUp.applyMiss();
				_shield.doHitAnimation();
			}
		}
		else
		{
			// stamping on an empty stamp means a hit counts if the side is filled
			res.wasHit = hitGrid->isFilled(false);
			if (res.wasHit)
				res.hitGridInfo.initFromGrid(*hitGrid, false);
			res.soundToPlay = (res.wasHit ? WAV_MATCH : WAV_SAME);

			// let the grid animate the impacted slots to empty
			((GameGrid*)hitGrid)->doCollision(nullptr, false);
		}

		LOGINFO("(GameCube::doStamp) This was a %s", (res.wasHit ? "hit" : "miss"));
		return true;
	}
	else
		LOGWARN("(GameCube::doStamp) Orient %s was invalid!", CubeUtil::axisToString(axis));

	return false;
}

bool GameCube::canRotate() const
{
	return (!_idStampAnimation && (_rotAxis == ROTATE_AXIS_NONE || _shuffleComplete) && !isGameOverAnimating() && !_idScale.isActive() && !_idExitDelay.isActive());
}

bool GameCube::canStamp() const
{
	return (!_idStampAnimation && (_rotAxis == ROTATE_AXIS_NONE || _shuffleComplete) && !isGameOverAnimating() && !_idScale.isActive() && !_idExitDelay.isActive());
}

bool GameCube::isGameOverAnimating() const
{
	return (_idWinAnim.isActive() || _idLoseAnim.isActive());
}

bool GameCube::isStationary() const
{
	return (_rotX == 0 && _rotY == 0 && _rotZ == 0 && _translate.x == 0 && _translate.y == 0 && _translate.z == 0);
}

bool GameCube::isVisible() const
{
	return (_scale > 0);
}

bool GameCube::isFilled() const
{
	return (countFilledGrids(_theGrids, NUM_GRIDS) == NUM_GRIDS);
}
