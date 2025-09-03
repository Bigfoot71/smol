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

#include "./sl__render.h"

/* === Global State === */

struct sl__render sl__render = { 0 };

/* === Module Functions === */

bool sl__render_init(int w, int h)
{
    /* --- Create registries --- */

    sl__render.reg_textures = sl__registry_create(32, sizeof(sl__texture_t));
    sl__render.reg_canvases = sl__registry_create(4, sizeof(sl__canvas_t));
    sl__render.reg_shaders = sl__registry_create(8, sizeof(sl__shader_t));
    sl__render.reg_meshes = sl__registry_create(8, sizeof(sl__mesh_t));
    sl__render.reg_fonts = sl__registry_create(4, sizeof(sl__font_t));

    /* --- Init default values --- */

    sl__render.current_texture = sl__render.default_texture = sl_texture_create((uint8_t[]){255}, 1, 1, SL_PIXEL_FORMAT_LUMINANCE8);
    sl__render.current_shader = sl__render.default_shader = sl_shader_create(NULL);
    sl__render.current_blend_mode = SL_BLEND_OPAQUE;
    sl__render.current_color = SL_WHITE;

    sl__render.matrix_proj = sl_mat4_ortho(0, w, h, 0, 0, 1);
    sl__render.matrix_transform = SL_MAT4_IDENTITY;
    sl__render.matrix_texture = SL_MAT4_IDENTITY;
    sl__render.matrix_normal = SL_MAT4_IDENTITY;
    sl__render.matrix_view = SL_MAT4_IDENTITY;

    sl__render.transform_is_identity = true;
    sl__render.texture_is_identity = true;

    /* --- Create batch buffers --- */

    glGenBuffers(1, &sl__render.vbo);
    glGenBuffers(1, &sl__render.ebo);

    glBindBuffer(GL_ARRAY_BUFFER, sl__render.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sl__render.vertex_buffer), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sl__render.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sl__render.index_buffer), NULL, GL_DYNAMIC_DRAW);

    /* --- Yayyy! --- */

    return true;
}

void sl__render_quit(void)
{
    /* --- Release non destroyed objects --- */

    for (int i = 0; i < sl__render.reg_canvases.elements.count; i++) {
        if (((bool*)sl__render.reg_canvases.valid_flags.data)[i]) {
            sl_canvas_destroy(i + 1);
        }
    }

    for (int i = 0; i < sl__render.reg_fonts.elements.count; i++) {
        if (((bool*)sl__render.reg_fonts.valid_flags.data)[i]) {
            sl_font_destroy(i + 1);
        }
    }

    for (int i = 0; i < sl__render.reg_shaders.elements.count; i++) {
        if (((bool*)sl__render.reg_shaders.valid_flags.data)[i]) {
            sl_shader_destroy(i + 1);
        }
    }

    for (int i = 0; i < sl__render.reg_meshes.elements.count; i++) {
        if (((bool*)sl__render.reg_meshes.valid_flags.data)[i]) {
            sl_mesh_destroy(i + 1);
        }
    }

    for (int i = 0; i < sl__render.reg_textures.elements.count; i++) {
        if (((bool*)sl__render.reg_textures.valid_flags.data)[i]) {
            sl_texture_destroy(i + 1);
        }
    }

    /* --- Release registries --- */

    sl__registry_destroy(&sl__render.reg_fonts);
    sl__registry_destroy(&sl__render.reg_shaders);
    sl__registry_destroy(&sl__render.reg_canvases);
    sl__registry_destroy(&sl__render.reg_textures);

    /* --- Release batch buffer objects --- */

    if (sl__render.ebo) {
        glDeleteBuffers(1, &sl__render.ebo);
        sl__render.ebo = 0;
    }

    if (sl__render.vbo) {
        glDeleteBuffers(1, &sl__render.vbo);
        sl__render.vbo = 0;
    }
}

/* === Font Functions === */

const sl__glyph_t* sl__glyph_info(const sl__font_t* font, int codepoint)
{
#   define FALLBACK 63 //< Fallback is '?'

    int index = 0, fallback_index = 0;

    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].value == codepoint) {
            index = i;
            break;
        }
        else if (font->glyphs[i].value == FALLBACK) {
            fallback_index = i;
        }
    }

    if (index == 0 && font->glyphs[0].value != codepoint) {
        index = fallback_index;
    }

    return &font->glyphs[index];
}

void sl__font_measure_text(float* w, float* h, const sl__font_t* font, const char* text, float font_size, float x_spacing, float y_spacing)
{
    if ((!w && !h) || font == NULL || text == NULL) {
        return;
    }

    float scale = font_size / font->base_size;

    float max_width = 0.0f;
    float current_width = 0.0f;
    float text_height = font_size;

    int max_chars_in_line = 0;
    int current_chars_in_line = 0;
    int text_length = (int)strlen(text);

    for (int i = 0; i < text_length;)
    {
        int codepoint_byte_count = 0;
        int letter = sl_codepoint_next(&text[i], &codepoint_byte_count);
        i += codepoint_byte_count;

        if (letter == '\n') {
            max_width = fmaxf(max_width, current_width);
            max_chars_in_line = SL_MAX(max_chars_in_line, current_chars_in_line);
            current_width = 0.0f;
            current_chars_in_line = 0;
            text_height += font_size + y_spacing;
        }
        else if (w != NULL) {
            const sl__glyph_t* glyph = sl__glyph_info(font, letter);
            float char_width = (glyph->x_advance > 0) ? glyph->x_advance : (glyph->w_atlas + glyph->x_offset);
            current_width += char_width;
            current_chars_in_line++;
        }
    }

    // Treat the last line
    max_width = fmaxf(max_width, current_width);
    max_chars_in_line = SL_MAX(max_chars_in_line, current_chars_in_line);

    if (w) *w = max_width * scale + (max_chars_in_line > 0 ? (max_chars_in_line - 1) * x_spacing : 0);
    if (h) *h = text_height;
}

void sl__font_measure_codepoints(float* w, float* h, const sl__font_t* font, const int* codepoints, int length, float font_size, float x_spacing, float y_spacing)
{
    float scale = font_size / font->base_size;

    float max_width = 0.0f;
    float current_width = 0.0f;
    float text_height = font_size;

    int max_chars_in_line = 0;
    int current_chars_in_line = 0;

    for (int i = 0; i < length; i++)
    {
        int letter = codepoints[i];

        if (letter == '\n') {
            max_width = fmaxf(max_width, current_width);
            max_chars_in_line = SL_MAX(max_chars_in_line, current_chars_in_line);
            current_width = 0.0f;
            current_chars_in_line = 0;
            text_height += font_size + y_spacing;
        }
        else {
            const sl__glyph_t* glyph = sl__glyph_info(font, letter);
            float char_width = (glyph->x_advance > 0) ? glyph->x_advance : (glyph->w_atlas + glyph->x_offset);
            current_width += char_width;
            current_chars_in_line++;
        }
    }

    // Treat the last line
    max_width = fmaxf(max_width, current_width);
    max_chars_in_line = SL_MAX(max_chars_in_line, current_chars_in_line);

    if (w) *w = max_width * scale + (max_chars_in_line > 0 ? (max_chars_in_line - 1) * x_spacing : 0);
    if (h) *h = text_height;
}
