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

sl_texture_id sl_texture_create(const void* pixels, int w, int h, sl_pixel_format_t format)
{
    if (!pixels || w <= 0 || h <= 0) {
        sl_loge("TEXTURE: Failed to create texture; Invalid input parameters");
        return 0;
    }

    /* --- Generate texture --- */

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    /* --- Setup texture --- */

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    GLenum gl_internal_format = GL_RGBA;
    GLenum gl_format = GL_RGBA;
    GLenum gl_type = GL_UNSIGNED_BYTE;

    switch (format) {
    case SL_PIXEL_FORMAT_LUMINANCE8:
        gl_internal_format = gl_format = GL_LUMINANCE;
        break;
    case SL_PIXEL_FORMAT_ALPHA8:
        gl_internal_format = gl_format = GL_ALPHA;
        break;
    case SL_PIXEL_FORMAT_LUMINANCE_ALPHA8:
        gl_internal_format = gl_format = GL_LUMINANCE_ALPHA;
        break;
    case SL_PIXEL_FORMAT_RGB8:
        gl_internal_format = gl_format = GL_RGB;
        break;
    case SL_PIXEL_FORMAT_RGBA8:
        gl_internal_format = gl_format = GL_RGBA;
        break;
    default:
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, w, h, 0, gl_format, gl_type, pixels);

    /* --- Push texture to the registry --- */

    sl__texture_t tex = {
        .id = texture,
        .w = w,
        .h = h
    };

    return sl__registry_add(&sl__render.reg_textures, &tex);
}

sl_texture_id sl_texture_load(const char* file_path, int* w, int* h)
{
    sl_image_t image = { 0 };
    sl_image_load(&image, file_path);
    if (image.pixels == NULL) {
        sl_loge("TEXTURE: Failed to load texture; Unable to load image '%s'", file_path);
        return 0;
    }

    sl_texture_id texture = sl_texture_create(image.pixels, image.w, image.h, image.format);
    if (texture == 0) {
        return 0;
    }

    if (w) *w = image.w;
    if (h) *h = image.h;

    sl_image_destroy(&image);

    return texture;
}

sl_texture_id sl_texture_load_from_memory(const sl_image_t* image)
{
    if (image == NULL || image->pixels == NULL) {
        sl_loge("TEXTURE: Failed to load texture from memory; Invalid input parameters");
        return 0;
    }

    sl_texture_id texture = sl_texture_create(image->pixels, image->w, image->h, image->format);
    if (texture == 0) {
        return 0;
    }

    return texture;
}

void sl_texture_destroy(sl_texture_id texture)
{
    if (texture == 0 || texture == sl__render.default_texture) {
        return;
    }

    if (sl__render.current_texture == texture) {
        sl__render.current_texture = sl__render.default_texture;
    }

    sl__texture_t* data = sl__registry_get(&sl__render.reg_textures, texture);
    if (data == NULL) return;

    glDeleteTextures(1, &data->id);

    sl__registry_remove(&sl__render.reg_textures, texture);
}

void sl_texture_generate_mipmap(sl_texture_id texture)
{
    sl__texture_t* data = sl__registry_get(&sl__render.reg_textures, texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->id);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void sl_texture_parameters(sl_texture_id texture, sl_filter_mode_t filter, sl_wrap_mode_t wrap)
{
    if (texture == 0 || texture == sl__render.default_texture) {
        return;
    }

    sl__texture_t* data = sl__registry_get(&sl__render.reg_textures, texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->id);

    switch (filter) {
    case SL_FILTER_NEAREST:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case SL_FILTER_BILINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case SL_FILTER_TRILINEAR:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }

    switch (wrap) {
    case SL_WRAP_CLAMP:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case SL_WRAP_REPEAT:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }
}

bool sl_texture_query(sl_texture_id texture, int* w, int* h)
{
    sl__texture_t* data = sl__registry_get(&sl__render.reg_textures, texture);
    if (data == NULL) return false;
    if (w) *w = data->w;
    if (h) *h = data->h;
    return true;
}
