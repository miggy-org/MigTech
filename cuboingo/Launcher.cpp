#include "pch.h"
#include "Launcher.h"
#include "CubeConst.h"
#include "CubeUtil.h"
#include "../core/MigUtil.h"

using namespace MigTech;
using namespace Cuboingo;

// special IDs for the launch buttons
static const int ID_LEFT_LAUNCH_BUTTON   = 97;
static const int ID_RIGHT_LAUNCH_BUTTON  = 98;
static const int ID_CENTER_LAUNCH_BUTTON = 99;

// frequency for the global hint grid lowing
static const int ANIM_GLOW_DUR = 1000;

///////////////////////////////////////////////////////////////////////////
// LaunchButton

bool LaunchButton::init(tinyxml2::XMLElement* xml, const Font* font)
{
	bool ret = PicButton::init(xml, font);

	// disable animations for these buttons
	_animEnabled = false;

	// invisible by default
	setVisible(false);

	return ret;
}

void LaunchButton::init(float u, float v, float h, float rot)
{
	// square-ish button (correcting for the fact that most screens aren't square)
	PicButton::init("arrow.png", u, v, h, h, rot);

	// disable animations for these buttons
	_animEnabled = false;
}

void LaunchButton::cancelExistingAnims()
{
	_idGlowAnim.clearAnim();
	_idTapAnim.clearAnim();
}

