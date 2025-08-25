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

#define SL__MUSIC_BUFFER_COUNT 3
#define SL__MUSIC_BUFFER_SIZE ((256 /* frames */) * (32 /* blocks */) * 2 /* ch */ * 2 /* bytes/sample */)

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
    ALuint buffer;
    ALuint* sources;
    bool* channel_in_use;
    int channel_count;
    ALenum format;
    size_t sample_rate;
    size_t pcm_data_size;
    void* pcm_data;
} sl__sound_t;

typedef struct {

    void* handle;
    int channels;
    int sample_rate;
    ALenum format;
    size_t total_samples;
    size_t current_sample;
    bool is_finished;
    void* data_buffer;

    size_t(*decode_func)(void* handle, void* buffer, size_t samples);
    void (*seek_func)(void* handle, size_t sample);
    void (*close_func)(void* handle);

} sl__decoder_t;

typedef struct {

    ALuint source;
    ALuint buffers[SL__MUSIC_BUFFER_COUNT];

    sl__decoder_t decoder;

    bool is_paused;
    bool should_loop;
    float volume;

    // Temporary decoding buffer
    int16_t* temp_buffer;
    size_t temp_buffer_size;

} sl__music_t;

/* === Global State === */

extern struct sl__audio {

    ALCcontext* context;
    ALCdevice* device;

    sl__registry_t reg_sounds;
    sl__registry_t reg_musics;

    // Volume controls (0.0 to 1.0)
    float volume_master;
    float volume_sound;
    float volume_music;

    // Music streaming thread
    SDL_Thread* music_thread;
    SDL_AtomicInt music_thread_should_stop;
    SDL_Mutex* music_mutex;
    SDL_Condition* music_condition;

    // Currently playing music
    sl_music_id current_music;
    bool music_thread_initialized;

} sl__audio;

/* === Module Functions === */

bool sl__audio_init(void);
void sl__audio_quit(void);

/* === Helper Functions === */

const char* sl__audio_get_format_name(ALenum format);
size_t sl__audio_get_sample_count(size_t pcm_date_size, ALenum format);
sl__audio_format_t sl__audio_get_format(const uint8_t* data, size_t size);

/* === Volume Functions === */

float sl__audio_calculate_final_sound_volume(void);
float sl__audio_calculate_final_music_volume(float music_volume);
void sl__audio_update_all_sound_volumes(void);
void sl__audio_update_music_volume(sl_music_id music_id);

/* === Sound Functions === */

bool sl__sound_load_wav(sl__sound_t* sound, const void* data, size_t data_size);
bool sl__sound_load_flac(sl__sound_t* sound, const void* data, size_t data_size);
bool sl__sound_load_mp3(sl__sound_t* sound, const void* data, size_t data_size);
bool sl__sound_load_ogg(sl__sound_t* sound, const void* data, size_t data_size);

/* === Stream Decoder Functions === */

bool sl__decoder_init(sl__decoder_t* decoder, const void* data, size_t data_size, sl__audio_format_t format);

/* === Music Functions === */

bool sl__music_thread_init(void);       // Called on first sl_music_play()
void sl__music_thread_shutdown(void);   // Called in sl__audio_quit()

int sl__music_stream_thread(void* data);
void sl__music_unqueue_all_buffers(sl__music_t* music);
void sl__music_prepare_buffers(sl__music_t* music);
void sl__music_switch_to(sl_music_id music_id);

#endif // SL__AUDIO_H
