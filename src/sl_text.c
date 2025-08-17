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
#include <SDL3/SDL_time.h>

/* === Public API === */

void sl_text_to_upper(char* str)
{
    while (*str) {
        *str = (char)SDL_toupper((int)*str);
        str++;
    }
}

void sl_text_to_lower(char* str)
{
    while (*str) {
        *str = (char)SDL_tolower((int)*str);
        str++;
    }
}

bool sl_text_compare(const char* str1, const char* str2)
{
    return SDL_strcmp(str1, str2) == 0;
}

bool sl_text_compare_ignore_case(const char* str1, const char* str2)
{
    return SDL_strcasecmp(str1, str2) == 0;
}

bool sl_text_starts_with(const char* str, const char* prefix)
{
    return SDL_strncmp(str, prefix, SDL_strlen(prefix)) == 0;
}

char* sl_text_ends_with(const char* str, const char* suffix)
{
    size_t str_len = SDL_strlen(str);
    size_t suffix_len = SDL_strlen(suffix);

    if (str_len < suffix_len) return NULL;

    if (SDL_strcmp(str + str_len - suffix_len, suffix) == 0) {
        return (char*)(str + str_len - suffix_len);
    }

    return NULL;
}

char* sl_text_contains(const char* str, const char* keyword)
{
    return SDL_strstr(str, keyword);
}

const char* sl_text_format(const char* text, ...)
{
    #define SL_FMT_BUFFER_LENGTH 512
    static char buffer[SL_FMT_BUFFER_LENGTH];

    if (!text) {
        buffer[0] = '\0';
        return buffer;
    }

    va_list args;
    va_start(args, text);
    int req_byte_count = SDL_vsnprintf(buffer, SL_FMT_BUFFER_LENGTH, text, args);
    va_end(args);

    if (req_byte_count >= SL_FMT_BUFFER_LENGTH) {
        if (SL_FMT_BUFFER_LENGTH >= 4) {
            strcpy(buffer + SL_FMT_BUFFER_LENGTH - 4, "...");
        }
    }

    return buffer;
}

char* sl_text_format_dynamic(const char* text, ...)
{
    if (!text) {
        return NULL;
    }

    va_list args1, args2;
    va_start(args1, text);
    va_copy(args2, args1);

    int size = SDL_vsnprintf(NULL, 0, text, args1) + 1;
    va_end(args1);

    char* buffer = SDL_malloc(size);
    if (buffer) {
        SDL_vsnprintf(buffer, size, text, args2);
    }
    va_end(args2);

    return buffer;
}

const char* sl_text_format_date_time(int64_t time)
{
    static char date_time_str[32];

    SDL_DateTime date_time = { 0 };
    SDL_TimeToDateTime(time, &date_time, true);

    SDL_snprintf(
        date_time_str, sizeof(date_time_str), "%04d-%02d-%02d %02d:%02d:%02d",
        date_time.year, date_time.month + 1, date_time.day,
        date_time.hour, date_time.minute, date_time.second
    );

    return date_time_str;
}

const char* sl_text_format_date(int64_t time)
{
    static char date_str[16];

    SDL_DateTime date_time = { 0 };
    SDL_TimeToDateTime(time, &date_time, true);

    SDL_snprintf(
        date_str, sizeof(date_str), "%04d-%02d-%02d",
        date_time.year, date_time.month + 1, date_time.day
    );

    return date_str;
}

const char* sl_text_format_time(int64_t time)
{
    static char time_str[16];

    SDL_DateTime date_time = { 0 };
    SDL_TimeToDateTime(time, &date_time, true);

    SDL_snprintf(
        time_str, sizeof(time_str), "%02d:%02d:%02d",
        date_time.hour, date_time.minute, date_time.second
    );

    return time_str;
}

char* sl_text_replace(const char* str, const char* old_word, const char* new_word)
{
    if (!str || !old_word || !new_word) return NULL;

    size_t old_len = SDL_strlen(old_word);
    size_t new_len = SDL_strlen(new_word);
    int count = 0;

    const char* ptr = str;
    while ((ptr = SDL_strstr(ptr, old_word)) != NULL) {
        count++;
        ptr += old_len;
    }

    size_t new_size = SDL_strlen(str) + count * (new_len - old_len) + 1;
    char* result = SDL_malloc(new_size);
    if (!result) return NULL;

    char* dst = result;
    while (*str) {
        if (SDL_strncmp(str, old_word, old_len) == 0) {
            SDL_memcpy(dst, new_word, new_len);
            dst += new_len;
            str += old_len;
        }
        else {
            *dst++ = *str++;
        }
    }
    *dst = '\0';

    return result;
}

char* sl_text_concat(const char* src1, const char* src2)
{
    if (!src1 || !src2) return NULL;

    size_t len1 = SDL_strlen(src1);
    size_t len2 = SDL_strlen(src2);
    size_t total_len = len1 + len2;

    char* result = SDL_malloc(total_len + 1);
    if (!result) return NULL;

    SDL_memcpy(result, src1, len1);
    SDL_memcpy(result + len1, src2, len2);
    result[total_len] = '\0';

    return result;
}

char* sl_text_copy(const char* src, size_t max_len)
{
    return SDL_strndup(src, max_len);
}

int sl_text_count_occurences(const char* str, const char* keyword)
{
    int count = 0;
    const char* ptr = str;
    while ((ptr = SDL_strstr(ptr, keyword)) != NULL) {
        count++;
        ptr += SDL_strlen(keyword);
    }
    return count;
}

int sl_text_word_count(const char* str)
{
    int count = 0;
    int in_word = 0;

    while (*str) {
        if (SDL_isspace((uint8_t)*str)) {
            in_word = 0;
        }
        else if (!in_word) {
            in_word = 1;
            count++;
        }
        str++;
    }
    return count;
}

void sl_text_extract_word(const char* str, int word_index, char* out_word, size_t max_len)
{
    int count = 0, in_word = 0;
    while (*str) {
        if (SDL_isspace((uint8_t)*str)) {
            in_word = 0;
        }
        else if (!in_word) {
            in_word = 1;
            if (count == word_index) {
                size_t len = 0;
                while (*str && !SDL_isspace((uint8_t)*str) && len < max_len - 1) {
                    out_word[len++] = *str++;
                }
                out_word[len] = '\0';
                return;
            }
            count++;
        }
        str++;
    }
    out_word[0] = '\0';
}

void sl_text_reverse(char* str)
{
    if (str == NULL) return;

    size_t len = SDL_strlen(str);
    for (size_t i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

void sl_text_trim(char* str)
{
    char* start = str;
    while (SDL_isspace((uint8_t)*start)) start++;
    char* end = start + SDL_strlen(start) - 1;
    while (end > start && SDL_isspace((uint8_t)*end)) *end-- = '\0';
    SDL_memmove(str, start, end - start + 2);
}
