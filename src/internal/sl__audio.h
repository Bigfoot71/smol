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

#ifndef SL__AUDIO_H
#define SL__AUDIO_H

#include <smol.h>

#include "./sl__registry.h"

#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>
#include <alc.h>
#include <al.h>

/* === Constants === */

#define SL__stream_BUFFER_COUNT 3
#define SL__stream_BUFFER_SIZE ((256 /* frames */) * (32 /* blocks */) * 2 /* ch */ * 2 /* bytes/sample */)

/* === Internal Enums === */

typedef enum {
    SL__AUDIO_UNKNOWN,
    SL__AUDIO_WAV,
    SL__AUDIO_FLAC,
    SL__AUDIO_MP3,
    SL__AUDIO_OGG,
} sl__audio_format_t;

/* === Internal Structs === */

typedef struct {
    ALenum format;
    size_t sample_rate;
    size_t pcm_data_size;
    void* pcm_data;
} sl__sample_raw_t;

typedef struct {
    ALuint buffer;
    ALuint* sources;
    int source_count;
} sl__sample_t;

typedef struct {

    void* handle;
    int channels;
    int sample_rate;
    ALenum format;
    size_t total_samples;
    size_t current_sample;
    bool is_finished;

    size_t(*decode_func)(void* handle, void* buffer, size_t samples);
    void (*seek_func)(void* handle, size_t sample);
    void (*close_func)(void* handle);

} sl__decoder_t;

typedef struct {

    ALuint source;
    ALuint buffers[SL__stream_BUFFER_COUNT];

    sl__decoder_t decoder;

    bool is_paused;
    bool should_loop;
    float volume;

    // Index of the assigned buffer in the pool (-1 if none)
    int assigned_buffer_index;

} sl__stream_t;

/* === Global State === */

extern struct sl__audio {

    ALCcontext* context;
    ALCdevice* device;

    sl__registry_t reg_samples;
    sl__registry_t reg_streams;

    // Volume controls (0.0 to 1.0)
    float volume_master;
    float volume_sample;
    float volume_stream;

    // Buffer pool for stream decoding
    int16_t** decode_buffer_pool;
    size_t decode_buffer_pool_size;
    bool* decode_buffer_in_use;
    size_t decode_buffer_count;
    size_t decode_buffer_capacity;

    // List of currently playing stream
    sl_stream_id* active_streams;
    size_t active_streams_count;
    size_t active_streams_capacity;

    // Stream streaming thread
    SDL_Thread* stream_thread;
    SDL_AtomicInt stream_thread_should_stop;
    SDL_Mutex* stream_mutex;
    SDL_Condition* stream_condition;
    bool stream_thread_initialized;

} sl__audio;

/* === Module Functions === */

bool sl__audio_init(void);
void sl__audio_quit(void);

/* === Helper Functions === */

const char* sl__audio_get_format_name(ALenum format);
size_t sl__audio_get_sample_count(size_t pcm_date_size, ALenum format);
sl__audio_format_t sl__audio_get_format(const uint8_t* data, size_t size);

/* === Volume Functions === */

float sl__audio_calculate_final_sample_volume(void);
float sl__audio_calculate_final_stream_volume(float stream_volume);
void sl__audio_update_all_sample_volumes(void);
void sl__audio_update_all_stream_volumes(void);

/* === Sample Functions === */

bool sl__audio_sample_load_wav(sl__sample_raw_t* out, const void* data, size_t data_size);
bool sl__audio_sample_load_flac(sl__sample_raw_t* out, const void* data, size_t data_size);
bool sl__audio_sample_load_mp3(sl__sample_raw_t* out, const void* data, size_t data_size);
bool sl__audio_sample_load_ogg(sl__sample_raw_t* out, const void* data, size_t data_size);

/* === Stream Functions === */

bool sl__audio_stream_thread_init(void);       // Called on first sl_stream_play()
void sl__audio_stream_thread_shutdown(void);   // Called in sl__audio_quit()
int sl__audio_stream_thread(void* data);

void sl__audio_stream_unqueue_all_buffers(sl__stream_t* stream);
void sl__audio_stream_prepare_buffers(sl__stream_t* stream);

bool sl__audio_stream_decoder_init(sl__decoder_t* decoder, const void* data, size_t data_size, sl__audio_format_t format);

int sl__audio_stream_acquire_decode_buffer(void);
void sl__audio_stream_release_decode_buffer(int buffer_index);
bool sl__audio_stream_ensure_buffer_pool_capacity(size_t needed_capacity);

bool sl__audio_stream_add_active(sl_stream_id stream_id);
void sl__audio_stream_remove_active(sl_stream_id stream_id);
bool sl__audio_stream_is_active(sl_stream_id stream_id);

#endif // SL__AUDIO_H
