#include "pch.h"
#include "FallingGrid.h"
#include "CubeConst.h"
#include "GameScripts.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

const int DEFAULT_FALL_TIME = 5000;
const float TAP_THRESHOLD = 0.3f;

///////////////////////////////////////////////////////////////////////////
// FallingGrid

FallingGrid::FallingGrid(const GridInfo& info, bool isBurst) : GridBase(),
	_origInfo(info), _isBurst(isBurst), _callback(nullptr), _baseDist(0), _accel(0), _rejectTrans(0)
{
	_gridDepth = defGridDepth;
}

bool FallingGrid::init(float dist)
{
	_baseDist = dist;

	// configure the new falling grid from the info passed in
	if (!GridBase::init(_origInfo.dimen, _origInfo.mapIndex, _origInfo.orient, defCubeRadius + 0.02f))
	{
		LOGWARN("(FallingGrid::init) configGrid() returned false!");
		return false;
	}
	setAllColors(_origInfo.emptyCol, _origInfo.fillCol);

	// hide slots in this falling grid that aren't filled in
	int numFillList = _origInfo.fillListLen();
	if (numFillList == getSlotCount())
	{
		for (int i = 0; i < numFillList; i++)
			_theSlots[i].invis = !_origInfo.fillList[i];
	}

	return true;
}

float FallingGrid::checkTap(float x, float y, const Matrix& proj, const Matrix& view) const
{
	if (!isTapped())
	{
		// center of grid in local coordinates
		Vector3 pos;
		pos.z = getTotalDist() + defCubeRadius;

		// compute world matrix
		static Matrix mat;
		mat.identity();
		switch (_orient)
		{
		case AXISORIENT_Z:
			if (_distFromOrigin < 0)
				mat.rotateY(rad180);
			break;
		case AXISORIENT_X:
			mat.rotateY(_distFromOrigin >= 0 ? rad90 : -rad90);
			break;
		case AXISORIENT_Y:
			mat.rotateX(_distFromOrigin >= 0 ? -rad90 : rad90);
			break;
		default:
			break;
		}

		// transform point into screen coordinates
		mat.transform(pos);
		view.transform(pos);
		proj.transform(pos);
		pos.x /= pos.z;
		pos.y /= pos.z;

		// convert to tap coordinates and return hit distance
		float dx = (float)fabs((2 * x - 1) - pos.x);
		float dy = (float)fabs((2 * (1 - y) - 1) - pos.y);
		if (dx < TAP_THRESHOLD && dy < TAP_THRESHOLD)
			return (float)sqrt(dx*dx + dy*dy);
	}

	// miss
	return -1;
}

bool FallingGrid::doFrame(int id, float newVal, void* optData)
{
	if (_idTapAnim == id)
	{
		_accel = newVal;
		updateSlotColorsForTap(newVal);
	}
	else if (_idObsoleteAnim == id)
	{
		// animate the alpha transparency of the slots
		int numSlots = getSlotCount();
		for (int i = 0; i < numSlots; i++)
			_theSlots[i].color.a = newVal;
	}
	else if (_idAnimFade == id)
	{
		// animate the alpha transparency of the slots
		int numSlots = getSlotCount();
		for (int i = 0; i < numSlots; i++)
		{
			Slot& thisSlot = _theSlots[i];
			if (!thisSlot.invis)
				thisSlot.color.a = 1 - newVal;
		}
	}
	else if (_idAnimBounce == id)
	{
		// animate the bounce
		_rejectTrans = newVal;
	}

	return true;
}

void FallingGrid::animComplete(int id, void* optData)
{
	if (_idObsoleteAnim == id)
	{
		// invoke the call back and ensure it doesn't call more than once
		_idObsoleteAnim = 0;
		if (_callback != nullptr)
			_callback->obsoleteAnimComplete(this);
	}
	else if (_idAnimFade == id)
	{
		// invoke the call back and ensure it doesn't call more than once
		_idAnimFade = 0;
		if (_callback != nullptr)
			_callback->rejectAnimComplete(this);
	}
	else if (_idTapAnim == id)
		_idTapAnim = 0;
	else if (_idAnimBounce == id)
		_idAnimBounce = 0;
}

