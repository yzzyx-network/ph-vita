/*************************************************************************/
/*  context_egl_vita.cpp                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "context_egl_vita.h"
#include <stdio.h>

ContextEGL_Vita::ContextEGL_Vita(bool gles2) {
	this->gles2_context = gles2;
}

ContextEGL_Vita::~ContextEGL_Vita() {
	cleanup();
}

Error ContextEGL_Vita::initialize() {
    // Initialize PVR_PSP2
    PVRSRV_PSP2_APPHINT hint;

    sceKernelLoadStartModule("app0:module/libgpu_es4_ext.suprx", 0, NULL, 0, NULL, NULL);
    sceKernelLoadStartModule("app0:module/libIMGEGL.suprx", 0, NULL, 0, NULL, NULL);

    PVRSRVInitializeAppHint(&hint);
	hint.ui32SwTexOpCleanupDelay = 16000;
    PVRSRVCreateVirtualAppHint(&hint);

	// Connect to the EGL default display
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!display) {
		sceClibPrintf("Could not connect to display! error: %d", eglGetError());
		goto _fail0;
	}

	// Initialize the EGL display connection
	eglInitialize(display, NULL, NULL);

	// Get an appropriate EGL framebuffer configuration
	EGLConfig config;
	EGLint numConfigs;
	static const EGLint attributeList[] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_STENCIL_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	eglChooseConfig(display, attributeList, &config, 1, &numConfigs);
	if (numConfigs == 0) {
		sceClibPrintf("No config found! error: %d", eglGetError());
		goto _fail1;
	}

	// Create an EGL window surface
    nwin.type = PSP2_DRAWABLE_TYPE_WINDOW;
    nwin.numFlipBuffers = 2;
    nwin.flipChainThrdAffinity = 0x20000;
    nwin.windowSize = PSP2_WINDOW_960X544;

    sceClibPrintf("Creating Window\n");
	surface = eglCreateWindowSurface(display, config, &nwin, NULL);
    sceClibPrintf("Created\n");
	if (!surface) {
		sceClibPrintf("Surface creation failed! error: %d", eglGetError());
		goto _fail1;
	}

	static const EGLint contextAttributeList[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	// Create an EGL rendering context
    sceClibPrintf("Creating Context\n");
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributeList);
    sceClibPrintf("Created\n");
	if (!context) {
		sceClibPrintf("Context creation failed! error: %d", eglGetError());
		goto _fail2;
	}

	// Connect the context to the surface
    sceClibPrintf("Make Current\n");
	eglMakeCurrent(display, surface, surface, context);
    sceClibPrintf("Done with all that");
	return OK;

_fail2:
	eglDestroySurface(display, surface);
	surface = NULL;
_fail1:
	eglTerminate(display);
	display = NULL;
_fail0:
	return ERR_UNCONFIGURED;
}

void ContextEGL_Vita::cleanup() {
	if (display) {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (context) {
			eglDestroyContext(display, context);
			context = NULL;
		}
		if (surface) {
			eglDestroySurface(display, surface);
			surface = NULL;
		}
		eglTerminate(display);
		display = NULL;
	}
}

void ContextEGL_Vita::reset() {
	cleanup();
	initialize();
}

void ContextEGL_Vita::release_current() {
	eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void ContextEGL_Vita::make_current() {
	eglMakeCurrent(display, surface, surface, context);
}

int ContextEGL_Vita::get_window_width() {
	return 960;
}

int ContextEGL_Vita::get_window_height() {
	return 544; // todo: stub
}

void ContextEGL_Vita::swap_buffers() {
	eglSwapBuffers(display, surface);
}