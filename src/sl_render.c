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

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_video.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "./internal/sl__registry.h"
#include "./internal/sl__render.h"
#include "./internal/sl__core.h"

/* === Internal Functions === */

static inline void sl__render_get_current_state(sl__render_state_t* state)
{
    state->shader = sl__render.current_shader;
    state->texture = sl__render.current_texture;
    state->blend_mode = sl__render.current_blend_mode;
}

static inline void sl__render_use_shader(sl_shader_id reg_id, const sl_mat4_t* mvp)
{
    sl__shader_t* shader = sl__registry_get(&sl__render.reg_shaders, reg_id);
    if (shader == NULL) {
        shader = sl__registry_get(&sl__render.reg_shaders, sl__render.default_shader);
    }

    glUseProgram(shader->id);
    glUniformMatrix4fv(shader->loc_mvp, 1, GL_FALSE, mvp->a);
}

static inline void sl__render_bind_texture(uint32_t slot, sl_texture_id reg_id)
{
    sl__texture_t* texture = sl__registry_get(&sl__render.reg_textures, reg_id);
    if (texture == NULL) {
        texture = sl__registry_get(&sl__render.reg_textures, sl__render.default_texture);
    }

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

static inline void sl__render_set_blend_mode(sl_blend_mode_t blend_mode)
{
    switch (blend_mode) {
    case SL_BLEND_OPAQUE:
        glDisable(GL_BLEND);
        break;
    case SL_BLEND_PREMUL:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case SL_BLEND_ALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case SL_BLEND_MUL:
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        break;
    case SL_BLEND_ADD:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
    default:
        glDisable(GL_BLEND);
        break;
    }
}

static void sl__render_commit_current_data(void)
{
    if (!sl__render.has_pending_data || sl__render.vertex_count == 0) {
        return;
    }

    // If we reach the draw call limit, we cannot create a new one
    // The calling function will have to handle the flush
    if (sl__render.draw_call_count >= SL__MAX_DRAW_CALLS) {
        return;
    }

    sl__draw_call_t* call = &sl__render.draw_calls[sl__render.draw_call_count];
    call->state = sl__render.last_state;
    call->vertex_start = (sl__render.draw_call_count == 0) ? 0 : 
                        sl__render.draw_calls[sl__render.draw_call_count - 1].vertex_start +
                        sl__render.draw_calls[sl__render.draw_call_count - 1].vertex_count;
    call->vertex_count = sl__render.vertex_count - call->vertex_start;
    call->index_start = (sl__render.draw_call_count == 0) ? 0 :
                       sl__render.draw_calls[sl__render.draw_call_count - 1].index_start +
                       sl__render.draw_calls[sl__render.draw_call_count - 1].index_count;
    call->index_count = sl__render.index_count - call->index_start;

    sl__render.draw_call_count++;
    sl__render.has_pending_data = false;
}

static void sl__render_flush_all(void)
{
    /* --- Commit current data if necessary --- */

    sl__render_commit_current_data();

    if (sl__render.draw_call_count == 0) {
        return;
    }

    /* --- Upload data --- */

    glBindBuffer(GL_ARRAY_BUFFER, sl__render.vbo);
    glBufferSubData(
        GL_ARRAY_BUFFER, 0, 
        sl__render.vertex_count * sizeof(sl_vertex_2d_t), 
        sl__render.vertex_buffer
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sl__render.ebo);
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER, 0, 
        sl__render.index_count * sizeof(GLushort), 
        sl__render.index_buffer
    );

    /* --- Setup vertex attributes --- */

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_2d_t), (void*)offsetof(sl_vertex_2d_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_2d_t), (void*)offsetof(sl_vertex_2d_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(sl_vertex_2d_t), (void*)offsetof(sl_vertex_2d_t, color));
    glEnableVertexAttribArray(3);

    /* --- Calculation of the projection view matrix --- */

    sl_mat4_t mvp = sl_mat4_mul(&sl__render.matrix_view, &sl__render.matrix_proj);

    /* --- Execute all draw calls --- */

    sl__render_state_t* current_state = NULL;

    for (int i = 0; i < sl__render.draw_call_count; i++)
    {
        sl__draw_call_t* call = &sl__render.draw_calls[i];

        if (current_state == NULL) {
            sl__render_use_shader(call->state.shader, &mvp);
            sl__render_bind_texture(0, call->state.texture);
            sl__render_set_blend_mode(call->state.blend_mode);
        }
        else {
            if (current_state->shader != call->state.shader) {
                sl__render_use_shader(call->state.shader, &mvp);
            }
            if (current_state->texture != call->state.texture) {
                sl__render_bind_texture(0, call->state.texture);
            }
            if (current_state->blend_mode != call->state.blend_mode) {
                sl__render_set_blend_mode(call->state.blend_mode);
            }
        }

        current_state = &call->state;

        glDrawElements(
            GL_TRIANGLES, call->index_count, GL_UNSIGNED_SHORT, 
            (void*)(call->index_start * sizeof(GLushort))
        );
    }

    /* --- Reset for the next frame --- */

    sl__render.vertex_count = 0;
    sl__render.index_count = 0;
    sl__render.draw_call_count = 0;
    sl__render.has_pending_data = false;
}

static void sl__render_check_state_change(void)
{
    sl__render_state_t current_state;
    sl__render_get_current_state(&current_state);

    // If this is the first draw or the state has changed
    if (sl__render.draw_call_count == 0 && !sl__render.has_pending_data) {
        sl__render.last_state = current_state;
        sl__render.has_pending_data = true;
    }
    // Change state, commit current data
    else if (SDL_memcmp(&current_state, &sl__render.last_state, sizeof(sl__render_state_t)) != 0) {
        sl__render_commit_current_data();
        sl__render.last_state = current_state;
        sl__render.has_pending_data = true;
    }
}

static void sl__render_check_space(int vertices_needed, int indices_needed)
{
    // Check if there is enough space in the buffers
    if (sl__render.vertex_count + vertices_needed > SL__VERTEX_BUFFER_SIZE || 
        sl__render.index_count + indices_needed > SL__INDEX_BUFFER_SIZE ||
        sl__render.draw_call_count >= SL__MAX_DRAW_CALLS) {
        sl__render_flush_all();
    }
}

static inline void sl__render_add_vertex(const sl_vertex_2d_t* v)
{
    SDL_assert(sl__render.vertex_count < SL__VERTEX_BUFFER_SIZE);

    sl_vertex_2d_t* vertex = &sl__render.vertex_buffer[sl__render.vertex_count];
    vertex->position = v->position;
    vertex->texcoord = v->texcoord;
    vertex->color = v->color;

    if (!sl__render.transform_is_identity) {
        vertex->position = sl_vec2_transform(vertex->position, &sl__render.matrix_transform);
    }

    if (!sl__render.texture_is_identity) {
        vertex->texcoord = sl_vec2_transform(vertex->texcoord, &sl__render.matrix_texture);
    }

    sl__render.vertex_count++;
}

static inline void sl__render_add_point(float x, float y, float u, float v)
{
    sl__render_add_vertex(&(sl_vertex_2d_t) {
        .position = SL_VEC2(x, y),
        .texcoord = SL_VEC2(u, v),
        .color = sl__render.current_color
    });
}

static inline void sl__render_add_index(GLushort index)
{
    if (sl__render.index_count >= SL__INDEX_BUFFER_SIZE) {
        return;
    }
    sl__render.index_buffer[sl__render.index_count++] = index;
}

