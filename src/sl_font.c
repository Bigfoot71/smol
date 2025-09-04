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

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_time.h>

 /* === STB TrueType Implementation === */

#define STB_TRUETYPE_IMPLEMENTATION

#define STBTT_malloc(x,u)  ((void)(u),SDL_malloc(x))
#define STBTT_free(x,u)    ((void)(u),SDL_free(x))

#include <stb_truetype.h>

/* === Internal Functions Declarations === */

static bool
generate_font_atlas(sl_image_t* image, const uint8_t* file_data, int data_size, sl_font_type_t font_type,
                    int base_size, int* codepoints, int codepoint_count, int padding, sl__glyph_t** out_glyphs);

/* === Public API === */

sl_font_id sl_font_load_from_memory(const void* file_data, size_t data_size, sl_font_type_t type,
                                    int base_size, int* codepoints, int codepoint_count)
{
#   define FONT_TTF_DEFAULT_SIZE           32
#   define FONT_TTF_DEFAULT_NUMCHARS       95
#   define FONT_TTF_DEFAULT_FIRST_CHAR     32
#   define FONT_TTF_DEFAULT_CHARS_PADDING   4

    sl__font_t font = { 0 };

    /* --- Base configuration --- */

    codepoint_count = (codepoint_count > 0) ? codepoint_count : FONT_TTF_DEFAULT_NUMCHARS;
    font.glyph_padding = FONT_TTF_DEFAULT_CHARS_PADDING;
    font.glyph_count = codepoint_count;
    font.base_size = base_size;
    font.type = type;

    /* --- Generation of the atlas image --- */

    sl_image_t atlas = { 0 };

    bool atlasGenerated = generate_font_atlas(
        &atlas, file_data, data_size,
        type, base_size,
        codepoints, codepoint_count,
        font.glyph_padding,
        &font.glyphs
    );

    if (!atlasGenerated) {
        SDL_free(font.glyphs);
        return 0;
    }

    /* --- Creating the atlas texture --- */

    font.texture = sl_texture_load_from_memory(&atlas);
    sl_image_destroy(&atlas);

    if (font.texture == 0) {
        SDL_free(font.glyphs);
        return 0;
    }

    switch (font.type) {
    case SL_FONT_BITMAP:
        sl_texture_parameters(font.texture, SL_FILTER_BILINEAR, SL_WRAP_CLAMP);
        break;
    case SL_FONT_PIXEL:
        sl_texture_parameters(font.texture, SL_FILTER_NEAREST, SL_WRAP_CLAMP);
        break;
    case SL_FONT_SDF:
        sl_texture_parameters(font.texture, SL_FILTER_BILINEAR, SL_WRAP_CLAMP);
        break;
    }

    /* --- Push texture to the registry --- */

    return sl__registry_add(&sl__render.reg_fonts, &font);
}

sl_font_id sl_font_load(const char* filePath, sl_font_type_t type, int base_size, int* codepoints, int codepoint_count)
{
    size_t data_size = 0;
    void* data = sl_file_load(filePath, &data_size);
    if (data == NULL) {
        return false;
    }

    sl_font_id id = sl_font_load_from_memory(data, (int)data_size, type, base_size, codepoints, codepoint_count);
    SDL_free(data);

    return id;
}

void sl_font_destroy(sl_font_id font)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* data = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (data == NULL) {
        return;
    }

    /* --- Release contained data --- */

    sl_texture_destroy(data->texture);
    SDL_free(data->glyphs);

    /* --- Remove font from the registry --- */

    sl__registry_remove(&sl__render.reg_fonts, font);
}

void sl_font_measure_text(float* w, float* h, const sl_font_id font, const char* text, float font_size, float x_spacing, float y_spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* data = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (data == NULL) {
        return;
    }

    /* --- Measure the text --- */

    sl__font_measure_text(w, h, data, text, font_size, x_spacing, y_spacing);
}

void sl_font_measure_codepoints(float* w, float* h, const sl_font_id font, const int* codepoints, int length, float font_size, float x_spacing, float y_spacing)
{
    /* --- Check and get the current font --- */

    if (sl__render.current_font == 0) {
        return;
    }

    sl__font_t* data = sl__registry_get(&sl__render.reg_fonts, sl__render.current_font);
    if (data == NULL) {
        return;
    }

    /* --- Measure the codepoints --- */

    sl__font_measure_codepoints(w, h, data, codepoints, length, font_size, x_spacing, y_spacing);
}

/* === Internal Functions === */

