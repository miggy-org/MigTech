#include "pch.h"
#include "CubeUtil.h"
#include "GameOverlays.h"
#include "GameScripts.h"
#include "../core/Timer.h"

using namespace Cuboingo;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// pause overlay

const int ID_GLOW_TEXT = 1;
const int ID_PLAY_BUTTON = 2;
const int ID_QUIT_BUTTON = 3;

void PauseOverlay::create()
{
	OverlayBase::create();
	_glowText = (Button*)_controls.getControlByID(ID_GLOW_TEXT);
}

bool PauseOverlay::update()
{
	if (_glowText != nullptr)
	{
		long millis = Timer::ticksToMilliSeconds(Timer::systemTime()) % 2000;
		millis = (millis > 1000 ? (2000 - millis) : millis);
		_glowText->setAlpha(millis / 1000.0f);
	}

	return OverlayBase::update();
}

void PauseOverlay::onKey(VIRTUAL_KEY key)
{
	OverlayBase::onKey(key);

	if (key == SPACE || key == ENTER)
		onBackKey();
}

bool PauseOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	startExitAnimation(0);
	return true;
}

void PauseOverlay::onClick(int id, ControlBase* sender)
{
	onBackKey();

	if (id == ID_QUIT_BUTTON)
		_callback->onOverlayCustom(1, nullptr);
}

///////////////////////////////////////////////////////////////////////////
// tips display overlay

void TipsOverlay::create()
{
	const Script& script = GameScripts::getCurrScript();
	if (!script.isLoaded())
		throw std::runtime_error("(TipsOverlay::create) Cannot initialize a TipOverlay w/o a script");

	std::vector<std::string>::const_iterator iter = script.tips.begin();
	if (iter == script.tips.end())
		throw std::runtime_error("(TipsOverlay::create) Cannot initialize a TipOVerlay w/ no tips in the script");

	int numTips = script.tips.size();
	for (int i = 0; iter != script.tips.end(); i++)
	{
		TextButton* item = new TextButton(0, false);
		item->init(MigUtil::theFont, (*iter), 0.5f, (i + 1) * (1.0f / (numTips + 1)), 0.04f, 1.0f, JUSTIFY_CENTER);
		_controls.addControl(item);

		iter++;
	}

	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->playMedia(WAV_PAUSE, AudioBase::AUDIO_CHANNEL_SOUND);
}

void TipsOverlay::onTap(float x, float y)
{
	OverlayBase::onTap(x, y);

	startExitAnimation();
}

void TipsOverlay::onKey(VIRTUAL_KEY key)
{
	OverlayBase::onKey(key);

	if (key == SPACE || key == ENTER)
		onBackKey();
}

bool TipsOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	startExitAnimation();
	return true;
}

///////////////////////////////////////////////////////////////////////////
// end of level score summary overlay

static const int ANIM_DUR_COUNT_SCORE = 1000;
static const int ANIM_DUR_PAUSE = 500;

static const int ID_TIMED_BONUS_HEADER = 1;
static const int ID_NO_MISS_HEADER = 2;
static const int ID_ALL_STAMPS_HEADER = 3;
static const int ID_TIMED_BONUS_TEXT = 4;
static const int ID_NO_MISS_TEXT = 5;
static const int ID_ALL_STAMPS_TEXT = 6;
static const int ID_TAP_TEXT = 7;

SummaryOverlay::SummaryOverlay(ScoreKeeper& sk) : OverlayBase("SummaryOverlay"),
	_scoreKeeper(sk),
	_timedBonusReal(0), _noMissBonusReal(0), _allStampsHitBonusReal(0),
	_timedBonusDisplay(nullptr), _noMissBonusDisplay(nullptr), _allStampsHitBonusDisplay(nullptr),
	_timedLabel(nullptr), _noMissLabel(nullptr), _allStampsLabel(nullptr),
	_tapLabel(nullptr), _match(nullptr)
{
}

static bool isStampBonusAvailable()
{
	return (GameScripts::getCurrLevel().stampCount > 0 && GameScripts::getCurrLevel().stampStyle != STAMPSTYLE_BONUS_ALWAYS);
}

