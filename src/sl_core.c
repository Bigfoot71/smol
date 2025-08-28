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

#include <smol.h>

#include "./internal/sl__render.h"
#include "./internal/sl__audio.h"
#include "./internal/sl__core.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>

bool sl_init(const char* title, int w, int h, sl_flags_t flags)
{
    sl_app_desc_t desc = {
        .flags = flags,
        .name = NULL,
        .version = NULL,
        .identifier = NULL,
        .memory = {}
    };

    return sl_init_ex(title, w, h, &desc);
}

bool sl_init_ex(const char* title, int w, int h, const sl_app_desc_t* desc)
{
    /* --- Ensures that the application description is valid --- */

    if (desc == NULL) {
        sl_loge("CORE: Failed to initialize Hyperion; App description cannot be null");
        return false;
    }

    /* --- Fill global states with zeroes --- */

    SDL_memset(&sl__render, 0, sizeof(sl__render));
    SDL_memset(&sl__audio, 0, sizeof(sl__audio));
    SDL_memset(&sl__core, 0, sizeof(sl__core));

    /* --- Init each modules --- */

    if (!sl__core_init(title, w, h, desc)) {
        return false;
    }

    if (!sl__render_init(w, h)) {
        return false;
    }

    if (!sl__audio_init()) {
        return false;
    }

    /* --- Init the random generator state --- */

    SDL_Time ticks = 0;
    SDL_GetCurrentTime(&ticks);
    sl_rand_seed(*(uint64_t*)(&ticks));

    /* --- Get first ticks at the end of init --- */

    sl__core.ticks_now = SDL_GetTicks();

    /* --- Oh yeaaaah :3 --- */

    return true;
}

void sl_quit(void)
{
    sl__audio_quit();
    sl__render_quit();
    sl__core_quit();
}

bool sl_frame_step(void)
{
    bool should_run = true;

    /* --- Framerate control --- */

    sl__core.ticks_now = SDL_GetTicks();
    sl__core.frame_time = (double)(sl__core.ticks_now - sl__core.ticks_last);
    if (sl__core.frame_time < sl__core.target_frame_time) {
        SDL_Delay((Uint32)(sl__core.target_frame_time - sl__core.frame_time));
        sl__core.ticks_now = SDL_GetTicks();
        sl__core.frame_time = (double)(sl__core.ticks_now - sl__core.ticks_last);
    }
    sl__core.ticks_last = sl__core.ticks_now;

    /* --- Time / FPS Counter --- */

    sl__core.total_time += sl__core.frame_time;
    sl__core.fps_timer += sl__core.frame_time;
    sl__core.frame_count++;

    if (sl__core.fps_timer >= 1000.0) {
        sl__core.fps_average = sl__core.frame_count / (sl__core.fps_timer / 1000.0f);
        sl__core.frame_count = 0;
        sl__core.fps_timer = 0;
    }

    /* --- Update input state --- */

    // Shift current >> previous state
    for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
        sl__core.keys[i] = (sl__core.keys[i] & 0xF0) | (sl__core.keys[i] >> 4);
    }

    sl__core.mouse_buttons[1] = sl__core.mouse_buttons[0];
    sl__core.mouse_wheel = SL_VEC2_ZERO;
    sl__core.mouse_delta = SL_VEC2_ZERO;

    /* --- Update system events --- */

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_EVENT_QUIT:
            should_run = false;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            glViewport(0, 0, ev.window.data1, ev.window.data2);
            if (!sl__render.use_custom_proj) {
                sl__render.matrix_proj = sl_mat4_ortho(0, ev.window.data1, ev.window.data2, 0, 0, 1);
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            sl__core.keys[ev.key.scancode] |= 0xF0;
            break;
        case SDL_EVENT_KEY_UP:
            sl__core.keys[ev.key.scancode] &= 0x0F;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            sl__core.mouse_buttons[0] |= SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            sl__core.mouse_buttons[0] &= ~SDL_BUTTON_MASK(ev.button.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            sl__core.mouse_position.x = ev.motion.x;
            sl__core.mouse_position.y = ev.motion.y;
            sl__core.mouse_delta.x = ev.motion.xrel;
            sl__core.mouse_delta.y = ev.motion.yrel;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            sl__core.mouse_wheel.x = ev.wheel.x;
            sl__core.mouse_wheel.y = ev.wheel.y;
            break;
        default:
            break;
        }
    }

    return should_run;
}

double sl_time(void)
{
    return 0.001 * sl__core.total_time;
}

double sl_frame_time(void)
{
    return 0.001 * sl__core.frame_time;
}

void sl_frame_target(int fps)
{
    sl__core.target_frame_time = 1.0 / (double)fps;
}

int sl_frame_per_second(void)
{
    return sl__core.fps_average;
}

float sl_display_get_scale(void)
{
    return SDL_GetWindowDisplayScale(sl__core.window);
}

float sl_display_get_dpi(void)
{
    float displayScale = SDL_GetWindowDisplayScale(sl__core.window);

#if defined(__ANDROID__) || defined(__IPHONEOS__)
    return displayScale * 160.0f;
#else
    return displayScale * 96.0f;
#endif
}

