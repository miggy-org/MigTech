#pragma once

#include "MigBase.h"
#include "Object.h"
#include "BgBase.h"
#include "SoundEffect.h"
#include "AnimList.h"
#include "OverlayBase.h"
#include "Controls.h"
#include "RenderBase.h"

namespace MigTech
{
	enum FADE_STYLE
	{
		FADE_STYLE_IN,
		FADE_STYLE_OUT
	};

	class ScreenBase : public IAnimTarget, public IOverlayCallback, public IControlsCallback
	{
	public:
		ScreenBase(const std::string& name);
		virtual ~ScreenBase();

		// screen management
		virtual ScreenBase* getNextScreen();
		virtual const std::string& getScreenName();

		// life cycle events
		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();
		virtual void visibilityChanged(bool vis);
		virtual void suspend();
		virtual void resume();
		virtual void destroyGraphics();
		virtual void destroy();

		// raw input events
		virtual bool pointerPressed(float x, float y);
		virtual void pointerReleased(float x, float y);
		virtual void pointerMoved(float x, float y, bool isInContact);
		virtual void keyDown(VIRTUAL_KEY key);
		virtual void keyUp(VIRTUAL_KEY key);
		virtual bool backKey();

		// frame events
		virtual bool update();
		virtual bool render();
		virtual bool renderOverlays();

		// multi-pass render support
		virtual const std::vector<RenderPass*>& getPassList() const;
		virtual bool renderPass(int index, RenderPass* pass);

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// IOverlayCallback
		virtual void onOverlayIntro(long duration);
		virtual void onOverlayExit(long duration);
		virtual void onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration);
		virtual void onOverlayCustom(int id, void* data);

	protected:
		// processed events produced from the raw events
		virtual void onTap(float x, float y);
		virtual void onSwipe(float x, float y, float dx, float dy, SWIPE_STYLE swipe);
		virtual void onKey(VIRTUAL_KEY key);
		virtual bool onBackKey();

		// events received by controls
		virtual void onClick(int id, ControlBase* sender);
		virtual void onSlide(int id, ControlBase* sender, float val);

		// allocator for custom controls
		virtual ControlBase* allocControl(const char* tag);

		// initializes the background screen, call before create() is called
		virtual void initBackgroundScreen(BgBase* bgHandler);
		virtual void initBackgroundScreen(const std::string& resID);
		virtual void initBackgroundScreen(tinyxml2::XMLElement* xml);
		virtual void initOverlayScreen(BgBase* overlayHandler);
		virtual void initOverlayScreen(const std::string& resID);
		virtual void initOverlayScreen(tinyxml2::XMLElement* xml);

		// fade control
		virtual void startFadeIn(long duration = -1);
		virtual void startFadeOut(long duration = -1);
		virtual void startFadeScreen(long duration, float start, float end, bool musicToo);
		virtual void doFade(float end);
		virtual void fadeComplete(float fadeAlpha, FADE_STYLE fadeStyle);

	protected:
		// starts background music for this screen
		void startMusic(const std::string& musicResourceID, bool loop = true);

		// use during rendering
		void clearScreen();
		void drawBackgroundScreen();
		void drawOverlays();

		// overlays
		void startNewOverlay(OverlayBase* newOverlay, float newAlpha, long duration, bool musicToo);
		void startNewOverlay(OverlayBase* newOverlay);
		void startNewOverlayAnimation(float newAlpha, long duration, bool musicToo);
		void freeOverlay();
		bool isOverlayVisible() const;

	protected:
		// screen name (debugging)
		std::string _screenName;

		// lifecycle collection
		LifeCycleCollection _lcList;

		// background/overlay handlers
		BgBase* _bgHandler;
		BgBase* _overlayHandler;

		// overlay rectangle
		Object* _fadeOverlay;
		AnimID _idFadeAnim;
		long _fadeDuration;
		FADE_STYLE _fadeStyle;

		// overlay
		OverlayBase* _overlay;
		OverlayBase* _expiredOverlay;

		// music player
		AnimID _idMusicAnim;
		bool _musicFade;
		bool _musicStopOnExit;

		// screen colors
		Color _clearColor;
		Color _fadeColor;

		// swipe tracking
		Vector2 _startPt;
		float _maxThreshold;
		float _tapThreshold;
		SWIPE_STYLE _swipeLocked;

		// controls container
		Font* _localFont;
		Controls _controls;

		// render pass list
		std::vector<RenderPass*> _renderPasses;
	};
}