static void sl__render_codepoint(const sl__font_t* font, int codepoint, float x, float y, float font_size)
{
    /* --- Get the character index position and it's data --- */

    const sl__glyph_t* glyph = sl__glyph_info(font, codepoint);

    /* --- Calculate the scale factor based on font size --- */

    float scale = font_size / font->base_size;

    /* --- Set temporary pipeline state --- */

    unsigned int previous_texture = sl__render.current_texture;
    sl_blend_mode_t previous_blend = sl__render.current_blend_mode;

    sl__render.current_texture = font->texture;
    sl__render.current_blend_mode = SL_BLEND_PREMUL;

    /* --- Calculate the padded source rect of the glyph --- */

    float x_glyph = (float)(glyph->x_atlas - font->glyph_padding);
    float y_glyph = (float)(glyph->y_atlas - font->glyph_padding);
    float w_glyph = (float)(glyph->w_atlas + 2 * font->glyph_padding);
    float h_glyph = (float)(glyph->h_atlas + 2 * font->glyph_padding);

    /* --- Calculate the destination of the character with scaling --- */

    float x_dst = x + (glyph->x_offset - font->glyph_padding) * scale;
    float y_dst = y + (glyph->y_offset - font->glyph_padding) * scale;
    float w_dst = w_glyph * scale;
    float h_dst = h_glyph * scale;

    /* --- Convert the source rect to texture coordinates --- */

    int w_atlas = 0, h_atlas = 0;
    sl_texture_query(font->texture, &w_atlas, &h_atlas);

    float u0 = x_glyph / w_atlas;
    float v0 = y_glyph / h_atlas;
    float u1 = u0 + (w_glyph / w_atlas);
    float v1 = v0 + (h_glyph / h_atlas);

    /* --- Push the character to the batch with scaled dimensions --- */

    sl_vertex_2d_t quad[4] = {
        SL_VERTEX_2D(SL_VEC2(x_dst, y_dst), SL_VEC2(u0, v0), sl__render.current_color),
        SL_VERTEX_2D(SL_VEC2(x_dst, y_dst + h_dst), SL_VEC2(u0, v1), sl__render.current_color),
        SL_VERTEX_2D(SL_VEC2(x_dst + w_dst, y_dst + h_dst), SL_VEC2(u1, v1), sl__render.current_color),
        SL_VERTEX_2D(SL_VEC2(x_dst + w_dst, y_dst), SL_VEC2(u1, v0), sl__render.current_color)
    };

    sl_render_quad_list(quad, 1);

    /* --- Reset the previous pipeline state --- */

    sl__render.current_texture = previous_texture;
    sl__render.current_blend_mode = previous_blend;
}

static void sl__render_codepoints(const sl__font_t* font, const int* codepoints, int length, float x, float y, float font_size, float x_spacing, float y_spacing)
{
    float y_offset = 0.0f;
    float x_offset = 0.0f;

    float scale = font_size / font->base_size;

    for (int i = 0; i < length; i++)
    {
        const sl__glyph_t* glyph = sl__glyph_info(font, codepoints[i]);

        if (codepoints[i] == '\n') {
            y_offset += (font_size + y_spacing);
            x_offset = 0.0f;
        }
        else {
            if (codepoints[i] != ' ' && codepoints[i] != '\t') {
                sl__render_codepoint(font, codepoints[i], x + x_offset, y + y_offset, font_size);
            }

            if (glyph->x_advance == 0) {
                x_offset += ((float)(glyph->w_atlas) * scale + x_spacing);
            }
            else {
                x_offset += ((float)(glyph->x_advance) * scale + x_spacing);
            }
        }
    }
}

static void sl__render_text(const sl__font_t* font, const char* text, float x, float y, float font_size, float x_spacing, float y_spacing)
{
    int size = (int)strlen(text);

    float x_offset = 0.0f;
    float y_offset = 0.0f;

    float scale = font_size / font->base_size;

    for (int i = 0; i < size;)
    {
        int codepointByteCount = 0;
        int codepoint = sl_codepoint_next(&text[i], &codepointByteCount);

        const sl__glyph_t* glyph = sl__glyph_info(font, codepoint);

        if (codepoint == '\n') {
            y_offset += (font_size + y_spacing);
            x_offset = 0.0f;
        }
        else {
            if (codepoint != ' ' && codepoint != '\t') {
                sl__render_codepoint(font, codepoint, x + x_offset, y + y_offset, font_size);
            }

            if (glyph->x_advance == 0) {
                x_offset += ((float)(glyph->w_atlas) * scale + x_spacing);
            }
            else {
                x_offset += ((float)(glyph->x_advance) * scale + x_spacing);
            }
        }

        i += codepointByteCount;
    }
}

/* === Public API === */

void sl_render_viewport(int x, int y, int w, int h)
{
    sl__render_flush_all();

    glViewport(x, y, w, h);
}

void sl_render_scissor(int x, int y, int w, int h)
{
    sl__render_flush_all();

    if (w != 0 && h != 0 || x != 0 || y != 0) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, w, h);
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void sl_render_stencil(sl_stencil_func_t func, int ref, uint32_t mask,
                       sl_stencil_op_t sfail, sl_stencil_op_t dpfail,
                       sl_stencil_op_t dppass)
{
    sl__render_flush_all();

    if(func == SL_STENCIL_DISABLE) {
        glDisable(GL_STENCIL_TEST);
        return;
    }

    static const GLenum func_table[] = {
        [SL_STENCIL_DISABLE] = GL_ALWAYS,
        [SL_STENCIL_ALWAYS] = GL_ALWAYS,
        [SL_STENCIL_NEVER] = GL_NEVER,
        [SL_STENCIL_LESS] = GL_LESS,
        [SL_STENCIL_LEQUAL] = GL_LEQUAL,
        [SL_STENCIL_EQUAL] = GL_EQUAL,
        [SL_STENCIL_GEQUAL] = GL_GEQUAL,
        [SL_STENCIL_GREATER] = GL_GREATER,
        [SL_STENCIL_NOTEQUAL] = GL_NOTEQUAL
    };

    static const GLenum op_table[] = {
        [SL_STENCIL_KEEP] = GL_KEEP,
        [SL_STENCIL_ZERO] = GL_ZERO,
        [SL_STENCIL_REPLACE] = GL_REPLACE,
        [SL_STENCIL_INCR] = GL_INCR,
        [SL_STENCIL_DECR] = GL_DECR,
        [SL_STENCIL_INVERT] = GL_INVERT
    };

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(func_table[func], ref, mask);
    glStencilOp(op_table[sfail], op_table[dpfail], op_table[dppass]);
}