int sl_display_get_index(void)
{
    return SDL_GetDisplayForWindow(sl__core.window);
}

sl_vec2_t sl_display_get_size(void)
{
    int displayIndex = SDL_GetDisplayForWindow(sl__core.window);

    SDL_Rect bounds;
    SDL_GetDisplayBounds(displayIndex, &bounds);

    return SL_VEC2(bounds.x, bounds.y);
}

const char* sl_window_get_title(void)
{
    return SDL_GetWindowTitle(sl__core.window);
}

void sl_window_set_title(const char* title)
{
    SDL_SetWindowTitle(sl__core.window, title);
}

int sl_window_get_width(void)
{
    int w = 0;
    SDL_GetWindowSize(sl__core.window, &w, NULL);
    return w;
}

int sl_window_get_height(void)
{
    int h = 0;
    SDL_GetWindowSize(sl__core.window, NULL, &h);
    return h;
}

sl_vec2_t sl_window_get_size(void)
{
    int w = 0, h = 0;
    SDL_GetWindowSize(sl__core.window, &w, &h);
    return SL_VEC2(w, h);
}

void sl_window_set_size(int w, int h)
{
    SDL_SetWindowSize(sl__core.window, w, h);
}

void sl_window_set_min_size(int w, int h)
{
    SDL_SetWindowMinimumSize(sl__core.window, w, h);
}

void sl_window_set_max_size(int w, int h)
{
    SDL_SetWindowMaximumSize(sl__core.window, w, h);
}

sl_vec2_t sl_window_get_position(void)
{
    int x = 0, y = 0;
    SDL_GetWindowPosition(sl__core.window, &x, &y);
    return SL_VEC2(x, y);
}

void sl_window_set_position(int x, int y)
{
    SDL_SetWindowPosition(sl__core.window, x, y);
}

