#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <jni.h>
#include <typeinfo>
#include <android/asset_manager_jni.h>

#include "pch.h"
#include "../core/MigUtil.h"
#include "../core/MigGame.h"
#include "OglRender.h"
#include "OslAudio.h"

using namespace MigTech;

static MigTech::MigGame* theGame = nullptr;
static jobject assetMgrObj = 0;
static AAssetManager* assets;
static std::string localFilesDir;
static std::string externalFilesDir;
static bool okToRun = true;
static pthread_t threadWatchdog = 0;

///////////////////////////////////////////////////////////////////////////
// application specific

extern MigTech::MigGame* allocGame();

///////////////////////////////////////////////////////////////////////////
// watchdog thread

void* WatchdogThread(void* arg)
{
	auto sleepPeriod = reinterpret_cast<std::uintptr_t>(arg);

	while (okToRun)
	{
		if (!MigUtil::checkWatchdog())
		{
			MigUtil::fatal("Watchdog triggered, dumping log file");
			MigUtil::dumpLogToFile();

			// log dumped so cancel the watchdog
			break;
		}

		sleep(sleepPeriod);
	}
	return nullptr;
}

///////////////////////////////////////////////////////////////////////////
// JNI entry points

void AndroidApp_create(AAssetManager* assetManager, const char* dataPath, const char* extPath)
{
	// get asset manager
	assets = assetManager;

	// get the local files directory and create it
	externalFilesDir = extPath;
	localFilesDir = dataPath;
	int resultCode = mkdir(localFilesDir.c_str(), 0770);
	if (resultCode != 0 && errno != EEXIST)
		LOGWARN("(AndroidApp_create) Unable to create local files dir '%s' %d", localFilesDir.c_str(), resultCode);

	if (theGame == nullptr)
	{
		try
		{
			// initialize MigTech (renderer initialization will come later)
			MigTech::AudioBase* pAudio = new MigTech::OslAudioManager();
			MigTech::PersistBase* pDataManager = new MigTech::SimplePersist();
			MigTech::MigGame::initGameEngine(pAudio, pDataManager);
		}
		catch (std::exception& ex)
		{
			MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
			MigTech::MigUtil::dumpLogToFile();
			okToRun = false;
		}
	}
}

void AndroidApp_create(JNIEnv* env, jobject assetMgr, jstring filesDir, jstring extDir)
{
	// secure a reference to the asset manager object
	if (assetMgrObj)
		env->DeleteGlobalRef(assetMgrObj);
	assetMgrObj = env->NewGlobalRef(assetMgr);

	AndroidApp_create(
		AAssetManager_fromJava(env, assetMgrObj),
		env->GetStringUTFChars(filesDir, NULL),
		env->GetStringUTFChars(extDir, NULL));
}

