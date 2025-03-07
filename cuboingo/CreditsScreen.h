#pragma once

#include "../core/MigInclude.h"
#include "../core/ScreenBase.h"
#include "../core/MigUtil.h"
#include "SplashCube.h"

using namespace MigTech;

namespace Cuboingo
{
	class ICreditsEvents
	{
	public:
		virtual void stageActionComplete(int action) = 0;
		virtual void cubeExitComplete() = 0;
	};

	class CreditsBg : public CycleBg
	{
	public:
		static const int STAGE_ACTION_FLASH		= 0;
		static const int STAGE_ACTION_FADE_OUT1 = 1;
		static const int STAGE_ACTION_FADE_IN2	= 2;
		static const int STAGE_ACTION_MAIN		= 3;
		static const int STAGE_ACTION_FADE_OUT2 = 4;
		static const int STAGE_ACTION_BLANK		= 5;

	public:
		CreditsBg(ICreditsEvents* events);
		//virtual ~CreditsBg();

		void fadeOut();

		virtual void create();
		virtual void createGraphics();
		virtual void destroyGraphics();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		virtual void render()const;

	protected:
		void clearOtherImage();

	protected:
		ICreditsEvents* _events;
		Object* _fadePoly;
		int _stage;
	};

	class CreditsCube : public SplashCube
	{
	public:
		CreditsCube(ICreditsEvents* events) : SplashCube(), _events(events) {}

		void start();
		void exit();

		virtual void animComplete(int id, void* optData);

	protected:
		virtual void applyTransform(Matrix& worldMatrix) const;

		void startIntroAnimation(int dur);
		void startMoveAnimation(int dur);
		void startExitAnimation(int dur);

	protected:
		ICreditsEvents* _events;
		AnimID _idPause;
	};

	struct CreditsEvent
	{
		CreditsEvent(int d, const char* i, const char* s) { dur = d; image = i; sound = s; }

		int dur;
		std::string image;
		std::string sound;
	};

	class CreditsScreen : public ScreenBase, public ICreditsEvents
	{
	public:
		CreditsScreen();

		virtual ScreenBase* getNextScreen();

		virtual void create();
		virtual void createGraphics();
		virtual void windowSizeChanged();

		// frame events
		virtual bool render();

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// ICreditsEvents
		virtual void stageActionComplete(int action);
		virtual void cubeExitComplete();

	protected:
		virtual void onTap(float x, float y);

		void initEventList();
		void startEvents();
		void nextEvent();

	protected:
		Matrix _viewMatrix;
		Matrix _projMatrix;

		CreditsCube _cube;
		bool _cubeVisible;
		std::vector<CreditsEvent> _events;
		int _currEvent;
		AnimID _idTimer;
		AnimID _idFade;

		MovieClip _clip;
		Vector3 _clipPos;
	};
}