void sl_render_clear(sl_color_t color)
{
    glClearColor((float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void sl_render_flush(void)
{
    sl__render_flush_all();
}

void sl_render_present(void)
{
    sl__render_flush_all();

    SDL_GL_SwapWindow(sl__core.window);
}

void sl_render_depth_test(bool enabled)
{
    sl__render_flush_all();

    (enabled ? glEnable : glDisable)(GL_DEPTH_TEST);
}

void sl_render_depth_write(bool enabled)
{
    sl__render_flush_all();

    glDepthMask(enabled);
}

void sl_render_depth_range(float near, float far)
{
    sl__render_flush_all();

    glDepthRangef(near, far);
}

void sl_render_cull_face(sl_cull_mode_t cull)
{
    sl__render_flush_all();

    if (cull == SL_CULL_NONE) {
        glDisable(GL_CULL_FACE);
    }
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(cull == SL_CULL_FRONT ? GL_FRONT : GL_BACK);
    }
}

void sl_render_color(sl_color_t color)
{
    sl__render.current_color = color;
}

void sl_render_sampler(uint32_t slot, sl_texture_id texture)
{
    if (texture == 0) {
        texture = sl__render.default_texture;
    }

    if (slot == 0) {
        sl__render.current_texture = texture;
    }
    else {
        sl__render_bind_texture(slot, texture);
    }
}

void sl_render_font(sl_font_id font)
{
    // NOTE: No default font
    // Without font no text will be rendered
    sl__render.current_font = font;
}

void sl_render_shader(sl_shader_id shader)
{
    if (shader == 0) {
        shader = sl__render.default_shader;
    }
    else {
        sl__shader_t* data = sl__registry_get(&sl__render.reg_shaders, shader);
        if (data == NULL) return;

        glUseProgram(data->id);
    }

    sl__render.current_shader = shader;
}

void sl_render_blend(sl_blend_mode_t blend)
{
    sl__render.current_blend_mode = blend;
}

void sl_render_canvas(sl_canvas_id canvas)
{
    if (sl__render.current_canvas == canvas) {
        return;
    }

    sl__render_flush_all();

    if (canvas == 0) {
        sl_vec2_t win_size = sl_window_get_size();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, win_size.x, win_size.y);
        sl__render.current_canvas = 0;
        if (!sl__render.use_custom_proj) {
            sl__render.matrix_proj = sl_mat4_ortho(0, win_size.x, win_size.y, 0, 0, 1);
        }
        return;
    }

    const sl__canvas_t* data = sl__registry_get(&sl__render.reg_canvases, canvas);
    if (data == NULL) return;

    glBindFramebuffer(GL_FRAMEBUFFER, data->framebuffer);
    glViewport(0, 0, data->w, data->h);
    sl__render.current_canvas = canvas;

    if (!sl__render.use_custom_proj) {
        sl__render.matrix_proj = sl_mat4_ortho(0, data->w, data->h, 0, 0, 1);
    }
}

void sl_render_uniform1i(int uniform, int32_t x)
{
    if (uniform >= 0) {
        glUniform1i(uniform, x);
    }
}

void sl_render_uniform2i(int uniform, int32_t x, int32_t y)
{
    if (uniform >= 0) {
        glUniform2i(uniform, x, y);
    }
}

void sl_render_uniform3i(int uniform, int32_t x, int32_t y, int32_t z)
{
    if (uniform >= 0) {
        glUniform3i(uniform, x, y, z);
    }
}

void sl_render_uniform4i(int uniform, int32_t x, int32_t y, int32_t z, int32_t w)
{
    if (uniform >= 0) {
        glUniform4i(uniform, x, y, z, w);
    }
}

void sl_render_uniform1f(int uniform, float x)
{
    if (uniform >= 0) {
        glUniform1f(uniform, x);
    }
}

void sl_render_uniform2f(int uniform, float x, float y)
{
    if (uniform >= 0) {
        glUniform2f(uniform, x, y);
    }
}

void sl_render_uniform3f(int uniform, float x, float y, float z)
{
    if (uniform >= 0) {
        glUniform3f(uniform, x, y, z);
    }
}

void sl_render_uniform4f(int uniform, float x, float y, float z, float w)
{
    if (uniform >= 0) {
        glUniform4f(uniform, x, y, z, w);
    }
}

void sl_render_uniform_vec2(int uniform, const sl_vec2_t* v, int count)
{
    if (uniform >= 0) {
        glUniform2fv(uniform, count, (float*)v);
    }
}

void sl_render_uniform_vec3(int uniform, const sl_vec3_t* v, int count)
{
    if (uniform >= 0) {
        glUniform3fv(uniform, count, (float*)v);
    }
}

void sl_render_uniform_vec4(int uniform, const sl_vec4_t* v, int count)
{
    if (uniform >= 0) {
        glUniform4fv(uniform, count, (float*)v);
    }
}

void sl_render_uniform_color3(int uniform, sl_color_t color)
{
    if (uniform >= 0) {
        glUniform3f(
            uniform,
            (float)color.r / 255,
            (float)color.g / 255,
            (float)color.b / 255
        );
    }
}

void sl_render_uniform_color4(int uniform, sl_color_t color)
{
    if (uniform >= 0) {
        glUniform4f(
            uniform,
            (float)color.r / 255,
            (float)color.g / 255,
            (float)color.b / 255,
            (float)color.a / 255
        );
    }
}

void sl_render_uniform_mat2(int uniform, float* v, int count)
{
    if (uniform >= 0 && v) {
        glUniformMatrix2fv(uniform, count, GL_FALSE, v);
    }
}

void sl_render_uniform_mat3(int uniform, float* v, int count)
{
    if (uniform >= 0 && v) {
        glUniformMatrix3fv(uniform, count, GL_FALSE, v);
    }
}

void sl_render_uniform_mat4(int uniform, float* v, int count)
{
    if (uniform >= 0 && v) {
        glUniformMatrix4fv(uniform, count, GL_FALSE, v);
    }
}

void sl_render_projection(const sl_mat4_t* matrix)
{
    sl__render_flush_all();

    if (matrix != NULL) {
        SDL_memcpy(sl__render.matrix_proj.a, matrix, sizeof(sl__render.matrix_proj));
        sl__render.use_custom_proj = true;
    }
    else {
        sl_vec2_t win_size = sl_window_get_size();
        sl__render.matrix_proj = sl_mat4_ortho(0, win_size.x, win_size.y, 0, 0, 1);
        sl__render.use_custom_proj = false;
    }
}

void sl_render_view(const sl_mat4_t* matrix)
{
    sl__render_flush_all();

    if (matrix == NULL) {
        sl__render.matrix_view = SL_MAT4_IDENTITY;
    }
    else {
        SDL_memcpy(sl__render.matrix_view.a, matrix->a, sizeof(sl_mat4_t));
    }
}

void sl_render_push(void)
{
    if (sl__render.matrix_transform_stack_pos < SL__MATRIX_STACK_SIZE - 1) {
        SDL_memcpy(sl__render.matrix_transform_stack[sl__render.matrix_transform_stack_pos].a, sl__render.matrix_transform.a, sizeof(sl_mat4_t));
        sl__render.matrix_transform_stack_pos++;
    }
}

void sl_render_pop(void)
{
    if (sl__render.matrix_transform_stack_pos > 0) {
        sl__render.matrix_transform_stack_pos--;
        SDL_memcpy(sl__render.matrix_transform.a, sl__render.matrix_transform_stack[sl__render.matrix_transform_stack_pos].a, sizeof(sl_mat4_t));
        sl__render.transform_is_identity = (0 == SDL_memcmp(sl__render.matrix_transform.a, &SL_MAT4_IDENTITY, sizeof(sl_mat4_t)));
    }
}

void sl_render_identity(void)
{
    sl__render.matrix_transform = SL_MAT4_IDENTITY;
    sl__render.transform_is_identity = true;
}

void sl_render_translate(sl_vec3_t v)
{
    sl_mat4_t translate = sl_mat4_translate(v);
    sl__render.matrix_transform = sl_mat4_mul(&sl__render.matrix_transform, &translate);
    sl__render.transform_is_identity = false;
}

void sl_render_rotate(sl_vec3_t v)
{
    sl_mat4_t* transform = &sl__render.matrix_transform;
    sl_mat4_t rotate;

    if (v.x != 0.0f) {
        rotate = sl_mat4_rotate_x(v.x);
        *transform = sl_mat4_mul(transform, &rotate);
    }
    if (v.y != 0.0f) {
        rotate = sl_mat4_rotate_y(v.y);
        *transform = sl_mat4_mul(transform, &rotate);
    }
    if (v.z != 0.0f) {
        rotate = sl_mat4_rotate_z(v.z);
        *transform = sl_mat4_mul(transform, &rotate);
    }

    sl__render.transform_is_identity = false;
}

void sl_render_scale(sl_vec3_t v)
{
    sl_mat4_t scale = sl_mat4_scale(v);
    sl__render.matrix_transform = sl_mat4_mul(&sl__render.matrix_transform, &scale);
    sl__render.transform_is_identity = false;
}

void sl_render_transform(const sl_mat4_t* matrix)
{
    sl__render.matrix_transform = sl_mat4_mul(&sl__render.matrix_transform, matrix);
    sl__render.transform_is_identity = false;
}

void sl_render_texture_identity(void)
{
    sl__render.matrix_texture = SL_MAT4_IDENTITY;
    sl__render.texture_is_identity = true;
}

void sl_render_texture_translate(sl_vec2_t v)
{
    sl_mat4_t translate = sl_mat4_translate(SL_VEC3(v.x, v.y, 0.0f));
    sl__render.matrix_texture = sl_mat4_mul(&sl__render.matrix_texture, &translate);
    sl__render.texture_is_identity = false;
}

void sl_render_texture_rotate(float radians)
{
    sl_mat4_t rotate = sl_mat4_rotate_z(radians);
    sl__render.matrix_texture = sl_mat4_mul(&sl__render.matrix_texture, &rotate);
    sl__render.texture_is_identity = false;
}

void sl_render_texture_scale(sl_vec2_t v)
{
    sl_mat4_t scale = sl_mat4_scale(SL_VEC3(v.x, v.y, 1.0f));
    sl__render.matrix_texture = sl_mat4_mul(&sl__render.matrix_texture, &scale);
    sl__render.texture_is_identity = false;
}

void sl_render_triangle_list(const sl_vertex_2d_t* triangles, int triangle_count)
{
    if (triangle_count <= 0) return;
    sl__render_check_state_change();

    for (int i = 0; i < triangle_count; i++) {
        sl__render_check_space(3, 3);
        int base_index = sl__render.vertex_count;
        const sl_vertex_2d_t* tri = &triangles[i * 3];

        for (int j = 0; j < 3; j++) {
            sl__render_add_vertex(&tri[j]);
        }

        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);
    }
}

void sl_render_triangle_strip(const sl_vertex_2d_t* vertices, int count)
{
    if (count < 3) return;
    sl__render_check_state_change();

    for (int i = 0; i < count - 2; i++) {
        sl__render_check_space(3, 3);
        int base_index = sl__render.vertex_count;

        if (i % 2 == 0) {
            sl__render_add_vertex(&vertices[i]);
            sl__render_add_vertex(&vertices[i + 1]);
            sl__render_add_vertex(&vertices[i + 2]);
        } else {
            sl__render_add_vertex(&vertices[i]);
            sl__render_add_vertex(&vertices[i + 2]);
            sl__render_add_vertex(&vertices[i + 1]);
        }

        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);
    }
}