void FallingGrid::updateSlotColorsForTap(float accel)
{
	// flash the tapped grid
	float newVal = 1 - accel;
	float red = _emptyCol.r + (1 - _emptyCol.r) * newVal;
	float grn = _emptyCol.g + (1 - _emptyCol.g) * newVal;
	float blu = _emptyCol.b + (1 - _emptyCol.b) * newVal;

	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].setColor(red, grn, blu);
}

const GridInfo* FallingGrid::getGrids(int& count) const
{
	count = 1;
	return &_origInfo;
}

// retrieve display color
const Color& FallingGrid::getDisplayColor() const
{
	return _origInfo.fillCol;
}

// standard falling pieces cannot lock any axis
bool FallingGrid::canLock() const
{
	return false;
}

// returns the time for this piece to fall from start to final position
int FallingGrid::getFallTime() const
{
	if (GameScripts::isGameScriptLoaded())
	{
		const Level& lvl = GameScripts::getCurrLevel();
		if (_isBurst && lvl.burstFallTime > 0)
			return lvl.burstFallTime;
		else if (lvl.fallTime > 0)
			return lvl.fallTime;
	}
	return DEFAULT_FALL_TIME;
}

// returns the score multiplier (none for standard falling pieces)
float FallingGrid::getScoreMultiplier() const
{
	return 1;
}

// determines if this piece is obsolete
bool FallingGrid::checkObsolete(int mapIndexObsolete)
{
	return (_origInfo.mapIndex == mapIndexObsolete);
}

// starts the tap animation
bool FallingGrid::startTapAnim(int animTime)
{
	// new animation to accelerate the tapped grid into the cube
	float fParams[] = { 0, 0.01f, 0.02f, 0.03f, 0.06f, 0.12f, 0.25f, 0.5f, 1 };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, 1, animTime, fParams, ARRAYSIZE(fParams));
	_idTapAnim = MigUtil::theAnimList->addItem(animItem);

	return true;
}

// starts an animation to indicate that this grid is obsolete
bool FallingGrid::startObsoleteAnim(int animTime, IFallingGridCallback* callback)
{
	AnimItem animItem(this);
	animItem.configSimpleAnim(1, 0, animTime, AnimItem::ANIM_TYPE_LINEAR);
	_idObsoleteAnim = MigUtil::theAnimList->addItem(animItem);

	_callback = callback;
	return true;
}

// starts an animation to indicate that this grid has been rejected
bool FallingGrid::startRejectAnim(int animTime, IFallingGridCallback* callback)
{
	// set up a fade animation
	AnimItem animItemF(this);
	animItemF.configSimpleAnim(0, 1, animTime, AnimItem::ANIM_TYPE_LINEAR);
	_idAnimFade = MigUtil::theAnimList->addItem(animItemF);

	// set up a bounce animation
	float fParams[] = { 0, 0.50f, 0.75f, 0.88f, 0.94f, 0.97f, 0.98f, 0.99f, 1 };
	AnimItem animItemB(this);
	animItemB.configParametricAnim(0, 0.5f, animTime, fParams, ARRAYSIZE(fParams));
	_idAnimBounce = MigUtil::theAnimList->addItem(animItemB);

	_callback = callback;
	return true;
}

void FallingGrid::applyOffset(Matrix& mat) const
{
	mat.translate(0, 0, getTotalDist() + _rejectTrans);
}

///////////////////////////////////////////////////////////////////////////
// EvilFallingGrid

// the period during which the grid will grow/shrink
static const int ANIM_DURATION = 500;
static const int DEFAULT_PERIOD_TIME = 2000;

EvilFallingGrid::EvilFallingGrid(GridInfo* grids, int numGrids, bool isBurst, IEvilFallingGridCallback* callback)
	: FallingGrid(grids[0], isBurst),
	_grids(grids), _numGrids(numGrids), _numRealGrids(numGrids), _callback(callback), _currGrid(0)
{
	// apply the maximum color restriction, if applicable
	const Level& lvl = GameScripts::getCurrLevel();
	if (lvl.evilMaxColors >= 2 && lvl.evilMaxColors < _numGrids)
		_numGrids = lvl.evilMaxColors;

	// generate the index list, so the order will appear random to the user
	generateGridIndexList();
	_origInfo = grids[_indexList[_currGrid]];
}

