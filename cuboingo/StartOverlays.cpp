#include "pch.h"
#include "CubeUtil.h"
#include "GameScripts.h"
#include "StartOverlays.h"
#include "../core/PerfMon.h"
#include "../core/Timer.h"

using namespace Cuboingo;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// main start screen overlay

const int ID_PLAY_BUTTON = 1;
const int ID_OPTIONS_BUTTON = 2;
const int ID_ABOUT_BUTTON = 3;

void StartOverlay::onKey(VIRTUAL_KEY key)
{
	OverlayBase::onKey(key);

	if (key == SPACE || key == ENTER)
	{
		_nextOverlay = new ScriptOverlay();
		startInterAnimation(OVERLAY_TRANSITION_FADE);
	}
}

bool StartOverlay::onBackKey()
{
	startExitAnimation();
	return true;
}

void StartOverlay::onClick(int id, ControlBase* sender)
{
	if (id == ID_PLAY_BUTTON)
	{
		_nextOverlay = new ScriptOverlay();
		startInterAnimation(OVERLAY_TRANSITION_FADE);
	}
	else if (id == ID_OPTIONS_BUTTON)
	{
		_nextOverlay = new OptionsOverlay();
		startInterAnimation(OVERLAY_TRANSITION_FADE);
	}
	else if (id == ID_ABOUT_BUTTON)
	{
		_nextOverlay = new AboutOverlay();
		startInterAnimation(OVERLAY_TRANSITION_FADE);
	}
	else
		OverlayBase::onClick(id, sender);
}

///////////////////////////////////////////////////////////////////////////
// script screen overlay

const int ID_PREV_BUTTON = 1;
const int ID_NEXT_BUTTON = 2;

const int ANIM_LENGTH = 500;
const int ANIM_TEXT_LENGTH = 1000;

ScriptItem::ScriptItem()
{
}

void ScriptItem::init(const Script& script)
{
	theScript = script;

	nameUpper.init(theScript.name, 0.1f, 0.25f, 0.04f, JUSTIFY_LEFT);
	diffUpper.init(theScript.difficulty, 0.75f, 0.25f, 0.04f, JUSTIFY_LEFT);
	desc1Upper.init(theScript.desc1, 0.1f, 0.5f, 0.03f, JUSTIFY_LEFT);
	desc2Upper.init(theScript.desc2, 0.1f, 0.6f, 0.03f, JUSTIFY_LEFT);
	desc3Upper.init(theScript.desc3, 0.1f, 0.7f, 0.03f, JUSTIFY_LEFT);
	desc4Upper.init(theScript.desc4, 0.1f, 0.8f, 0.03f, JUSTIFY_LEFT);

	if (theScript.scoring && MigUtil::thePersist != nullptr)
		highStr = MigUtil::getString("high") + " : " + MigUtil::intToString(MigUtil::thePersist->getValue(script.uniqueID, 0));
	else
		highStr = MigUtil::getString("high") + " : N/A";
	highUpper.init(highStr, 0.1f, 0.35f, 0.04f, JUSTIFY_LEFT);

	updateTextLength(0);
}

int ScriptItem::getMaxTextLength() const
{
	auto len = theScript.name.length();
	len = max(len, theScript.desc1.length());
	len = max(len, theScript.desc2.length());
	len = max(len, theScript.desc3.length());
	len = max(len, theScript.desc4.length());
	len = max(len, highStr.length());
	return len;
}

void ScriptItem::updateTextLength(int len)
{
	nameUpper.update(CubeUtil::getUpperSubString(theScript.name, len));
	diffUpper.update(CubeUtil::getUpperSubString(theScript.difficulty, len));
	desc1Upper.update(CubeUtil::getUpperSubString(theScript.desc1, len));
	desc2Upper.update(CubeUtil::getUpperSubString(theScript.desc2, len));
	desc3Upper.update(CubeUtil::getUpperSubString(theScript.desc3, len));
	desc4Upper.update(CubeUtil::getUpperSubString(theScript.desc4, len));
	highUpper.update(CubeUtil::getUpperSubString(highStr, len));
}