void sl_render_triangle_fan(const sl_vertex_2d_t* vertices, int count)
{
    if (count < 3) return;
    sl__render_check_state_change();

    for (int i = 1; i < count - 1; i++) {
        sl__render_check_space(3, 3);
        int base_index = sl__render.vertex_count;

        sl__render_add_vertex(&vertices[0]);
        sl__render_add_vertex(&vertices[i]);
        sl__render_add_vertex(&vertices[i + 1]);

        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);
    }
}

void sl_render_quad_list(const sl_vertex_2d_t* quads, int quad_count)
{
    if (quad_count <= 0) return;
    sl__render_check_state_change();

    for (int i = 0; i < quad_count; i++) {
        sl__render_check_space(4, 6);
        int base_index = sl__render.vertex_count;
        const sl_vertex_2d_t* quad = &quads[i * 4];

        for (int j = 0; j < 4; j++) {
            sl__render_add_vertex(&quad[j]);
        }

        // Triangle 1: 0, 1, 2
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);

        // Triangle 2: 0, 2, 3
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 2);
        sl__render_add_index(base_index + 3);
    }
}

void sl_render_quad_strip(const sl_vertex_2d_t* vertices, int count)
{
    if (count < 4 || count % 2 != 0) return;
    sl__render_check_state_change();

    for (int i = 0; i < count - 2; i += 2) {
        sl__render_check_space(4, 6);
        int base_index = sl__render.vertex_count;

        sl__render_add_vertex(&vertices[i]);
        sl__render_add_vertex(&vertices[i + 1]);
        sl__render_add_vertex(&vertices[i + 3]);
        sl__render_add_vertex(&vertices[i + 2]);

        // Triangle 1: 0, 1, 2
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);

        // Triangle 2: 0, 2, 3
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 2);
        sl__render_add_index(base_index + 3);
    }
}

void sl_render_quad_fan(const sl_vertex_2d_t* vertices, int count)
{
    if (count < 4) return;
    sl__render_check_state_change();

    for (int i = 1; i < count - 2; i += 2) {
        if (i + 2 >= count) break;

        sl__render_check_space(4, 6);
        int base_index = sl__render.vertex_count;

        sl__render_add_vertex(&vertices[0]);
        sl__render_add_vertex(&vertices[i]);
        sl__render_add_vertex(&vertices[i + 1]);
        sl__render_add_vertex(&vertices[i + 2]);

        // Triangle 1: 0, 1, 2
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1);
        sl__render_add_index(base_index + 2);

        // Triangle 2: 0, 2, 3
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 2);
        sl__render_add_index(base_index + 3);
    }
}

void sl_render_line_list(const sl_vec2_t* lines, int line_count, float thickness)
{
    if (line_count <= 0) return;

    for (int i = 0; i < line_count; i++) {
        const sl_vec2_t* line = &lines[i * 2];
        sl_render_line(line[0], line[1], thickness);
    }
}

void sl_render_line_strip(const sl_vec2_t* points, int count, float thickness)
{
    if (count < 2) return;

    for (int i = 0; i < count - 1; i++) {
        sl_render_line(points[i], points[i + 1], thickness);
    }
}

void sl_render_line_loop(const sl_vec2_t* points, int count, float thickness)
{
    if (count < 2) return;

    for (int i = 0; i < count - 1; i++) {
        sl_render_line(points[i], points[i + 1], thickness);
    }

    sl_render_line(points[count - 1], points[0], thickness);
}

void sl_render_triangle(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2)
{
    sl__render_check_space(3, 3);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(p0.x, p0.y, 0.0f, 0.0f);
    sl__render_add_point(p1.x, p1.y, 0.5f, 1.0f);
    sl__render_add_point(p2.x, p2.y, 1.0f, 0.0f);

    sl__render_add_index(base_index);
    sl__render_add_index(base_index + 1);
    sl__render_add_index(base_index + 2);
}

void sl_render_triangle_lines(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, float thickness)
{
    sl_render_line(p0, p1, thickness);
    sl_render_line(p1, p2, thickness);
    sl_render_line(p2, p0, thickness);
}

void sl_render_quad(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3)
{
    sl__render_check_space(4, 6);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(p0.x, p0.y, 0.0f, 0.0f);
    sl__render_add_point(p1.x, p1.y, 1.0f, 0.0f);
    sl__render_add_point(p2.x, p2.y, 1.0f, 1.0f);
    sl__render_add_point(p3.x, p3.y, 0.0f, 1.0f);

    // Triangle 1: 0, 1, 2
    sl__render_add_index(base_index);
    sl__render_add_index(base_index + 1);
    sl__render_add_index(base_index + 2);

    // Triangle 2: 0, 2, 3
    sl__render_add_index(base_index);
    sl__render_add_index(base_index + 2);
    sl__render_add_index(base_index + 3);
}

void sl_render_quad_lines(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3, float thickness)
{
    sl_render_line(p0, p1, thickness);
    sl_render_line(p1, p2, thickness);
    sl_render_line(p2, p3, thickness);
    sl_render_line(p3, p0, thickness);
}

void sl_render_rectangle(float x, float y, float w, float h)
{
    sl_render_quad(
        SL_VEC2(x, y),
        SL_VEC2(x + w, y),
        SL_VEC2(x + w, y + h),
        SL_VEC2(x, y + h)
    );
}

void sl_render_rectangle_lines(float x, float y, float w, float h, float thickness)
{
    sl_render_quad_lines(
        SL_VEC2(x, y),
        SL_VEC2(x + w, y),
        SL_VEC2(x + w, y + h),
        SL_VEC2(x, y + h),
        thickness
    );
}

void sl_render_rectangle_ex(sl_vec2_t center, sl_vec2_t size, float rotation)
{
    float half_w = size.x * 0.5f;
    float half_h = size.y * 0.5f;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    sl_vec2_t tl = {
        center.x + (-half_w * cos_r - -half_h * sin_r),
        center.y + (-half_w * sin_r + -half_h * cos_r)
    };
    
    sl_vec2_t tr = {
        center.x + ( half_w * cos_r - -half_h * sin_r),
        center.y + ( half_w * sin_r + -half_h * cos_r)
    };
    
    sl_vec2_t br = {
        center.x + ( half_w * cos_r -  half_h * sin_r),
        center.y + ( half_w * sin_r +  half_h * cos_r)
    };
    
    sl_vec2_t bl = {
        center.x + (-half_w * cos_r -  half_h * sin_r),
        center.y + (-half_w * sin_r +  half_h * cos_r)
    };

    sl_render_quad(tl, tr, br, bl);
}

void sl_render_rectangle_lines_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float thickness)
{
    float half_w = size.x * 0.5f;
    float half_h = size.y * 0.5f;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    sl_vec2_t tl = {
        center.x + (-half_w * cos_r - -half_h * sin_r),
        center.y + (-half_w * sin_r + -half_h * cos_r)
    };
    
    sl_vec2_t tr = {
        center.x + ( half_w * cos_r - -half_h * sin_r),
        center.y + ( half_w * sin_r + -half_h * cos_r)
    };
    
    sl_vec2_t br = {
        center.x + ( half_w * cos_r -  half_h * sin_r),
        center.y + ( half_w * sin_r +  half_h * cos_r)
    };
    
    sl_vec2_t bl = {
        center.x + (-half_w * cos_r -  half_h * sin_r),
        center.y + (-half_w * sin_r +  half_h * cos_r)
    };

    sl_render_quad_lines(tl, tr, br, bl, thickness);
}

void sl_render_rounded_rectangle(float x, float y, float w, float h, float radius, int segments)
{
    float max_radius = fminf(w * 0.5f, h * 0.5f);
    radius = fminf(radius, max_radius);

    // Center
    sl_render_quad(
        SL_VEC2(x + radius, y),
        SL_VEC2(x + w - radius, y),
        SL_VEC2(x + w - radius, y + h),
        SL_VEC2(x + radius, y + h)
    );

    // Left
    sl_render_quad(
        SL_VEC2(x, y + radius),
        SL_VEC2(x + radius, y + radius),
        SL_VEC2(x + radius, y + h - radius),
        SL_VEC2(x, y + h - radius)
    );

    // Right
    sl_render_quad(
        SL_VEC2(x + w - radius, y + radius),
        SL_VEC2(x + w, y + radius),
        SL_VEC2(x + w, y + h - radius),
        SL_VEC2(x + w - radius, y + h - radius)
    );

    // Corners
    sl_render_pie_slice(SL_VEC2(x + radius, y + radius), radius, SL_PI, SL_PI * 1.5f, segments);              // Top-left
    sl_render_pie_slice(SL_VEC2(x + w - radius, y + radius), radius, SL_PI * 1.5f, SL_PI * 2.0f, segments);   // Top-right
    sl_render_pie_slice(SL_VEC2(x + w - radius, y + h - radius), radius, 0, SL_PI * 0.5f, segments);          // Bottom-right
    sl_render_pie_slice(SL_VEC2(x + radius, y + h - radius), radius, SL_PI * 0.5f, SL_PI, segments);          // Bottom-left
}

