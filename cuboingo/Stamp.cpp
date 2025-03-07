#include "pch.h"
#include "CubeConst.h"
#include "CubeUtil.h"
#include "Stamp.h"
#include "../core/AnimList.h"

using namespace MigTech;
using namespace Cuboingo;

const std::string stampVertexShader = "cvs_Stamp";
const std::string stampPixelShader = "cps_Stamp";

static const int ANIM_FADE_DURATION = 500;
static const int ANIM_GRID_DURATION = 500;
static const int ANIM_BOUNCE_DURATION = 500;

///////////////////////////////////////////////////////////////////////////
// StampGrid

StampGrid::StampGrid(IStampGridUpdate* callback) : GridBase()
{
	_callback = callback;
	_grids = nullptr;
	_numGrids = 0;
	_currGrid = 0;
	_slotRadius = 0;
	_visible = false;
}

bool StampGrid::init(const GridInfo& info, float dist)
{
	if (_grids == nullptr)
		_grids = new GridInfo[1];
	_grids[0] = info;
	_numGrids = 1;
	_currGrid = 0;

	// configure the new stamp grid from the info passed in
	if (!GridBase::init(info.dimen, info.mapIndex, AXISORIENT_Z, dist))
	{
		LOGWARN("(StampGrid::init) configGrid() returned false (dimen=%f)!", info.dimen);
		return false;
	}
	updateAllForNewGrid();
	setSlotRadius(0);
	setVisible(true);

	return true;
}

bool StampGrid::init(GridInfo* infoArray, int numArray, float dist)
{
	if (infoArray == nullptr || numArray < 1)
		throw std::invalid_argument("(StampGrid::init) Invalid grid info array");
	if (_grids != nullptr)
		delete[] _grids;
	_grids = infoArray;
	_numGrids = numArray;
	_currGrid = 0;

	// configure the new stamp grid from the info passed in
	if (!GridBase::init(infoArray[0].dimen, -1, AXISORIENT_Z, dist))
	{
		LOGWARN("(StampGrid::init) configGrid() returned false (dimen=%f)!", infoArray[0].dimen);
		return false;
	}
	updateAllForNewGrid();
	setSlotRadius(0);
	setVisible(true);

	return true;
}

void StampGrid::setSlotRadius(float newVal)
{
	_slotRadius = newVal;
	updateSlotRadius(newVal, false);
}

void StampGrid::setSlotAlpha(float newVal)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].color.a = newVal;
}

void StampGrid::updateSlotColors(const Color& newColor)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].setColor(newColor);
}

void StampGrid::updateSlotVisibility(const bool* fillList)
{
	int numSlots = getSlotCount();
	for (int i = 0; i < numSlots; i++)
		_theSlots[i].invis = !fillList[i];
}

void StampGrid::updateAllForNewGrid()
{
	updateMapIndex(_grids[_currGrid].mapIndex);
	updateSlotColors(_grids[_currGrid].fillCol);
	updateSlotVisibility(_grids[_currGrid].fillList);
}

void StampGrid::startGridAnim(int duration, bool fadeIn)
{
	AnimItem animItem(this);
	if (fadeIn)
	{
		float fParams[] = { 0.0f, 0.35f, 0.6f, 0.8f, 0.95f, 1.05f, 1.12f, 1.15f, 1.15f, 1.1f, 1.0f };
		animItem.configParametricAnim(0, 1, duration, fParams, ARRAYSIZE(fParams));
	}
	else
	{
		float fParams[] = { 1.0f, 1.1f, 1.15f, 1.15f, 1.12f, 1.05f, 0.95f, 0.8f, 0.6f, 0.35f, 0.0f };
		animItem.configParametricAnim(0, 1, duration, fParams, ARRAYSIZE(fParams));
	}
	_idSlotAnim = MigUtil::theAnimList->addItem(animItem);
}

void StampGrid::startFadeAnim(int duration, bool fadeIn)
{
	float fParams[] = { 1.0f, 1.0f, 1.0f, 0.9f, 0.7f, 0.4f, 0.0f };
	AnimItem animItemF(this);
	animItemF.configParametricAnim((fadeIn ? 1.0f : 0), (fadeIn ? 0 : 1.0f), duration, fParams, ARRAYSIZE(fParams));
	_idSlotFade = MigUtil::theAnimList->addItem(animItemF);
}