void ScriptItem::draw(float alpha, const Matrix& worldMatrix)
{
	nameUpper.draw(1, 1, 1, alpha, worldMatrix);
	diffUpper.draw(1, 1, 1, alpha, worldMatrix);
	desc1Upper.draw(1, 1, 1, alpha, worldMatrix);
	desc2Upper.draw(1, 1, 1, alpha, worldMatrix);
	desc3Upper.draw(1, 1, 1, alpha, worldMatrix);
	desc4Upper.draw(1, 1, 1, alpha, worldMatrix);
	highUpper.draw(1, 1, 1, alpha, worldMatrix);
}

void ScriptOverlay::create()
{
	OverlayBase::create();

	_prevBtn = (Button*)_controls.getControlByID(ID_PREV_BUTTON);
	_nextBtn = (Button*)_controls.getControlByID(ID_NEXT_BUTTON);

	// create the items that will draw the script text
	int nItems = GameScripts::getScriptHeaderCount();
	_items.resize(nItems);
	for (int i = 0; i < nItems; i++)
		_items[i].init(GameScripts::getScriptHeader(i));

	// load the last selected script index
	_currItem = (MigUtil::thePersist != nullptr ? MigUtil::thePersist->getValue(KEY_LAST_USED_SCRIPT_INDEX, 0) : 0);
	_otherItem = -1;

	// init button visibility based upon the item index
	updateBtnVisibility();

	// and create an animation to show the text char by char
	startTextLengthAnim();
}

bool ScriptOverlay::doFrame(int id, float newVal, void* optData)
{
	if (_idTransAnim == id)
		_transX = newVal;
	else if (_idLengthAnim == id)
		_items[_currItem].updateTextLength((int)newVal);

	return OverlayBase::doFrame(id, newVal, optData);
}

void ScriptOverlay::animComplete(int id, void* optData)
{
	if (_idTransAnim == id)
	{
		_idTransAnim = 0;
		_transX = 0;
		_otherItem = -1;

		updateBtnVisibility();
	}
	else if (_idLengthAnim == id)
		_idLengthAnim = 0;

	OverlayBase::animComplete(id, optData);
}

bool ScriptOverlay::update()
{
	OverlayBase::update();

	long clamped = Timer::gameTimeMillis() % 2000;
	float alpha = (clamped > 1000 ? (2000 - clamped) : clamped) / 1000.0f;
	if (_prevBtn)
		_prevBtn->setAlpha(alpha);
	if (_nextBtn)
		_nextBtn->setAlpha(alpha);
	return true;
}

void ScriptOverlay::draw(float alpha, const Matrix& mat)
{
	static Matrix localMat;
	localMat.identity();
	if (_transX != 0)
		localMat.translate(_transX, 0, 0);
	localMat.multiply(mat);

	if (_otherItem != -1)
	{
		// draw the item moving away
		alpha = (float)((2 - fabs(_transX)) / 2);
		_items[_otherItem].draw(alpha, localMat);

		// and then draw the item moving in
		int dir = (_transX < 0 ? 1 : -1);
		localMat.translate(2.0f * dir, 0, 0);
		alpha = (float)(fabs(_transX) / 2.0);
		_items[_currItem].draw(alpha, localMat);
	}
	else
		_items[_currItem].draw(alpha, localMat);
}

void ScriptOverlay::onTap(float x, float y)
{
	LOGINFO("(ScriptOverlay::onTap) Tap at (%f, %f) detected", x, y);

	if (!_controls.onTap(x, y) && _idTransAnim == 0)
		pickSelectedScript();
}

