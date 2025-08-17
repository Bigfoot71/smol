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

#include <SDL3/SDL_stdinc.h>
#include <smol.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO

#define STBI_MALLOC(sz)         SDL_malloc(sz)
#define STBI_REALLOC(p,newsz)   SDL_realloc(p,newsz)
#define STBI_FREE(p)            SDL_free(p)

#define STBI_NO_LINEAR
#define STBI_NO_HDR

#include "stb_image.h"

/* === Internal Functions === */

static inline int sl__pixel_size(sl_pixel_format_t format)
{
    static const int sizes[] = { 1, 1, 2, 3, 4 };
    return sizes[format];
}

static inline uint8_t* sl__pixel_ptr(const sl_image_t* image, int x, int y)
{
    return (uint8_t*)image->pixels + (y * image->w + x) * sl__pixel_size(image->format);
}

static inline uint8_t sl__rgb_to_luma(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r * 77) >> 8) + ((g * 150) >> 8) + ((b * 29) >> 8);
}

static inline void sl__pixel_set_rgba8(uint32_t* ptr, sl_color_t color)
{
    *ptr = ((uint32_t)color.a << 24) | ((uint32_t)color.b << 16) | 
           ((uint32_t)color.g << 8) | (uint32_t)color.r;
}

static inline sl_color_t sl__pixel_get_rgba8(const uint32_t* ptr)
{
    uint32_t packed = *ptr;

    return (sl_color_t) {
        .r = packed & 0xFF,
        .g = (packed >> 8) & 0xFF,
        .b = (packed >> 16) & 0xFF,
        .a = (packed >> 24) & 0xFF
    };
}

/* === Public API === */

bool sl_image_create(sl_image_t* image, int w, int h, sl_pixel_format_t format)
{
    if (!image || w <= 0 || h <= 0) {
        return false;
    }

    const int pixel_size = sl__pixel_size(format);
    const size_t total_size = (size_t)w * h * pixel_size;
    
    image->w = w;
    image->h = h;
    image->format = format;
    image->pixels = SDL_calloc(1, total_size);

    return (image->pixels != NULL);
}

bool sl_image_load(sl_image_t* image, const char* file_path)
{
    if (!image || !file_path) {
        return false;
    }

    size_t data_size = 0;
    void* data = sl_file_load(file_path, &data_size);
    if (!data) {
        return false;
    }

    int channels;
    uint8_t* pixels = stbi_load_from_memory(
        data, (int)data_size,
        &image->w, &image->h,
        &channels, 0
    );
    SDL_free(data);

    if (!pixels) {
        return false;
    }

    switch (channels) {
    case 1:
        image->format = SL_PIXEL_FORMAT_LUMINANCE8;
        break;
    case 2:
        image->format = SL_PIXEL_FORMAT_LUMINANCE_ALPHA8;
        break;
    case 3:
        image->format = SL_PIXEL_FORMAT_RGB8;
        break;
    case 4:
        image->format = SL_PIXEL_FORMAT_RGBA8;
        break;
    }

    image->pixels = pixels;

    return true;
}

void sl_image_destroy(sl_image_t* image)
{
    if (image && image->pixels) {
        SDL_free(image->pixels);
        image->pixels = NULL;
        image->w = image->h = 0;
    }
}

void sl_image_set_pixel(const sl_image_t* image, int x, int y, sl_color_t color)
{
    if (!image || !image->pixels || x < 0 || y < 0 || x >= image->w || y >= image->h) {
        return;
    }

    uint8_t* ptr = sl__pixel_ptr(image, x, y);

    switch (image->format) {
    case SL_PIXEL_FORMAT_LUMINANCE8:
        *ptr = sl__rgb_to_luma(color.r, color.g, color.b);
        break;
    case SL_PIXEL_FORMAT_ALPHA8:
        *ptr = color.a;
        break;
    case SL_PIXEL_FORMAT_LUMINANCE_ALPHA8:
        ptr[0] = sl__rgb_to_luma(color.r, color.g, color.b);
        ptr[1] = color.a;
        break;
    case SL_PIXEL_FORMAT_RGB8:
        ptr[0] = color.r;
        ptr[1] = color.g;
        ptr[2] = color.b;
        break;
    case SL_PIXEL_FORMAT_RGBA8:
        sl__pixel_set_rgba8((uint32_t*)ptr, color);
        break;
    }
}