void LaunchButton::startAnim(const Color& col)
{
	if (!_idGlowAnim.isActive())
	{
		setVisible(true);

		// if visible, start the glow animation
		setColor(Color(col, 0.25f));

		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 1, ANIM_DUR_GLOW, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
		_idGlowAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

void LaunchButton::startTapAnim()
{
	cancelExistingAnims();

	AnimItem animItem(this);
	animItem.configSimpleAnim(1, 0, ANIM_DUR_TAP, AnimItem::ANIM_TYPE_LINEAR);
	_idTapAnim = MigUtil::theAnimList->addItem(animItem);

	setColor(MigTech::colWhite);
}

void LaunchButton::update(const Color& col)
{
	cancelExistingAnims();
	startAnim(col);
}

void LaunchButton::stop()
{
	_idGlowAnim.clearAnim();
	if (!_idTapAnim.isActive())
		setVisible(false);
}

bool LaunchButton::doFrame(int id, float newVal, void* optData)
{
	if (isVisible())
	{
		if (_idGlowAnim == id)
		{
			newVal = newVal - (int) newVal;
			setAlpha(0.25f + (newVal > 0.5f ? 1 - newVal : newVal));
		}
		else if (_idTapAnim == id)
			setAlpha(newVal);
	}
	return PicButton::doFrame(id, newVal, optData);
}

void LaunchButton::animComplete(int id, void* optData)
{
	if (_idTapAnim == id)
	{
		setVisible(false);
		_idTapAnim = 0;
	}

	PicButton::animComplete(id, optData);
}

///////////////////////////////////////////////////////////////////////////
// Launcher

Launcher::Launcher(const GameCube& cube) :
	_gameCube(cube),
	_leftBtn(nullptr), _rightBtn(nullptr), _centerBtn(nullptr),
	_idleTime(0), _glowTime(0), _fallTime(0), _glowCount(0),
	_lastAxis(0), _lastIndex(0),
	_tapSound(nullptr), _callback(nullptr),
	_burstMode(BURST_NONE)
{
}

Launcher::~Launcher()
{
	if (_tapSound != nullptr)
		MigUtil::theAudio->deleteMedia(_tapSound);

	GameScripts::removeCallback(this);
	_callback = nullptr;
}

void Launcher::init(ILauncherCallback* callback)
{
	// preload sound effects
	if (MigUtil::theAudio != nullptr)
		_tapSound = MigUtil::theAudio->loadMedia(WAV_TAP, AudioBase::AUDIO_CHANNEL_SOUND);

	// we want to know when the level changes
	GameScripts::addCallback(this);

	// create a global glowing animation for hint grids to use
	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, ANIM_GLOW_DUR / 2, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
	_idGlobalGlow = MigUtil::theAnimList->addItem(animItem);

	// remember the call back
	_callback = callback;
}

// note that the given controls collection takes ownership of the controls
void Launcher::configControls(Controls& ctrls)
{
	_leftBtn = (LaunchButton*)ctrls.getControlByID(ID_LEFT_LAUNCH_BUTTON);
	_rightBtn = (LaunchButton*)ctrls.getControlByID(ID_RIGHT_LAUNCH_BUTTON);
	_centerBtn = (LaunchButton*)ctrls.getControlByID(ID_CENTER_LAUNCH_BUTTON);
}

void Launcher::configLaunchSettings(int idle, int glow, int fall, int count)
{
	_idleTime = idle;
	_glowTime = glow;
	_fallTime = fall;
	_glowCount = count;
}

void Launcher::createGraphics()
{
	_lightBeam.createGraphics();
}

void Launcher::destroyGraphics()
{
	_lightBeam.destroyGraphics();
}

bool Launcher::findFirstPieceColor(AxisOrient orient, Color& col) const
{
	// find the first falling grid that matches the chosen orientation
	std::list<LaunchStage*>::const_iterator iter;
	for (iter = _fallList.begin(); iter != _fallList.end(); iter++)
	{
		if ((*iter)->fallGrid != nullptr && (*iter)->fallGrid->getOrient() == orient && !(*iter)->fallGrid->isTapped())
		{
			col = (*iter)->fallGrid->getDisplayColor();
			return true;
		}
	}

	// no falling grid matched, so look in the hint list instead
	for (iter = _hintList.begin(); iter != _hintList.end(); iter++)
	{
		if ((*iter)->glowGrid != nullptr && (*iter)->glowGrid->getOrient() == orient && !(*iter)->fallGrid->isTapped())
		{
			col = (*iter)->glowGrid->getFilledColor();
			return true;
		}
	}

	return false;
}

LaunchButton* Launcher::axisToLaunchButton(AxisOrient axis)
{
	if (axis == AXISORIENT_X)
		return _rightBtn;
	if (axis == AXISORIENT_Y)
		return _centerBtn;
	else if (axis == AXISORIENT_Z)
		return _leftBtn;
	return nullptr;
}

// evaluates and updates all of the launch buttons given the current state of falling pieces
void Launcher::evalLaunchButton(AxisOrient axis)
{
	LaunchButton* btn = axisToLaunchButton(axis);
	if (btn != nullptr)
	{
		Color col;
		if (findFirstPieceColor(axis, col))
			btn->update(col);
		else
			btn->stop();
	}
}

// evaluates and updates all of the launch buttons given the current state of falling pieces
void Launcher::evalLaunchButtons()
{
	evalLaunchButton(AXISORIENT_Z);
	evalLaunchButton(AXISORIENT_X);
	evalLaunchButton(AXISORIENT_Y);
}

// called when a falling piece has completed it's descent to retrieve the grid and scoring info
void Launcher::invokeCollisionCallback(const FallingGrid* fallGrid)
{
	if (fallGrid != nullptr && _callback != nullptr)
	{
		int numGrids = 0;
		const GridInfo* gridInfo = fallGrid->getGrids(numGrids);

		float tapBonus = fallGrid->getAccel();
		float scoreMultiplier = fallGrid->getScoreMultiplier();
		_callback->onFallingPieceComplete(gridInfo, numGrids, tapBonus, scoreMultiplier);
	}

	// good place to show/hide launch buttons
	if (fallGrid != nullptr)
		evalLaunchButton(fallGrid->getOrient());
}

// callback used by the animation interface
bool Launcher::doFrame(int id, float newVal, void* optData)
{
	LaunchStage* stage = (LaunchStage*)optData;
	if (stage != nullptr)
	{
		if (stage->animType == LaunchStage::ANIM_TYPE_GLOW)
		{
			if (!stage->glowGrid->getCancel())
			{
				newVal = _glowCount * newVal;
				newVal = (newVal - (int)newVal);
				bool isGrowing = (newVal < 0.5f);
				newVal = (!isGrowing ? 1 - newVal : newVal);

				if (stage->glowGrid->setGlowParam(2 * newVal, isGrowing))
				{
					// time for the external hint (disabled for now)
					//if (_callback != nullptr)
					//{
						//const GridInfo& gridInfo = stage->glowGrid->getOrigGrid();
						//_callback->onDisplayHint(gridInfo);
					//}
				}
			}
			else
			{
				// cancel this animation and move to the next step
				return false;
			}
		}
		else if (stage->animType == LaunchStage::ANIM_TYPE_FALL)
		{
			stage->fallGrid->setBaseDist(newVal);

			// if the falling grid has impacted the cube this animation is done
			if (stage->fallGrid->getTotalDist() == 0)
			{
				//_fallList.remove(stage);
				//invokeCollisionCallback(stage->fallGrid);
				return false;
			}
		}
	}
	else if (_idGlobalGlow == id)
	{
		newVal = 1 - (newVal - (int)newVal);

		std::list<LaunchStage*>::iterator iter = _fallList.begin();
		while (iter != _fallList.end())
		{
			if ((*iter)->glowGrid != nullptr)
				(*iter)->glowGrid->setFallParam(newVal);
			iter++;
		}
	}

	return true;
}

// indicates that the given animation is complete
void Launcher::animComplete(int id, void* optData)
{
	LaunchStage* stage = (LaunchStage*)optData;
	if (stage != nullptr)
	{
		if (stage->animType == LaunchStage::ANIM_TYPE_IDLE)
		{
			_idleList.remove(stage);
			_hintList.push_back(stage);

			// create a glowing grid
			stage->glowGrid->init();
			//stage->glowGrid->createGraphics();

			// next stage
			stage->animType = LaunchStage::ANIM_TYPE_GLOW;

			// new animation
			AnimItem animItem(this);
			animItem.configSimpleAnim(0, 1, _glowTime, AnimItem::ANIM_TYPE_LINEAR, stage);
			stage->idAnim = MigUtil::theAnimList->addItem(animItem);

			// show the corresponding launch button
			LaunchButton* btn = axisToLaunchButton(stage->glowGrid->getOrient());
			if (btn != nullptr)
				btn->startAnim(stage->fallGrid->getDisplayColor());
		}
		else if (stage->animType == LaunchStage::ANIM_TYPE_GLOW)
		{
			// remove the hint grid from the list to render
			_hintList.remove(stage);
			_fallList.push_back(stage);

			// create a falling grid
			stage->fallGrid->init(defFallStartPos);
			//stage->fallGrid->createGraphics();

			// next stage
			stage->animType = LaunchStage::ANIM_TYPE_FALL;

			// new animation
			int fallTime = (_fallTime > 0 ? _fallTime : stage->fallGrid->getFallTime());
			AnimItem animItem(this);
			animItem.configSimpleAnim(defFallStartPos, 0, fallTime, AnimItem::ANIM_TYPE_LINEAR, stage);
			stage->idAnim = MigUtil::theAnimList->addItem(animItem);

			// reset the launch button color, just in case something changed.
			evalLaunchButton(stage->fallGrid->getOrient());
		}
		else if (stage->animType == LaunchStage::ANIM_TYPE_FALL)
		{
			stage->idAnim = 0;

			// remove the falling grid from the list to render and remember the grid
			_fallList.remove(stage);
			invokeCollisionCallback(stage->fallGrid);

			// clean up the launch stage
			delete stage;

			// clear burst mode, if necessary
			if (_burstMode != BURST_NONE && _fallList.empty() && _hintList.empty() && _idleList.empty())
				_burstMode = BURST_NONE;
		}
	}
}

// GameScriptUpdate override - a new level is starting
void Launcher::newLevel(int level)
{
	const Level& lvl = GameScripts::getCurrLevel();

	// load some basic settings
	_idleTime = lvl.idleTime;
	_glowTime = lvl.glowTime;
	_glowCount = lvl.glowCount;
	_fallTime = 0;	// this means defer to the grid itself to determine it's fall time

	// load the new direction count
	int dirCount = lvl.directions;
	if (dirCount <= 0 || dirCount > 3)
		dirCount = 3;

	// produce a candidate list of axis directions
	_axisDirections.push_back(AXISORIENT_Z);
	_axisDirections.push_back(AXISORIENT_X);
	_axisDirections.push_back(AXISORIENT_Y);

	// randomly pick them until we hit the direction count
	for (int i = 0; i < 3 - dirCount; i++)
	{
		int random = (int)MigUtil::pickRandom(_axisDirections.size());
		std::list<AxisOrient>::iterator iter = _axisDirections.begin();
		for (int i = 0; i < random; i++)
			iter++;
		_axisDirections.remove(*iter);
	}
}

// call back used by RejectGrid to indicate it's animation is complete 
void Launcher::rejectAnimComplete(FallingGrid* rejGrid)
{
	// remove the rejection grid from the list to render
	_rejList.remove(rejGrid);
	delete rejGrid;
}

// call back used by FallingGrid to indicate it's obsolete animation is complete 
void Launcher::obsoleteAnimComplete(FallingGrid* fallGrid)
{
	// remove the grid from the list to render
	_obsoleteList.remove(fallGrid);
	delete fallGrid;
}

// callback used by EvilFallingGrid to indicate a grid info change
void Launcher::onGridInfoChange(AxisOrient axis, const GridInfo& newGridInfo)
{
	LaunchButton* btn = axisToLaunchButton(axis);
	if (btn != nullptr)
		btn->update(newGridInfo.fillCol);

	// update the first falling glowing grid that matches the orientation
	std::list<LaunchStage*>::const_iterator iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		LaunchStage* item = (*iter);
		if (item->glowGrid != nullptr && item->glowGrid->getOrient() == newGridInfo.orient)
		{
			item->glowGrid->updateGridInfo(newGridInfo);
			break;
		}
		iter++;
	}
}

static int getListLockCount(const std::list<LaunchStage*>& list)
{
	int lockCount = 0;

	std::list<LaunchStage*>::const_iterator iter = list.begin();
	while (iter != list.end())
	{
		if ((*iter)->fallGrid != nullptr && (*iter)->fallGrid->canLock())
			lockCount++;
		iter++;
	}
	return lockCount;
}

bool Launcher::canLaunch() const
{
	// can never launch while burst mode is active
	if (_burstMode != BURST_NONE)
		return false;

	// if axis locking isn't on then we can always launch
	const Level& lvl = GameScripts::getCurrLevel();
	if (lvl.evilAxisLock == AXISLOCK_NONE && lvl.wildAxisLock == AXISLOCK_NONE)
		return true;

	// count the number of falling pieces that will lock an axis
	int lockCount = getListLockCount(_idleList);
	lockCount += getListLockCount(_hintList);
	lockCount += getListLockCount(_fallList);

	// determine if it exceeds the threshold
	if (lvl.evilAxisLock == AXISLOCK_ALL && lockCount > 0)
		return false;
	else if (lvl.wildAxisLock == AXISLOCK_ALL && lockCount > 0)
		return false;
	else if (lockCount >= lvl.directions)
		return false;
	return true;
}

static void checkListForValidOrientations(std::list<AxisOrient>& validOrient, std::list<LaunchStage*> list)
{
	std::list<LaunchStage*>::const_iterator iter = list.begin();
	while (iter != list.end())
	{
		const FallingGrid* grid = (*iter)->fallGrid;
		if (grid != nullptr && grid->canLock())
			validOrient.remove(grid->getOrient());
		iter++;
	}
}

const std::list<AxisOrient>& Launcher::getUnlockedAxisList() const
{
	// if there's no axis locking then just return the complete list of available axes
	const Level& lvl = GameScripts::getCurrLevel();
	if (lvl.evilAxisLock == AXISLOCK_NONE && lvl.wildAxisLock == AXISLOCK_NONE)
		return _axisDirections;

	// use a local copy
	static std::list<AxisOrient> locList;
	locList = _axisDirections;

	// produce a list of available unlocked axes
	checkListForValidOrientations(locList, _idleList);
	checkListForValidOrientations(locList, _hintList);
	checkListForValidOrientations(locList, _fallList);
	return locList;
}

static AxisOrient fromListByIndex(const std::list<AxisOrient>& list, int index)
{
	std::list<AxisOrient>::const_iterator iter = list.begin();
	for (int i = 0; i < index; i++)
		iter++;
	return (*iter);
}

AxisOrient Launcher::pickRandomOrientation()
{
	// get the list of axis orientations that are unlocked
	const std::list<AxisOrient>& list = getUnlockedAxisList();

	// and then pick one of them randomly
	int index = MigUtil::pickRandom(list.size());
	return fromListByIndex(list, index);
}

AxisOrient Launcher::pickSequentialOrientation()
{
	// get the list of axis orientations that are unlocked
	const std::list<AxisOrient>& list = getUnlockedAxisList();

	// go to the next axis
	_lastAxis = (_lastAxis + 1) % list.size();
	return fromListByIndex(list, _lastAxis);
}

AxisOrient Launcher::pickSequentialByColorOrientation(const GridInfo& grid)
{
	// get the list of axis orientations that are unlocked
	const std::list<AxisOrient>& list = getUnlockedAxisList();

	// go to the next axis only when the map-index changes
	if (grid.mapIndex != _lastIndex)
	{
		_lastAxis++;
		_lastIndex = grid.mapIndex;
	}
	_lastAxis = _lastAxis % list.size();
	return fromListByIndex(list, _lastAxis);
}

void Launcher::startIdleTimer(LaunchStage* stage, int idleTime)
{
	AnimItem animItem(this);
	animItem.configTimer(idleTime, false, stage);
	stage->idAnim = MigUtil::theAnimList->addItem(animItem);
}

bool Launcher::startGridLaunch(GridInfo& newGrid, AxisOrient orient, int idleTime, bool isBurst)
{
	if (orient == AXISORIENT_NONE)
		return false;

	// orientation was given to us
	newGrid.orient = orient;
	LaunchStage* stage = new LaunchStage();
	stage->glowGrid = new HintGrid(_gameCube, _lightBeam, newGrid);
	stage->fallGrid = new FallingGrid(newGrid, isBurst);

	// start the idle timer
	startIdleTimer(stage, idleTime);

	// remember this for tracking purposes only
	_idleList.push_back(stage);
	LOGINFO("(Launcher::startGridLaunch) Axis=%s", CubeUtil::axisToString(orient));
	return true;
}

bool Launcher::startGridLaunch(GridInfo& newGrid, AxisOrient orient)
{
	return startGridLaunch(newGrid, orient, _idleTime, false);
}

bool Launcher::startGridLaunch(GridInfo& newGrid)
{
	// launch style will determine either a random or sequential style
	AxisOrient orient = AXISORIENT_NONE;
	switch (GameScripts::getCurrLevel().launchAxisStyle)
	{
	case LAUNCHSTYLE_SEQUENTIAL:
		orient = pickSequentialOrientation();
		break;
	case LAUNCHSTYLE_BY_COLOR:
		orient = pickSequentialByColorOrientation(newGrid);
		break;
	default:    // LAUNCHSTYLE_NORMAL
		orient = pickRandomOrientation();
		break;
	}
	return startGridLaunch(newGrid, orient);
}

bool Launcher::startEvilGridLaunch(GridInfo* newGrids, int numGrids, EvilGridStyle evilStyle, AxisOrient orient)
{
	// need to have an array of grids at least 2 in length
	if (newGrids == nullptr)
		return false;
	if (numGrids < 2)
	{
		delete[] newGrids;
		return false;
	}

	for (int i = 0; i < numGrids; i++)
	{
		newGrids[i].orient = orient;

		// need to invert the fill lists
		newGrids[i].invertFillList();

		// adjust the grids based upon the style of evil falling piece
		if (evilStyle == EVILGRIDSTYLE_COLOR_ONLY)
			newGrids[i].setFillList(true);
		else if (evilStyle == EVILGRIDSTYLE_RANDOMIZED)
			newGrids[i].randomizeFillList(GameScripts::getCurrLevel().fallingHoleLimit);
	}

	// create a hint grid that's all black and completely filled in
	GridInfo hintGrid;
	hintGrid.init(orient, newGrids[0].dimen, -1);
	hintGrid.emptyCol = hintGrid.fillCol = MigTech::colBlack;

	// create the launch stage object to shepherd the falling grid through the stages
	LaunchStage* stage = new LaunchStage();
	stage->glowGrid = new HintGrid(_gameCube, _lightBeam, hintGrid);
	stage->fallGrid = new EvilFallingGrid(newGrids, numGrids, false, this);

	// start the idle timer
	startIdleTimer(stage, _idleTime);

	// remember this for tracking purposes only
	_idleList.push_back(stage);
	LOGINFO("(Launcher::startEvilGridLaunch) Axis=%s, grids=%d, style=%d", CubeUtil::axisToString(orient), numGrids, evilStyle);
	return true;
}

bool Launcher::startEvilGridLaunch(GridInfo* newGrids, int numGrids, EvilGridStyle evilStyle)
{
	// need to randomize the orientation
	AxisOrient orient = pickRandomOrientation();
	return startEvilGridLaunch(newGrids, numGrids, evilStyle, orient);
}

bool Launcher::startEvilGridLaunch(GridInfo* newGrids, int numGrids)
{
	// get the evil style from the script
	const Level& lvl = GameScripts::getCurrLevel();
	return startEvilGridLaunch(newGrids, numGrids, lvl.evilStyle);
}

bool Launcher::startWildGridLaunch(GridInfo* newGrids, int numGrids, WildGridStyle wildStyle)
{
	// need to have an array of grids at least 2 in length
	if (newGrids == nullptr)
		return false;
	if (numGrids < 2)
	{
		delete[] newGrids;
		return false;
	}

	// by definition wild card falling pieces are randomly filled in for now,
	//  and since they all have to be the same we'll set the first one here and copy the rest
	newGrids[0].setFillList(true);
	newGrids[0].randomizeFillList(GameScripts::getCurrLevel().fallingHoleLimit);

	// need to randomize the orientation
	AxisOrient orient = pickRandomOrientation();
	for (int i = 0; i < numGrids; i++)
	{
		newGrids[i].orient = orient;

		// copy the fill list from the first one
		if (i > 0)
			newGrids[i].copyFillList(newGrids[0]);
	}

	// create a hint grid that's all white and completely filled in
	GridInfo hintGrid;
	hintGrid.init(orient, newGrids[0].dimen, -1);
	hintGrid.copyFillList(newGrids[0]);	// make the shafts match the first one
	hintGrid.emptyCol = hintGrid.fillCol = MigTech::colWhite;

	// create the launch stage object to shepherd the falling grid through the stages
	LaunchStage* stage = new LaunchStage();
	stage->glowGrid = new HintGrid(_gameCube, _lightBeam, hintGrid);
	stage->fallGrid = new WildFallingGrid(newGrids, numGrids, false);

	// start the idle timer
	startIdleTimer(stage, _idleTime);

	// remember this for tracking purposes only
	_idleList.push_back(stage);
	LOGINFO("(Launcher::startWildGridLaunch) Axis=%s, grids=%d, style=%d", CubeUtil::axisToString(orient), numGrids, wildStyle);
	return true;
}

bool Launcher::startWildGridLaunch(GridInfo* newGrids, int numGrids)
{
	// get the wild-card style from the script
	const Level& lvl = GameScripts::getCurrLevel();
	return startWildGridLaunch(newGrids, numGrids, lvl.wildStyle);
}

bool Launcher::startBurstLaunch(GridInfo* newGrids, int numGrids)
{
	if (newGrids == nullptr)
		return false;

	// compute the idle base time
	const Level& lvl = GameScripts::getCurrLevel();
	int idleBase = 0;
	if (_hintList.size() > 0)
		idleBase += (lvl.glowTime / 2);
	else if (_fallList.size() > 0)
		idleBase += (lvl.fallTime / 2);

	AxisOrient axes[3] = { AXISORIENT_X, AXISORIENT_Y, AXISORIENT_Z };
	if (lvl.gridLock != GRIDLOCK_NONE)
	{
		LOGINFO("(Launcher::startBurstLaunch) Starting burst launch from %d candidates, grid locking is on", numGrids);

		// chose a random grid from the candidates
		int randIndex = MigUtil::pickRandom(numGrids);
		GridInfo chosenGrid = newGrids[randIndex];

		// create a comparison array
		int len = chosenGrid.fillListLen();
		int* slotArray = new int[len];
		memset(slotArray, 0, len*sizeof(int));

		// assign at least one to each new falling piece
		slotArray[0] = 1;
		slotArray[1] = 2;
		slotArray[2] = 3;

		// randomly assign the rest
		for (int i = 3; i < len; i++)
			slotArray[i] = 1 + MigUtil::pickRandom(3);

		for (int i = 0; i < 3; i++)
		{
			chosenGrid.assignFillList(slotArray, i + 1);
			startGridLaunch(chosenGrid, axes[i], idleBase + i * 100, true);
		}
		delete[] slotArray;
	}
	else if (numGrids >= 3)
	{
		LOGINFO("(Launcher::startBurstLaunch) Starting burst launch from %d candidates, grid locking is off", numGrids);
		for (int i = 0; i < 3; i++)
		{
			newGrids[i].invertFillList();
			newGrids[i].randomizeFillList(lvl.fallingHoleLimit);
			startGridLaunch(newGrids[i], axes[i], idleBase + i * 100, true);
		}
	}

	_burstMode = BURST_NORMAL;
	return true;
}

bool Launcher::startEvilBurstLaunch(GridInfo* newGrids, int numGrids)
{
	if (newGrids == nullptr)
		return false;
	LOGINFO("(Launcher::startEvilBurstLaunch) Starting evil burst launch from %d candidates", numGrids);

	// compute the idle base time
	const Level& lvl = GameScripts::getCurrLevel();
	int idleBase = 0;
	if (_hintList.size() > 0)
		idleBase += (lvl.glowTime / 2);
	else if (_fallList.size() > 0)
		idleBase += (lvl.fallTime / 2);

	AxisOrient axes[3] = { AXISORIENT_X, AXISORIENT_Y, AXISORIENT_Z };
	for (int i = 0; i < 3; i++)
	{
		// pick 2 random grids (note that previously numGrids is at least 3)
		int pick1 = MigUtil::pickRandom(numGrids);
		int pick2 = MigUtil::pickRandom(numGrids);
		while (pick2 == pick1)
			pick2 = MigUtil::pickRandom(numGrids);

		// produce a new array composed of those 2 grid candidates
		GridInfo* localGrids = new GridInfo[2];
		localGrids[0] = newGrids[pick1];
		localGrids[0].orient = axes[i];
		localGrids[0].invertFillList();
		localGrids[0].randomizeFillList(2);
		localGrids[1] = newGrids[pick2];
		localGrids[1].orient = axes[i];
		localGrids[1].invertFillList();
		localGrids[1].randomizeFillList(2);

		// create a hint grid that's all black and completely filled in
		GridInfo hintGrid;
		hintGrid.init(axes[i], localGrids[0].dimen, -1);
		hintGrid.emptyCol = hintGrid.fillCol = MigTech::colBlack;

		// create the launch stage object to shepherd the falling grid through the stages
		LaunchStage* stage = new LaunchStage();
		stage->glowGrid = new HintGrid(_gameCube, _lightBeam, hintGrid);
		stage->fallGrid = new EvilFallingGrid(localGrids, 2, true, this);

		// start the idle timer
		startIdleTimer(stage, idleBase + i * 100);

		// remember this for tracking purposes only
		_idleList.push_back(stage);
	}

	return true;
}

void Launcher::addRejection(const GridInfo& rejGridInfo)
{
	// create a new object to render the rejection
	FallingGrid* fallGrid = new FallingGrid(rejGridInfo, false);
	fallGrid->init(0);
	//fallGrid->createGraphics();
	if (fallGrid->startRejectAnim(defRejectAnimDuration, this))
		_rejList.push_back(fallGrid);
}

void Launcher::startObsoleteAnim(LaunchStage* stage)
{
	// start the obsolete animation
	if (stage->fallGrid->startObsoleteAnim(defRejectAnimDuration, this))
	{
		_obsoleteList.push_back(stage->fallGrid);

		// this is so the grid won't be deleted when the stage is deleted
		stage->fallGrid = nullptr;
	}
}

void Launcher::checkListForObsoleteFallingPieces(std::list<LaunchStage*>& list, int mapIndex, bool doObsoleteAnim)
{
	std::list<LaunchStage*>::iterator iter = list.begin();
	while (iter != list.end())
	{
		LaunchStage* stage = *iter;
		iter++;

		FallingGrid* grid = stage->fallGrid;
		if (grid->checkObsolete(mapIndex))
		{
			// might need to fade away
			if (doObsoleteAnim)
				startObsoleteAnim(stage);

			// remove it from the given list
			list.remove(stage);
			delete stage;
		}
	}
}

void Launcher::checkForObsoleteFallingPieces(int mapIndex)
{
	// check for any pieces in the queue that are now obsolete
	checkListForObsoleteFallingPieces(_idleList, mapIndex, false);
	checkListForObsoleteFallingPieces(_hintList, mapIndex, false);
	checkListForObsoleteFallingPieces(_fallList, mapIndex, true);

	// in case some went obsolete, update the launch buttons
	evalLaunchButtons();
}

void Launcher::startTapAnim(FallingGrid* hitGrid)
{
	LOGINFO("(Launcher::startTapAnim) Starting tap animation (%s)", CubeUtil::axisToString(hitGrid->getOrient()));

	//CubeUtil.info("Falling grid matched, beginning accel animation");
	hitGrid->startTapAnim(defTapAccelAnimDuration);

	// play the tap sound
	if (_tapSound != nullptr)
		_tapSound->playSound(false);
}

bool Launcher::doPieceAccel(AxisOrient orient)
{
	// find the first falling grid that matches the chosen orientation
	FallingGrid* hitGrid = nullptr;
	std::list<LaunchStage*>::iterator iter = _fallList.begin();
	while (hitGrid == nullptr && iter != _fallList.end())
	{
		FallingGrid* fallGrid = (*iter)->fallGrid;
		if ((orient == AXISORIENT_NONE || fallGrid->getOrient() == orient) && !fallGrid->isTapped())
			hitGrid = fallGrid;
		iter++;
	}

	if (hitGrid == nullptr)
	{
		// no falling grid matched, so look in the hint list instead
		iter = _hintList.begin();
		while (hitGrid == nullptr && iter != _hintList.end())
		{
			HintGrid* hintGrid = (*iter)->glowGrid;
			FallingGrid* fallGrid = (*iter)->fallGrid;
			if ((orient == AXISORIENT_NONE || hintGrid->getOrient() == orient) && !fallGrid->isTapped())
			{
				// cancel the hint grid and start the tap animation
				hintGrid->setCancel(true);
				hitGrid = fallGrid;
			}
			iter++;
		}
	}

	if (hitGrid != nullptr)
		startTapAnim(hitGrid);
	return (hitGrid != nullptr);
}

bool Launcher::onTap(float x, float y, const Matrix& proj, const Matrix& view)
{
	LOGINFO("(Launcher::onTap) Checking for tap at (%f,%f)", x, y);

	// narrow down candidates to an axis based upon tap region
	AxisOrient axis = AXISORIENT_NONE;
	if (x < 0.45f && y > 0.5f)
		axis = AXISORIENT_Z;
	else if (x > 0.55f && y > 0.5f)
		axis = AXISORIENT_X;
	else if (x > 0.4f && x < 0.6f && y < 0.5f)
		axis = AXISORIENT_Y;

	FallingGrid* bestMatch = nullptr;
	float bestDist = 99999;

	// look for the closest match
	std::list<LaunchStage*>::const_iterator iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		FallingGrid* grid = (*iter)->fallGrid;
		if (axis == grid->getOrient())
		{
			float dist = grid->checkTap(x, y, proj, view);
			if (dist >= 0 && dist < bestDist)
			{
				bestMatch = grid;
				bestDist = dist;
			}
		}
		iter++;
	}

	if (bestMatch != nullptr)
		bestMatch->startTapAnim(defTapAccelAnimDuration);
	return (bestMatch != nullptr);
}