void sl_render_rounded_rectangle_lines(float x, float y, float w, float h, float radius, float thickness, int segments)
{
    float max_radius = fminf(w * 0.5f, h * 0.5f);
    radius = fminf(radius, max_radius);

    // Sides
    sl_render_line(SL_VEC2(x + radius, y), SL_VEC2(x + w - radius, y), thickness);
    sl_render_line(SL_VEC2(x + w, y + radius), SL_VEC2(x + w, y + h - radius), thickness);
    sl_render_line(SL_VEC2(x + w - radius, y + h), SL_VEC2(x + radius, y + h), thickness);
    sl_render_line(SL_VEC2(x, y + h - radius), SL_VEC2(x, y + radius), thickness);

    // Corners
    sl_render_arc(SL_VEC2(x + radius, y + radius), radius, SL_PI, SL_PI * 1.5f, thickness, segments);             // Top-left
    sl_render_arc(SL_VEC2(x + w - radius, y + radius), radius, SL_PI * 1.5f, SL_PI * 2.0f, thickness, segments);  // Top-right
    sl_render_arc(SL_VEC2(x + w - radius, y + h - radius), radius, 0, SL_PI * 0.5f, thickness, segments);         // Bottom-right
    sl_render_arc(SL_VEC2(x + radius, y + h - radius), radius, SL_PI * 0.5f, SL_PI, thickness, segments);         // Bottom-left
}

void sl_render_rounded_rectangle_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float radius)
{
    // For rotated rectangles, a tessellation approach is used
    // because it is complex to transform the arcs directly

    // You can also use the basic functions with a manual
    // transformation if you need a specific number of segments

    float max_radius = fminf(size.x * 0.5f, size.y * 0.5f);
    radius = fminf(radius, max_radius);

    #define SEGMENTS 8
    #define TOTAL_POINTS (4 * SEGMENTS)

    sl_vec2_t points[TOTAL_POINTS];

    float half_w = size.x * 0.5f;
    float half_h = size.y * 0.5f;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    float angle_step = (SL_PI * 0.5f) / (SEGMENTS - 1);
    float cos_step = cosf(angle_step);
    float sin_step = sinf(angle_step);

    int point_index = 0;

    for (int corner = 0; corner < 4; corner++) {
        float corner_x, corner_y, start_angle;
        switch (corner) {
            case 0: // Top-left
                corner_x = -half_w + radius;
                corner_y = -half_h + radius;
                start_angle = SL_PI;
                break;
            case 1: // Top-right
                corner_x = half_w - radius;
                corner_y = -half_h + radius;
                start_angle = SL_PI * 1.5f;
                break;
            case 2: // Bottom-right
                corner_x = half_w - radius;
                corner_y = half_h - radius;
                start_angle = 0;
                break;
            case 3: // Bottom-left
                corner_x = -half_w + radius;
                corner_y = half_h - radius;
                start_angle = SL_PI * 0.5f;
                break;
        }

        float cos_current = cosf(start_angle);
        float sin_current = sinf(start_angle);

        for (int i = 0; i < SEGMENTS; i++) {
            float local_x = corner_x + radius * cos_current;
            float local_y = corner_y + radius * sin_current;

            float x_rot = local_x * cos_r - local_y * sin_r;
            float y_rot = local_x * sin_r + local_y * cos_r;

            points[point_index].x = center.x + x_rot;
            points[point_index].y = center.y + y_rot;
            point_index++;

            if (i < SEGMENTS - 1) {
                float new_cos = cos_current * cos_step - sin_current * sin_step;
                float new_sin = sin_current * cos_step + cos_current * sin_step;
                cos_current = new_cos;
                sin_current = new_sin;
            }
        }
    }

    for (int i = 1; i < TOTAL_POINTS - 1; i++) {
        sl_render_triangle(points[0], points[i], points[i + 1]);
    }

    #undef TOTAL_POINTS
    #undef SEGMENTS
}

void sl_render_rounded_rectangle_lines_ex(sl_vec2_t center, sl_vec2_t size, float rotation, float radius, float thickness)
{
    float max_radius = fminf(size.x * 0.5f, size.y * 0.5f);
    radius = fminf(radius, max_radius);

    #define SEGMENTS 8
    #define TOTAL_POINTS (4 * SEGMENTS)

    sl_vec2_t points[TOTAL_POINTS];

    float half_w = size.x * 0.5f;
    float half_h = size.y * 0.5f;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    float angle_step = (SL_PI * 0.5f) / (SEGMENTS - 1);
    float cos_step = cosf(angle_step);
    float sin_step = sinf(angle_step);

    int point_index = 0;

    for (int corner = 0; corner < 4; corner++) {
        float corner_x, corner_y, start_angle;
        switch (corner) {
        case 0:
            corner_x = -half_w + radius;
            corner_y = -half_h + radius;
            start_angle = SL_PI;
            break;
        case 1:
            corner_x = half_w - radius;
            corner_y = -half_h + radius;
            start_angle = SL_PI * 1.5f;
            break;
        case 2:
            corner_x = half_w - radius;
            corner_y = half_h - radius;
            start_angle = 0;
            break;
        case 3:
            corner_x = -half_w + radius;
            corner_y = half_h - radius;
            start_angle = SL_PI * 0.5f;
            break;
        }

        float cos_current = cosf(start_angle);
        float sin_current = sinf(start_angle);

        for (int i = 0; i < SEGMENTS; i++) {
            float local_x = corner_x + radius * cos_current;
            float local_y = corner_y + radius * sin_current;

            float x_rot = local_x * cos_r - local_y * sin_r;
            float y_rot = local_x * sin_r + local_y * cos_r;

            points[point_index].x = center.x + x_rot;
            points[point_index].y = center.y + y_rot;
            point_index++;

            if (i < SEGMENTS - 1) {
                float new_cos = cos_current * cos_step - sin_current * sin_step;
                float new_sin = sin_current * cos_step + cos_current * sin_step;
                cos_current = new_cos;
                sin_current = new_sin;
            }
        }
    }

    for (int i = 0; i < TOTAL_POINTS; i++) {
        int next = (i + 1) % TOTAL_POINTS;
        sl_render_line(points[i], points[next], thickness);
    }

    #undef TOTAL_POINTS
    #undef SEGMENTS
}

void sl_render_circle(sl_vec2_t center, float radius, int segments)
{
    if (segments < 3) segments = 32;

    sl__render_check_space(segments + 1, segments * 3);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(center.x, center.y, 0.5f, 0.5f);

    float delta = (2.0f * SL_PI) / (float)segments;
    float cos_delta = cosf(delta);
    float sin_delta = sinf(delta);

    float inv2r = 1.0f / (2.0f * radius);

    float cx = radius;
    float cy = 0.0f;

    for (int i = 0; i < segments; i++) {
        float x = center.x + cx;
        float y = center.y + cy;

        float u = 0.5f + cx * inv2r;
        float v = 0.5f + cy * inv2r;

        sl__render_add_point(x, y, u, v);

        float new_cx = cx * cos_delta - cy * sin_delta;
        cy = cx * sin_delta + cy * cos_delta;
        cx = new_cx;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1 + i);
        sl__render_add_index(base_index + 1 + next);
    }
}

void sl_render_circle_lines(sl_vec2_t p, float radius, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    float delta = (2.0f * SL_PI) / (float)segments;
    float cos_delta = cosf(delta);
    float sin_delta = sinf(delta);

    float cx = radius;
    float cy = 0.0f;

    sl_vec2_t prev = { p.x + cx, p.y + cy };

    for (int i = 1; i <= segments; i++) {
        float new_cx = cx * cos_delta - cy * sin_delta;
        float new_cy = cx * sin_delta + cy * cos_delta;
        cx = new_cx;
        cy = new_cy;

        sl_vec2_t curr = { p.x + cx, p.y + cy };

        // Ligne entre prev et curr
        sl_render_line(prev, curr, thickness);

        prev = curr;
    }
}