void ScriptOverlay::onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe)
{
	OverlayBase::onSwipe(x, y, dx, dy, swipe);

	// we only care about horizontal swipes here
	if (swipe == SWIPE_HORIZONTAL)
	{
		int dir = (dx > 0 ? 1 : -1);
		doSwipe(dir);
	}
}

void ScriptOverlay::onClick(int id, ControlBase* sender)
{
	OverlayBase::onClick(id, sender);

	if (id == ID_NEXT_BUTTON)
		onClickRight();
	else if (id == ID_PREV_BUTTON)
		onClickLeft();
}

bool ScriptOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	_nextOverlay = new StartOverlay();
	startInterAnimation(OVERLAY_TRANSITION_FADE);
	return true;
}

void ScriptOverlay::onKey(VIRTUAL_KEY key)
{
	OverlayBase::onKey(key);

	if (key == LEFT || key == NUMPAD4)
		doSwipe(1);
	else if (key == RIGHT || key == NUMPAD6)
		doSwipe(-1);
	else if (key == ENTER || key == SPACE)
		pickSelectedScript();
}

void ScriptOverlay::startTextLengthAnim()
{
	int len = _items[_currItem].getMaxTextLength();

	AnimItem animItem(this);
	animItem.configSimpleAnim(1.0f, (float) len, ANIM_TEXT_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
	_idLengthAnim = MigUtil::theAnimList->addItem(animItem);
}

void ScriptOverlay::updateBtnVisibility()
{
	if (_prevBtn)
		_prevBtn->setVisible(_currItem > 0);
	if (_nextBtn)
		_nextBtn->setVisible(_currItem < (int) _items.size() - 1);
}

void ScriptOverlay::pickSelectedScript()
{
	// load the selected script
	GameScripts::loadGameScript(_items[_currItem].theScript.scriptID);

	// save the selected script index
	if (MigUtil::thePersist != nullptr)
	{
		if (!MigUtil::thePersist->putValue(KEY_LAST_USED_SCRIPT_INDEX, _currItem))
			LOGWARN("(ScriptOverlay::pickSelectedScript) Unable to persist last used script index");
		MigUtil::thePersist->commit();
	}

	// create an animation to scale/fade out
	startExitAnimation(ANIM_LENGTH);
}

void ScriptOverlay::onClickLeft()
{
	doSwipe(1);
}

void ScriptOverlay::onClickRight()
{
	doSwipe(-1);
}

void ScriptOverlay::doSwipe(int dir)
{
	// cancel any existing animations when doing a new swipe
	_idTransAnim.clearAnim();
	_otherItem = -1;

	if (dir == 1 && _currItem == 0)
	{
		// tried to swipe right but we can't
		float ft[] = { 0.0f, 0.34f, 0.64f, 0.87f, 0.98f, 0.87f, 0.64f, 0.34f, 0.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, 2 / 3.0f, ANIM_LENGTH, ft, ARRAYSIZE(ft));
		_idTransAnim = MigUtil::theAnimList->addItem(animItem);
	}
	else if (dir == -1 && _currItem == _items.size() - 1)
	{
		// tried to swipe left but we can't
		float ft[] = { 0.0f, 0.34f, 0.64f, 0.87f, 0.98f, 0.87f, 0.64f, 0.34f, 0.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, -2 / 3.0f, ANIM_LENGTH, ft, ARRAYSIZE(ft));
		_idTransAnim = MigUtil::theAnimList->addItem(animItem);
	}
	else
	{
		// good swipe
		_otherItem = _currItem;
		_currItem -= dir;

		// current item is changing so we must cancel the text length animation
		_idLengthAnim.clearAnim();

		// start the swipe animation
		float ft[] = { 0.0f, 0.17f, 0.34f, 0.5f, 0.64f, 0.77f, 0.87f, 0.94f, 0.98f, 1.0f };
		AnimItem animItem(this);
		animItem.configParametricAnim(0, dir * 2.0f, ANIM_LENGTH, ft, ARRAYSIZE(ft));
		_idTransAnim = MigUtil::theAnimList->addItem(animItem);

		// create an animation to show the text char by char
		startTextLengthAnim();
	}

	if (_idTransAnim.isActive())
	{
		if (_prevBtn)
			_prevBtn->setVisible(false);
		if (_nextBtn)
			_nextBtn->setVisible(false);
	}
}

///////////////////////////////////////////////////////////////////////////
// options screen overlay

const int ID_MUSIC_VOLUME_CONTROL = 1;
const int ID_SOUND_VOLUME_CONTROL = 2;
const int ID_MUSIC_VOLUME_DISPLAY = 3;
const int ID_SOUND_VOLUME_DISPLAY = 4;
const int ID_REFLECTIONS_CONTROL = 5;
const int ID_SHADOWS_CONTROL = 6;
const int ID_PERFMON_CONTROL = 7;

#pragma warning(push)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS

static const char* toPercent(float value)
{
	static char buf[8];
	sprintf(buf, "%d%%", (int) (100*value));
	return buf;
}

#pragma warning(pop)

void OptionsOverlay::create()
{
	OverlayBase::create();

	ControlBase* ctrl = _controls.getControlByID(ID_MUSIC_VOLUME_CONTROL);
	if (ctrl != nullptr && ctrl->getType() == SLIDER_BUTTON_CONTROL)
	{
		if (MigUtil::theAudio != nullptr)
			((SliderButton*)ctrl)->setSliderValue(MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_MUSIC));
		else
			((SliderButton*)ctrl)->setEnabled(false);
	}

	ctrl = _controls.getControlByID(ID_SOUND_VOLUME_CONTROL);
	if (ctrl != nullptr && ctrl->getType() == SLIDER_BUTTON_CONTROL)
	{
		if (MigUtil::theAudio != nullptr)
			((SliderButton*)ctrl)->setSliderValue(MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_SOUND));
		else
			((SliderButton*)ctrl)->setEnabled(false);
	}

	ctrl = _controls.getControlByID(ID_MUSIC_VOLUME_DISPLAY);
	if (ctrl != nullptr && ctrl->getType() == TEXT_BUTTON_CONTROL)
	{
		if (MigUtil::theAudio != nullptr)
			((TextButton*)ctrl)->updateText(toPercent(MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_MUSIC)));
		else
			((TextButton*)ctrl)->setEnabled(false);
	}

	ctrl = _controls.getControlByID(ID_SOUND_VOLUME_DISPLAY);
	if (ctrl != nullptr && ctrl->getType() == TEXT_BUTTON_CONTROL)
	{
		if (MigUtil::theAudio != nullptr)
			((TextButton*)ctrl)->updateText(toPercent(MigUtil::theAudio->getChannelVolume(AudioBase::AUDIO_CHANNEL_SOUND)));
		else
			((TextButton*)ctrl)->setEnabled(false);
	}

	ctrl = _controls.getControlByID(ID_REFLECTIONS_CONTROL);
	if (ctrl != nullptr && ctrl->getType() == CHECKBOX_BUTTON_CONTROL)
		((CheckBoxButton*)ctrl)->setChecked(CubeUtil::useReflections);

	ctrl = _controls.getControlByID(ID_SHADOWS_CONTROL);
	if (ctrl != nullptr && ctrl->getType() == CHECKBOX_BUTTON_CONTROL)
		((CheckBoxButton*)ctrl)->setChecked(CubeUtil::useShadows);

	ctrl = _controls.getControlByID(ID_PERFMON_CONTROL);
	if (ctrl != nullptr && ctrl->getType() == CHECKBOX_BUTTON_CONTROL)
		((CheckBoxButton*)ctrl)->setChecked(PerfMon::isFPSOn());
}

