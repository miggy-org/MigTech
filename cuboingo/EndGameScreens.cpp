#include "pch.h"
#include "CubeUtil.h"
#include "EndGameScreens.h"
#include "SplashScreen.h"
#include "CreditsScreen.h"
#include "GameScripts.h"
#include "../core/PersistBase.h"
#include "../core/Timer.h"

using namespace Cuboingo;
using namespace MigTech;

///////////////////////////////////////////////////////////////////////////
// you lose screen

static const int ID_GAME_OVER_TEXT = 1;

ScreenBase* LoseScreen::getNextScreen()
{
	return new SplashScreen();
}

void LoseScreen::create()
{
	ScreenBase::create();

	// get a pointer to the text display
	_text = (TextButton*) _controls.getControlByID(ID_GAME_OVER_TEXT);
	if (_text != nullptr)
	{
		_textColor = _text->getColor();

		// start an animation for the text
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 2, 3000, AnimItem::ANIM_TYPE_LINEAR_INFINITE);
		_idTextAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

bool LoseScreen::doFrame(int id, float newVal, void* optData)
{
	if (_idTextAnim == id && _text != nullptr)
	{
		newVal -= 2 * ((int) newVal / 2);
		_textColor.r = 0.5f*(newVal > 1 ? 2 - newVal : newVal) + 0.5f;
		_textColor.a = _textColor.r;
		_text->setColor(_textColor);
	}

	return ScreenBase::doFrame(id, newVal, optData);
}

void LoseScreen::onTap(float x, float y)
{
	ScreenBase::onTap(x, y);

	startFadeOut();
}

void LoseScreen::onKey(VIRTUAL_KEY key)
{
	ScreenBase::onKey(key);

	if (key == SPACE || key == ENTER)
		startFadeOut();
}

///////////////////////////////////////////////////////////////////////////
// you win screen

ScreenBase* WinScreen::getNextScreen()
{
	const Script& currScript = GameScripts::getCurrScript();

	// show the credits screen if the script was hard and scored
	if (currScript.difficulty.compare("hard") == 0 && currScript.scoring && MigUtil::thePersist != nullptr)
	{
		// and only if all scored scripts have been beaten
		int nScripts = GameScripts::getScriptHeaderCount();
		for (int i = 0; i < nScripts; i++)
		{
			const Script& script = GameScripts::getScriptHeader(i);
			if (script.scoring)
			{
				// this is a scored script, if the high score is 0 then it hasn't been beaten
				int highScore = MigUtil::thePersist->getValue(script.uniqueID, 0);
				if (highScore == 0)
					return new SplashScreen();
			}
		}

		// all scored scripts have been beaten, show the credits screen
		return new CreditsScreen();
	}
	else if (currScript.difficulty.compare("insane") == 0)
	{
		// experimental script always triggers the credits screen
		return new CreditsScreen();
	}

	// not a hard script, doesn't qualify for the credits screen
	return new SplashScreen();
}

void WinScreen::onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration)
{
	ScreenBase::onOverlayComplete(transType, duration);

	startFadeOut();
}

void WinScreen::onTap(float x, float y)
{
	ScreenBase::onTap(x, y);

	userAction();
}

void WinScreen::onKey(VIRTUAL_KEY key)
{
	LOGINFO("(WinScreen::onKey) Key press (%d) detected", key);

	if (key == ESCAPE || key == SPACE || key == ENTER)
		userAction();
}

void WinScreen::userAction()
{
	if (GameScripts::getCurrScript().scoring)
	{
		// show an overlay to compare to the high score
		startNewOverlay(new HighScoreOverlay(_userScore));
	}
	else
	{
		// not a scored script
		startFadeOut(defFadeDuration);
	}
}

///////////////////////////////////////////////////////////////////////////
// high score overlay

static const int ID_YOUR_SCORE_DISPLAY = 4;
static const int ID_HIGH_SCORE_DISPLAY = 5;
static const int ID_CONGRATS_TEXT = 6;

static const int ANIM_LENGTH = 500;

