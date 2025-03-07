#pragma once

#include <jni.h>
#include <android/asset_manager.h>

#include "../core/MigInclude.h"
#include "../core/MigGame.h"

///////////////////////////////////////////////////////////////////////////
// application specific

extern MigTech::MigGame* allocGame();

///////////////////////////////////////////////////////////////////////////
// entry points for JNI code

void AndroidApp_create(JNIEnv* env, jobject assetMgr, jstring filesDir);
void AndroidApp_createGraphics(void);
void AndroidApp_init(int width, int height);
void AndroidApp_step(void);
void AndroidApp_suspend(void);
void AndroidApp_resume(void);
void AndroidApp_destroyGraphics(void);
void AndroidApp_destroy(JNIEnv* env);
void AndroidApp_pointerPressed(float x, float y);
void AndroidApp_pointerReleased(float x, float y);
void AndroidApp_pointerMoved(float x, float y);
bool AndroidApp_backKey();

///////////////////////////////////////////////////////////////////////////
// additional utility functions

AAssetManager* AndroidUtil_getAssetManager();
int AndroidUtil_getAssetBuffer(const std::string& name, void* buf, int bufLen);
const std::string& AndroidUtil_getFilesDir();
const std::string& AndroidUtil_getExtFilesDir();

GLenum checkGLError(const char* callerName, const char* funcName);