bool Launcher::onClick(int id)
{
	AxisOrient axis = AXISORIENT_NONE;
	if (id == ID_LEFT_LAUNCH_BUTTON)
		axis = AXISORIENT_Z;
	else if (id == ID_RIGHT_LAUNCH_BUTTON)
		axis = AXISORIENT_X;
	else if (id == ID_CENTER_LAUNCH_BUTTON)
		axis = AXISORIENT_Y;

	if (axis != AXISORIENT_NONE)
	{
		doPieceAccel(axis);
		LaunchButton* btn = axisToLaunchButton(axis);
		if (btn != nullptr)
			btn->startTapAnim();
	}
	return (axis != AXISORIENT_NONE);
}

bool Launcher::onKey(VIRTUAL_KEY key)
{
	if (key == SPACE || key == ENTER)
	{
		doPieceAccel(AXISORIENT_NONE);
		return true;
	}
	return false;
}

void Launcher::clearAllItems()
{
	std::list<LaunchStage*>::iterator iter = _idleList.begin();
	while (iter != _idleList.end())
	{
		delete (*iter);
		iter++;
	}
	_idleList.clear();

	iter = _hintList.begin();
	while (iter != _hintList.end())
	{
		delete (*iter);
		iter++;
	}
	_hintList.clear();

	// any remaining falling pieces become obsolete
	iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		startObsoleteAnim(*iter);
		delete (*iter);
		iter++;
	}
	_fallList.clear();

	// this should hide all of the launch buttons
	evalLaunchButtons();
}