sl_color_t sl_image_get_pixel(const sl_image_t* image, int x, int y)
{
    sl_color_t black = {0, 0, 0, 255};

    if (!image || !image->pixels || x < 0 || y < 0 || x >= image->w || y >= image->h) {
        return black;
    }

    uint8_t* ptr = sl__pixel_ptr(image, x, y);

    switch (image->format) {
    case SL_PIXEL_FORMAT_LUMINANCE8:
        {
            uint8_t luma = *ptr;
            return (sl_color_t){ luma, luma, luma, 255 };
        }
    case SL_PIXEL_FORMAT_ALPHA8:
        return (sl_color_t){ 0, 0, 0, *ptr };
    case SL_PIXEL_FORMAT_LUMINANCE_ALPHA8:
        {
            uint8_t luma = ptr[0];
            return (sl_color_t){ luma, luma, luma, ptr[1] };
        }
    case SL_PIXEL_FORMAT_RGB8:
        return (sl_color_t){ ptr[0], ptr[1], ptr[2], 255 };
    case SL_PIXEL_FORMAT_RGBA8:
        return sl__pixel_get_rgba8((uint32_t*)ptr);
    }

    return black;
}

void sl_image_fill(const sl_image_t* image, sl_color_t color)
{
    if (!image || !image->pixels) return;

    uint8_t* ptr = (uint8_t*)image->pixels;
    const int total_pixels = image->w * image->h;

    switch (image->format) {
    case SL_PIXEL_FORMAT_LUMINANCE8:
        {
            uint8_t luma = sl__rgb_to_luma(color.r, color.g, color.b);
            memset(ptr, luma, total_pixels);
        }
        break;
    case SL_PIXEL_FORMAT_ALPHA8:
        memset(ptr, color.a, total_pixels);
        break;
    case SL_PIXEL_FORMAT_RGBA8:
        {
            uint32_t color_packed = ((uint32_t)color.a << 24) | ((uint32_t)color.b << 16) |
                                    ((uint32_t)color.g << 8) | (uint32_t)color.r;

            uint32_t* ptr32 = (uint32_t*)ptr;

            int i = 0;
            for (; i < total_pixels - 3; i += 4) {
                ptr32[i] = color_packed;
                ptr32[i+1] = color_packed;
                ptr32[i+2] = color_packed;
                ptr32[i+3] = color_packed;
            }
            for (; i < total_pixels; i++) {
                ptr32[i] = color_packed;
            }
        }
        break;
    default:
        for (int y = 0; y < image->h; y++) {
            for (int x = 0; x < image->w; x++) {
                sl_image_set_pixel(image, x, y, color);
            }
        }
        break;
    }
}