void HighScoreOverlay::create()
{
	OverlayBase::create();

	_yourScore = (TextButton*)_controls.getControlByID(ID_YOUR_SCORE_DISPLAY);
	_highScore = (TextButton*)_controls.getControlByID(ID_HIGH_SCORE_DISPLAY);
	_congratsText = (TextButton*)_controls.getControlByID(ID_CONGRATS_TEXT);

	_currHighScore = (MigUtil::thePersist != nullptr ? MigUtil::thePersist->getValue(GameScripts::getCurrScript().uniqueID, 0) : 0);
}

bool HighScoreOverlay::doFrame(int id, float newVal, void* optData)
{
	if (_idTallyAnim == id)
	{
		_yourScore->updateText(MigUtil::intToString((int)(newVal * getScore())));
		_highScore->updateText(MigUtil::intToString((int)(newVal * getHighScore())));
	}
	else if (_idHighAnim == id)
	{
		_highScore->updateText(MigUtil::intToString((int)newVal));
	}

	return OverlayBase::doFrame(id, newVal, optData);
}

void HighScoreOverlay::animComplete(int id, void* optData)
{
	if (_idTimer == id)
	{
		// create an animation to make the high score match the user score
		AnimItem animItem(this);
		animItem.configSimpleAnim((float)getHighScore(), (float)getScore(), ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
		_idHighAnim = MigUtil::theAnimList->addItem(animItem);

		// update the high score in the database
		updateHighScore();

		_idTimer = 0;
	}
	else if (_idTallyAnim == id)
	{
		if (getScore() > getHighScore())
		{
			_congratsText->setVisible(true);

			// create another delay timer
			AnimItem animItem(this);
			animItem.configTimer(ANIM_LENGTH, false);
			_idTimer = MigUtil::theAnimList->addItem(animItem);
		}

		// play a sound
		if (MigUtil::theAudio != nullptr)
			MigUtil::theAudio->playMedia(WAV_MATCH, AudioBase::AUDIO_CHANNEL_SOUND);

		_idTallyAnim = 0;
	}
	else if (_idHighAnim == id)
	{
		// play a sound
		if (MigUtil::theAudio != nullptr)
			MigUtil::theAudio->playMedia(WAV_MATCH, AudioBase::AUDIO_CHANNEL_SOUND);

		_idHighAnim = 0;
	}

	OverlayBase::animComplete(id, optData);
}

bool HighScoreOverlay::update()
{
	// flashing congrats string
	if (_congratsText->isVisible())
	{
		// the congrats string fades in and out
		float alpha = (float)(Timer::gameTimeMillis() % 2000);
		alpha = (alpha > 1000 ? (2000 - alpha) : alpha) / 1000.0f;
		_congratsText->setAlpha(alpha);
	}

	return OverlayBase::update();
}

void HighScoreOverlay::onTap(float x, float y)
{
	OverlayBase::onTap(x, y);

	userAction();
}

void HighScoreOverlay::onKey(VIRTUAL_KEY key)
{
	LOGINFO("(HighScoreOverlay::onKey) Key press (%d) detected", key);

	if (key == ESCAPE || key == SPACE || key == ENTER)
		userAction();
}

void HighScoreOverlay::onAnimComplete(OVERLAY_TRANSITION_TYPE transType)
{
	OverlayBase::onAnimComplete(transType);

	if (transType == OVERLAY_TRANSITION_INTRO)
	{
		// start the tally animation
		AnimItem animItem(this);
		animItem.configSimpleAnim(0, 1, ANIM_LENGTH, AnimItem::ANIM_TYPE_LINEAR);
		_idTallyAnim = MigUtil::theAnimList->addItem(animItem);
	}
}

// updates the high score w/ the user score, if necessary
void HighScoreOverlay::updateHighScore() const
{
	// put the new high score in persistent storage, if necessary
	if (getScore() > getHighScore() && MigUtil::thePersist != nullptr)
		MigUtil::thePersist->putValue(GameScripts::getCurrScript().uniqueID, getScore());
}

void HighScoreOverlay::userAction()
{
	// make sure the high score has been updated
	if (!_idHighAnim.isActive())
		updateHighScore();

	// clear the congrats string
	_congratsText->setVisible(false);

	// create an animation to scale/fade out
	startExitAnimation();
}
