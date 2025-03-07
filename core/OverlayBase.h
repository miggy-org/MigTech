#pragma once

#include "AnimList.h"
#include "Matrix.h"
#include "Object.h"
#include "Controls.h"

namespace MigTech
{
	class ScreenBase;

	enum OVERLAY_TRANSITION_TYPE
	{
		OVERLAY_TRANSITION_NONE,
		OVERLAY_TRANSITION_INTRO,
		OVERLAY_TRANSITION_EXIT,
		OVERLAY_TRANSITION_ROTATE_NEXT,
		OVERLAY_TRANSITION_ROTATE_BACK,
		OVERLAY_TRANSITION_FADE
	};

	class IOverlayCallback
	{
	public:
		// called when the overlay intro animation starts
		virtual void onOverlayIntro(long duration) = 0;

		// called when the overlay exit animation starts
		virtual void onOverlayExit(long duration) = 0;

		// called when the overlay animation of any kind is complete
		virtual void onOverlayComplete(OVERLAY_TRANSITION_TYPE transType, long duration) = 0;

		// called to pass custom data to an overlay callback
		virtual void onOverlayCustom(int id, void* data) = 0;
	};

	class OverlayBase : public IAnimTarget, public IControlsCallback
	{
		friend ScreenBase;

	public:
		OverlayBase(const std::string& name);
		virtual ~OverlayBase();

		// overlay management
		virtual const std::string& getOverlayName() { return _overlayName; }

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

		// frame events
		virtual bool update();
		virtual void render();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// animation start
		virtual void startIntroAnimation(long duration = -1);
		virtual void startInterAnimation(OVERLAY_TRANSITION_TYPE transType, long duration = -1, bool isIncoming = false);
		virtual void startExitAnimation(long duration = -1);

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

		// internal overlay events
		virtual void onAnimComplete(OVERLAY_TRANSITION_TYPE transType);

		// internal draw
		virtual void draw(float alpha, const Matrix& mat);

	protected:
		// overlay name (debugging)
		std::string _overlayName;

		// lifecycle collection
		LifeCycleCollection _lcList;

		// configuration
		float _parentAlpha;
		long _fadeDuration;
		long _fadeDurationInter;
		bool _fadeMusic;

		// animations
		OVERLAY_TRANSITION_TYPE _transType;
		AnimID _idTimerAnim;
		float _alpha;
		AnimID _idAlphaAnim;
		float _scaleX;
		AnimID _idScaleXAnim;
		float _scaleY;
		AnimID _idScaleYAnim;
		float _rotateX;
		AnimID _idRotateXAnim;
		float _rotateY;
		AnimID _idRotateYAnim;
		float _rotateZ;
		AnimID _idRotateZAnim;
		long _animDuration;

		// next overlay screen
		OverlayBase* _nextOverlay;

		// callback to receive events from this overlay
		IOverlayCallback* _callback;

		// controls container
		Font* _localFont;
		Controls _controls;
	};
}