void sl_render_ellipse(sl_vec2_t center, sl_vec2_t radius, int segments)
{
    if (segments < 3) segments = 32;

    sl__render_check_space(segments + 1, segments * 3);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(center.x, center.y, 0.5f, 0.5f);

    float delta = (2.0f * SL_PI) / (float)segments;
    float cos_delta = cosf(delta);
    float sin_delta = sinf(delta);

    float inv2rx = 1.0f / (2.0f * radius.x);
    float inv2ry = 1.0f / (2.0f * radius.y);

    float ux = 1.0f;
    float uy = 0.0f;

    for (int i = 0; i < segments; i++) {
        float cx = radius.x * ux;
        float cy = radius.y * uy;

        float x = center.x + cx;
        float y = center.y + cy;

        float u = 0.5f + cx * inv2rx; // = 0.5f + 0.5f * ux
        float v = 0.5f + cy * inv2ry; // = 0.5f + 0.5f * uy

        sl__render_add_point(x, y, u, v);

        float new_ux = ux * cos_delta - uy * sin_delta;
        uy = ux * sin_delta + uy * cos_delta;
        ux = new_ux;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1 + i);
        sl__render_add_index(base_index + 1 + next);
    }
}

void sl_render_ellipse_lines(sl_vec2_t p, sl_vec2_t r, int segments, float thickness)
{
    if (segments < 3) segments = 32;

    float delta = (2.0f * SL_PI) / (float)segments;
    float cos_delta = cosf(delta);
    float sin_delta = sinf(delta);

    float ux = 1.0f;
    float uy = 0.0f;

    sl_vec2_t prev = { p.x + r.x * ux, p.y + r.y * uy };

    for (int i = 1; i <= segments; i++) {
        float new_ux = ux * cos_delta - uy * sin_delta;
        float new_uy = ux * sin_delta + uy * cos_delta;
        ux = new_ux;
        uy = new_uy;

        sl_vec2_t curr = (sl_vec2_t){ p.x + r.x * ux, p.y + r.y * uy };

        sl_render_line(prev, curr, thickness);
        prev = curr;
    }
}

void sl_render_pie_slice(sl_vec2_t center, float radius, float start_angle, float end_angle, int segments)
{
    if (segments < 1) segments = 16;

    float angle_diff = end_angle - start_angle;
    angle_diff = sl_wrap_radians(angle_diff);
    if (angle_diff < 0.0f) angle_diff += SL_TAU;

    float delta_angle = angle_diff / (float)segments;

    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = cosf(start_angle);
    float sin_a = sinf(start_angle);

    sl__render_check_space(segments + 2, segments * 3);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(center.x, center.y, 0.5f, 0.5f);

    for (int i = 0; i <= segments; i++) {
        float x = center.x + radius * cos_a;
        float y = center.y + radius * sin_a;

        float u = 0.5f + 0.5f * cos_a;
        float v = 0.5f + 0.5f * sin_a;

        sl__render_add_point(x, y, u, v);

        float new_cos = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos;
        sin_a = new_sin;
    }

    for (int i = 0; i < segments; i++) {
        sl__render_add_index(base_index);
        sl__render_add_index(base_index + 1 + i);
        sl__render_add_index(base_index + 1 + i + 1);
    }
}

void sl_render_pie_slice_lines(sl_vec2_t center, float radius, float start_angle, float end_angle, int segments, float thickness)
{
    if (segments < 1) segments = 16;

    float angle_diff = end_angle - start_angle;
    angle_diff = sl_wrap_radians(angle_diff);
    if (angle_diff < 0.0f) angle_diff += SL_TAU;

    float delta_angle = angle_diff / (float)segments;
    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = cosf(start_angle);
    float sin_a = sinf(start_angle);

    sl_vec2_t start_pt = { center.x + radius * cos_a, center.y + radius * sin_a };
    sl_render_line(center, start_pt, thickness);

    sl_vec2_t prev = start_pt;
    for (int i = 1; i <= segments; i++) {
        float new_cos = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos;
        sin_a = new_sin;

        sl_vec2_t curr = { center.x + radius * cos_a, center.y + radius * sin_a };
        sl_render_line(prev, curr, thickness);
        prev = curr;
    }

    sl_render_line(prev, center, thickness);
}

void sl_render_ring(sl_vec2_t center, float inner_radius, float outer_radius, int segments)
{
    if (segments < 3) segments = 32;
    if (inner_radius >= outer_radius) return;

    sl__render_check_space(segments * 2, segments * 6);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    float delta_angle = SL_TAU / (float)segments;
    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = 1.0f;
    float sin_a = 0.0f;
    float inner_scale = inner_radius / outer_radius;

    for (int i = 0; i < segments; i++) {
        float outer_x = center.x + outer_radius * cos_a;
        float outer_y = center.y + outer_radius * sin_a;
        float outer_u = 0.5f + 0.5f * cos_a;
        float outer_v = 0.5f + 0.5f * sin_a;
        sl__render_add_point(outer_x, outer_y, outer_u, outer_v);

        float inner_x = center.x + inner_radius * cos_a;
        float inner_y = center.y + inner_radius * sin_a;
        float inner_u = 0.5f + 0.5f * inner_scale * cos_a;
        float inner_v = 0.5f + 0.5f * inner_scale * sin_a;
        sl__render_add_point(inner_x, inner_y, inner_u, inner_v);

        float new_cos = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos;
        sin_a = new_sin;
    }

    for (int i = 0; i < segments; i++) {
        int next = (i + 1) % segments;
        int outer_curr = base_index + i * 2;
        int inner_curr = base_index + i * 2 + 1;
        int outer_next = base_index + next * 2;
        int inner_next = base_index + next * 2 + 1;

        sl__render_add_index(outer_curr);
        sl__render_add_index(inner_curr);
        sl__render_add_index(outer_next);

        sl__render_add_index(inner_curr);
        sl__render_add_index(inner_next);
        sl__render_add_index(outer_next);
    }
}

void sl_render_ring_lines(sl_vec2_t center, float inner_radius, float outer_radius, int segments, float thickness)
{
    if (segments < 3) segments = 32;
    if (inner_radius >= outer_radius) return;

    float delta_angle = SL_TAU / (float)segments;
    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = 1.0f;
    float sin_a = 0.0f;
    sl_vec2_t prev_outer = { center.x + outer_radius * cos_a, center.y + outer_radius * sin_a };
    sl_vec2_t prev_inner = { center.x + inner_radius * cos_a, center.y + inner_radius * sin_a };

    for (int i = 1; i <= segments; i++) {
        float new_cos = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos;
        sin_a = new_sin;

        sl_vec2_t curr_outer = { center.x + outer_radius * cos_a, center.y + outer_radius * sin_a };
        sl_vec2_t curr_inner = { center.x + inner_radius * cos_a, center.y + inner_radius * sin_a };

        sl_render_line(prev_outer, curr_outer, thickness);
        sl_render_line(prev_inner, curr_inner, thickness);

        prev_outer = curr_outer;
        prev_inner = curr_inner;
    }
}

void sl_render_ring_arc(sl_vec2_t center, float inner_radius, float outer_radius,
                        float start_angle, float end_angle, int segments)
{
    if (segments < 1) segments = 16;
    if (inner_radius >= outer_radius) return;

    float angle_diff = end_angle - start_angle;
    angle_diff = sl_wrap_radians(angle_diff);
    if (angle_diff < 0.0f) angle_diff += SL_TAU;

    float delta_angle = angle_diff / (float)segments;

    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = cosf(start_angle);
    float sin_a = sinf(start_angle);

    sl__render_check_space((segments + 1) * 2, segments * 6);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    for (int i = 0; i <= segments; i++) {
        float outer_x = center.x + outer_radius * cos_a;
        float outer_y = center.y + outer_radius * sin_a;
        float outer_u = 0.5f + 0.5f * cos_a;
        float outer_v = 0.5f + 0.5f * sin_a;
        sl__render_add_point(outer_x, outer_y, outer_u, outer_v);

        float inner_scale = inner_radius / outer_radius;
        float inner_x = center.x + inner_radius * cos_a;
        float inner_y = center.y + inner_radius * sin_a;
        float inner_u = 0.5f + 0.5f * inner_scale * cos_a;
        float inner_v = 0.5f + 0.5f * inner_scale * sin_a;
        sl__render_add_point(inner_x, inner_y, inner_u, inner_v);

        float new_cos_a = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin_a = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos_a;
        sin_a = new_sin_a;
    }

    for (int i = 0; i < segments; i++) {
        int outer_curr = base_index + i * 2;
        int inner_curr = base_index + i * 2 + 1;
        int outer_next = base_index + (i + 1) * 2;
        int inner_next = base_index + (i + 1) * 2 + 1;

        sl__render_add_index(outer_curr);
        sl__render_add_index(inner_curr);
        sl__render_add_index(outer_next);

        sl__render_add_index(inner_curr);
        sl__render_add_index(inner_next);
        sl__render_add_index(outer_next);
    }
}

