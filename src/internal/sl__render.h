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

#ifndef SL__RENDER_H
#define SL__RENDER_H

#include <smol.h>

#include "./sl__registry.h"
#include <glad/gles2.h>
#include <stdint.h>

/* === Constants === */

#define SL__VERTEX_BUFFER_SIZE 2048
#define SL__INDEX_BUFFER_SIZE (SL__VERTEX_BUFFER_SIZE * 3)
#define SL__MATRIX_STACK_SIZE 8
#define SL__MAX_DRAW_CALLS 256

/* === Internal Structs === */

typedef struct {
    uint32_t id;
    int w, h;
} sl__texture_t;

typedef struct {
    int value;              ///< Unicode codepoint value
    int x_offset;           ///< Horizontal offset when drawing the glyph
    int y_offset;           ///< Vertical offset when drawing the glyph
    int x_advance;          ///< Horizontal advance to next character position
    int x_atlas;            ///< X-coordinate position in texture atlas
    int y_atlas;            ///< Y-coordinate position in texture atlas
    int w_atlas;            ///< Width of glyph in texture atlas
    int h_atlas;            ///< Height of glyph in texture atlas
} sl__glyph_t;

typedef struct {
    int base_size;          ///< Base font size (default character height in pixels)
    int glyph_count;        ///< Total number of glyphs available in this font
    int glyph_padding;      ///< Padding around glyphs in the texture atlas
    uint32_t texture;       ///< Texture atlas containing all glyph images
    sl__glyph_t* glyphs;    ///< Array of glyph information structures
    sl_font_type_t type;    ///< Font rendering type used during text rendering
} sl__font_t;

typedef struct {
    GLuint vbo;
    GLuint ebo;
} sl__mesh_t;

typedef struct {
    uint32_t id;
    int loc_mvp;
} sl__shader_t;

typedef struct {
    GLuint framebuffer;
    sl_texture_id color;
    sl_texture_id depth;
    int w, h;
} sl__canvas_t;

/* === Batching Structs === */

typedef struct {
    sl_shader_id shader;
    sl_texture_id texture;
    sl_blend_mode_t blend_mode;
} sl__render_state_t;

typedef struct {
    sl__render_state_t state;
    uint16_t vertex_start;
    uint16_t vertex_count;
    uint16_t index_start;
    uint16_t index_count;
} sl__draw_call_t;

/* === Global State === */

extern struct sl__render {

    sl__registry_t reg_textures;
    sl__registry_t reg_canvases;
    sl__registry_t reg_shaders;
    sl__registry_t reg_meshes;
    sl__registry_t reg_fonts;

    sl_texture_id default_texture;
    sl_shader_id default_shader;

    sl_texture_id current_texture;
    sl_canvas_id current_canvas;
    sl_shader_id current_shader;
    sl_font_id current_font;

    sl_blend_mode_t current_blend_mode;
    sl_color_t current_color;

    sl_mat4_t matrix_transform_stack[SL__MATRIX_STACK_SIZE];
    sl_mat4_t matrix_transform;
    sl_mat4_t matrix_texture;
    sl_mat4_t matrix_view;
    sl_mat4_t matrix_proj;

    int matrix_transform_stack_pos;
    bool transform_is_identity;
    bool texture_is_identity;
    bool use_custom_proj;

    sl_vertex_2d_t vertex_buffer[SL__VERTEX_BUFFER_SIZE];
    GLushort index_buffer[SL__INDEX_BUFFER_SIZE];
    int vertex_count;
    int index_count;

    sl__draw_call_t draw_calls[SL__MAX_DRAW_CALLS];
    int draw_call_count;

    GLuint vbo;
    GLuint ebo;
    GLuint vao;
    sl__render_state_t last_state;
    bool has_pending_data;

} sl__render;

/* === Module Functions === */

bool sl__render_init(int w, int h);
void sl__render_quit(void);

/* === Font Functions === */

const sl__glyph_t* sl__glyph_info(const sl__font_t* font, int codepoint);
void sl__font_measure_text(float* w, float* h, const sl__font_t* font, const char* text, float font_size, float x_spacing, float y_spacing);
void sl__font_measure_codepoints(float* w, float* h, const sl__font_t* font, const int* codepoints, int length, float font_size, float x_spacing, float y_spacing);

#endif // SL__RENDER_H