void sl_image_blit(const sl_image_t* dst, int x_dst, int y_dst, int w_dst, int h_dst,
                   const sl_image_t* src, int x_src, int y_src, int w_src, int h_src)
{
    /* --- Helper Macros --- */

#define BLIT_LOOP(PIXEL_OPERATION) \
    for (int dy = 0; dy < h_dst; dy++) { \
        const int sy = ((dy * scale_y) >> 16) + y_src; \
        const int dst_y = y_dst + dy; \
        if (sy < 0 || sy >= src->h || dst_y < 0 || dst_y >= dst->h) { \
            continue; \
        } \
        uint8_t* dst_row = dst_data + dst_y * dst_row_stride; \
        uint8_t* src_row = src_data + sy * src_row_stride; \
        for (int dx = 0; dx < w_dst; dx++) { \
            const int sx = ((dx * scale_x) >> 16) + x_src; \
            const int dst_x = x_dst + dx; \
            if (sx < 0 || sx >= src->w || dst_x < 0 || dst_x >= dst->w) { \
                continue; \
            } \
            uint8_t* dst_pixel = dst_row + dst_x * dst_pixel_size; \
            uint8_t* src_pixel = src_row + sx * src_pixel_size; \
            PIXEL_OPERATION \
        } \
    }

#define COPY_PIXEL \
    switch (dst_pixel_size) { \
        case 1: *dst_pixel = *src_pixel; break; \
        case 2: *(uint16_t*)dst_pixel = *(uint16_t*)src_pixel; break; \
        case 3: \
            dst_pixel[0] = src_pixel[0]; \
            dst_pixel[1] = src_pixel[1]; \
            dst_pixel[2] = src_pixel[2]; \
            break; \
        case 4: *(uint32_t*)dst_pixel = *(uint32_t*)src_pixel; break; \
    }

#define CONVERT_PIXEL \
    sl_color_t color = sl_image_get_pixel(src, sx, sy); \
    sl_image_set_pixel(dst, dst_x, dst_y, color); \

    /* --- Input check --- */

    if (!dst || !src || !dst->pixels || !src->pixels) return;
    if (w_dst <= 0 || h_dst <= 0 || w_src <= 0 || h_src <= 0) return;

    /* --- Fixed point scales (16.16) --- */

    const int scale_x = (w_src << 16) / w_dst;
    const int scale_y = (h_src << 16) / h_dst;
    const int dst_pixel_size = sl__pixel_size(dst->format);
    const int src_pixel_size = sl__pixel_size(src->format);
    uint8_t* dst_data = (uint8_t*)dst->pixels;
    uint8_t* src_data = (uint8_t*)src->pixels;
    const int dst_row_stride = dst->w * dst_pixel_size;
    const int src_row_stride = src->w * src_pixel_size;

    /* --- Blit handling cases of non scaling or identical format --- */

    if (dst->format == src->format && w_dst == w_src && h_dst == h_src) {
        for (int dy = 0; dy < h_dst; dy++) {
            const int src_y = dy + y_src;
            const int dst_y = dy + y_dst;
            if (src_y < 0 || src_y >= src->h || dst_y < 0 || dst_y >= dst->h) {
                continue;
            }
            const int copy_start_x = SL_MAX(0, SL_MAX(-x_dst, -x_src));
            const int copy_end_x = SL_MIN(w_dst, SL_MIN(dst->w - x_dst, src->w - x_src));
            if (copy_end_x <= copy_start_x) {
                continue;
            }
            uint8_t* dst_row = dst_data + dst_y * dst_row_stride + (x_dst + copy_start_x) * dst_pixel_size;
            uint8_t* src_row = src_data + src_y * src_row_stride + (x_src + copy_start_x) * src_pixel_size;
            SDL_memcpy(dst_row, src_row, (copy_end_x - copy_start_x) * dst_pixel_size);
        }
    }
    else if (dst->format == src->format) {
        BLIT_LOOP(COPY_PIXEL)
    }
    else {
        BLIT_LOOP(CONVERT_PIXEL)
    }

    /* -- Undefs --- */

#undef BLIT_LOOP
#undef COPY_PIXEL
#undef CONVERT_PIXEL
}

bool sl_image_resize(sl_image_t* image, int new_w, int new_h)
{
    if (!image || new_w <= 0 || new_h <= 0) {
        return false;
    }

    sl_image_t temp;
    if (!sl_image_create(&temp, new_w, new_h, image->format)) {
        return false;
    }

    sl_image_blit(&temp, 0, 0, new_w, new_h, image, 0, 0, image->w, image->h);

    SDL_free(image->pixels);
    *image = temp;

    return true;
}

