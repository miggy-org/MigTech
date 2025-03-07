#include <jni.h>

#include "pch.h"
#include "../../../../../../../android/AndroidApp.h"

///////////////////////////////////////////////////////////////////////////
// JNI entry points

extern "C" {
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_create(JNIEnv * env, jobject obj, jobject assetMgr, jstring filesDir);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_createGraphics(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_step(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_suspend(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_resume(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_destroyGraphics(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_destroy(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerPressed(JNIEnv * env, jobject obj,  jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerReleased(JNIEnv * env, jobject obj,  jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerMoved(JNIEnv * env, jobject obj,  jfloat x, jfloat y);
};

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_create(JNIEnv * env, jobject obj, jobject assetMgr, jstring filesDir)
{
	AndroidApp_create(env, assetMgr, filesDir);
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_createGraphics(JNIEnv * env, jobject obj)
{
	AndroidApp_createGraphics();
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
	AndroidApp_init(width, height);
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_step(JNIEnv * env, jobject obj)
{
	AndroidApp_step();
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_suspend(JNIEnv * env, jobject obj)
{
	AndroidApp_suspend();
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_resume(JNIEnv * env, jobject obj)
{
	AndroidApp_resume();
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_destroyGraphics(JNIEnv * env, jobject obj)
{
	AndroidApp_destroyGraphics();
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_destroy(JNIEnv * env, jobject obj)
{
	AndroidApp_destroy(env);
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerPressed(JNIEnv * env, jobject obj,  jfloat x, jfloat y)
{
	AndroidApp_pointerPressed(x, y);
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerReleased(JNIEnv * env, jobject obj,  jfloat x, jfloat y)
{
	AndroidApp_pointerReleased(x, y);
}

JNIEXPORT void JNICALL Java_com_jordan_testgame_TestGameLib_pointerMoved(JNIEnv * env, jobject obj,  jfloat x, jfloat y)
{
	AndroidApp_pointerMoved(x, y);
}
