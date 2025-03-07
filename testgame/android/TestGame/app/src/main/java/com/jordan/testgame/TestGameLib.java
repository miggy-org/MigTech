/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.jordan.testgame;

import android.content.res.AssetManager;

// Wrapper for native library

public class TestGameLib {

     static {
         System.loadLibrary("testgame");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void create(AssetManager mgr, String filesDir);
     public static native void createGraphics();
     public static native void init(int width, int height);
     public static native void step();
     public static native void suspend();
     public static native void resume();
     public static native void destroyGraphics();
     public static native void destroy();

     public static native void pointerPressed(float x, float y);
     public static native void pointerReleased(float x, float y);
     public static native void pointerMoved(float x, float y);
}
