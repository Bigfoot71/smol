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

#ifndef SL__CORE_H
#define SL__CORE_H

#include <smol.h>

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

/* === Global State === */

extern struct sl__core {

    SDL_Window* window;
    SDL_GLContext gl;

    Uint64 ticks_now;
    Uint64 ticks_last;

    double target_frame_time;
    double frame_time;
    double total_time;
    double fps_timer;
    int frame_count;
    int fps_average;

    Uint8 keys[SDL_SCANCODE_COUNT];         //< MSB = Current | LSB = Previous
    SDL_MouseButtonFlags mouse_buttons[2];  //< [0] = Current | [1] = Previous

    sl_vec2_t mouse_position;
    sl_vec2_t mouse_delta;
    sl_vec2_t mouse_wheel;

} sl__core;

/* === Module Functions === */

bool sl__core_init(const char* title, int w, int h, const sl_app_desc_t* flags);
void sl__core_quit(void);

#endif // SL__CORE_H