EvilFallingGrid::~EvilFallingGrid()
{
	delete[] _grids;
}

// simple initialization
bool EvilFallingGrid::init(float dist)
{
	if (_numRealGrids < 2)
		return false;
	if (!FallingGrid::init(dist))
		return false;

	// start the intro animation
	startIntroAnim();

	// important that we send the grid info change event
	if (_callback != nullptr)
		_callback->onGridInfoChange(_orient, _origInfo);
	return true;
}

// callback used by the animation interface
bool EvilFallingGrid::doFrame(int id, float newVal, void* optData)
{
	if (_idIntro == id)
		updateSlotRadius(newVal, false);
	else if (_idExit == id)
		updateSlotRadius(newVal, false);

	return FallingGrid::doFrame(id, newVal, optData);
}

// indicates that the given animation is complete
void EvilFallingGrid::animComplete(int id, void* optData)
{
	if (_idIntro == id)
	{
		// move to the timer
		startTimerAnim();
		_idIntro = 0;
	}
	else if (_idTimer == id)
	{
		// start the exit animation
		startExitAnim();
		_idTimer = 0;
	}
	else if (_idExit == id)
	{
		// start the intro animation
		startIntroAnim();
		_idExit = 0;

		// and cycle to the next grid
		_currGrid = (_currGrid + 1) % _numGrids;
		updateAllForNewGrid();
	}

	FallingGrid::animComplete(id, optData);
}

// starts an intro animation
void EvilFallingGrid::startIntroAnim()
{
	float fParams[] = { 0.0f, 0.35f, 0.6f, 0.8f, 0.95f, 1.05f, 1.12f, 1.15f, 1.15f, 1.1f, 1.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, 1, ANIM_DURATION, fParams, ARRAYSIZE(fParams));
	_idIntro = MigUtil::theAnimList->addItem(animItem);
}

// starts a timer
void EvilFallingGrid::startTimerAnim()
{
	const Script& script = GameScripts::getCurrScript();
	int period = (script.isLoaded() ? GameScripts::getCurrLevel().evilPeriod : DEFAULT_PERIOD_TIME);
	AnimItem animItem(this);
	animItem.configTimer(period, false);
	_idTimer = MigUtil::theAnimList->addItem(animItem);
}

// starts an exit animation
void EvilFallingGrid::startExitAnim()
{
	float fParams[] = { 1.0f, 1.1f, 1.15f, 1.15f, 1.12f, 1.05f, 0.95f, 0.8f, 0.6f, 0.35f, 0.0f };
	AnimItem animItem(this);
	animItem.configParametricAnim(0, 1, ANIM_DURATION, fParams, ARRAYSIZE(fParams));
	_idExit = MigUtil::theAnimList->addItem(animItem);
}

// clears any animation in progress
void EvilFallingGrid::clearAnim()
{
	_idIntro.clearAnim();
	_idTimer.clearAnim();
	_idExit.clearAnim();
}

const GridInfo* EvilFallingGrid::getGrids(int& count) const
{
	//clearAnim();
	count = 1;
	return &_grids[_indexList[_currGrid]];
}

const Color& EvilFallingGrid::getDisplayColor() const
{
	return _grids[_indexList[_currGrid]].fillCol;
}

int EvilFallingGrid::getFallTime() const
{
	const Level& lvl = GameScripts::getCurrLevel();
	if (_isBurst && lvl.evilBurstFallTime > 0)
		return lvl.evilBurstFallTime;
	return (lvl.evilFallTime > 0 ? lvl.evilFallTime : DEFAULT_FALL_TIME);
}

bool EvilFallingGrid::canLock() const
{
	return true;
}

float EvilFallingGrid::getScoreMultiplier() const
{
	const Script& script = GameScripts::getCurrScript();
	return (script.isLoaded() ? GameScripts::getCurrLevel().scoreCfg.evilMultiplier : FallingGrid::getScoreMultiplier());
}

void EvilFallingGrid::updateSlotColors(const Color& newColor)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].setColor(newColor);
}

void EvilFallingGrid::updateSlotVisibility(const bool* fillList)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].invis = !fillList[i];
}

