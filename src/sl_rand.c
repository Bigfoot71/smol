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

#include <SDL3/SDL_stdinc.h>

/* === Internal Implementation === */

static struct {
    uint64_t state;  // Internal 64-bit state
    uint64_t inc;    // Increment (must be odd)
} sl__rand = {
    0x853c49e6748fea9bULL,
    0xda3e39cb94b95bdbULL
};

#define PCG32_MULT 0x5851f42d4c957f2dULL

static inline uint32_t pcg32_rotr(uint32_t value, uint32_t rot)
{
    return (value >> rot) | (value << ((-rot) & 31));
}

static inline uint32_t pcg32_next(void)
{
    uint64_t oldstate = sl__rand.state;
    sl__rand.state = oldstate * PCG32_MULT + sl__rand.inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return pcg32_rotr(xorshifted, rot);
}

/* === Public API === */

void sl_rand_seed(uint64_t seed)
{
    sl__rand.state = 0U;
    sl__rand.inc = (seed << 1u) | 1u;  // Make sure inc is odd
    pcg32_next();
    sl__rand.state += seed;
    pcg32_next();
}

int32_t sl_randi(void)
{
    // Generates a signed integer using the high 31 bits
    // to avoid problematic negative values
    return (int)(pcg32_next() >> 1);
}

uint32_t sl_randui(void)
{
    return pcg32_next();
}

float sl_randf(void)
{
    // Convert to float [0.0, 1.0] using 24-bit precision
    // Divide by 2^24 for uniform distribution
    return (pcg32_next() >> 8) * 0x1.0p-24f;
}

int sl_randi_range(int min, int max)
{
    if (min >= max) return min;

    uint32_t range = (uint32_t)(max - min);
    uint32_t threshold = -range % range;

    uint32_t r;
    do r = pcg32_next();
    while (r < threshold);

    return min + (int)(r % range);
}

uint32_t sl_randui_range(uint32_t min, uint32_t max)
{
    if (min >= max) return min;

    uint32_t range = max - min;
    uint32_t threshold = -range % range;

    uint32_t r;
    do r = pcg32_next();
    while (r < threshold);

    return min + (r % range);
}

float sl_randf_range(float min, float max)
{
    return min + (max - min) * sl_randf();
}

void sl_rand_shuffle(void* array, size_t element_size, size_t count)
{
    if (!array || count <= 1 || element_size == 0) {
        return;
    }

    char* arr = (char*)array;

    if (element_size % sizeof(size_t) == 0)
    {
        size_t words = element_size / sizeof(size_t);
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = sl_randui_range(0, i + 1);
            if (i != j) {
                size_t* a = (size_t*)(arr + i * element_size);
                size_t* b = (size_t*)(arr + j * element_size);
                for (size_t w = 0; w < words; w++) {
                    size_t tmp = a[w];
                    a[w] = b[w];
                    b[w] = tmp;
                }
            }
        }
    }
    else if (element_size <= 64)
    {
        char temp[64];
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = sl_randui_range(0, i + 1);
            if (i != j) {
                char* elem_i = arr + i * element_size;
                char* elem_j = arr + j * element_size;
                SDL_memcpy(temp, elem_i, element_size);
                SDL_memcpy(elem_i, elem_j, element_size);
                SDL_memcpy(elem_j, temp, element_size);
            }
        }
    }
    else
    {
        char* temp = SDL_malloc(element_size);
        if (!temp) {
            return;
        }
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = sl_randui_range(0, i + 1);
            if (i != j) {
                char* elem_i = arr + i * element_size;
                char* elem_j = arr + j * element_size;
                SDL_memcpy(temp, elem_i, element_size);
                SDL_memcpy(elem_i, elem_j, element_size);
                SDL_memcpy(elem_j, temp, element_size);
            }
        }
        SDL_free(temp);
    }
}