bool sl_window_is_fullscreen(void)
{
    Uint64 flags = SDL_GetWindowFlags(sl__core.window);
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void sl_window_set_fullscreen(bool enabled)
{
    SDL_SetWindowFullscreen(sl__core.window, enabled);
}

bool sl_window_is_resizable(void)
{
    Uint64 flags = SDL_GetWindowFlags(sl__core.window);
    return (flags & SDL_WINDOW_RESIZABLE) != 0;
}

void sl_window_set_resizable(bool resizable)
{
    SDL_SetWindowResizable(sl__core.window, resizable);
}

bool sl_window_is_visible(void)
{
    Uint64 flags = SDL_GetWindowFlags(sl__core.window);
    return (flags & SDL_WINDOW_HIDDEN) == 0;
}

void sl_window_minimize(void)
{
    SDL_MinimizeWindow(sl__core.window);
}

void sl_window_maximize(void)
{
    SDL_MaximizeWindow(sl__core.window);
}

void sl_window_restore(void)
{
    SDL_RestoreWindow(sl__core.window);
}

void sl_window_show(void)
{
    SDL_ShowWindow(sl__core.window);
}

void sl_window_hide(void)
{
    SDL_HideWindow(sl__core.window);
}

bool sl_window_is_focused(void)
{
    Uint64 flags = SDL_GetWindowFlags(sl__core.window);
    return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void sl_window_focus(void)
{
    SDL_RaiseWindow(sl__core.window);
}

bool sl_window_is_bordered(void)
{
    Uint64 flags = SDL_GetWindowFlags(sl__core.window);
    return (flags & SDL_WINDOW_BORDERLESS) == 0;
}

void sl_window_set_bordered(bool bordered)
{
    SDL_SetWindowBordered(sl__core.window, bordered);
}

bool sl_cursor_is_grabbed(void)
{
    return SDL_GetWindowMouseGrab(sl__core.window);
}

void sl_cursor_grab(bool grab)
{
    SDL_SetWindowMouseGrab(sl__core.window, grab);
}

void sl_cursor_show(void)
{
    SDL_ShowCursor();
}

void sl_cursor_hide(void)
{
    SDL_HideCursor();
}

bool sl_cursor_is_visible(void)
{
    return SDL_CursorVisible();
}

void sl_mouse_capture(bool enabled)
{
    SDL_SetWindowRelativeMouseMode(sl__core.window, enabled);
}

bool sl_mouse_button_is_pressed(sl_mouse_button_t button)
{
    return sl__core.mouse_buttons[0] & SDL_BUTTON_MASK(button);
}

bool sl_mouse_button_is_released(sl_mouse_button_t button)
{
    return !(sl__core.mouse_buttons[0] & SDL_BUTTON_MASK(button));
}

bool sl_mouse_button_just_pressed(sl_mouse_button_t button)
{
    button = SDL_BUTTON_MASK(button);
    return (sl__core.mouse_buttons[0] & button)
       && !(sl__core.mouse_buttons[1] & button);
}

bool sl_mouse_button_just_released(sl_mouse_button_t button)
{
    button = SDL_BUTTON_MASK(button);
    return !(sl__core.mouse_buttons[0] & button)
         && (sl__core.mouse_buttons[1] & button);
}

sl_vec2_t sl_mouse_get_position(void)
{
    return sl__core.mouse_position;
}

void sl_mouse_set_position(sl_vec2_t p)
{
    SDL_WarpMouseInWindow(sl__core.window, p.x, p.y);
    sl__core.mouse_position = p;
}

sl_vec2_t sl_mouse_get_delta(void)
{
    return sl__core.mouse_delta;
}

sl_vec2_t sl_mouse_get_wheel(void)
{
    return sl__core.mouse_wheel;
}

bool sl_key_is_pressed(sl_key_t key)
{
    if (key >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return (sl__core.keys[key] & 0xF0);
}

bool sl_key_is_released(sl_key_t key)
{
    if (key < 0 || key >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return !(sl__core.keys[key] & 0xF0);
}

bool sl_key_just_pressed(sl_key_t key)
{
    if (key < 0 || key >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return (sl__core.keys[key] & 0xF0)
       && !(sl__core.keys[key] & 0x0F);
}

bool sl_key_just_released(sl_key_t key)
{
    if (key < 0 || key >= SDL_SCANCODE_COUNT) {
        return false;
    }

    return !(sl__core.keys[key] & 0xF0)
         && (sl__core.keys[key] & 0x0F);
}

sl_vec2_t sl_key_vector(sl_key_t left, sl_key_t right, sl_key_t up, sl_key_t down)
{
    int x = sl_key_is_pressed(right) - sl_key_is_pressed(left);
    int y = sl_key_is_pressed(down) - sl_key_is_pressed(up);

    return sl_vec2_normalize(SL_VEC2(x, y));
}

const char* sl_file_base_path(void)
{
    return SDL_GetBasePath();
}

void* sl_file_load(const char* file_path, size_t* size)
{
    if (!file_path || !size) {
        return NULL;
    }

    SDL_IOStream* file = SDL_IOFromFile(file_path, "rb");
    if (!file) {
        *size = 0;
        return NULL;
    }

    Sint64 file_size = SDL_GetIOSize(file);
    if (file_size < 0) {
        SDL_CloseIO(file);
        *size = 0;
        return NULL;
    }

    void* buffer = SDL_malloc((size_t)file_size);
    if (!buffer) {
        SDL_CloseIO(file);
        *size = 0;
        return NULL;
    }

    size_t bytes_read = SDL_ReadIO(file, buffer, (size_t)file_size);
    SDL_CloseIO(file);

    if (bytes_read != (size_t)file_size) {
        SDL_free(buffer);
        *size = 0;
        return NULL;
    }

    *size = (size_t)file_size;
    return buffer;
}

char* sl_file_load_text(const char* file_path)
{
    if (!file_path) {
        return NULL;
    }

    SDL_IOStream* file = SDL_IOFromFile(file_path, "r");
    if (!file) {
        return NULL;
    }

    Sint64 file_size = SDL_GetIOSize(file);
    if (file_size < 0) {
        SDL_CloseIO(file);
        return NULL;
    }

    char* buffer = (char*)SDL_malloc((size_t)file_size + 1);
    if (!buffer) {
        SDL_CloseIO(file);
        return NULL;
    }

    size_t bytes_read = SDL_ReadIO(file, buffer, (size_t)file_size);
    SDL_CloseIO(file);

    if (bytes_read != (size_t)file_size) {
        SDL_free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';

    return buffer;
}

bool sl_file_write(const char* file_path, const void* data, size_t size)
{
    if (!file_path || !data || size == 0) {
        return false;
    }

    SDL_IOStream* file = SDL_IOFromFile(file_path, "wb");
    if (!file) {
        return false;
    }

    size_t bytes_written = SDL_WriteIO(file, data, size);
    SDL_CloseIO(file);

    return bytes_written == size;
}

bool sl_file_write_text(const char* file_path, const char* data, size_t size)
{
    if (!file_path || !data || size == 0) {
        return false;
    }

    SDL_IOStream* file = SDL_IOFromFile(file_path, "w");
    if (!file) {
        return false;
    }

    size_t bytes_written = SDL_WriteIO(file, data, size);
    SDL_CloseIO(file);

    return bytes_written == size;
}

void sl_log_priority(sl_log_t log)
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log);
}

void sl_log(sl_log_t log, const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
    va_end(args);
}

void sl_log_v(sl_log_t log, const char* msg, va_list args)
{
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, (SDL_LogPriority)log, msg, args);
}

void sl_logt(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void sl_logv(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, msg, args);
    va_end(args);
}

void sl_logd(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, msg, args);
    va_end(args);
}

void sl_logi(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, msg, args);
    va_end(args);
}

void sl_logw(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, msg, args);
    va_end(args);
}

void sl_loge(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, msg, args);
    va_end(args);
}

void sl_logf(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, msg, args);
    va_end(args);
}

void* sl_malloc(size_t size)
{
    return SDL_malloc(size);
}

void* sl_calloc(size_t nmemb, size_t size)
{
    return SDL_calloc(nmemb, size);
}

void* sl_realloc(void* ptr, size_t size)
{
    return SDL_realloc(ptr, size);
}

void sl_free(void* ptr)
{
    SDL_free(ptr);
}