void Launcher::draw(const Matrix& mat) const
{
	// draw the falling grids
	std::list<LaunchStage*>::const_iterator iter = _fallList.begin();
	while (iter != _fallList.end())
	{
		(*iter)->fallGrid->draw(mat);
		iter++;
	}

	// activate blending for obsolete and rejection grids
	MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);

	// draw the obsolete falling grids, while they are still visible
	std::list<FallingGrid*>::const_iterator iter2 = _obsoleteList.begin();
	while (iter2 != _obsoleteList.end())
	{
		(*iter2)->draw(mat);
		iter2++;
	}

	// draw the rejection grids
	iter2 = _rejList.begin();
	while (iter2 != _rejList.end())
	{
		(*iter2)->draw(mat);
		iter2++;
	}

	if (CubeUtil::renderPass == RENDER_PASS_FINAL)
	{
		// the hint grids will continue to glow during the fall stage to illumate where pieces will land
		iter = _fallList.begin();
		while (iter != _fallList.end())
		{
			if ((*iter)->glowGrid != nullptr)
				(*iter)->glowGrid->draw(mat, false);
			iter++;
		}

		// draw the glowing grids (during the glow stage)
		iter = _hintList.begin();
		while (iter != _hintList.end())
		{
			(*iter)->glowGrid->draw(mat, true);
			iter++;
		}
	}
}

void Launcher::draw() const
{
	static Matrix locMatrix;
	draw(locMatrix);
}
