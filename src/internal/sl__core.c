/**
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "./sl__core.h"

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <glad/gles2.h>

/* === Global State === */

struct sl__core sl__core = { 0 };

/* === Module Functions === */

bool sl__core_init(const char* title, int w, int h, sl_flags_t flags)
{
    /* --- Init SDL stuff --- */

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        sl_loge("CORE: Failed to init video subsystem; %s", SDL_GetError());
        return false;
    }

    /* --- Configure log system --- */

    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_TRACE,    "[T] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_VERBOSE,  "[V] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_DEBUG,    "[D] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_INFO,     "[I] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_WARN,     "[W] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_ERROR,    "[E] ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_CRITICAL, "[F] ");

#ifndef NDEBUG
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
#endif

    /* --- Define OpenGL attributes --- */

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (flags & SL_FLAG_MSAA_X4) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }

    /* --- Create the SDL window --- */

    SDL_WindowFlags windowFlags = 0;
    if (flags & SL_FLAG_FULLSCREEN) windowFlags |= SDL_WINDOW_FULLSCREEN;
    if (flags & SL_FLAG_WINDOW_OCCLUDED) windowFlags |= SDL_WINDOW_OCCLUDED;
    if (flags & SL_FLAG_WINDOW_HIDDEN) windowFlags |= SDL_WINDOW_HIDDEN;
    if (flags & SL_FLAG_WINDOW_BORDERLESS) windowFlags |= SDL_WINDOW_BORDERLESS;
    if (flags & SL_FLAG_WINDOW_RESIZABLE) windowFlags |= SDL_WINDOW_RESIZABLE;
    if (flags & SL_FLAG_WINDOW_MINIMIZED) windowFlags |= SDL_WINDOW_MINIMIZED;
    if (flags & SL_FLAG_WINDOW_MAXIMIZED) windowFlags |= SDL_WINDOW_MAXIMIZED;
    if (flags & SL_FLAG_WINDOW_TOPMOST) windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (flags & SL_FLAG_WINDOW_TRANSPARENT) windowFlags |= SDL_WINDOW_TRANSPARENT;
    if (flags & SL_FLAG_WINDOW_NOT_FOCUSABLE) windowFlags |= SDL_WINDOW_NOT_FOCUSABLE;
    if (flags & SL_FLAG_MOUSE_GRABBED) windowFlags |= SDL_WINDOW_MOUSE_GRABBED;
    if (flags & SL_FLAG_MOUSE_CAPTURE) windowFlags |= SDL_WINDOW_MOUSE_CAPTURE;
    if (flags & SL_FLAG_MOUSE_RELATIVE) windowFlags |= SDL_WINDOW_MOUSE_RELATIVE_MODE;
    if (flags & SL_FLAG_MOUSE_FOCUS) windowFlags |= SDL_WINDOW_MOUSE_FOCUS;
    if (flags & SL_FLAG_INPUT_FOCUS) windowFlags |= SDL_WINDOW_INPUT_FOCUS;
    if (flags & SL_FLAG_KEYBOARD_GRABBED) windowFlags |= SDL_WINDOW_KEYBOARD_GRABBED;
    if (flags & SL_FLAG_HIGH_PIXEL_DENSITY) windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    sl__core.window = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | windowFlags);
    if (!sl__core.window) {
        sl_loge("CORE: Failed to create window; %s", SDL_GetError());
        return false;
    }

    SDL_SetWindowPosition(sl__core.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    /* --- Create GLES2 context --- */

    sl__core.gl = SDL_GL_CreateContext(sl__core.window);
    if (!sl__core.gl) {
        sl_loge("CORE: Failed to create GLES2 context: %s", SDL_GetError());
        SDL_DestroyWindow(sl__core.window);
        return false;
    }

    /* --- Load GLES2 functions --- */

    if (gladLoadGLES2(SDL_GL_GetProcAddress) < 0) {
        sl_loge("CORE: Failed to load GLES2 functions");
        SDL_GL_DestroyContext(sl__core.gl);
        SDL_DestroyWindow(sl__core.window);
        return false;
    }

    /* --- Print debug infos --- */

#ifndef NDEBUG
    sl_logd("CORE: GL Vendor   : %s", glGetString(GL_VENDOR));
    sl_logd("CORE: GL Renderer : %s", glGetString(GL_RENDERER));
    sl_logd("CORE: GL Version  : %s", glGetString(GL_VERSION));
    sl_logd("CORE: GL Shading Language Version : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif

    /* --- Yayyy! --- */

    return true;
}

void sl__core_quit(void)
{
    if (sl__core.gl) {
        SDL_GL_DestroyContext(sl__core.gl);
        sl__core.gl = NULL;
    }

    if (sl__core.window) {
        SDL_DestroyWindow(sl__core.window);
        sl__core.window = NULL;
    }

    SDL_Quit();
}