void sl_render_ring_arc_lines(sl_vec2_t center, float inner_radius, float outer_radius,
                              float start_angle, float end_angle, int segments, float thickness)
{
    if (segments < 1) segments = 16;
    if (inner_radius >= outer_radius) return;

    float angle_diff = end_angle - start_angle;
    angle_diff = sl_wrap_radians(angle_diff);
    if (angle_diff < 0.0f) angle_diff += SL_TAU;

    float delta_angle = angle_diff / (float)segments;
    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = cosf(start_angle);
    float sin_a = sinf(start_angle);

    // Points initiaux
    sl_vec2_t start_outer = { center.x + outer_radius * cos_a, center.y + outer_radius * sin_a };
    sl_vec2_t start_inner = { center.x + inner_radius * cos_a, center.y + inner_radius * sin_a };

    // premier trait radial
    sl_render_line(start_inner, start_outer, thickness);

    sl_vec2_t prev_outer = start_outer;
    sl_vec2_t prev_inner = start_inner;

    for (int i = 1; i <= segments; i++) {
        float new_cos = cos_a * cos_delta - sin_a * sin_delta;
        float new_sin = sin_a * cos_delta + cos_a * sin_delta;
        cos_a = new_cos;
        sin_a = new_sin;

        sl_vec2_t curr_outer = { center.x + outer_radius * cos_a, center.y + outer_radius * sin_a };
        sl_vec2_t curr_inner = { center.x + inner_radius * cos_a, center.y + inner_radius * sin_a };

        // arcs extérieur et intérieur
        sl_render_line(prev_outer, curr_outer, thickness);
        sl_render_line(prev_inner, curr_inner, thickness);

        prev_outer = curr_outer;
        prev_inner = curr_inner;
    }

    // dernier trait radial
    sl_render_line(prev_inner, prev_outer, thickness);
}

void sl_render_line(sl_vec2_t p0, sl_vec2_t p1, float thickness)
{
    sl__render_check_space(4, 6);
    sl__render_check_state_change();

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float len_sq = dx * dx + dy * dy;

    if (len_sq < 1e-6f) {
        return;
    }

    float inv_len = 1.0f / sqrtf(len_sq);

    dx *= inv_len;
    dy *= inv_len;

    float nx = -dy * thickness * 0.5f;
    float ny = +dx * thickness * 0.5f;

    int base_index = sl__render.vertex_count;

    sl__render_add_point(p0.x + nx, p0.y + ny, 0.0f, 0.0f);
    sl__render_add_point(p0.x - nx, p0.y - ny, 1.0f, 0.0f);
    sl__render_add_point(p1.x - nx, p1.y - ny, 1.0f, 1.0f);
    sl__render_add_point(p1.x + nx, p1.y + ny, 0.0f, 1.0f);

    // Triangle 1: 0, 1, 2
    sl__render_add_index(base_index);
    sl__render_add_index(base_index + 1);
    sl__render_add_index(base_index + 2);

    // Triangle 2: 0, 2, 3
    sl__render_add_index(base_index);
    sl__render_add_index(base_index + 2);
    sl__render_add_index(base_index + 3);
}

void sl_render_arc(sl_vec2_t center, float radius,
                   float start_angle, float end_angle,
                   float thickness, int segments)
{
    if (segments < 1) segments = 16;

    float angle_diff = end_angle - start_angle;
    angle_diff = sl_wrap_radians(angle_diff);
    if (angle_diff < 0.0f) angle_diff += SL_TAU;

    float delta_angle = angle_diff / (float)segments;

    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float x = radius * cosf(start_angle);
    float y = radius * sinf(start_angle);

    float prev_x = center.x + x;
    float prev_y = center.y + y;

    for (int i = 1; i <= segments; i++) {
        float new_x = x * cos_delta - y * sin_delta;
        float new_y = x * sin_delta + y * cos_delta;
        x = new_x;
        y = new_y;

        float curr_x = center.x + x;
        float curr_y = center.y + y;

        sl_render_line(SL_VEC2(prev_x, prev_y), SL_VEC2(curr_x, curr_y), thickness);

        prev_x = curr_x;
        prev_y = curr_y;
    }
}

void sl_render_bezier_quad(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, int segments)
{
    if (segments < 1) segments = 20;

    float dt = 1.0f / (float)segments;
    float dt2 = dt * dt;

    float x  = p0.x;
    float y  = p0.y;
    float dx = 2.0f * (p1.x - p0.x) * dt;
    float dy = 2.0f * (p1.y - p0.y) * dt;

    float d2x = 2.0f * (p0.x - 2.0f * p1.x + p2.x) * dt2;
    float d2y = 2.0f * (p0.y - 2.0f * p1.y + p2.y) * dt2;

    float hd2x = d2x * 0.5f;
    float hd2y = d2y * 0.5f;

    float prev_x = x;
    float prev_y = y;

    for (int i = 1; i <= segments; i++) {
        x  += dx + hd2x;
        y  += dy + hd2y;
        dx += d2x;
        dy += d2y;

        sl_render_line(SL_VEC2(prev_x, prev_y), SL_VEC2(x, y), 1.0f);

        prev_x = x;
        prev_y = y;
    }
}

void sl_render_bezier_cubic(sl_vec2_t p0, sl_vec2_t p1, sl_vec2_t p2, sl_vec2_t p3, int segments)
{
    if (segments < 1) segments = 30;

    float dt  = 1.0f / (float)segments;
    float dt2 = dt * dt;
    float dt3 = dt2 * dt;

    float ax = -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x;
    float bx =  3.0f * (p0.x - 2.0f * p1.x + p2.x);
    float cx =  3.0f * (p1.x - p0.x);
    float dx =  p0.x;

    float ay = -p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y;
    float by =  3.0f * (p0.y - 2.0f * p1.y + p2.y);
    float cy =  3.0f * (p1.y - p0.y);
    float dy =  p0.y;

    float x = dx;
    float dx1 = cx * dt + bx * dt2 + ax * dt3;
    float dx2 = 2.0f * bx * dt2 + 6.0f * ax * dt3;
    float dx3 = 6.0f * ax * dt3;

    float y = dy;
    float dy1 = cy * dt + by * dt2 + ay * dt3;
    float dy2 = 2.0f * by * dt2 + 6.0f * ay * dt3;
    float dy3 = 6.0f * ay * dt3;

    float prev_x = x;
    float prev_y = y;

    for (int i = 1; i <= segments; ++i) {
        x  += dx1;
        dx1 += dx2;
        dx2 += dx3;

        y  += dy1;
        dy1 += dy2;
        dy2 += dy3;

        sl_render_line(SL_VEC2(prev_x, prev_y), SL_VEC2(x, y), 1.0f);

        prev_x = x;
        prev_y = y;
    }
}

void sl_render_spline(const sl_vec2_t* points, int count, int segments)
{
    if (count < 4) return;
    if (segments < 1) segments = 20;

    for (int i = 1; i < count - 2; i++) {
        sl_vec2_t p0 = points[i - 1];
        sl_vec2_t p1 = points[i];
        sl_vec2_t p2 = points[i + 1];
        sl_vec2_t p3 = points[i + 2];

        float prev_x = p1.x;
        float prev_y = p1.y;

        for (int j = 1; j <= segments; j++) {
            float t = (float)j / (float)segments;
            float t2 = t * t;
            float t3 = t2 * t;

            // Catmull-Rom coeffs
            float c0 = -0.5f * t3 + t2 - 0.5f * t;
            float c1 =  1.5f * t3 - 2.5f * t2 + 1.0f;
            float c2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
            float c3 =  0.5f * t3 - 0.5f * t2;

            float x = c0 * p0.x + c1 * p1.x + c2 * p2.x + c3 * p3.x;
            float y = c0 * p0.y + c1 * p1.y + c2 * p2.y + c3 * p3.y;

            sl_render_line(SL_VEC2(prev_x, prev_y), SL_VEC2(x, y), 1.0f);

            prev_x = x;
            prev_y = y;
        }
    }
}

