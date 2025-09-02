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

/* === Public API === */

int sl_codepoint_next(const char* text, int* codepoint_size)
{
    const char* ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepoint_size = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0])) {
        // 4 byte UTF-8 codepoint
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) return codepoint; // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepoint_size = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0])) {
        // 3 byte UTF-8 codepoint */
        if (((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) return codepoint; // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepoint_size = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0])) {
        // 2 byte UTF-8 codepoint
        if ((ptr[1] & 0xC0) ^ 0x80) return codepoint; // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepoint_size = 2;
    }
    else if (0x00 == (0x80 & ptr[0])) {
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepoint_size = 1;
    }

    return codepoint;
}

int sl_codepoint_prev(const char* text, int* codepoint_size)
{
    const char* ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    int cp_size = 0;
    *codepoint_size = 0;

    // Move to previous codepoint
    do ptr--; while (((0x80 & ptr[0]) != 0) && ((0xc0 & ptr[0]) == 0x80));

    codepoint = sl_codepoint_next(ptr, &cp_size);

    if (codepoint != 0) {
        *codepoint_size = cp_size;
    }

    return codepoint;
}

int sl_codepoint_count(const char* text)
{
    uint32_t length = 0;
    const char* ptr = text;

    while (*ptr != '\0') {
        int next = 0;
        sl_codepoint_next(ptr, &next);
        ptr += next;
        length++;
    }

    return length;
}

const char* sl_codepoint_to_utf8(int codepoint, int* utf8_size)
{
    static char utf8[6];
    SDL_memset(utf8, 0, 6);
    int size = 0;

    if (codepoint <= 0x7f) {
        utf8[0] = (char)codepoint;
        size = 1;
    }
    else if (codepoint <= 0x7ff) {
        utf8[0] = (char)(((codepoint >> 6) & 0x1f) | 0xc0);
        utf8[1] = (char)((codepoint & 0x3f) | 0x80);
        size = 2;
    }
    else if (codepoint <= 0xffff) {
        utf8[0] = (char)(((codepoint >> 12) & 0x0f) | 0xe0);
        utf8[1] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
        utf8[2] = (char)((codepoint & 0x3f) | 0x80);
        size = 3;
    }
    else if (codepoint <= 0x10ffff) {
        utf8[0] = (char)(((codepoint >> 18) & 0x07) | 0xf0);
        utf8[1] = (char)(((codepoint >> 12) & 0x3f) | 0x80);
        utf8[2] = (char)(((codepoint >> 6) & 0x3f) | 0x80);
        utf8[3] = (char)((codepoint & 0x3f) | 0x80);
        size = 4;
    }

    *utf8_size = size;

    return utf8;
}

