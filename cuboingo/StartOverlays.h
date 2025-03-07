#pragma once

#include "../core/MigInclude.h"
#include "../core/OverlayBase.h"
#include "../core/MigUtil.h"
#include "GameScripts.h"

using namespace MigTech;

namespace Cuboingo
{
	///////////////////////////////////////////////////////////////////////////
	// main start screen overlay

	class StartOverlay : public OverlayBase
	{
	public:
		StartOverlay() : OverlayBase("StartOverlay") { }

	protected:
		// processed events produced from the raw events
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);
	};

	///////////////////////////////////////////////////////////////////////////
	// script screen overlay

	class ScriptItem
	{
	public:
		ScriptItem();

		void init(const Script& script);
		int getMaxTextLength() const;
		void updateTextLength(int len);
		void draw(float alpha, const Matrix& worldMatrix);

	public:
		Script theScript;
		Text nameUpper;
		Text diffUpper;
		Text desc1Upper;
		Text desc2Upper;
		Text desc3Upper;
		Text desc4Upper;
		Text highUpper;
		std::string highStr;
	};

	class ScriptOverlay : public OverlayBase
	{
	public:
		ScriptOverlay() : OverlayBase("ScriptOverlay"), _transX(0), _prevBtn(nullptr), _nextBtn(nullptr) {}

		virtual void create();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual bool update();
		virtual void draw(float alpha, const Matrix& mat);

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe);
		virtual void onClick(int id, ControlBase* sender);
		virtual bool onBackKey();
		virtual void onKey(VIRTUAL_KEY key);

		void startTextLengthAnim();
		void updateBtnVisibility();
		void pickSelectedScript();
		void onClickLeft();
		void onClickRight();
		void doSwipe(int dir);

	protected:
		std::vector<ScriptItem> _items;
		int _currItem;
		int _otherItem;

		// animation
		float _transX;
		AnimID _idTransAnim;
		AnimID _idLengthAnim;

		Button* _prevBtn;
		Button* _nextBtn;
	};

	///////////////////////////////////////////////////////////////////////////
	// options screen overlay

	class OptionsOverlay : public OverlayBase
	{
	public:
		OptionsOverlay() : OverlayBase("OptionsOverlay"), _sample(nullptr) { }

		virtual void create();

	protected:
		// processed events produced from the raw events
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);
		virtual void onSlide(int id, ControlBase* sender, float val);

	protected:
		SoundEffect* _sample;
	};

	///////////////////////////////////////////////////////////////////////////
	// about screen overlay

	class AboutOverlay : public OverlayBase
	{
	public:
		AboutOverlay() : OverlayBase("AboutOverlay") { }

	protected:
		// processed events produced from the raw events
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);
	};

	///////////////////////////////////////////////////////////////////////////
	// debug screen overlay

	class DebugOverlay : public OverlayBase
	{
	public:
		DebugOverlay() : OverlayBase("DebugOverlay") { }

	protected:
		// processed events produced from the raw events
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);
	};

	///////////////////////////////////////////////////////////////////////////
	// debug screen overlay

	class DumpLogOverlay : public OverlayBase
	{
	public:
		DumpLogOverlay() : OverlayBase("DumpLogOverlay") { }

		virtual void create();

	protected:
		// processed events produced from the raw events
		virtual bool onBackKey();
	};
}
