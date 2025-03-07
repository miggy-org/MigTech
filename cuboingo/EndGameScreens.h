#pragma once

#include "../core/MigInclude.h"
#include "../core/ScreenBase.h"
#include "../core/MigUtil.h"

using namespace MigTech;

namespace Cuboingo
{
	class LoseScreen : public MigTech::ScreenBase
	{
	public:
		LoseScreen() : ScreenBase("LoseScreen"), _text(nullptr) { }

		virtual ScreenBase* getNextScreen();

		virtual void create();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);

	protected:
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);

	protected:
		TextButton* _text;
		Color _textColor;
		AnimID _idTextAnim;
	};

	class WinScreen : public MigTech::ScreenBase
	{
	public:
		WinScreen(int userScore) : ScreenBase("WinScreen") { _userScore = userScore; }

		virtual ScreenBase* getNextScreen();

		virtual void onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration);

	protected:
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);

		void userAction();

	protected:
		int _userScore;
	};

	class HighScoreOverlay : public MigTech::OverlayBase
	{
	public:
		HighScoreOverlay(int userScore) : OverlayBase("HighScoreOverlay"),
			_congratsText(nullptr), _yourScore(nullptr), _highScore(nullptr),
			_currHighScore(0), _userScore(userScore) {}

		// life cycle events
		virtual void create();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// frame events
		virtual bool update();

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);

		// internal overlay events
		virtual void onAnimComplete(OVERLAY_TRANSITION_TYPE transType);

		void updateHighScore() const;
		void userAction();

		int getScore() const { return _userScore; }
		int getHighScore() const { return _currHighScore; }

	protected:
		// string labels
		TextButton* _congratsText;

		// score strings
		TextButton* _yourScore;
		TextButton* _highScore;

		// animations
		AnimID _idTimer;
		AnimID _idTallyAnim;
		AnimID _idHighAnim;

		// high score cache
		int _userScore;
		int _currHighScore;
	};
}