int sl_codepoint_from_utf8(const char* text, int* codepoint_size)
{
    /*
        UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

        Char. number range  |        UTF-8 byte sequence
          (hexadecimal)    |              (binary)
        --------------------+---------------------------------------------
        0000 0000-0000 007F | 0xxxxxxx
        0000 0080-0000 07FF | 110xxxxx 10xxxxxx
        0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
        0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
    // NOTE: on decode errors we return as soon as possible

    int codepoint = 0x3f;   // Codepoint (defaults to '?')
    int byte = (uint8_t)(text[0]); // The first UTF8 byte
    *codepoint_size = 1;

    if (byte <= 0x7f) {
        // Only one byte (ASCII range x00-7F)
        codepoint = text[0];
    }
    else if ((byte & 0xe0) == 0xc0) {
        // Two bytes

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        uint8_t byte1 = text[1];

        if ((byte1 == '\0') || ((byte1 >> 6) != 2)) {
            *codepoint_size = 2;
            return codepoint;
        } // Unexpected sequence

        if ((byte >= 0xc2) && (byte <= 0xdf)) {
            codepoint = ((byte & 0x1f) << 6) | (byte1 & 0x3f);
            *codepoint_size = 2;
        }
    }
    else if ((byte & 0xf0) == 0xe0) {
        // Three bytes
        uint8_t byte1 = text[1];
        uint8_t byte2 = '\0';

        if ((byte1 == '\0') || ((byte1 >> 6) != 2)) {
            *codepoint_size = 2;
            return codepoint;
        } // Unexpected sequence

        byte2 = text[2];

        if ((byte2 == '\0') || ((byte2 >> 6) != 2)) {
            *codepoint_size = 3;
            return codepoint;
        } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((byte == 0xe0) && !((byte1 >= 0xa0) && (byte1 <= 0xbf))) ||
            ((byte == 0xed) && !((byte1 >= 0x80) && (byte1 <= 0x9f)))) {
            *codepoint_size = 2;
            return codepoint;
        }

        if ((byte >= 0xe0) && (byte <= 0xef)) {
            codepoint = ((byte & 0xf) << 12) | ((byte1 & 0x3f) << 6) | (byte2 & 0x3f);
            *codepoint_size = 3;
        }
    }
    else if ((byte & 0xf8) == 0xf0) {
        // Four bytes
        if (byte > 0xf4) return codepoint;

        uint8_t byte1 = text[1];
        uint8_t byte2 = '\0';
        uint8_t byte3 = '\0';

        if ((byte1 == '\0') || ((byte1 >> 6) != 2)) {
            *codepoint_size = 2;
            return codepoint;
        } // Unexpected sequence

        byte2 = text[2];

        if ((byte2 == '\0') || ((byte2 >> 6) != 2)) {
            *codepoint_size = 3;
            return codepoint;
        } // Unexpected sequence

        byte3 = text[3];

        if ((byte3 == '\0') || ((byte3 >> 6) != 2)) {
            *codepoint_size = 4;
            return codepoint;
        } // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((byte == 0xf0) && !((byte1 >= 0x90) && (byte1 <= 0xbf))) ||
            ((byte == 0xf4) && !((byte1 >= 0x80) && (byte1 <= 0x8f)))) {
            *codepoint_size = 2;
            return codepoint;
        } // Unexpected sequence

        if (byte >= 0xf0) {
            codepoint = ((byte & 0x7) << 18) | ((byte1 & 0x3f) << 12) | ((byte2 & 0x3f) << 6) | (byte3 & 0x3f);
            *codepoint_size = 4;
        }
    }

    if (codepoint > 0x10ffff) { // Codepoints after U+10ffff are invalid
        codepoint = 0x3f;
    }

    return codepoint;
}

char* sl_codepoint_to_ut8_str(const int* codepoints, int length)
{
    // We allocate enough memory to fit all possible codepoints
    // NOTE: 5 bytes for every codepoint should be enough
    char* text = (char*)SDL_calloc(length * 5, 1);
    const char* utf8 = NULL;
    int size = 0;

    for (int i = 0, bytes = 0; i < length; i++) {
        utf8 = sl_codepoint_to_utf8(codepoints[i], &bytes);
        SDL_memcpy(text + size, utf8, bytes);
        size += bytes;
    }

    // Resize memory to text length + string NULL terminator
    void* ptr = SDL_realloc(text, size + 1);

    if (ptr != NULL) text = (char*)ptr;

    return text;
}

int* sl_codepoint_from_utf8_str(const char* text, int* count)
{
    int text_length = SDL_strlen(text);

    int codepoint_size = 0;
    int codepoint_count = 0;

    // Allocate a big enough buffer to store as many codepoints as text bytes
    int* codepoints = SDL_calloc(text_length, sizeof(int));

    for (int i = 0; i < text_length; codepoint_count++) {
        codepoints[codepoint_count] = sl_codepoint_next(text + i, &codepoint_size);
        i += codepoint_size;
    }

    // Re-allocate buffer to the actual number of codepoints loaded
    codepoints = SDL_realloc(codepoints, codepoint_count * sizeof(int));

    *count = codepoint_count;

    return codepoints;
}