bool generate_font_atlas(sl_image_t* atlas, const uint8_t* file_data, int data_size, sl_font_type_t font_type,
                         int base_size, int* codepoints, int codepoint_count, int padding, sl__glyph_t** out_glyphs)
{
    assert(atlas != NULL);

    /* --- Constants --- */

#   define FONT_SDF_CHAR_PADDING         4
#   define FONT_SDF_ON_EDGE_VALUE        128
#   define FONT_SDF_PIXEL_DIST_SCALE     32.0f
#   define FONT_BITMAP_ALPHA_THRESHOLD   80

    *out_glyphs = NULL;

    /* --- Cleanup Variables --- */

    uint8_t* work_buffer = NULL;
    int* default_codepoints = NULL;
    stbrp_context* pack_context = NULL;
    stbrp_node* pack_nodes = NULL;

    /* --- Font Validation and Init --- */

    if (!file_data) {
        return false;
    }

    stbtt_fontinfo font_info = { 0 };
    if (!stbtt_InitFont(&font_info, (uint8_t*)file_data, 0)) {
        return false;
    }

    /* --- Font Scale and Metrics --- */

    float scale = stbtt_ScaleForPixelHeight(&font_info, (float)base_size);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &lineGap);
    int scaled_ascent = (int)roundf((float)ascent * scale);

    /* --- Generate Default Codepoints if Needed --- */

    if (!codepoints) {
        codepoint_count = (codepoint_count > 0) ? codepoint_count : 95;
        default_codepoints = SDL_malloc(codepoint_count * sizeof(int));
        if (!default_codepoints) {
            goto cleanup;
        }
        // Generate ASCII printable characters (32-126)
        for (int i = 0; i < codepoint_count; i++) {
            default_codepoints[i] = i + 32;
        }
        codepoints = default_codepoints;
    }

    /* --- Allocate Working Buffers --- */

    size_t buffer_size = codepoint_count * (sizeof(stbrp_rect) + sizeof(sl__glyph_t));
    work_buffer = SDL_malloc(buffer_size);
    if (!work_buffer) {
        goto cleanup;
    }

    // Setup buffer pointers
    stbrp_rect* pack_rects = (stbrp_rect*)work_buffer;
    sl__glyph_t* glyphs = (sl__glyph_t*)(work_buffer + codepoint_count * sizeof(stbrp_rect));

    memset(glyphs, 0, codepoint_count * sizeof(sl__glyph_t));

    /* --- First Pass: Calculate Glyph Dimensions --- */

    int total_area = 0;
    int max_h_glyph = 0;

    for (int i = 0; i < codepoint_count; i++)
    {
        int ch = codepoints[i];
        glyphs[i].value = ch;

        int glyphIndex = stbtt_FindGlyphIndex(&font_info, ch);
        if (glyphIndex == 0) {
            pack_rects[i].w = pack_rects[i].h = 0;
            continue;
        }

        if (ch == 32) { // Space character
            stbtt_GetCodepointHMetrics(&font_info, ch, &glyphs[i].x_advance, NULL);
            glyphs[i].x_advance = (int)((float)glyphs[i].x_advance * scale);
            glyphs[i].x_offset = glyphs[i].y_offset = 0;

            pack_rects[i].w = glyphs[i].x_advance + 2 * padding;
            pack_rects[i].h = base_size + 2 * padding;
        }
        else { // Regular character
            int x0, y0, x1, y1;
            stbtt_GetCodepointBitmapBox(&font_info, ch, scale, scale, &x0, &y0, &x1, &y1);

            int w_glyph = x1 - x0;
            int h_glyph = y1 - y0;

            // Add SDF padding if needed
            if (font_type == SL_FONT_SDF) {
                w_glyph += 2 * FONT_SDF_CHAR_PADDING;
                h_glyph += 2 * FONT_SDF_CHAR_PADDING;
                glyphs[i].x_offset = x0 - FONT_SDF_CHAR_PADDING;
                glyphs[i].y_offset = y0 - FONT_SDF_CHAR_PADDING + scaled_ascent;
            }
            else {
                glyphs[i].x_offset = x0;
                glyphs[i].y_offset = y0 + scaled_ascent;
            }

            pack_rects[i].w = w_glyph + 2 * padding;
            pack_rects[i].h = h_glyph + 2 * padding;

            stbtt_GetCodepointHMetrics(&font_info, ch, &glyphs[i].x_advance, NULL);
            glyphs[i].x_advance = (int)((float)glyphs[i].x_advance * scale);

            if (h_glyph > max_h_glyph) {
                max_h_glyph = h_glyph;
            }
        }

        pack_rects[i].id = i;
        total_area += pack_rects[i].w * pack_rects[i].h;
    }

    /* --- Calculate Atlas Dimensions --- */

    total_area = (int)(total_area * 1.3f); // Safety margin
    int atlas_size = (int)roundf(sqrtf((float)total_area));

    // Get next po2 if necessary
    if ((atlas_size & (atlas_size - 1)) != 0) {
        --atlas_size;
        atlas_size |= atlas_size >> 1;
        atlas_size |= atlas_size >> 2;
        atlas_size |= atlas_size >> 4;
        atlas_size |= atlas_size >> 8;
        atlas_size |= atlas_size >> 16;
        ++atlas_size;
    }

    // Try rectangle first (wider than tall)
    atlas->w = atlas_size;
    atlas->h = atlas_size / 2;

    // Use square if rectangle is too small
    if (total_area > atlas->w * atlas->h) {
        atlas->h = atlas_size;
    }

    /* --- Create Atlas Image --- */

    atlas->pixels = SDL_calloc(atlas->w * atlas->h, 2);
    if (!atlas->pixels) {
        goto cleanup;
    }
    atlas->format = SL_PIXEL_FORMAT_LUMINANCE_ALPHA8;

    /* --- Rectangle Packing Setup --- */

    pack_context = SDL_malloc(sizeof(*pack_context));
    pack_nodes = SDL_malloc(atlas->w * sizeof(*pack_nodes));

    if (!pack_context || !pack_nodes) {
        goto cleanup;
    }

    stbrp_init_target(pack_context, atlas->w, atlas->h, pack_nodes, atlas->w);
    stbrp_pack_rects(pack_context, pack_rects, codepoint_count);

    /* --- Second Pass: Render Glyphs to Atlas --- */

    for (int i = 0; i < codepoint_count; i++)
    {
        if (!pack_rects[i].was_packed) {
            continue;
        }

        int ch = glyphs[i].value;
        int x_dst = pack_rects[i].x + padding;
        int y_dst = pack_rects[i].y + padding;

        if (ch == 32) { // Space - transparent pixels
            glyphs[i].x_atlas = (float)x_dst;
            glyphs[i].y_atlas = (float)y_dst;
            glyphs[i].w_atlas = (float)glyphs[i].x_advance;
            glyphs[i].h_atlas = (float)base_size;
            continue;
        }

        /* --- Generate Glyph Bitmap --- */

        uint8_t* glyph_bitmap = NULL;
        int w_glyph, h_glyph, x_offset, y_offset;

        if (font_type == SL_FONT_SDF) {
            glyph_bitmap = stbtt_GetCodepointSDF(
                &font_info, scale, ch, FONT_SDF_CHAR_PADDING,
                FONT_SDF_ON_EDGE_VALUE, FONT_SDF_PIXEL_DIST_SCALE,
                &w_glyph, &h_glyph, &x_offset, &y_offset
            );
        }
        else {
            glyph_bitmap = stbtt_GetCodepointBitmap(
                &font_info, scale, scale, ch,
                &w_glyph, &h_glyph, &x_offset, &y_offset
            );
        }

        if (!glyph_bitmap) {
            continue;
        }

        // Apply threshold for pixel fonts
        if (font_type == SL_FONT_PIXEL) {
            for (int p = 0; p < w_glyph * h_glyph; p++) {
                glyph_bitmap[p] = (glyph_bitmap[p] < FONT_BITMAP_ALPHA_THRESHOLD) ? 0 : 255;
            }
        }

        // Copy glyph to atlas (line by line)
        uint8_t* atlasData = (uint8_t*)atlas->pixels;
        for (int y = 0; y < h_glyph; y++) {
            uint8_t* dstLine = &atlasData[((y_dst + y) * atlas->w + x_dst) * 2];
            const uint8_t* srcLine = &glyph_bitmap[y * w_glyph];
            for (int x = 0; x < w_glyph; x++) {
                uint8_t v = srcLine[x];
                dstLine[x * 2 + 0] = v; // R
                dstLine[x * 2 + 1] = v; // G
            }
        }

        stbtt_FreeBitmap(glyph_bitmap, NULL);

        glyphs[i].x_atlas = (float)x_dst;
        glyphs[i].y_atlas = (float)y_dst;
        glyphs[i].w_atlas = (float)w_glyph;
        glyphs[i].h_atlas = (float)h_glyph;
    }

    /* --- Allocate Output Arrays --- */

    *out_glyphs = SDL_malloc(codepoint_count * sizeof(sl__glyph_t));
    if (!*out_glyphs) {
        goto cleanup;
    }

    memcpy(*out_glyphs, glyphs, codepoint_count * sizeof(sl__glyph_t));

    /* --- Cleanup --- */

    SDL_free(work_buffer);
    SDL_free(pack_nodes);
    SDL_free(pack_context);
    SDL_free(default_codepoints);

    return true;

cleanup:

    /* --- Error Cleanup --- */

    SDL_free(atlas->pixels);
    SDL_free(work_buffer);
    SDL_free(pack_nodes);
    SDL_free(pack_context);
    SDL_free(default_codepoints);
    SDL_free(*out_glyphs);

    *out_glyphs = NULL;
    SDL_memset(atlas, 0, sizeof(*atlas));

    return false;
}