bool OptionsOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	CubeUtil::savePersistentSettings();

	if (_sample != nullptr)
		MigUtil::theAudio->deleteMedia(_sample);

	_nextOverlay = new StartOverlay();
	startInterAnimation(OVERLAY_TRANSITION_FADE);
	return true;
}

void OptionsOverlay::onClick(int id, ControlBase* sender)
{
	OverlayBase::onClick(id, sender);

	if (id == ID_MUSIC_VOLUME_CONTROL)
	{
		if (sender->getType() == SLIDER_BUTTON_CONTROL && MigUtil::theAudio != nullptr)
			MigUtil::theAudio->setChannelVolume(AudioBase::AUDIO_CHANNEL_MUSIC, ((SliderButton*)sender)->getSliderValue());
	}
	else if (id == ID_SOUND_VOLUME_CONTROL)
	{
		float volume = 1;
		if (sender->getType() == SLIDER_BUTTON_CONTROL)
			volume = ((SliderButton*)sender)->getSliderValue();
		if (MigUtil::theAudio != nullptr)
			MigUtil::theAudio->setChannelVolume(AudioBase::AUDIO_CHANNEL_SOUND, volume);

		if (_sample == nullptr && MigUtil::theAudio != nullptr)
			_sample = MigUtil::theAudio->loadMedia(WAV_FILL1, AudioBase::AUDIO_CHANNEL_SOUND);
		if (_sample != nullptr)
		{
			_sample->setVolume(volume);
			_sample->playSound(false);
		}
	}
	else if (id == ID_REFLECTIONS_CONTROL)
	{
		if (sender->getType() == CHECKBOX_BUTTON_CONTROL)
			CubeUtil::useReflections = ((CheckBoxButton*)sender)->isChecked();
	}
	else if (id == ID_SHADOWS_CONTROL)
	{
		if (sender->getType() == CHECKBOX_BUTTON_CONTROL)
			CubeUtil::useShadows = ((CheckBoxButton*)sender)->isChecked();
	}
	else if (id == ID_PERFMON_CONTROL)
	{
		if (sender->getType() == CHECKBOX_BUTTON_CONTROL)
			PerfMon::showFPS(((CheckBoxButton*)sender)->isChecked());
	}
}