void sl_image_flip_horizontal(const sl_image_t* image)
{
    if (!image || !image->pixels) {
        return;
    }

    const int pixel_size = sl__pixel_size(image->format);
    const int row_size = image->w * pixel_size;
    uint8_t* temp_row = SDL_malloc(row_size);

    if (!temp_row) {
        return;
    }

    uint8_t* data = (uint8_t*)image->pixels;

    for (int y = 0; y < image->h; y++) {
        uint8_t* row = data + y * row_size;
        SDL_memcpy(temp_row, row, row_size);
        for (int x = 0; x < image->w; x++) {
            SDL_memcpy(
                row + x * pixel_size, 
                temp_row + (image->w - 1 - x) * pixel_size, 
                pixel_size
            );
        }
    }

    SDL_free(temp_row);
}

void sl_image_flip_vertical(const sl_image_t* image)
{
    if (!image || !image->pixels) {
        return;
    }

    const int pixel_size = sl__pixel_size(image->format);
    const int row_size = image->w * pixel_size;
    uint8_t* temp_row = SDL_malloc(row_size);

    if (!temp_row) {
        return;
    }

    uint8_t* data = (uint8_t*)image->pixels;

    for (int y = 0; y < image->h / 2; y++) {
        uint8_t* top_row = data + y * row_size;
        uint8_t* bottom_row = data + (image->h - 1 - y) * row_size;
        SDL_memcpy(temp_row, top_row, row_size);
        SDL_memcpy(top_row, bottom_row, row_size);
        SDL_memcpy(bottom_row, temp_row, row_size);
    }

    SDL_free(temp_row);
}

void sl_image_draw_rectangle(const sl_image_t* image, int x, int y, int w, int h, sl_color_t color, bool filled)
{
    if (!image || !image->pixels || w <= 0 || h <= 0) {
        return;
    }

    if (filled) {
        for (int dy = 0; dy < h; dy++) {
            for (int dx = 0; dx < w; dx++) {
                sl_image_set_pixel(image, x + dx, y + dy, color);
            }
        }
    }
    else {
        for (int dx = 0; dx < w; dx++) {
            sl_image_set_pixel(image, x + dx, y, color);
            sl_image_set_pixel(image, x + dx, y + h - 1, color);
        }
        for (int dy = 1; dy < h - 1; dy++) {
            sl_image_set_pixel(image, x, y + dy, color);
            sl_image_set_pixel(image, x + w - 1, y + dy, color);
        }
    }
}

void sl_image_draw_circle(const sl_image_t* image, int cx, int cy, int radius, sl_color_t color, bool filled)
{
    if (!image || !image->pixels || radius <= 0) return;

    const int r2 = radius * radius;

    if (filled) {
        for (int y = -radius; y <= radius; y++) {
            for (int x = -radius; x <= radius; x++) {
                if (x*x + y*y <= r2) {
                    sl_image_set_pixel(image, cx + x, cy + y, color);
                }
            }
        }
    }
    else {
        int x = 0, y = radius;
        int d = 3 - 2 * radius;

        while (y >= x) {
            sl_image_set_pixel(image, cx + x, cy + y, color);
            sl_image_set_pixel(image, cx - x, cy + y, color);
            sl_image_set_pixel(image, cx + x, cy - y, color);
            sl_image_set_pixel(image, cx - x, cy - y, color);
            sl_image_set_pixel(image, cx + y, cy + x, color);
            sl_image_set_pixel(image, cx - y, cy + x, color);
            sl_image_set_pixel(image, cx + y, cy - x, color);
            sl_image_set_pixel(image, cx - y, cy - x, color);

            x++;
            if (d > 0) {
                y--;
                d = d + 4 * (x - y) + 10;
            } else {
                d = d + 4 * x + 6;
            }
        }
    }
}

void sl_image_draw_line(const sl_image_t* image, int x0, int y0, int x1, int y1, sl_color_t color)
{
    if (!image || !image->pixels) {
        return;
    }

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        sl_image_set_pixel(image, x0, y0, color);

        if (x0 == x1 && y0 == y1) {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
