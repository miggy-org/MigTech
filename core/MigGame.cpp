#include "pch.h"
#include "MigInclude.h"
#include "Timer.h"
#include "PerfMon.h"

using namespace MigTech;
using namespace tinyxml2;

///////////////////////////////////////////////////////////////////////////
// platform specific

extern unsigned int plat_getBits();

///////////////////////////////////////////////////////////////////////////
// static game functions

bool MigGame::initGameEngine(AudioBase* audioManager, PersistBase* dataManager)
{
	if (!MigUtil::init())
		return false;
	LOGINFO("(MigGame::initGameEngine) MigTech game engine starting");

	if (!Timer::init())
		return false;

	if (audioManager != nullptr)
	{
		audioManager->initAudio();
		MigUtil::theAudio = audioManager;
	}

	if (dataManager != nullptr)
	{
		dataManager->open();
		MigUtil::thePersist = dataManager;
	}

	LOGINFO("(MigGame::initGameEngine) MigTech game engine initialized");
	return true;
}

bool MigGame::initRenderer(RenderBase* rtObj)
{
	LOGINFO("(MigGame::initRenderer) MigTech renderer starting");

	if (rtObj == nullptr)
		throw std::invalid_argument("Game: MigTech doesn't have a renderer");
	if (!rtObj->initRenderer())
		return false;
	MigUtil::theRend = rtObj;

	LOGINFO("(MigGame::initRenderer) MigTech renderer initialized");
	return true;
}

bool MigGame::termRenderer()
{
	LOGINFO("(MigGame::termRenderer) MigTech renderer stopping");

	if (MigUtil::theRend != nullptr)
	{
		MigUtil::theRend->termRenderer();
		delete MigUtil::theRend;
	}
	MigUtil::theRend = nullptr;

	LOGINFO("(MigGame::termRenderer) MigTech renderer stopped");
	return true;
}

bool MigGame::termGameEngine()
{
	LOGINFO("(MigGame::termGameEngine) MigTech game engine stopping");

	if (MigUtil::thePersist != nullptr)
	{
		MigUtil::thePersist->close();
		delete MigUtil::thePersist;
	}
	MigUtil::thePersist = nullptr;

	if (MigUtil::theAudio != nullptr)
	{
		MigUtil::theAudio->termAudio();
		delete MigUtil::theAudio;
	}
	MigUtil::theAudio = nullptr;

	LOGINFO("(MigGame::termGameEngine) MigTech game engine stopped");
	return true;
}

unsigned int MigGame::queryPlatformBits()
{
	return plat_getBits();
}

///////////////////////////////////////////////////////////////////////////
// Game class implementation

MigGame::MigGame(const std::string& appName) :
	_appName(appName), _cfgDoc(nullptr), _currScreen(nullptr), _newScreenOnNextUpdate(false)
{
	if (MigUtil::theGame != nullptr)
		throw std::runtime_error("(MigGame::MigGame) MigTech game singleton already set");
	MigUtil::theGame = this;
}

MigGame::~MigGame()
{
	MigUtil::theGame = nullptr;
}

void MigGame::onCreate()
{
	LOGINFO("(MigGame::onCreate) MigTech game created");

	// load app configuration document
	_cfgDoc = XMLDocFactory::loadDocument(_appName + ".xml");
	if (_cfgDoc != nullptr)
	{
		_cfgRoot = _cfgDoc->FirstChildElement("config");
		if (_cfgRoot != nullptr)
		{
			// watchdog configuration
			XMLElement* elem = _cfgRoot->FirstChildElement("watchdog");
			if (elem != nullptr)
			{
				int period = MigUtil::parseInt(elem->Attribute("period"), 5);
				int lookback = MigUtil::parseInt(elem->Attribute("lookback"), 1);
				MigUtil::setWatchdog(period, lookback);
			}

			// perfmon configuration
			elem = _cfgRoot->FirstChildElement("perfmon");
			if (elem != nullptr)
			{
				bool active = MigUtil::parseBool(elem->Attribute("active"), false);
				PerfMon::showFPS(active);
			}

			// global font (optional)
			elem = _cfgRoot->FirstChildElement("fonts");
			if (elem != nullptr)
			{
				elem = elem->FirstChildElement("global");
				if (elem != nullptr)
					MigUtil::theFont = new Font("global");
			}
		}
	}

	if (MigUtil::theFont != nullptr)
	{
		LOGINFO("(MigGame::onCreate) Creating default font");
		MigUtil::theFont->create();

		// pass the font to the performance monitor
		PerfMon::init(MigUtil::theFont);
	}

	_currScreen = createStartupScreen();
	if (_currScreen != nullptr)
	{
		LOGINFO("(MigGame::onCreate) Startup screen created (%s)", _currScreen->getScreenName().c_str());
		_currScreen->create();
	}
	else
		throw std::runtime_error("(MigGame::onCreate) No startup screen provided");
}