void OptionsOverlay::onSlide(int id, ControlBase* sender, float val)
{
	OverlayBase::onSlide(id, sender, val);

	if (id == ID_MUSIC_VOLUME_CONTROL)
	{
		ControlBase* ctrl = _controls.getControlByID(ID_MUSIC_VOLUME_DISPLAY);
		if (ctrl != nullptr && ctrl->getType() == TEXT_BUTTON_CONTROL)
			((TextButton*)ctrl)->updateText(toPercent(val));
		if (_callback != nullptr)
		{
			static float data;
			data = val;
			_callback->onOverlayCustom(1, (void*)&data);
		}
	}
	else if (id == ID_SOUND_VOLUME_CONTROL)
	{
		ControlBase* ctrl = _controls.getControlByID(ID_SOUND_VOLUME_DISPLAY);
		if (ctrl != nullptr && ctrl->getType() == TEXT_BUTTON_CONTROL)
			((TextButton*)ctrl)->updateText(toPercent(val));
	}
}

///////////////////////////////////////////////////////////////////////////
// about screen overlay

const int ID_DUMP_CONTROL  = 1;
const int ID_DEBUG_CONTROL = 2;

bool AboutOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	_nextOverlay = new StartOverlay();
	startInterAnimation(OVERLAY_TRANSITION_FADE);
	return true;
}

