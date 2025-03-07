#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

project(Cuboingo)

cmake_minimum_required(VERSION 3.4.1)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

add_library(mtcore STATIC
		../../../../../../../core/AnimList.cpp
		../../../../../../../core/AudioBase.cpp
		../../../../../../../core/BgBase.cpp
		../../../../../../../core/Controls.cpp
		../../../../../../../core/DemoBase.cpp
		../../../../../../../core/Dialog.cpp
		../../../../../../../core/Font.cpp
		../../../../../../../core/Matrix.cpp
		../../../../../../../core/MigBase.cpp
		../../../../../../../core/MigGame.cpp
		../../../../../../../core/MigUtil.cpp
		../../../../../../../core/MovieClip.cpp
		../../../../../../../core/OverlayBase.cpp
		../../../../../../../core/PerfMon.cpp
		../../../../../../../core/PersistBase.cpp
		../../../../../../../core/RenderBase.cpp
		../../../../../../../core/ScreenBase.cpp
		../../../../../../../core/Timer.cpp
		../../../../../../../android/AndroidApp.cpp
		../../../../../../../android/OglImage.cpp
		../../../../../../../android/OglMatrix.cpp
		../../../../../../../android/OglObject.cpp
		../../../../../../../android/OglProgram.cpp
		../../../../../../../android/OglRender.cpp
		../../../../../../../android/OglShader.cpp
		../../../../../../../android/OslAudio.cpp
		../../../../../../../android/OslAudioSound.cpp
		../../../../../../../android/Platform.cpp)

add_library(libjpeg STATIC
		../../../../../../../core/libjpeg/jaricom.c
		../../../../../../../core/libjpeg/jcapimin.c
		../../../../../../../core/libjpeg/jcapistd.c
		../../../../../../../core/libjpeg/jcarith.c
		../../../../../../../core/libjpeg/jccoefct.c
		../../../../../../../core/libjpeg/jccolor.c
		../../../../../../../core/libjpeg/jcdctmgr.c
		../../../../../../../core/libjpeg/jchuff.c
		../../../../../../../core/libjpeg/jcinit.c
		../../../../../../../core/libjpeg/jcmainct.c
		../../../../../../../core/libjpeg/jcmarker.c
		../../../../../../../core/libjpeg/jcmaster.c
		../../../../../../../core/libjpeg/jcomapi.c
		../../../../../../../core/libjpeg/jcparam.c
		../../../../../../../core/libjpeg/jcprepct.c
		../../../../../../../core/libjpeg/jcsample.c
		../../../../../../../core/libjpeg/jctrans.c
		../../../../../../../core/libjpeg/jdapimin.c
		../../../../../../../core/libjpeg/jdapistd.c
		../../../../../../../core/libjpeg/jdarith.c
		../../../../../../../core/libjpeg/jdatadst.c
		../../../../../../../core/libjpeg/jdatasrc.c
		../../../../../../../core/libjpeg/jdcoefct.c
		../../../../../../../core/libjpeg/jdcolor.c
		../../../../../../../core/libjpeg/jddctmgr.c
		../../../../../../../core/libjpeg/jdhuff.c
		../../../../../../../core/libjpeg/jdinput.c
		../../../../../../../core/libjpeg/jdmainct.c
		../../../../../../../core/libjpeg/jdmarker.c
		../../../../../../../core/libjpeg/jdmaster.c
		../../../../../../../core/libjpeg/jdmerge.c
		../../../../../../../core/libjpeg/jdpostct.c
		../../../../../../../core/libjpeg/jdsample.c
		../../../../../../../core/libjpeg/jdtrans.c
		../../../../../../../core/libjpeg/jerror.c
		../../../../../../../core/libjpeg/jfdctflt.c
		../../../../../../../core/libjpeg/jfdctfst.c
		../../../../../../../core/libjpeg/jfdctint.c
		../../../../../../../core/libjpeg/jidctflt.c
		../../../../../../../core/libjpeg/jidctfst.c
		../../../../../../../core/libjpeg/jidctint.c
		../../../../../../../core/libjpeg/jquant1.c
		../../../../../../../core/libjpeg/jquant2.c
		../../../../../../../core/libjpeg/jutils.c
		../../../../../../../core/libjpeg/jmemmgr.c
		../../../../../../../core/libjpeg/jmemnobs.c)

add_compile_options(-DPNG_ARM_NEON_OPT=0)

add_library(libpng STATIC
		../../../../../../../core/libpng/png.c
		../../../../../../../core/libpng/pngerror.c
		../../../../../../../core/libpng/pngget.c
		../../../../../../../core/libpng/pngmem.c
		../../../../../../../core/libpng/pngpread.c
		../../../../../../../core/libpng/pngread.c
		../../../../../../../core/libpng/pngrio.c
		../../../../../../../core/libpng/pngrtran.c
		../../../../../../../core/libpng/pngrutil.c
		../../../../../../../core/libpng/pngset.c
		../../../../../../../core/libpng/pngtrans.c
		../../../../../../../core/libpng/pngwio.c
		../../../../../../../core/libpng/pngwrite.c
		../../../../../../../core/libpng/pngwtran.c
		../../../../../../../core/libpng/pngwutil.c)

add_library(tinyxml STATIC
		../../../../../../../core/tinyxml/tinyxml2.cpp)

add_library(zlib STATIC
		../../../../../../../core/zlib/adler32.c
		../../../../../../../core/zlib/compress.c
		../../../../../../../core/zlib/crc32.c
		../../../../../../../core/zlib/deflate.c
		../../../../../../../core/zlib/gzclose.c
		../../../../../../../core/zlib/gzlib.c
		../../../../../../../core/zlib/gzread.c
		../../../../../../../core/zlib/gzwrite.c
		../../../../../../../core/zlib/infback.c
		../../../../../../../core/zlib/inffast.c
		../../../../../../../core/zlib/inflate.c
		../../../../../../../core/zlib/inftrees.c
		../../../../../../../core/zlib/trees.c
		../../../../../../../core/zlib/uncompr.c
		../../../../../../../core/zlib/zutil.c)

add_library(cuboingo SHARED
    ../../../../../../CreditsScreen.cpp
	../../../../../../CubeBase.cpp
	../../../../../../CubeUtil.cpp
	../../../../../../CuboingoGame.cpp
	../../../../../../DemoCuboingoScreens.cpp
	../../../../../../EndGameScreens.cpp
	../../../../../../FallingGrid.cpp
	../../../../../../GameCube.cpp
	../../../../../../GameGrid.cpp
	../../../../../../GameOverlays.cpp
	../../../../../../GameScreen.cpp
	../../../../../../GameScripts.cpp
	../../../../../../GridBase.cpp
	../../../../../../HintGrid.cpp
	../../../../../../Launcher.cpp
	../../../../../../LightBeam.cpp
	../../../../../../Particles.cpp
	../../../../../../PowerUp.cpp
	../../../../../../ScoreKeeper.cpp
	../../../../../../ShadowPass.cpp
	../../../../../../SplashCube.cpp
	../../../../../../SplashScreen.cpp
	../../../../../../Stamp.cpp
	../../../../../../StartOverlays.cpp)

target_include_directories(mtcore PRIVATE
		${ANDROID_NDK}/sources/android/native_app_glue
		../../../../../../../android/
		../../../../../../../core/)

target_include_directories(cuboingo PRIVATE
		../../../../../../../android/
		../../../../../../../core/)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
		"${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# add lib dependencies
target_link_libraries(cuboingo
	mtcore
	libjpeg
	libpng
	tinyxml
	zlib
    android
    native_app_glue
    EGL
    GLESv2
    log
    OpenSLES)
