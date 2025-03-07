#pragma once

#include "MigDefines.h"
#include "AnimList.h"
#include "RenderBase.h"
#include "AudioBase.h"
#include "ScreenBase.h"
#include "PersistBase.h"

namespace MigTech
{
	// implement this for a given game
	class MigGame
	{
	public:
		// called once to init/term the game engine
		static bool initGameEngine(AudioBase* audioManager, PersistBase* dataManager);
		static bool initRenderer(RenderBase* rtObj);
		static bool termRenderer();
		static bool termGameEngine();

		// query platform information
		static unsigned int queryPlatformBits();

	public:
		MigGame(const std::string& appName);
		virtual ~MigGame();

		virtual void onCreate();
		virtual void onCreateGraphics();
		virtual void onWindowSizeChanged();
		virtual void onVisibilityChanged(bool vis);
		virtual void onSuspending();
		virtual void onResuming();
		virtual void onDestroyGraphics();
		virtual void onDestroy();

		virtual void onPointerPressed(float x, float y);
		virtual void onPointerReleased(float x, float y);
		virtual void onPointerMoved(float x, float y, bool isInContact);
		virtual void onKeyDown(VIRTUAL_KEY key);
		virtual void onKeyUp(VIRTUAL_KEY key);
		virtual bool onBackKey();

		virtual void update();
		virtual bool render();

		// getters
		const std::string& getAppName() const { return _appName; }
		const tinyxml2::XMLElement* getConfigRoot() const { return _cfgRoot; }

	protected:
		virtual ScreenBase* createStartupScreen() = 0;

	protected:
		std::string _appName;
		tinyxml2::XMLDocument* _cfgDoc;
		tinyxml2::XMLElement* _cfgRoot;

		ScreenBase* _currScreen;
		bool _newScreenOnNextUpdate;
	};
}