void MigGame::onCreateGraphics()
{
	LOGINFO("(MigGame::onCreateGraphics) MigTech game graphics initializing");

	// preload MigTech shaders
	LOGINFO("(MigGame::onCreateGraphics) Preloading MigTech shaders");
	MigUtil::theRend->loadVertexShader(MIGTECH_VSHADER_POS_NO_TRANSFORM, VDTYPE_POSITION, SHADER_HINT_NONE);
	MigUtil::theRend->loadVertexShader(MIGTECH_VSHADER_POS_TEX_NO_TRANSFORM, VDTYPE_POSITION_TEXTURE, SHADER_HINT_NONE);
	MigUtil::theRend->loadVertexShader(MIGTECH_VSHADER_POS_TRANSFORM, VDTYPE_POSITION, SHADER_HINT_MVP);
	MigUtil::theRend->loadVertexShader(MIGTECH_VSHADER_POS_TEX_TRANSFORM, VDTYPE_POSITION_TEXTURE, SHADER_HINT_MVP);
	MigUtil::theRend->loadPixelShader(MIGTECH_PSHADER_COLOR, SHADER_HINT_NONE);
	MigUtil::theRend->loadPixelShader(MIGTECH_PSHADER_TEX, SHADER_HINT_NONE);
	MigUtil::theRend->loadPixelShader(MIGTECH_PSHADER_TEX_ALPHA, SHADER_HINT_NONE);
	LOGDBG("(MigGame::onCreateGraphics) Shaders loaded");

	if (MigUtil::theFont != nullptr)
		MigUtil::theFont->createGraphics();

	if (MigUtil::theDialog != nullptr)
		MigUtil::theDialog->createGraphics();

	if (_currScreen != nullptr)
		_currScreen->createGraphics();
}

void MigGame::onWindowSizeChanged()
{
	Size newSize = MigUtil::theRend->getOutputSize();
	LOGINFO("(MigGame::onWindowSizeChanged) MigTech game window size changed to %f,%f", newSize.width, newSize.height);

	if (_currScreen != nullptr)
		_currScreen->windowSizeChanged();
}

void MigGame::onVisibilityChanged(bool vis)
{
	LOGINFO("(MigGame::onVisibilityChanged) MigTech game window is now %s", (vis ? "visible" : "invisible"));

	if (_currScreen != nullptr)
		_currScreen->visibilityChanged(vis);
}

void MigGame::onSuspending()
{
	LOGINFO("(MigGame::onSuspending) MigTech game suspending");

	if (_currScreen != nullptr)
		_currScreen->suspend();

	if (MigUtil::theRend != nullptr)
		MigUtil::theRend->onSuspending();
	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->onSuspending();
}

void MigGame::onResuming()
{
	LOGINFO("(MigGame::onResuming) MigTech game resuming");

	if (MigUtil::theRend != nullptr)
		MigUtil::theRend->onResuming();
	if (MigUtil::theAudio != nullptr)
		MigUtil::theAudio->onResuming();

	if (_currScreen != nullptr)
		_currScreen->resume();
}

void MigGame::onDestroyGraphics()
{
	LOGINFO("(MigGame::onDestroyGraphics) MigTech game graphics destroying");

	if (_currScreen != nullptr)
		_currScreen->destroyGraphics();

	if (MigUtil::theDialog != nullptr)
		MigUtil::theDialog->destroyGraphics();

	if (MigUtil::theFont != nullptr)
		MigUtil::theFont->destroyGraphics();
}

void MigGame::onDestroy()
{
	LOGINFO("(MigGame::onDestroy) MigTech game destroying");

	if (_currScreen != nullptr)
	{
		_currScreen->destroy();
		delete _currScreen;
	}

	if (MigUtil::theFont != nullptr)
	{
		MigUtil::theFont->destroy();
		delete MigUtil::theFont;
	}

	if (_cfgDoc != nullptr)
		delete _cfgDoc;
}

void MigGame::onPointerPressed(float x, float y)
{
	LOGINFO("(MigGame::onPointerPressed) %f,%f", x, y);

	if (MigUtil::theDialog != nullptr)
		MigUtil::theDialog->pointerPressed(x, y);
	else if (_currScreen != nullptr)
		_currScreen->pointerPressed(x, y);
}

