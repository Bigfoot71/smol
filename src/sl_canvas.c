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

/* === Public API === */

sl_canvas_id sl_canvas_create(int w, int h, sl_pixel_format_t format, bool depth)
{
    /* --- Get color target format --- */

    GLenum gl_color_format = GL_RGBA;
    GLenum gl_color_type = GL_UNSIGNED_BYTE;

    switch (format) {
    case SL_PIXEL_FORMAT_LUMINANCE8:
        gl_color_format = GL_LUMINANCE;
        break;
    case SL_PIXEL_FORMAT_ALPHA8:
        gl_color_format = GL_ALPHA;
        break;
    case SL_PIXEL_FORMAT_LUMINANCE_ALPHA8:
        gl_color_format = GL_LUMINANCE_ALPHA;
        break;
    case SL_PIXEL_FORMAT_RGB8:
        gl_color_format = GL_RGB;
        break;
    case SL_PIXEL_FORMAT_RGBA8:
        gl_color_format = GL_RGBA;
        break;
    default:
        break;
    }

    /* --- Create framebuffer's targets --- */

    GLuint targets[2] = { 0 };

    glGenTextures(depth ? 2 : 1, targets);
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, targets[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_color_format, w, h, 0, gl_color_format, gl_color_type, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (depth) {
        glBindTexture(GL_TEXTURE_2D, targets[1]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL_OES, w, h, 0, GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    /* --- Create the framebuffer --- */

    GLuint framebuffer = 0;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targets[0], 0);

    if (depth) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, targets[1], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, targets[1], 0);
    }

    /* --- Check completeness --- */

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(2, targets);
        return 0;
    }

    /* --- Re-bind the previous framebuffer --- */

    if (sl__render.current_canvas != 0) {
        sl__canvas_t* data = sl__registry_get(&sl__render.reg_canvases, sl__render.current_canvas);
        glBindFramebuffer(GL_FRAMEBUFFER, data->framebuffer);
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /* --- Push the textures to the registry --- */

    sl_texture_id color_target = 0;
    sl_texture_id depth_target = 0;

    color_target = sl__registry_add(&sl__render.reg_textures, &(sl__texture_t) { .id = targets[0], .w = w, .h = h });
    if (depth) {
        depth_target = sl__registry_add(&sl__render.reg_textures, &(sl__texture_t) { .id = targets[1], .w = w, .h = h });
    }

    /* --- Create and push canvas to the registry --- */

    sl__canvas_t canvas = { 0 };

    canvas.framebuffer = framebuffer;
    canvas.color = color_target;
    canvas.depth = depth_target;
    canvas.w = w;
    canvas.h = h;

    return sl__registry_add(&sl__render.reg_canvases, &canvas);
}

void sl_canvas_destroy(sl_canvas_id canvas)
{
    if (canvas == 0) {
        return;
    }

    if (sl__render.current_canvas == canvas) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        sl__render.current_canvas = 0;
    }

    sl__canvas_t* data = sl__registry_get(&sl__render.reg_canvases, canvas);
    if (data == NULL) return;

    glDeleteFramebuffers(1, &data->framebuffer);
    glDeleteTextures(data->depth ? 2 : 1, &data->color);

    sl__registry_remove(&sl__render.reg_canvases, canvas);
}

bool sl_canvas_query(sl_canvas_id canvas, sl_texture_id* color, sl_texture_id* depth, int* w, int* h)
{
    sl__canvas_t* data = sl__registry_get(&sl__render.reg_canvases, canvas);
    if (data == NULL) return false;

    if (color) *color = data->color;
    if (depth) *depth = data->depth;
    if (w) *w = data->w;
    if (h) *h = data->h;

    return true;
}
