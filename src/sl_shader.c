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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "./internal/sl__render.h"
#include "internal/sl__registry.h"

/* === Shader Templates === */

static const char* sl__shader_vertex_header_str =
{
    "#version 100\n"
    "#define VERTEX\n"
    "attribute vec3 a_position;"
    "attribute vec2 a_texcoord;"
    "attribute vec3 a_normal;"
    "attribute vec4 a_color;"
    "uniform mat4 u_mvp;"
    "varying vec3 v_position;"
    "varying vec2 v_texcoord;"
    "varying vec3 v_normal;"
    "varying vec4 v_color;\n"
};

static const char* sl__shader_vertex_function_str =
{
    "vec4 vertex(mat4 mvp, vec3 position)"
    "{"
    "    return mvp * vec4(position, 1.0);"
    "}"
};

static const char* sl__shader_vertex_main_str =
{
    "\nvoid main()"
    "{"
    "    gl_Position = vertex(u_mvp, a_position);"
    "    v_position = a_position;"
    "    v_texcoord = a_texcoord;"
    "    v_normal = a_normal;"
    "    v_color = a_color;"
    "}"
};

static const char* sl__shader_fragment_header_str =
{
    "#version 100\n"
    "#define PIXEL\n"
    "precision mediump float;"
    "uniform sampler2D u_texture;"
    "varying vec3 v_position;"
    "varying vec2 v_texcoord;"
    "varying vec3 v_normal;"
    "varying vec4 v_color;\n"
};

static const char* sl__shader_fragment_function_str =
{
    "vec4 pixel(vec4 color, sampler2D tex, vec2 uv, vec2 screen_pos)"
    "{"
    "    return color * texture2D(tex, uv);"
    "}"
};

static const char* sl__shader_fragment_main_str =
{
    "\nvoid main()"
    "{"
    "    gl_FragColor = pixel(v_color, u_texture, v_texcoord, gl_FragCoord.xy);"
    "}"
};

/* === Internal Functions === */

static int sl__shader_has_function(const char* code, const char* function_name)
{
    if (!code) return 0;

    size_t pattern_len = SDL_strlen("vec4 ") + SDL_strlen(function_name) + SDL_strlen("(") + 1;
    char* search_pattern = SDL_malloc(pattern_len);
    if (!search_pattern) return 0;

    SDL_snprintf(search_pattern, pattern_len, "vec4 %s(", function_name);

    int found = (SDL_strstr(code, search_pattern) != NULL);
    SDL_free(search_pattern);

    return found;
}

static size_t sl__shader_calculate_source_size(const char* header, 
                                               const char* user_code, 
                                               const char* default_function, 
                                               const char* main_code,
                                               int has_user_function)
{
    size_t size = 0;

    /* --- Header --- */

    size += SDL_strlen(header);
    size += 1; // \n after header

    /* --- User code or default functions --- */

    if (has_user_function && user_code) {
        size += SDL_strlen(user_code);
    } else {
        size += SDL_strlen(default_function);
    }

    size += 1; // \n before main

    /* --- Main function --- */

    size += SDL_strlen(main_code);

    size += 1; // \0 final

    return size;
}

static char* sl__shader_build_source(const char* header, 
                                     const char* user_code, 
                                     const char* default_function, 
                                     const char* main_code,
                                     const char* function_name)
{
    int has_user_function = sl__shader_has_function(user_code, function_name);

    size_t total_size = sl__shader_calculate_source_size(
        header, user_code, default_function, main_code, has_user_function
    );

    char* shader_source = SDL_malloc(total_size);
    if (!shader_source) return NULL;

    strcpy(shader_source, header);
    strcat(shader_source, "\n");
    
    if (has_user_function && user_code) {
        strcat(shader_source, user_code);
    } else {
        strcat(shader_source, default_function);
    }

    strcat(shader_source, "\n");
    strcat(shader_source, main_code);

    return shader_source;
}

static GLuint sl__shader_compile(const char* source, GLenum shader_type)
{
    GLuint shader = glCreateShader(shader_type);
    if (!shader) return 0;

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
        sl_loge("SHADER: Failed to compile %s shader: %s\n", 
               shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment", 
               info_log);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

static GLuint sl__shader_program_link(const char* vertex_source, const char* fragment_source)
{
    GLuint vertex_shader = sl__shader_compile(vertex_source, GL_VERTEX_SHADER);
    if (!vertex_shader) return 0;

    GLuint fragment_shader = sl__shader_compile(fragment_source, GL_FRAGMENT_SHADER);
    if (!fragment_shader) {
        glDeleteShader(vertex_shader);
        return 0;
    }

    GLuint program = glCreateProgram();
    if (!program) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[1024];
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
        sl_loge("SHADER: Failed to link program shader; %s\n", info_log);
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

/* === Public API === */

sl_shader_id sl_shader_create(const char* code)
{
    char* vertex_source = NULL;
    char* fragment_source = NULL;
    GLuint program = 0;
    sl_shader_id result = 0;

    /* --- Build vertex shader --- */

    vertex_source = sl__shader_build_source(
        sl__shader_vertex_header_str,
        code,
        sl__shader_vertex_function_str,
        sl__shader_vertex_main_str,
        "vertex"
    );

    if (!vertex_source) goto cleanup;

    /* --- Build fragment shader --- */

    fragment_source = sl__shader_build_source(
        sl__shader_fragment_header_str,
        code,
        sl__shader_fragment_function_str,
        sl__shader_fragment_main_str,
        "pixel"
    );

    if (!fragment_source) goto cleanup;

    /* --- Link program --- */

    program = sl__shader_program_link(vertex_source, fragment_source);
    if (!program) goto cleanup;

    /* --- Push shader to the registry --- */

    sl__shader_t shader = {
        .id = program,
        .loc_mvp = glGetUniformLocation(program, "u_mvp")
    };

    result = sl__registry_add(&sl__render.reg_shaders, &shader);

cleanup:
    SDL_free(vertex_source);
    SDL_free(fragment_source);

    return result;
}

sl_shader_id sl_shader_load(const char* file_path)
{
    char* code = sl_file_load_text(file_path);
    sl_shader_id result = sl_shader_create(code);
    SDL_free(code);
    return result;
}

void sl_shader_destroy(sl_shader_id shader)
{
    if (shader == 0 || shader == sl__render.default_shader) {
        return;
    }

    if (sl__render.current_shader == shader) {
        sl__render.current_shader = sl__render.default_shader;
    }

    sl__shader_t* data = sl__registry_get(&sl__render.reg_shaders, shader);
    if (data == NULL) return;

    glDeleteProgram(data->id);
    sl__registry_remove(&sl__render.reg_shaders, shader);
}

int sl_shader_uniform(sl_shader_id shader, const char* name)
{
    sl__shader_t* data = sl__registry_get(&sl__render.reg_shaders, shader);
    if (!data) return -1;

    return glGetUniformLocation(data->id, name);
}