void SummaryOverlay::create()
{
	OverlayBase::create();

	const Script& script = GameScripts::getCurrScript();
	if (!script.isLoaded())
		throw std::runtime_error("(SummaryOverlay::create) Cannot initialize a TipOverlay w/o a script");
	const ScoreConfig& scoreCfg = GameScripts::getCurrLevel().scoreCfg;

	// load match sound
	if (MigUtil::theAudio != nullptr)
		_match = MigUtil::theAudio->loadMedia(WAV_MATCH, AudioBase::AUDIO_CHANNEL_SOUND);

	// load controls
	_timedLabel = (TextButton*)_controls.getControlByID(ID_TIMED_BONUS_HEADER);
	_noMissLabel = (TextButton*)_controls.getControlByID(ID_NO_MISS_HEADER);
	_allStampsLabel = (TextButton*)_controls.getControlByID(ID_ALL_STAMPS_HEADER);
	_timedBonusDisplay = (TextButton*)_controls.getControlByID(ID_TIMED_BONUS_TEXT);
	_noMissBonusDisplay = (TextButton*)_controls.getControlByID(ID_NO_MISS_TEXT);
	_allStampsHitBonusDisplay = (TextButton*)_controls.getControlByID(ID_ALL_STAMPS_TEXT);
	_tapLabel = (TextButton*)_controls.getControlByID(ID_TAP_TEXT);

	int nItems = 0;

	int timedBonusPerSecond = scoreCfg.timedBonusPerSecond;
	if (timedBonusPerSecond > 0)
	{
		_timedBonusReal = timedBonusPerSecond * _scoreKeeper.getRemainingTime() / 1000;
		nItems++;
	}

	int noMissBonus = scoreCfg.noMissBonus;
	if (noMissBonus > 0)
	{
		_noMissBonusReal = (_scoreKeeper.getLevelMissCount() == 0 ? noMissBonus : 0);
		nItems++;
	}

	int allStampsHitBonus = scoreCfg.allStampsHitBonus;
	if (allStampsHitBonus > 0 && isStampBonusAvailable())
	{
		_allStampsHitBonusReal = (_scoreKeeper.getStampMissCount() == 0 ? allStampsHitBonus : 0);
		nItems++;
	}
	if (nItems == 0)
		throw std::runtime_error("(SummaryOverlay::create) Cannot initialize SummaryOverlay w/ no items to display");

	float vPositions[] = { 0.2f, 0.4f, 0.6f, 0.25f, 0.55f, 0.4f };
	int vIndex = (nItems == 3 ? 0 : (nItems == 2 ? 3 : 5));

	// load resource strings
	if (timedBonusPerSecond > 0)
	{
		_timedLabel->updatePos(0.1f, vPositions[vIndex], 0.04f, JUSTIFY_LEFT);
		_timedBonusDisplay->updatePos(0.7f, vPositions[vIndex++], 0.05f, JUSTIFY_LEFT);
		_timedLabel->setVisible(true);
		_timedBonusDisplay->setVisible(true);
	}
	if (noMissBonus > 0)
	{
		_noMissLabel->updatePos(0.1f, vPositions[vIndex], 0.04f, JUSTIFY_LEFT);
		_noMissBonusDisplay->updatePos(0.7f, vPositions[vIndex++], 0.05f, JUSTIFY_LEFT);
		_noMissLabel->setVisible(true);
		_noMissBonusDisplay->setVisible(true);
	}
	if (allStampsHitBonus > 0 && isStampBonusAvailable())
	{
		_allStampsLabel->updatePos(0.1f, vPositions[vIndex], 0.04f, JUSTIFY_LEFT);
		_allStampsHitBonusDisplay->updatePos(0.7f, vPositions[vIndex++], 0.05f, JUSTIFY_LEFT);
		_allStampsLabel->setVisible(true);
		_allStampsHitBonusDisplay->setVisible(true);
	}

	// start a pause
	startPauseTimer();
}

void SummaryOverlay::destroy()
{
	if (MigUtil::theAudio != nullptr && _match != nullptr)
		MigUtil::theAudio->deleteMedia(_match);

	OverlayBase::destroy();
}

bool SummaryOverlay::update()
{
	// tap to continue on the bottom
	if (_tapLabel != nullptr)
	{
		long newAlpha = Timer::gameTimeMillis() % 2000;
		float alpha = (newAlpha > 1000 ? (2000 - newAlpha) : newAlpha) / 1000.0f;
		_tapLabel->setAlpha(alpha);
	}

	return OverlayBase::update();
}