void AndroidApp_createGraphics(void)
{
	try
	{
		// initialize MigTech renderer
		MigTech::RenderBase* pRtObj = new MigTech::OglRender();
		MigTech::MigGame::initRenderer(pRtObj);

		// if the GL context is simply being recreated then the game will need to know
		if (theGame != nullptr)
			theGame->onCreateGraphics();
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_init(int width, int height)
{
	try
	{
		MigTech::MigUtil::theRend->setOutputSize(MigTech::Size(width, height));

		if (theGame == nullptr)
		{
			// brand new game instance
			theGame = allocGame();
			theGame->onCreate();
			theGame->onCreateGraphics();

			// start watchdog thread
			int watchdogPeriod = MigUtil::getWatchdogPeriod();
			if (watchdogPeriod > 0)
			{
				LOGINFO("(::AndroidApp_init) Starting watchdog thread, period is %d seconds", watchdogPeriod);
				pthread_create(&threadWatchdog, nullptr, WatchdogThread, reinterpret_cast<void*>(watchdogPeriod));
			}
		}

		theGame->onWindowSizeChanged();
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_step(void)
{
	try
	{
		if (theGame != nullptr)
		{
			theGame->update();
			theGame->render();
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_suspend(void)
{
	try
	{
		MigTech::MigUtil::suspendWatchdog();

		if (theGame != nullptr)
		{
			theGame->onVisibilityChanged(false);
			theGame->onSuspending();
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_resume(void)
{
	try
	{
		if (theGame != nullptr)
		{
			theGame->onResuming();
			theGame->onVisibilityChanged(true);
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_destroyGraphics(void)
{
	try
	{
		MigTech::MigUtil::suspendWatchdog();

		if (theGame != nullptr)
			theGame->onDestroyGraphics();
		MigTech::MigGame::termRenderer();
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_destroy(JNIEnv* env)
{
	try
	{
		if (theGame != nullptr)
		{
			theGame->onDestroy();
			delete theGame;
		}
		theGame = nullptr;

		MigTech::MigGame::termGameEngine();
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
	}

	if (env != nullptr && assetMgrObj != nullptr)
		env->DeleteGlobalRef(assetMgrObj);
	assetMgrObj = nullptr;

	okToRun = false;
}

void AndroidApp_pointerPressed(float x, float y)
{
	try
	{
		if (theGame != nullptr)
		{
			MigTech::Size size = MigTech::MigUtil::theRend->getOutputSize();
			theGame->onPointerPressed(x / size.width, y / size.height);
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_pointerReleased(float x, float y)
{
	try
	{
		if (theGame != nullptr)
		{
			MigTech::Size size = MigTech::MigUtil::theRend->getOutputSize();
			theGame->onPointerReleased(x / size.width, y / size.height);
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

void AndroidApp_pointerMoved(float x, float y)
{
	try
	{
		if (theGame != nullptr)
		{
			MigTech::Size size = MigTech::MigUtil::theRend->getOutputSize();
			theGame->onPointerMoved(x / size.width, y / size.height, true);
		}
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
}

bool AndroidApp_backKey()
{
	bool ret = false;
	try
	{
		if (theGame != nullptr)
			ret = theGame->onBackKey();
	}
	catch (std::exception& ex)
	{
		MigTech::MigUtil::fatal("[%s] %s", typeid(ex).name(), ex.what());
		MigTech::MigUtil::dumpLogToFile();
		okToRun = false;
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////
// additional utility functions

AAssetManager* AndroidUtil_getAssetManager()
{
	return assets;
}

int AndroidUtil_getAssetBuffer(const std::string& name, void* buf, int bufLen)
{
	if (buf == nullptr && bufLen > 0)
		throw std::invalid_argument("(AndroidUtil_getAssetBuffer) No buffer provided");
	off_t len = -1;

	AAsset* file = AAssetManager_open(assets, name.c_str(), AASSET_MODE_BUFFER);
	if (file)
	{
		len = AAsset_getLength(file);
		if (len <= bufLen)
		{
			const void* pdata = AAsset_getBuffer(file);
			if (pdata)
			{
				memset(buf, 0, bufLen);
				memcpy(buf, pdata, len);
			}
			else
			{
				len = -1;  // this is an error
				LOGWARN("(AndroidUtil_getAssetBuffer) Failed to get the buffer\n");
			}
		}
		else if (bufLen > 0)
			LOGWARN("(AndroidUtil_getAssetBuffer) Provided buffer size is too small\n");

		AAsset_close(file);
	}
	else
		LOGWARN("(AndroidUtil_getAssetBuffer) Failed to open %s\n", name.c_str());

	return (int) len;
}

const std::string& AndroidUtil_getFilesDir()
{
	return localFilesDir;
}

const std::string& AndroidUtil_getExtFilesDir()
{
	return externalFilesDir;
}

GLenum checkGLError(const char* callerName, const char* funcName)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::string fmt = "(";
		fmt += callerName;
		fmt += ") ";
		fmt += funcName;
		fmt += " returned %d";
		LOGWARN(fmt.c_str(), err);
	}
	return err;
}

///////////////////////////////////////////////////////////////////////////
// for use by native activities

#include <EGL/egl.h>
#include <android_native_app_glue.h>

// shared state for our app
struct engine
{
    struct android_app* app;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    bool isAnimating;
};

// checks a config attribute against a required value
static bool checkConfigProperty(EGLDisplay display, EGLConfig config, EGLint attr, EGLint reqVal)
{
	EGLint retVal;
	if (!eglGetConfigAttrib(display, config, attr, &retVal))
		return false;
	//LOGINFO("Config %d attribute %d is %d", (int) config, attr, retVal);
	return (retVal >= reqVal);
}

// initialize an EGL context for the current display
static int engine_init_display(struct engine* engine)
{
	// specify the attributes of the desired configuration
    const EGLint attribs[] =
	{
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_NONE
    };

	// get the default display and initialize
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

	// get the number of configurations available for this display
    EGLint numConfigs;
    eglChooseConfig(display, attribs, NULL, 0, &numConfigs);
	LOGINFO("eglChooseConfig() returned %d configs", numConfigs);

	// get the list
    EGLConfig* configs = new EGLConfig[numConfigs];
    eglChooseConfig(display, attribs, configs, numConfigs, &numConfigs);

	// choose a configuration that matches a desired profile
	EGLConfig finalConfig = 0;
	for (int i = 0; i < numConfigs; i++)
	{
		// needs a depth buffer of at least 16 bits
		if (!checkConfigProperty(display, configs[i], EGL_DEPTH_SIZE, 16))
			continue;

		// needs rgb of 8 bits each
		if (!checkConfigProperty(display, configs[i], EGL_RED_SIZE, 8))
			continue;
		if (!checkConfigProperty(display, configs[i], EGL_GREEN_SIZE, 8))
			continue;
		if (!checkConfigProperty(display, configs[i], EGL_BLUE_SIZE, 8))
			continue;

		// would need alpha if this was a transparent view
		//if (!checkConfigProperty(display, configs[i], EGL_ALPHA_SIZE, 8))
		//	continue;

		// if we've made it this far we have a compatible config
		finalConfig = configs[i];
		LOGINFO("Compatible EGL config found (%d)", i);
		break;
	}
	delete [] configs;

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    EGLint format;
    eglGetConfigAttrib(display, finalConfig, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	// create the GL surface
    EGLSurface surface = eglCreateWindowSurface(display, finalConfig, engine->app->window, NULL);
	if (surface == EGL_NO_SURFACE)
	{
        LOGWARN("eglCreateWindowSurface() failed");
        return -1;
	}

	// create the GL context (needs to support OpenGLES2)
    const EGLint contextAttribs[] =
	{
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
	EGLContext context = eglCreateContext(display, finalConfig, NULL, contextAttribs);
	if (context == EGL_NO_CONTEXT)
	{
		LOGWARN("eglCreateContext() failed");
        return -1;
    }

	// made that context the current context for the surface
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		LOGWARN("eglMakeCurrent() failed");
        return -1;
    }

	// save the necessary vars
    engine->display = display;
    engine->context = context;
    engine->surface = surface;

	// get dimensions
    EGLint w, h;
    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	LOGINFO("Surface dimensions are %d, %d", w, h);

	// init the MigTech graphics engine
	AndroidApp_createGraphics();

	// set the game dimensions
	AndroidApp_init(w, h);

    return 0;
}

// draw a frame
static void engine_draw_frame(struct engine* engine)
{
    if (engine->display != NULL)
	{
		// draw a frame
		AndroidApp_step();

		// swap buffers
	    eglSwapBuffers(engine->display, engine->surface);
    }
}

// tear down the EGL context currently associated with the display
static void engine_term_display(struct engine* engine)
{
	// destroy the graphics engine
	AndroidApp_destroyGraphics();

    if (engine->display != EGL_NO_DISPLAY)
	{
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT)
            eglDestroyContext(engine->display, engine->context);
        if (engine->surface != EGL_NO_SURFACE)
            eglDestroySurface(engine->display, engine->surface);
        eglTerminate(engine->display);
    }
    engine->isAnimating = false;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

// process the next input event
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		int32_t action = AKeyEvent_getAction(event);
		switch (action & AMOTION_EVENT_ACTION_MASK)
		{
		case AMOTION_EVENT_ACTION_DOWN:
			{
				float x = AMotionEvent_getX(event, 0);
				float y = AMotionEvent_getY(event, 0);
				AndroidApp_pointerPressed(x, y);
			}
	        return 1;
		case AMOTION_EVENT_ACTION_MOVE:
			{
				float x = AMotionEvent_getX(event, 0);
				float y = AMotionEvent_getY(event, 0);
				AndroidApp_pointerMoved(x, y);
			}
	        return 1;
		case AMOTION_EVENT_ACTION_UP:
		//case AMOTION_EVENT_ACTION_CANCEL:
			{
				float x = AMotionEvent_getX(event, 0);
				float y = AMotionEvent_getY(event, 0);
				AndroidApp_pointerReleased(x, y);
			}
	        return 1;
		}
    }
	else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
	{
		int32_t action = AKeyEvent_getAction(event);
		int32_t keycode = AKeyEvent_getKeyCode(event);
		if (action == AKEY_EVENT_ACTION_UP && keycode == AKEYCODE_BACK)
			return AndroidApp_backKey();
	}
    return 0;
}

// process the next main command
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // the system has asked us to save our current state
            //engine->app->savedState = malloc(sizeof(struct saved_state));
            //*((struct saved_state*)engine->app->savedState) = engine->state;
            //engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL)
			{
                engine_init_display(engine);
                //engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
			// resume animating
            engine->isAnimating = true;
			AndroidApp_resume();
            break;
        case APP_CMD_LOST_FOCUS:
			// stop animating
			AndroidApp_suspend();
            engine->isAnimating = false;
            //engine_draw_frame(engine);
            break;
    }
}

// this is the actual entry point expected for a native activity
extern "C"
void android_main(struct android_app* state)
{
	// initialize the engine struct that will be used throughout the lifecycle
    struct engine engine;
    memset(&engine, 0, sizeof(engine));
    engine.app = state;
	engine.isAnimating = true;

	// assign state variables
	state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    //if (state->savedState != NULL)
	//{
        // we are starting with a previous saved state; restore from it
    //    engine.state = *(struct saved_state*)state->savedState;
    //}

	// init the MigTech game engine
	AndroidApp_create(
		state->activity->assetManager,
		state->activity->internalDataPath,
		state->activity->externalDataPath);

    // loop waiting for stuff to do.
	while (okToRun)
	{
        // read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.isAnimating ? 0 : -1, NULL, &events,
                (void**)&source)) >= 0)
		{
            // process this event
            if (source != NULL)
                source->process(state, source);

            // check if we are exiting
            if (state->destroyRequested != 0)
			{
                //engine_term_display(&engine);

				// destroy the game engine
				AndroidApp_destroy(NULL);
                return;
            }
        }

        if (engine.isAnimating)
            engine_draw_frame(&engine);
    }
}