void MigGame::onPointerReleased(float x, float y)
{
	LOGINFO("(MigGame::onPointerReleased) %f,%f", x, y);

	if (MigUtil::theDialog != nullptr)
		MigUtil::theDialog->pointerReleased(x, y);
	else if (_currScreen != nullptr)
		_currScreen->pointerReleased(x, y);
}

void MigGame::onPointerMoved(float x, float y, bool isInContact)
{
	//LOGDBG("(MigGame::onPointerMoved) %f,%f,%d", x, y, isInContact);

	if (MigUtil::theDialog != nullptr)
		MigUtil::theDialog->pointerMoved(x, y, isInContact);
	else if (_currScreen != nullptr)
		_currScreen->pointerMoved(x, y, isInContact);
}

void MigGame::onKeyDown(VIRTUAL_KEY key)
{
	LOGINFO("(MigGame::onKeyDown) %d", key);

	if (MigUtil::theDialog == nullptr && _currScreen != nullptr)
		_currScreen->keyDown(key);
}

void MigGame::onKeyUp(VIRTUAL_KEY key)
{
	LOGINFO("(MigGame::onKeyUp) %d", key);

	if (MigUtil::theDialog == nullptr && _currScreen != nullptr)
		_currScreen->keyUp(key);
}

bool MigGame::onBackKey()
{
	LOGINFO("(MigGame::onBackKey)");

	if (MigUtil::theDialog != nullptr)
		return true;
	return (_currScreen != nullptr ? _currScreen->backKey() : false);
}

void MigGame::update()
{
	// update the current game time
	Timer::updateGameTime();

	// pet the watchdog
	MigUtil::petWatchdog();

	// run the animation in the animation list
	if (MigUtil::theAnimList != nullptr)
		MigUtil::theAnimList->doAnimations();

	// clear the dialog if it has expired
	if (MigUtil::theDialog != nullptr && !MigUtil::theDialog->isActive())
	{
		delete MigUtil::theDialog;
		MigUtil::theDialog = nullptr;
	}

	if (_currScreen != nullptr)
	{
		if (_newScreenOnNextUpdate)
		{
			_currScreen->destroyGraphics();
			_currScreen->destroy();

			ScreenBase* nextScreen = _currScreen->getNextScreen();
			delete _currScreen;
			_currScreen = nextScreen;

			if (_currScreen != nullptr)
			{
				LOGINFO("(MigGame::update) New screen created (%s)", _currScreen->getScreenName().c_str());
				_currScreen->create();
				_currScreen->createGraphics();	// TODO: does Windows require this to be on a separate thread?
				_currScreen->windowSizeChanged();
			}
			else
				throw std::runtime_error("(MigGame::update) Expired screen didn't provide a next screen");

			_newScreenOnNextUpdate = false;
		}

		if (!_currScreen->update())
		{
			_newScreenOnNextUpdate = true;
		}
	}
}

bool MigGame::render()
{
	if (_currScreen != nullptr)
	{
		const std::vector<RenderPass*>& passList = _currScreen->getPassList();

		// render the pre-render passes first, if any
		for (int i = 0; i < (int) passList.size(); i++)
		{
			RenderPass* pass = passList[i];
			if (pass != nullptr && pass->getType() == RenderPass::RENDER_PASS_PRE && pass->isValid())
			{
				MigUtil::theRend->preRender(i + 1, pass);
				if (pass->preRender())
					_currScreen->renderPass(i + 1, pass);
				pass->postRender();
				MigUtil::theRend->postRender(i + 1, pass);
			}
		}

		// main rendering pass
		MigUtil::theRend->preRender(0, nullptr);
		_currScreen->render();
		MigUtil::theRend->postRender(0, nullptr);

		// render the post-render passes, if any
		for (int i = 0; i < (int)passList.size(); i++)
		{
			RenderPass* pass = passList[i];
			if (pass != nullptr && pass->getType() == RenderPass::RENDER_PASS_POST && pass->isValid())
			{
				MigUtil::theRend->preRender(i + 1, pass);
				if (pass->preRender())
					_currScreen->renderPass(i + 1, pass);
				pass->postRender();
				MigUtil::theRend->postRender(i + 1, pass);
			}
		}

		// overlay rendering pass (including perf monitor)
		MigUtil::theRend->preRender(-1, nullptr);
		_currScreen->renderOverlays();
		if (MigUtil::theDialog != nullptr)
			MigUtil::theDialog->draw();
		PerfMon::doFPS();
		MigUtil::theRend->postRender(-1, nullptr);

		// present the resulting image
		MigUtil::theRend->present();
	}

	return true;
}
