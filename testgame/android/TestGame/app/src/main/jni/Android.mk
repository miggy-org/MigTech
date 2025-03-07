# Copyright (C) 2009 The Android Open Source Project
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
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libtestgame
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := ../../../../../../main.cpp \
				   ../../../../../../cube.cpp \
				   ../../../../../../overlay.cpp \
				   ../../../../../../screen.cpp \
				   ../../../../../../texture.cpp \
				   ../../../../../../../core/AnimList.cpp \
				   ../../../../../../../core/AudioBase.cpp \
				   ../../../../../../../core/BgBase.cpp \
				   ../../../../../../../core/Controls.cpp \
				   ../../../../../../../core/DemoBase.cpp \
				   ../../../../../../../core/Dialog.cpp \
				   ../../../../../../../core/Font.cpp \
				   ../../../../../../../core/Matrix.cpp \
				   ../../../../../../../core/MigBase.cpp \
				   ../../../../../../../core/MigGame.cpp \
				   ../../../../../../../core/MigUtil.cpp \
				   ../../../../../../../core/MovieClip.cpp \
				   ../../../../../../../core/OverlayBase.cpp \
				   ../../../../../../../core/PerfMon.cpp \
				   ../../../../../../../core/PersistBase.cpp \
				   ../../../../../../../core/RenderBase.cpp \
				   ../../../../../../../core/ScreenBase.cpp \
				   ../../../../../../../core/Timer.cpp \
				   ../../../../../../../android/AndroidApp.cpp \
				   ../../../../../../../android/OglImage.cpp \
				   ../../../../../../../android/OglMatrix.cpp \
				   ../../../../../../../android/OglObject.cpp \
				   ../../../../../../../android/OglProgram.cpp \
				   ../../../../../../../android/OglRender.cpp \
				   ../../../../../../../android/OglShader.cpp \
				   ../../../../../../../android/OslAudio.cpp \
				   ../../../../../../../android/OslAudioSound.cpp \
				   ../../../../../../../android/Platform.cpp \
				   ../../../../../../../core/libjpeg/jaricom.c \
				   ../../../../../../../core/libjpeg/jcapimin.c \
				   ../../../../../../../core/libjpeg/jcapistd.c \
				   ../../../../../../../core/libjpeg/jcarith.c \
				   ../../../../../../../core/libjpeg/jccoefct.c \
				   ../../../../../../../core/libjpeg/jccolor.c \
				   ../../../../../../../core/libjpeg/jcdctmgr.c \
				   ../../../../../../../core/libjpeg/jchuff.c \
				   ../../../../../../../core/libjpeg/jcinit.c \
				   ../../../../../../../core/libjpeg/jcmainct.c \
				   ../../../../../../../core/libjpeg/jcmarker.c \
				   ../../../../../../../core/libjpeg/jcmaster.c \
				   ../../../../../../../core/libjpeg/jcomapi.c \
				   ../../../../../../../core/libjpeg/jcparam.c \
				   ../../../../../../../core/libjpeg/jcprepct.c \
				   ../../../../../../../core/libjpeg/jcsample.c \
				   ../../../../../../../core/libjpeg/jctrans.c \
				   ../../../../../../../core/libjpeg/jdapimin.c \
				   ../../../../../../../core/libjpeg/jdapistd.c \
				   ../../../../../../../core/libjpeg/jdarith.c \
				   ../../../../../../../core/libjpeg/jdatadst.c \
				   ../../../../../../../core/libjpeg/jdatasrc.c \
				   ../../../../../../../core/libjpeg/jdcoefct.c \
				   ../../../../../../../core/libjpeg/jdcolor.c \
				   ../../../../../../../core/libjpeg/jddctmgr.c \
				   ../../../../../../../core/libjpeg/jdhuff.c \
				   ../../../../../../../core/libjpeg/jdinput.c \
				   ../../../../../../../core/libjpeg/jdmainct.c \
				   ../../../../../../../core/libjpeg/jdmarker.c \
				   ../../../../../../../core/libjpeg/jdmaster.c \
				   ../../../../../../../core/libjpeg/jdmerge.c \
				   ../../../../../../../core/libjpeg/jdpostct.c \
				   ../../../../../../../core/libjpeg/jdsample.c \
				   ../../../../../../../core/libjpeg/jdtrans.c \
				   ../../../../../../../core/libjpeg/jerror.c \
				   ../../../../../../../core/libjpeg/jfdctflt.c \
				   ../../../../../../../core/libjpeg/jfdctfst.c \
				   ../../../../../../../core/libjpeg/jfdctint.c \
				   ../../../../../../../core/libjpeg/jidctflt.c \
				   ../../../../../../../core/libjpeg/jidctfst.c \
				   ../../../../../../../core/libjpeg/jidctint.c \
				   ../../../../../../../core/libjpeg/jquant1.c \
				   ../../../../../../../core/libjpeg/jquant2.c \
				   ../../../../../../../core/libjpeg/jutils.c \
				   ../../../../../../../core/libjpeg/jmemmgr.c \
				   ../../../../../../../core/libjpeg/jmemnobs.c \
				   ../../../../../../../core/libpng/png.c \
				   ../../../../../../../core/libpng/pngerror.c \
				   ../../../../../../../core/libpng/pngget.c \
				   ../../../../../../../core/libpng/pngmem.c \
				   ../../../../../../../core/libpng/pngpread.c \
				   ../../../../../../../core/libpng/pngread.c \
				   ../../../../../../../core/libpng/pngrio.c \
				   ../../../../../../../core/libpng/pngrtran.c \
				   ../../../../../../../core/libpng/pngrutil.c \
				   ../../../../../../../core/libpng/pngset.c \
				   ../../../../../../../core/libpng/pngtrans.c \
				   ../../../../../../../core/libpng/pngwio.c \
				   ../../../../../../../core/libpng/pngwrite.c \
				   ../../../../../../../core/libpng/pngwtran.c \
				   ../../../../../../../core/libpng/pngwutil.c \
				   ../../../../../../../core/tinyxml/tinyxml2.cpp \
				   ../../../../../../../core/zlib/adler32.c \
				   ../../../../../../../core/zlib/compress.c \
				   ../../../../../../../core/zlib/crc32.c \
				   ../../../../../../../core/zlib/deflate.c \
				   ../../../../../../../core/zlib/gzclose.c \
				   ../../../../../../../core/zlib/gzlib.c \
				   ../../../../../../../core/zlib/gzread.c \
				   ../../../../../../../core/zlib/gzwrite.c \
				   ../../../../../../../core/zlib/infback.c \
				   ../../../../../../../core/zlib/inffast.c \
				   ../../../../../../../core/zlib/inflate.c \
				   ../../../../../../../core/zlib/inftrees.c \
				   ../../../../../../../core/zlib/trees.c \
				   ../../../../../../../core/zlib/uncompr.c \
				   ../../../../../../../core/zlib/zutil.c
LOCAL_LDLIBS    := -llog -lGLESv2 -lEGL -lOpenSLES -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