void StampGrid::cancelAnim()
{
	_idSlotAnim.clearAnim();
	_idSlotFade.clearAnim();
	_idLifeCycle.clearAnim();
	_idInternalCycle.clearAnim();
}

bool StampGrid::doFrame(int id, float newVal, void* optData)
{
	if (_idSlotFade == id)
	{
		// animate the slot transparency
		setSlotAlpha(newVal);
	}
	else if (_idSlotAnim == id)
	{
		// animate the slot sizes
		setSlotRadius(newVal);
	}

	return true;
}

void StampGrid::animComplete(int id, void* optData)
{
	if (_idSlotAnim == id)
	{
		_idSlotAnim = 0;

		// no callbacks if these animations complete w/in a lifecycle
		bool animIn = (getSlotRadius() > 0);
		if (!_idLifeCycle.isActive())
		{
			if (_callback != nullptr)
				_callback->gridAnimComplete(animIn);

			if (animIn)
			{
				// grid animation in is complete, start the life cycle timer
				AnimItem animItem(this);
				animItem.configTimer(GameScripts::getCurrLevel().stampDuration, false);
				_idLifeCycle = MigUtil::theAnimList->addItem(animItem);
			}
		}

		if (_numGrids > 1 && _idLifeCycle.isActive())
		{
			if (animIn)
			{
				if (GameScripts::getCurrLevel().stampCycle > 0)
				{
					AnimItem animItem(this);
					animItem.configTimer(GameScripts::getCurrLevel().stampCycle, false);
					_idInternalCycle = MigUtil::theAnimList->addItem(animItem);
				}
				else
					startGridAnim(ANIM_GRID_DURATION, false);
			}
			else if (!animIn)
			{
				// move to the next grid to display
				_currGrid = (_currGrid + 1) % _numGrids;
				updateAllForNewGrid();

				startGridAnim(ANIM_GRID_DURATION, true);
			}
		}
	}
	else if (_idLifeCycle == id)
	{
		_idLifeCycle = 0;

		if (isVisible() && _callback != nullptr)
			_callback->lifecycleExpired();
	}
	else if (_idInternalCycle == id)
	{
		_idInternalCycle = 0;
		startGridAnim(ANIM_GRID_DURATION, false);
	}
	else if (_idSlotFade == id)
	{
		_idSlotFade = 0;
		setVisible(false);
	}
}

void StampGrid::draw(const Matrix& mat) const
{
	// if the slot alpha is animating then we need alpha blending
	if (_idSlotFade.isActive())
		MigUtil::theRend->setBlending(BLEND_STATE_SRC_ALPHA);

	GridBase::draw(mat);
}

///////////////////////////////////////////////////////////////////////////
// Stamp

// fixed colors
static Color COLOR_DARK(0.1f, 0.1f, 0.1f);
static Color COLOR_MID(0.2f, 0.2f, 0.2f);
static Color COLOR_BRIGHT(0.3f, 0.3f, 0.3f);

Stamp::Stamp(AxisOrient orient, IStampUpdate* callback)
	: CubeBase(Cuboingo::defStampRadius, Cuboingo::defStampRadius, Cuboingo::defStampDepth),
	_stampGrid(this)
{
	_vertexShader = stampVertexShader;
	_pixelShader = stampPixelShader;

	// don't use rounded edges
	_isRounded = false;

	_orient = orient;
	_callback = callback;

	// reset translation (only Z actually, rotation takes care of the rest)
	resetTranslation();

	// rotation depends on orientation
	_rotX = MigUtil::convertToRadians(orient == AXISORIENT_Y ? -90.0f : 0.0f);
	_rotY = MigUtil::convertToRadians(orient == AXISORIENT_X ? 90.0f : 0.0f);
	_rotZ = 0;

	// stamps are invisible by default
	_color.a = 0;
}

bool Stamp::doFrame(int id, float newVal, void* optData)
{
	if (_idOpacity == id)
	{
		// animate the slot transparency as well
		_stampGrid.setSlotAlpha(newVal);
	}
	return CubeBase::doFrame(id, newVal, optData);
}

void Stamp::animComplete(int id, void* optData)
{
	if (_idOpacity == id)
	{
		_idOpacity = 0;

		if (_color.a > 0)
		{
			if (_stampGrid.isVisible())
			{
				// fade in is complete, start the grid animation
				_stampGrid.startGridAnim(ANIM_GRID_DURATION, true);
			}
		}
		else
		{
			// fade out is complete, we're done
			//_stampGrid.setVisible(false);
			resetTranslation();
		}
	}
	CubeBase::animComplete(id, optData);
}