void sl_render_cross(sl_vec2_t center, float size, float thickness)
{
    float half_size = size * 0.5f;
    float half_thickness = thickness * 0.5f;

    sl_render_rectangle(center.x - half_size, center.y - half_thickness, size, thickness);
    sl_render_rectangle(center.x - half_thickness, center.y - half_size, thickness, size);
}

void sl_render_grid(float x, float y, float w, float h, int cols, int rows, float thickness)
{
    if (cols < 1 || rows < 1) return;

    float cell_w = w / (float)cols;
    float cell_h = h / (float)rows;

    for (int i = 0; i <= cols; i++) { 
        float line_x = x + (float)i * cell_w;
        sl_render_line(SL_VEC2(line_x, y), SL_VEC2(line_x, y + h), thickness);
    }

    for (int i = 0; i <= rows; i++) {
        float line_y = y + (float)i * cell_h;
        sl_render_line(SL_VEC2(x, line_y), SL_VEC2(x + w, line_y), thickness);
    }
}

void sl_render_arrow(sl_vec2_t from, sl_vec2_t to, float head_size, float thickness)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float len_sq = dx * dx + dy * dy;

    if (len_sq < 1e-6f) {
        return;
    }

    float inv_len = 1.0f / sqrtf(len_sq);

    dx *= inv_len;
    dy *= inv_len;

    const float line_shorten = 0.8f;
    const float head_width_ratio = 0.6f;

    sl_vec2_t line_end = {
        to.x - head_size * line_shorten * dx,
        to.y - head_size * line_shorten * dy
    };

    sl_render_line(from, line_end, thickness);

    float base_x = to.x - head_size * dx;
    float base_y = to.y - head_size * dy;

    float perp_x = -dy;
    float perp_y = dx;

    float head_width = head_size * head_width_ratio;

    sl_vec2_t base1 = { base_x + head_width * perp_x, base_y + head_width * perp_y };
    sl_vec2_t base2 = { base_x - head_width * perp_x, base_y - head_width * perp_y };

    sl_render_triangle(to, base1, base2);
}

void sl_render_star(sl_vec2_t center, float outer_radius, float inner_radius, int points)
{
    if (points < 3) return;

    int vertex_count = points * 2 + 1;
    int triangle_count = points * 2;

    sl__render_check_space(vertex_count, triangle_count * 3);
    sl__render_check_state_change();

    int base_index = sl__render.vertex_count;

    sl__render_add_point(center.x, center.y, 0.5f, 0.5f);

    float delta_angle = SL_PI / (float)points;
    float cos_delta = cosf(delta_angle);
    float sin_delta = sinf(delta_angle);

    float cos_a = 1.0f;
    float sin_a = 0.0f;

    float inner_scale = inner_radius / outer_radius;

    for (int i = 0; i < points; i++)
    {
        /* --- Exterior point --- */

        float outer_x = center.x + outer_radius * cos_a;
        float outer_y = center.y + outer_radius * sin_a;
        float outer_u = 0.5f + 0.5f * cos_a;
        float outer_v = 0.5f + 0.5f * sin_a;
        sl__render_add_point(outer_x, outer_y, outer_u, outer_v);

        float cos_b = cos_a * cos_delta - sin_a * sin_delta;
        float sin_b = sin_a * cos_delta + cos_a * sin_delta;

        /* --- Interior point --- */

        float inner_x = center.x + inner_radius * cos_b;
        float inner_y = center.y + inner_radius * sin_b;
        float inner_u = 0.5f + 0.5f * inner_scale * cos_b;
        float inner_v = 0.5f + 0.5f * inner_scale * sin_b;
        sl__render_add_point(inner_x, inner_y, inner_u, inner_v);

        cos_a = cos_b * cos_delta - sin_b * sin_delta;
        sin_a = sin_b * cos_delta + cos_b * sin_delta;
    }

    for (int i = 0; i < points; i++) {
        int outer_curr = base_index + 1 + i * 2;
        int inner_curr = base_index + 1 + i * 2 + 1;
        int outer_next = base_index + 1 + ((i + 1) % points) * 2;

        sl__render_add_index(base_index);
        sl__render_add_index(outer_curr);
        sl__render_add_index(inner_curr);

        sl__render_add_index(base_index);
        sl__render_add_index(inner_curr);
        sl__render_add_index(outer_next);
    }
}

void sl_render_codepoint(int codepoint, sl_vec2_t position, float font_size)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* font = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (font == NULL) {
        return;
    }

    /* --- Render the codepoint --- */

    sl__render_codepoint(font, codepoint, position.x, position.y, font_size);
}

void sl_render_codepoints(const int* codepoints, int length, sl_vec2_t position, float font_size, sl_vec2_t spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* font = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (font == NULL) {
        return;
    }

    /* --- Render the codepoints --- */

    sl__render_codepoints(font, codepoints, length, position.x, position.y, font_size, spacing.x, spacing.y);
}

void sl_render_codepoints_centered(const int* codepoints, int length, sl_vec2_t position, float font_size, sl_vec2_t spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* font = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (font == NULL) {
        return;
    }

    /* --- Measure and render the codepoints --- */

    float w = 0, h = 0;
    sl__font_measure_codepoints(&w, &h, font, codepoints, length, font_size, spacing.x, spacing.y);
    sl__render_codepoints(font, codepoints, length, position.x - w * 0.5f, position.y - h * 0.5f, font_size, spacing.x, spacing.y);
}

void sl_render_text(const char* text, sl_vec2_t position, float font_size, sl_vec2_t spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* font = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (font == NULL) {
        return;
    }

    /* --- Render the text --- */

    sl__render_text(font, text, position.x, position.y, font_size, spacing.x, spacing.y);
}

void sl_render_text_centered(const char* text, sl_vec2_t position, float font_size, sl_vec2_t spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* font = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (font == NULL) {
        return;
    }

    /* --- Measure and render the text --- */

    float w = 0, h = 0;
    sl__font_measure_text(&w, &h, font, text, font_size, spacing.x, spacing.y);
    sl__render_text(font, text, position.x - w * 0.5f, position.y - h * 0.5f, font_size, spacing.x, spacing.y);
}

void sl_render_mesh(sl_mesh_id mesh, uint32_t count)
{
    // TODO: Find a solution for normal transformation.
    //       One possible approach could be to at least allow access
    //       to the internal matrices, enabling manual submission
    //       of the matrix to a custom shader.

    sl__mesh_t* data = sl__registry_get(&sl__render.reg_meshes, mesh);
    if (data == NULL) return;

    /* --- Calculate the mvp --- */

    sl_mat4_t mvp = sl_mat4_mul(&sl__render.matrix_transform, &sl__render.matrix_view);
    mvp = sl_mat4_mul(&mvp, &sl__render.matrix_proj);

    /* --- Configure the pipeline --- */

    sl__render_use_shader(sl__render.current_shader, &mvp);
    sl__render_bind_texture(0, sl__render.current_texture);
    sl__render_set_blend_mode(sl__render.current_blend_mode);

    /* --- Bind buffer and setup vertex attributes --- */

    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, normal));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, color));
    glEnableVertexAttribArray(3);

    /* --- Draw! --- */

    if (data->ebo == 0) {
        glDrawArrays(GL_TRIANGLES, 0, count);
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, NULL);
    }
}

void sl_render_mesh_lines(sl_mesh_id mesh, uint32_t count)
{
    sl__mesh_t* data = sl__registry_get(&sl__render.reg_meshes, mesh);
    if (data == NULL) return;

    /* --- Calculate the mvp --- */

    sl_mat4_t mvp = sl_mat4_mul(&sl__render.matrix_transform, &sl__render.matrix_view);
    mvp = sl_mat4_mul(&mvp, &sl__render.matrix_proj);

    /* --- Configure the pipeline --- */

    sl__render_use_shader(sl__render.current_shader, &mvp);
    sl__render_bind_texture(0, sl__render.current_texture);
    sl__render_set_blend_mode(sl__render.current_blend_mode);

    /* --- Bind buffer and setup vertex attributes --- */

    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, texcoord));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, normal));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(sl_vertex_3d_t), (void*)offsetof(sl_vertex_3d_t, color));
    glEnableVertexAttribArray(3);

    /* --- Draw! --- */

    if (data->ebo == 0) {
        glDrawArrays(GL_LINES, 0, count);
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
        glDrawElements(GL_LINES, count, GL_UNSIGNED_SHORT, NULL);
    }
}
