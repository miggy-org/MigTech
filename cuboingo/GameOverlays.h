#pragma once

#include "../core/MigInclude.h"
#include "../core/OverlayBase.h"
#include "../core/MigUtil.h"
#include "ScoreKeeper.h"

using namespace MigTech;

namespace Cuboingo
{
	///////////////////////////////////////////////////////////////////////////
	// pause overlay

	class PauseOverlay : public OverlayBase
	{
	public:
		PauseOverlay() : OverlayBase("PauseOverlay"), _glowText(nullptr) { }

		virtual void create();
		virtual bool update();

	protected:
		// processed events produced from the raw events
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);

	protected:
		Button* _glowText;
	};

	///////////////////////////////////////////////////////////////////////////
	// tips overlay

	class TipsOverlay : public OverlayBase
	{
	public:
		TipsOverlay() : OverlayBase("TipsOverlay") { }

		virtual void create();

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();
	};

	///////////////////////////////////////////////////////////////////////////
	// summary overlay

	class SummaryOverlay : public OverlayBase
	{
	public:
		SummaryOverlay(ScoreKeeper& sk);

		virtual void create();
		virtual void destroy();
		virtual bool update();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		// internal overlay events
		virtual void onAnimComplete(OVERLAY_TRANSITION_TYPE transType);

		void startNextBonusAnimation();
		void startPauseTimer();
		bool cancelAllAnimations();
		void userAction();

	protected:
		ScoreKeeper& _scoreKeeper;

		int _timedBonusReal;
		AnimID _idTimedBonusAnim;
		int _noMissBonusReal;
		AnimID _idNoMissBonusAnim;
		int _allStampsHitBonusReal;
		AnimID _idAllStampsHitBonusAnim;
		AnimID _idPauseTimer;

		TextButton* _timedBonusDisplay;
		TextButton* _noMissBonusDisplay;
		TextButton* _allStampsHitBonusDisplay;
		TextButton* _timedLabel;
		TextButton* _noMissLabel;
		TextButton* _allStampsLabel;
		TextButton* _tapLabel;

		SoundEffect* _match;
	};
}