void Stamp::gridAnimComplete(bool animIn)
{
	if (!animIn)
	{
		// grid animation out is complete, start fade out
		AnimItem animItem(this);
		animItem.configSimpleAnim(1, 0, ANIM_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
		_idOpacity = MigUtil::theAnimList->addItem(animItem);
	}
}

void Stamp::lifecycleExpired()
{
	// this counts as an expired stamp
	if (_callback != nullptr)
		_callback->stampExpired(_orient);

	// life cycle complete, start the grid animation out
	hideStamp();
}

void Stamp::draw(Matrix& mat) const
{
	CubeBase::draw(mat);

	// draw the grid only if the fade in is complete
	if (_stampGrid.isVisible() && _stampGrid.getSlotRadius() > 0)
	{
		// draw the grid
		_stampGrid.draw(mat);
	}
}

void Stamp::draw() const
{
	CubeBase::draw();
}

// shows a stamp w/ a particular grid overlaid on it
void Stamp::showStamp(const GridInfo& grid)
{
	LOGINFO("(Stamp::showStamp) Orient=%s", CubeUtil::axisToString(_orient));

	_stampGrid.init(grid, _radiusZ + 0.01f);

	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, ANIM_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idOpacity = MigUtil::theAnimList->addItem(animItem);
}

// shows a stamp w/ a cycling series of grids overlaid on it
void Stamp::showStamp(GridInfo* infoArray, int numArray)
{
	LOGINFO("(Stamp::showStamp) Orient=%s, cycling %d grids", CubeUtil::axisToString(_orient), numArray);

	_stampGrid.init(infoArray, numArray, _radiusZ + 0.01f);

	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, ANIM_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idOpacity = MigUtil::theAnimList->addItem(animItem);
}

// shows a stamp w/ no grid overlay
void Stamp::showStamp()
{
	LOGINFO("(Stamp::showStamp) Orient=%s, no overlay", CubeUtil::axisToString(_orient));

	_stampGrid.setVisible(false);

	AnimItem animItem(this);
	animItem.configSimpleAnim(0, 1, ANIM_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
	_idOpacity = MigUtil::theAnimList->addItem(animItem);
}

// performs collision logic when the cube hits the stamp
void Stamp::doCollision(bool wasHit, const GridInfo& newGrid)
{
	const Script& script = GameScripts::getCurrScript();
	if (script.isLoaded() && GameScripts::getCurrLevel().stampStyle != STAMPSTYLE_BONUS_ALWAYS)
	{
		// the stamp grid disappears
		if (wasHit)
			_stampGrid.setVisible(false);

		// start fade out
		hideStamp(false);

		// start a rapid recoil
		float tParams[] = { 0.0f, 0.6f, 0.9f, 1.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(_translate.z, _translate.z - defStampRecoil, ANIM_FADE_DURATION / 2, tParams, ARRAYSIZE(tParams));
		_idTransZ = MigUtil::theAnimList->addItem(animItem);
	}
	else
	{
		// start a bounce
		float tParams[] = { 0.0f, 0.6f, 0.9f, 1.0f, 0.9f, 0.8f, 0.6f, 0.4f, 0.2f, 0.1f, 0.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(_translate.z, _translate.z - defStampRecoil / 2, ANIM_BOUNCE_DURATION, tParams, ARRAYSIZE(tParams));
		_idTransZ = MigUtil::theAnimList->addItem(animItem);

		GridInfo newInfo = newGrid;
		newInfo.orient = _orient;
		_stampGrid.init(newInfo, _radiusZ + 0.01f);
		_stampGrid.setSlotRadius(1);    // by default these display
		_stampGrid.setVisible(true);

		// start a fade out of the grid
		_stampGrid.startFadeAnim(ANIM_FADE_DURATION, false);
	}
}

// cancels any stamp animations and fades it out
void Stamp::hideStamp(bool doSlotAnim)
{
	// cancel any existing animations
	_stampGrid.cancelAnim();
	_idOpacity.clearAnim();

	if (doSlotAnim && _stampGrid.isVisible() && _stampGrid.getSlotRadius() > 0)
	{
		// start the grid animation out
		_stampGrid.startGridAnim(ANIM_GRID_DURATION, false);
	}
	else if (_color.a > 0)
	{
		// grid wasn't visible so start the fade out
		AnimItem animItem(this);
		animItem.configSimpleAnim(1, 0, ANIM_FADE_DURATION, AnimItem::ANIM_TYPE_LINEAR);
		_idOpacity = MigUtil::theAnimList->addItem(animItem);
	}
}

// resets the Z translation to the default value (w/o recoil)
void Stamp::resetTranslation()
{
	// note that after translation, the orientation will rotate it to the correct axis
	if (_orient == AXISORIENT_Y)
		_translate.z = -defStampDistY;
	else
		_translate.z = -defStampDistXZ;
}

///////////////////////////////////////////////////////////////////////////
// StampList

StampList::StampList(ScoreKeeper* sk) :
	_scoreKeeper(sk),
	_stampX(AXISORIENT_X, this),
	_stampY(AXISORIENT_Y, this),
	_stampZ(AXISORIENT_Z, this)
{
}

StampList::~StampList()
{
	GameScripts::removeCallback(this);
}

bool StampList::init()
{
	// we want to know when the level changes
	GameScripts::addCallback(this);

	// these are the regions on the screen the stamps will occupy
	_stampX.setScreenRect(Rect(0.21f, 0.35f, 0.16f, 0.21f));
	_stampY.setScreenRect(Rect(0.40f, 0.75f, 0.20f, 0.25f));
	_stampZ.setScreenRect(Rect(0.63f, 0.35f, 0.16f, 0.21f));

	// init the power-up icons
	_powerUps[AXISORIENT_X].initIcon(Vector3(-0.35f, 0.4f, 0), Size(0.16f, 0.20f));
	_powerUps[AXISORIENT_Z].initIcon(Vector3( 0.35f, 0.4f, 0), Size(0.16f, 0.20f));

	return true;
}

void StampList::createGraphics()
{
	_stampX.createGraphics();
	_stampY.createGraphics();
	_stampZ.createGraphics();

	PowerUp::createGraphics();
}

void StampList::destroyGraphics()
{
	_stampX.destroyGraphics();
	_stampY.destroyGraphics();
	_stampZ.destroyGraphics();

	PowerUp::destroyGraphics();
}

static void drawStamp(const Stamp& theStamp)
{
	if (theStamp.isVisible())
		theStamp.draw();
}

void StampList::draw() const
{
	drawStamp(_stampX);
	drawStamp(_stampY);
	drawStamp(_stampZ);
}

void StampList::drawIcons() const
{
	std::map<AxisOrient, PowerUp>::const_iterator iter = _powerUps.begin();
	while (iter != _powerUps.end())
	{
		iter->second.drawIcon();
		iter++;
	}
}

// IGameScriptUpdate override - a new level is starting
void StampList::newLevel(int level)
{
	const Level& lvl = GameScripts::getCurrLevel();
	if (lvl.stampStyle == STAMPSTYLE_BONUS_ALWAYS)
	{
		int nStamps = lvl.stampCount;

		if (!_stampX.isVisible() && (nStamps == 2 || nStamps == 3))
			_stampX.showStamp();
		else if (_stampX.isVisible())
			_stampX.hideStamp(false);

		if (!_stampY.isVisible() && (nStamps == 1 || nStamps == 3))
			_stampY.showStamp();
		else if (_stampY.isVisible())
			_stampY.hideStamp(false);

		if (!_stampZ.isVisible() && (nStamps == 2 || nStamps == 3))
			_stampZ.showStamp();
		else if (_stampZ.isVisible())
			_stampZ.hideStamp(false);
	}
	else
	{
		_stampX.hideStamp(false);
		_stampY.hideStamp(false);
		_stampZ.hideStamp(false);
	}
}

// IStampUpdate override - stamp expired event
void StampList::stampExpired(AxisOrient orient)
{
	// an expired stamp only counts against the score if it wasn't a power-up type
	if (orient != AXISORIENT_NONE && _scoreKeeper != nullptr && _powerUps[orient] != POWERUPTYPE_NONE)
		_scoreKeeper->stampExpired(orient);
	_powerUps[orient] = POWERUPTYPE_NONE;
}

// returns true if stamping is active for this level
bool StampList::isStampingActive() const
{
	return (GameScripts::getCurrLevel().stampCount > 0);
}

// returns true if any stamp for this level is available for use
bool StampList::isStampAvailable() const
{
	int nStamps = GameScripts::getCurrLevel().stampCount;
	if (nStamps == 1)
		return (!_stampY.isVisible());
	else if (nStamps == 2)
		return (!_stampX.isVisible() || !_stampZ.isVisible());
	else if (nStamps == 3)
		return (!_stampX.isVisible() || !_stampY.isVisible() || !_stampZ.isVisible());
	return false;
}

Stamp* StampList::axisToStamp(AxisOrient axis)
{
	if (axis == AXISORIENT_X)
		return &_stampX;
	else if (axis == AXISORIENT_Y)
		return &_stampY;
	else if (axis == AXISORIENT_Z)
		return &_stampZ;
	return nullptr;
}

const Stamp* StampList::axisToStamp(AxisOrient axis) const
{
	if (axis == AXISORIENT_X)
		return &_stampX;
	else if (axis == AXISORIENT_Y)
		return &_stampY;
	else if (axis == AXISORIENT_Z)
		return &_stampZ;
	return nullptr;
}

// returns true if the given stamp axis is active (displayed)
bool StampList::isStampAxisActive(AxisOrient axis) const
{
	const Stamp* s = axisToStamp(axis);
	return (s != nullptr ? s->isVisible() : false);
}

// returns true if the given stamp axis can be stamped
bool StampList::isStampAxisStampable(AxisOrient axis) const
{
	const Stamp* s = axisToStamp(axis);
	return (s != nullptr ? s->isStampable() : false);
}

// returns true if the given map index is used by any stamp
bool StampList::isMapIndexUsed(int mapIndex) const
{
	if (mapIndex == _stampX.getMapIndex())
		return true;
	else if (mapIndex == _stampY.getMapIndex())
		return true;
	else if (mapIndex == _stampZ.getMapIndex())
		return true;
	return false;
}

// processes a tap and returns an axis if it hits a visible stamp
AxisOrient StampList::processTap(float x, float y) const
{
	if (_stampX.isInStamp(x, y))
		return AXISORIENT_X;
	else if (_stampY.isInStamp(x, y))
		return AXISORIENT_Y;
	else if (_stampZ.isInStamp(x, y))
		return AXISORIENT_Z;
	return AXISORIENT_NONE;
}

Stamp* StampList::getRandomAvailableStamp()
{
	std::vector<Stamp*> potentials;

	int nStamps = GameScripts::getCurrLevel().stampCount;
	if (nStamps == 2 || nStamps == 3)
	{
		if (!_stampX.isVisible())
			potentials.push_back(&_stampX);
	}
	if (nStamps == 1 || nStamps == 3)
	{
		if (!_stampY.isVisible())
			potentials.push_back(&_stampY);
	}
	if (nStamps == 2 || nStamps == 3)
	{
		if (!_stampZ.isVisible())
			potentials.push_back(&_stampZ);
	}

	int size = potentials.size();
	return (size > 0 ? potentials[MigUtil::pickRandom(size)] : nullptr);
}

// displays the stamp w/ the given grid overlay
bool StampList::startNewStamp(const GridInfo& newGrid)
{
	Stamp* theStamp = getRandomAvailableStamp();
	if (theStamp != nullptr)
	{
		theStamp->showStamp(newGrid);
		_powerUps[theStamp->getOrient()] = POWERUPTYPE_NONE;
	}

	return (theStamp != nullptr);
}

// displays the stamp w/ the given grid overlay
bool StampList::startNewStamp(const GridInfo& newGrid, AxisOrient axis)
{
	if (axis == AXISORIENT_X)
		_stampX.showStamp(newGrid);
	else if (axis == AXISORIENT_Y)
		_stampY.showStamp(newGrid);
	else if (axis == AXISORIENT_Z)
		_stampZ.showStamp(newGrid);
	_powerUps[axis] = POWERUPTYPE_NONE;

	return true;
}

// displays the stamp w/o any overlay
bool StampList::startNewStamp(AxisOrient axis)
{
	if (axis == AXISORIENT_X)
		_stampX.showStamp();
	else if (axis == AXISORIENT_Y)
		_stampY.showStamp();
	else if (axis == AXISORIENT_Z)
		_stampZ.showStamp();
	_powerUps[axis] = POWERUPTYPE_NONE;

	return true;
}

// displays the stamp w/ the given grids cycling
bool StampList::startNewStamp(GridInfo* newGrids, int numGrids)
{
	if (newGrids != nullptr && numGrids > 1)
	{
		Stamp* theStamp = getRandomAvailableStamp();
		if (theStamp != nullptr)
		{
			theStamp->showStamp(newGrids, numGrids);

			// assign a random power-up, if possible
			const Level& lvl = GameScripts::getCurrLevel();
			std::vector<PowerUpType> _powerUpOptions;
			if (lvl.powerUpInvDur > 0 || lvl.powerUpInvMax > 0)
				_powerUpOptions.push_back(POWERUPTYPE_INVULNERABLE);
			if (GameScripts::getCurrLevel().scoreCfg.isValid() && lvl.powerUpMultDur > 0)
				_powerUpOptions.push_back(POWERUPTYPE_MULTIPLIER);
			//if ((_scoreKeeper->getLevelMissCount() > 0 || _scoreKeeper->getGameMissCount() > 0))
			//	_powerUpOptions.push_back(POWERUPTYPE_DROP_MISS);
			if (!_powerUpOptions.empty())
			{
				int index = MigUtil::pickRandom(_powerUpOptions.size());
				PowerUp& powerUp = _powerUps[theStamp->getOrient()];
				powerUp = _powerUpOptions[index];
				powerUp.startIconAnim();
			}

			return true;
		}
	}

	delete [] newGrids;
	return false;
}

static void swapItemsInFillList(bool* fillList, int i1, int i2)
{
	bool t = fillList[i1];
	fillList[i1] = fillList[i2];
	fillList[i2] = t;
}

// gets the grid info that overlays a given stamp (and inverts the slots if necessary since they are facing the cube back faces)
bool StampList::getStampGridInfo(GridInfo& info, AxisOrient axis, bool needToInvert)
{
	Stamp* s = axisToStamp(axis);
	if (s == nullptr)
		return false;
	if (!s->isStampGridVisible())
		return false;

	info = s->getStampGridInfo();
	info.orient = s->getOrient();	// we override the orientation w/ the stamp orientation

	if (needToInvert)
	{
		if (axis == AXISORIENT_X || axis == AXISORIENT_Z)
		{
			if (info.dimen == 2)
			{
				swapItemsInFillList(info.fillList, 0, 1);
				swapItemsInFillList(info.fillList, 2, 3);
			}
			else if (info.dimen == 3)
			{
				swapItemsInFillList(info.fillList, 0, 2);
				swapItemsInFillList(info.fillList, 3, 5);
				swapItemsInFillList(info.fillList, 6, 8);
			}
		}
		else
		{
			if (info.dimen == 2)
			{
				swapItemsInFillList(info.fillList, 0, 2);
				swapItemsInFillList(info.fillList, 1, 3);
			}
			else if (info.dimen == 3)
			{
				swapItemsInFillList(info.fillList, 0, 6);
				swapItemsInFillList(info.fillList, 1, 7);
				swapItemsInFillList(info.fillList, 2, 8);
			}
		}
	}
	return true;
}

// performs stamp collision logic
void StampList::doStampResult(AxisOrient axis, bool wasHit, const GridInfo& newGrid)
{
	Stamp* s = axisToStamp(axis);
	if (s != nullptr)
		s->doCollision(wasHit, newGrid);

	// apply the power-up, if relevent
	CubeUtil::currPowerUp.clear();
	if (wasHit && _powerUps.find(axis) != _powerUps.end())
	{
		CubeUtil::currPowerUp = _powerUps[axis];
		CubeUtil::currPowerUp.startTimer();

		_powerUps[axis].startIconHitAnim();
	}
	else
		_powerUps[axis] = POWERUPTYPE_NONE;
}

// looks for any potentially obsolete stamps and hides them
void StampList::checkForObsoleteStamps(int mapIndex)
{
	// only certain stamp styles can go obsolete
	const Script& script = GameScripts::getCurrScript();
	if (script.isLoaded() && GameScripts::getCurrLevel().stampStyle == STAMPSTYLE_COMPLETES_FACE)
	{
		if (mapIndex == _stampX.getMapIndex())
			_stampX.hideStamp();
		if (mapIndex == _stampY.getMapIndex())
			_stampY.hideStamp();
		if (mapIndex == _stampZ.getMapIndex())
			_stampZ.hideStamp();
	}
}

// clears all displayed stamps
void StampList::clearAllStamps()
{
	_stampX.hideStamp();
	_stampY.hideStamp();
	_stampZ.hideStamp();

	_powerUps[AXISORIENT_X] = POWERUPTYPE_NONE;
	_powerUps[AXISORIENT_Y] = POWERUPTYPE_NONE;
	_powerUps[AXISORIENT_Z] = POWERUPTYPE_NONE;
}