void AboutOverlay::onClick(int id, ControlBase* sender)
{
	OverlayBase::onClick(id, sender);

	static long lastClickTime = 0;
	if (id == ID_DUMP_CONTROL)
	{
		long clickTime = Timer::gameTimeMillis();
		if (clickTime - lastClickTime < 500)
			SimpleDialog::ShowDialog(this, "DUMP LOGS?", "YES", "NO");
		lastClickTime = clickTime;
	}
	else if (id == ID_DEBUG_CONTROL)
	{
		long clickTime = Timer::gameTimeMillis();
		if (clickTime - lastClickTime < 500 && MigUtil::dumpLogFileExists())
		{
			_nextOverlay = new DebugOverlay();
			startInterAnimation(OVERLAY_TRANSITION_FADE);
		}
		lastClickTime = clickTime;
	}
	else if (id == SimpleDialog::ID_DIALOG_BTN1)
	{
		MigUtil::dumpLogToFile("(AboutOverlay::onClick) Manual log dump requested");
	}
}

///////////////////////////////////////////////////////////////////////////
// debug screen overlay

const int ID_VIEW_BUTTON = 1;
const int ID_DUMP_BUTTON = 2;
const int ID_SDCARD_BUTTON = 3;
const int ID_DELETE_BUTTON = 4;
const int ID_STATUS_TEXT = 5;

bool DebugOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	_nextOverlay = new AboutOverlay();
	startInterAnimation(OVERLAY_TRANSITION_FADE);
	return true;
}

void DebugOverlay::onClick(int id, ControlBase* sender)
{
	OverlayBase::onClick(id, sender);

	if (id == ID_VIEW_BUTTON)
	{
		_nextOverlay = new DumpLogOverlay();
		startInterAnimation(OVERLAY_TRANSITION_FADE);
	}
	else if (id == ID_DUMP_BUTTON)
	{
		bool ok = MigUtil::dumpLogFileToDebugger();
		ControlBase* statusText = _controls.getControlByID(ID_STATUS_TEXT);
		if (statusText != nullptr && statusText->getType() == TEXT_BUTTON_CONTROL)
			((TextButton*)statusText)->updateText((ok ? "DUMP SUCCEEDED" : "DUMP FAILED"), JUSTIFY_CENTER);
	}
	else if (id == ID_SDCARD_BUTTON)
	{
		bool ok = MigUtil::dumpLogFileToExternalStorage();
		ControlBase* statusText = _controls.getControlByID(ID_STATUS_TEXT);
		if (statusText != nullptr && statusText->getType() == TEXT_BUTTON_CONTROL)
			((TextButton*)statusText)->updateText((ok ? "COPY SUCCEEDED" : "COPY FAILED"), JUSTIFY_CENTER);
	}
	else if (id == ID_DELETE_BUTTON)
	{
		bool ok = MigUtil::deleteLogFile();
		if (ok)
			onBackKey();
		else
		{
			ControlBase* statusText = _controls.getControlByID(ID_STATUS_TEXT);
			if (statusText != nullptr && statusText->getType() == TEXT_BUTTON_CONTROL)
				((TextButton*)statusText)->updateText("DELETE FAILED");
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// dump log screen overlay

static const int NUM_STATIC_LINES = 14;

static void setLineOfText(const Controls& controls, int line, const std::string& text)
{
	ControlBase* pstatic = controls.getControlByID(line);
	if (pstatic != nullptr && pstatic->getType() == TEXT_BUTTON_CONTROL)
		((TextButton*)pstatic)->updateText(text);
}

void DumpLogOverlay::create()
{
	OverlayBase::create();

	std::vector<std::string> logLines;
	if (MigUtil::dumpLogFileToStrings(logLines))
	{
		int numLines = logLines.size();
		int numLinesToDump = min(numLines, NUM_STATIC_LINES);
		for (int i = 0; i < numLinesToDump; i++)
		{
			const std::string& line = logLines[numLines - (numLinesToDump - i)];
			setLineOfText(_controls, i + 1, MigUtil::toUpper(line));
		}
	}
	else
		setLineOfText(_controls, 2, "COULD NOT GET LOG FILE");
}

bool DumpLogOverlay::onBackKey()
{
	OverlayBase::onBackKey();

	_nextOverlay = new DebugOverlay();
	startInterAnimation(OVERLAY_TRANSITION_FADE);
	return true;
}