bool SummaryOverlay::doFrame(int id, float newVal, void* optData)
{
	if (_idTimedBonusAnim == id)
		_timedBonusDisplay->updateText(MigUtil::intToString((int)newVal));
	else if (_idNoMissBonusAnim == id)
		_noMissBonusDisplay->updateText(MigUtil::intToString((int)newVal));
	else if (_idAllStampsHitBonusAnim == id)
		_allStampsHitBonusDisplay->updateText(MigUtil::intToString((int)newVal));

	return OverlayBase::doFrame(id, newVal, optData);
}

void SummaryOverlay::animComplete(int id, void* optData)
{
	if (_idTimedBonusAnim == id || _idNoMissBonusAnim == id || _idAllStampsHitBonusAnim == id)
	{
		_idTimedBonusAnim = 0;
		_idNoMissBonusAnim = 0;
		_idAllStampsHitBonusAnim = 0;

		// start a pause
		startPauseTimer();

		// and play a sound
		if (_match != nullptr)
			_match->playSound(false);
	}
	else if (_idPauseTimer == id)
	{
		_idPauseTimer = 0;

		// start the next bonus animation, if any
		startNextBonusAnimation();
	}

	OverlayBase::animComplete(id, optData);
}

void SummaryOverlay::onAnimComplete(OVERLAY_TRANSITION_TYPE transType)
{
	if (_tapLabel != nullptr)
		_tapLabel->setVisible(true);

	OverlayBase::onAnimComplete(transType);
}

void SummaryOverlay::onTap(float x, float y)
{
	OverlayBase::onTap(x, y);

	userAction();
}

void SummaryOverlay::onKey(VIRTUAL_KEY key)
{
	OverlayBase::onKey(key);

	if (key == SPACE || key == ENTER)
		userAction();
}

bool SummaryOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	userAction();
	return true;
}

void SummaryOverlay::startNextBonusAnimation()
{
	if (_timedBonusReal > 0 && !_timedBonusDisplay->getText().compare("0"))
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, (float)_timedBonusReal, ANIM_DUR_COUNT_SCORE, AnimItem::ANIM_TYPE_LINEAR);
		_idTimedBonusAnim = MigUtil::theAnimList->addItem(animItem);
	}
	else if (_noMissBonusReal > 0 && !_noMissBonusDisplay->getText().compare("0"))
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, (float)_noMissBonusReal, ANIM_DUR_COUNT_SCORE, AnimItem::ANIM_TYPE_LINEAR);
		_idNoMissBonusAnim = MigUtil::theAnimList->addItem(animItem);
	}
	else if (_allStampsHitBonusReal > 0 && !_allStampsHitBonusDisplay->getText().compare("0"))
	{
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, (float)_allStampsHitBonusReal, ANIM_DUR_COUNT_SCORE, AnimItem::ANIM_TYPE_LINEAR);
		_idAllStampsHitBonusAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

void SummaryOverlay::startPauseTimer()
{
	AnimItem animItem(this);
	animItem.configTimer(ANIM_DUR_PAUSE, false);
	_idPauseTimer = MigUtil::theAnimList->addItem(animItem);
}

bool SummaryOverlay::cancelAllAnimations()
{
	bool cancelled = _idTimedBonusAnim.clearAnim();
	cancelled |= _idNoMissBonusAnim.clearAnim();
	cancelled |= _idAllStampsHitBonusAnim.clearAnim();
	cancelled |= _idPauseTimer.clearAnim();
	return cancelled;
}

void SummaryOverlay::userAction()
{
	if (cancelAllAnimations())
	{
		// this tap will cancel any animations and just display results
		if (_timedBonusDisplay != nullptr)
			_timedBonusDisplay->updateText(MigUtil::intToString(_timedBonusReal));
		if (_noMissBonusDisplay != nullptr)
			_noMissBonusDisplay->updateText(MigUtil::intToString(_noMissBonusReal));
		if (_allStampsHitBonusDisplay != nullptr)
			_allStampsHitBonusDisplay->updateText(MigUtil::intToString(_allStampsHitBonusReal));

		// and play a sound
		if (_match != nullptr)
			_match->playSound(false);
	}
	else
	{
		// add the total bonus to the real score display
		int total = _timedBonusReal + _noMissBonusReal + _allStampsHitBonusReal;
		_scoreKeeper.addToScore(total);

		// clear the tap text
		if (_tapLabel != nullptr)
			_tapLabel->setVisible(false);

		// quit the overlay
		startExitAnimation();
	}
}