void EvilFallingGrid::updateAllForNewGrid()
{
	int realIndex = _indexList[_currGrid];
	updateMapIndex(_grids[realIndex].mapIndex);
	updateSlotColors(_grids[realIndex].emptyCol);
	updateSlotVisibility(_grids[realIndex].fillList);

	if (_callback != nullptr)
		_callback->onGridInfoChange(_orient, _grids[realIndex]);
}

void EvilFallingGrid::generateGridIndexList()
{
	_indexList.clear();

	std::vector<int> src;
	for (int i = 0; i < _numRealGrids; i++)
		src.push_back(i);

	while (!src.empty())
	{
		int index = MigUtil::pickRandom(src.size());
		_indexList.push_back(src[index]);

		while (index < (int)src.size() - 1)
		{
			src[index] = src[index + 1];
			index++;
		}
		src.resize(src.size() - 1);
	}
}

bool EvilFallingGrid::checkObsolete(int mapIndexObsolete)
{
	return false;
}

bool EvilFallingGrid::startTapAnim(int animTime)
{
	clearAnim();
	return FallingGrid::startTapAnim(animTime);
}

// evil pieces never go obsolete
bool EvilFallingGrid::startObsoleteAnim(int animTime, IFallingGridCallback* callback)
{
	clearAnim();
	return FallingGrid::startObsoleteAnim(animTime, callback);
}

///////////////////////////////////////////////////////////////////////////
// WildFallingGrid

WildFallingGrid::WildFallingGrid(GridInfo* grids, int numGrids, bool isBurst)
	: FallingGrid(grids[0], isBurst),
	_grids(grids), _numGrids(numGrids)
{
}

WildFallingGrid::~WildFallingGrid()
{
	clearAnim();
	delete[] _grids;
}

// simple initialization
bool WildFallingGrid::init(float dist)
{
	if (_numGrids < 2)
		return false;
	if (!FallingGrid::init(dist))
		return false;

	// start the color animation
	int period = GameScripts::getCurrLevel().wildPeriod;
	AnimItem animItem(this);
	animItem.configSimpleAnim(0.0f, (float) _numGrids, _numGrids * period, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
	_idColorAnim = MigUtil::theAnimList->addItem(animItem);

	return true;
}

// simple interpolation
static float interpolate(float v1, float v2, float param)
{
	float diff = v2 - v1;
	return v1 + diff * param;
}

// updates the displayed slot color
void WildFallingGrid::updateSlotColors(float r, float g, float b)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].setColor(r, g, b);
}

// callback used by the animation interface
bool WildFallingGrid::doFrame(int id, float newVal, void* optData)
{
	if (_idColorAnim == id)
	{
		//newVal = newVal % _numGrids;
		newVal = (int)newVal % _numGrids + (newVal - (int)newVal);
		int curr = (int)newVal;
		int next = (int)(newVal + 1);
		if (next >= _numGrids)
			next = 0;
		const GridInfo& gc = _grids[curr];
		const GridInfo& gn = _grids[next];

		float inter = newVal - curr;
		float r = interpolate(gc.fillCol.r, gn.fillCol.r, inter);
		float g = interpolate(gc.fillCol.g, gn.fillCol.g, inter);
		float b = interpolate(gc.fillCol.b, gn.fillCol.b, inter);
		updateSlotColors(r, g, b);
	}

	return FallingGrid::doFrame(id, newVal, optData);
}

// clears any animation in progress
void WildFallingGrid::clearAnim()
{
	_idColorAnim.clearAnim();
}

// retrieve the grids that this piece will match
const GridInfo* WildFallingGrid::getGrids(int& count) const
{
	//clearAnim();
	count = _numGrids;
	return _grids;
}

// the wild falling pieces have a separate fall time control
int WildFallingGrid::getFallTime() const
{
	const Level& lvl = GameScripts::getCurrLevel();
	return (lvl.wildFallTime > 0 ? lvl.wildFallTime : DEFAULT_FALL_TIME);
}

// wild falling pieces can lock an axis, if so configured by the game script
bool WildFallingGrid::canLock() const
{
	return true;
}

// wild pieces never go obsolete
bool WildFallingGrid::checkObsolete(int mapIndexObsolete)
{
	return false;
}
