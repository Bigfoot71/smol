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

#include "./internal/sl__audio.h"
#include <SDL3/SDL_timer.h>

/* === Public API === */

sl_music_id sl_music_load(const char* file_path)
{
    sl__music_t music = { 0 };

    /* --- Load file data --- */

    if (!file_path) {
        sl_loge("AUDIO: Invalid file path provided to sl_music_load");
        return 0;
    }

    size_t file_size = 0;
    void* file_data = sl_file_load(file_path, &file_size);
    if (!file_data) {
        sl_loge("AUDIO: Failed to load music file: %s", file_path);
        return 0;
    }

    /* --- Check file format --- */

    sl__audio_format_t format = sl__audio_get_format((const uint8_t*)file_data, file_size);
    if (format == SL__AUDIO_UNKNOWN) {
        sl_loge("AUDIO: Unknown or unsupported audio format in file: %s", file_path);
        SDL_free(file_data);
        return 0;
    }

    /* --- Initialize the decoder --- */

    if (!sl__decoder_init(&music.decoder, file_data, file_size, format)) {
        sl_loge("AUDIO: Failed to initialize decoder for file: %s", file_path);
        SDL_free(file_data);
        return 0;
    }

    SDL_free(file_data); // We can free file_data now because the decoders have copied the data

    /* --- Create OpenAL buffers --- */

    alGenBuffers(SL__MUSIC_BUFFER_COUNT, music.buffers);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to generate OpenAL buffers for music");
        music.decoder.close_func(music.decoder.handle);
        if (music.decoder.data_buffer) {
            SDL_free(music.decoder.data_buffer);
        }
        return 0;
    }

    /* --- Create the OpenAL source --- */

    alGenSources(1, &music.source);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to generate OpenAL source for music");
        alDeleteBuffers(SL__MUSIC_BUFFER_COUNT, music.buffers);
        music.decoder.close_func(music.decoder.handle);
        if (music.decoder.data_buffer) {
            SDL_free(music.decoder.data_buffer);
        }
        return 0;
    }

    /* --- Allocate the temporary buffer --- */

    music.temp_buffer_size = SL__MUSIC_BUFFER_SIZE;
    music.temp_buffer = SDL_malloc(music.temp_buffer_size);
    if (!music.temp_buffer) {
        sl_loge("AUDIO: Failed to allocate temporary decode buffer for music");
        alDeleteSources(1, &music.source);
        alDeleteBuffers(SL__MUSIC_BUFFER_COUNT, music.buffers);
        music.decoder.close_func(music.decoder.handle);
        if (music.decoder.data_buffer) {
            SDL_free(music.decoder.data_buffer);
        }
        return 0;
    }

    /* --- Initialize states --- */

    music.is_paused = false;
    music.should_loop = false;
    music.volume = 1.0f;

    /* --- Pre-fill buffers for first playback --- */

    sl__music_prepare_buffers(&music);

    /* --- Set initial volume --- */

    float volume = sl__audio_calculate_final_music_volume(music.volume);
    alSourcef(music.source, AL_GAIN, volume);

    sl_music_id music_id = sl__registry_add(&sl__audio.reg_musics, &music);
    if (music_id == 0) {
        sl_loge("AUDIO: Failed to register music in registry");
        alDeleteSources(1, &music.source);
        alDeleteBuffers(SL__MUSIC_BUFFER_COUNT, music.buffers);
        SDL_free(music.temp_buffer);
        music.decoder.close_func(music.decoder.handle);
        if (music.decoder.data_buffer) {
            SDL_free(music.decoder.data_buffer);
        }
        return 0;
    }

    return music_id;
}

void sl_music_destroy(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to destroy invalid music [ID %d]", music);
        return;
    }

    /* --- Make sure this music is not currently playing --- */

    if (sl__audio.music_thread_initialized) {
        SDL_LockMutex(sl__audio.music_mutex);
        if (sl__audio.current_music == music) {
            sl__audio.current_music = 0;
            SDL_SignalCondition(sl__audio.music_condition);
        }
        SDL_UnlockMutex(sl__audio.music_mutex);
    }

    /* --- Stop the OpenAL source --- */

    alSourceStop(data->source);

    /* --- Flush queue buffers --- */

    sl__music_unqueue_all_buffers(data);

    /* --- Clean OpenAL resources --- */

    alDeleteSources(1, &data->source);
    alDeleteBuffers(SL__MUSIC_BUFFER_COUNT, data->buffers);

    /* --- Clean the decoder and associated data --- */

    if (data->decoder.handle) {
        data->decoder.close_func(data->decoder.handle);
        data->decoder.handle = NULL;
    }

    if (data->decoder.data_buffer) {
        SDL_free(data->decoder.data_buffer);
        data->decoder.data_buffer = NULL;
    }

    /* --- Clean up temporary buffer --- */

    if (data->temp_buffer) {
        SDL_free(data->temp_buffer);
        data->temp_buffer = NULL;
    }

    /* --- Remove from registry --- */

    sl__registry_remove(&sl__audio.reg_musics, music);
}

void sl_music_play(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to play invalid music [ID %d]", music);
        return;
    }

    /* --- Initialize music thread lazily on first play --- */

    if (!sl__music_thread_init()) {
        sl_loge("AUDIO: Failed to initialize music streaming thread");
        return;
    }

    SDL_LockMutex(sl__audio.music_mutex);

    /* --- Resume music if paused or start new one --- */

    if (sl__audio.current_music == music && data->is_paused) {
        data->is_paused = false;
        alSourcePlay(data->source);

        // Update volume when resuming playback
        float volume = sl__audio_calculate_final_music_volume(data->volume);
        alSourcef(data->source, AL_GAIN, volume);

        // Wake up thread
        SDL_SignalCondition(sl__audio.music_condition);
    }
    else {
        // Switch to this music (will stop any currently playing music)
        if (sl__audio.current_music != music) {
            // Stop current music if any
            if (sl__audio.current_music != 0) {
                sl__music_t* current = sl__registry_get(&sl__audio.reg_musics, sl__audio.current_music);
                if (current) {
                    alSourceStop(current->source);
                    sl__music_unqueue_all_buffers(current);
                }
            }
            sl__audio.current_music = music;
        }

        data->is_paused = false;

        /* --- Update volume start playback --- */

        float volume = sl__audio_calculate_final_music_volume(data->volume);
        alSourcef(data->source, AL_GAIN, volume);
        alSourcePlay(data->source);

        /* --- Wake up the streaming thread --- */

        SDL_SignalCondition(sl__audio.music_condition);
    }

    SDL_UnlockMutex(sl__audio.music_mutex);
}

void sl_music_pause(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to pause invalid music [ID %d]", music);
        return;
    }

    if (!sl__audio.music_thread_initialized) {
        sl_logw("AUDIO: Cannot pause music [ID %d], streaming thread not initialized", music);
        return;
    }

    SDL_LockMutex(sl__audio.music_mutex);

    if (sl__audio.current_music == music && !data->is_paused) {
        data->is_paused = true;
        alSourcePause(data->source);
    }
    else {
        sl_logw("AUDIO: Cannot pause music [ID %d] (not currently playing or already paused)", music);
    }

    SDL_UnlockMutex(sl__audio.music_mutex);
}

void sl_music_stop(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to stop invalid music [ID %d]", music);
        return;
    }

    if (!sl__audio.music_thread_initialized) {
        sl_logw("AUDIO: Cannot stop music [ID %d], streaming thread not initialized", music);
        return;
    }

    SDL_LockMutex(sl__audio.music_mutex);

    if (sl__audio.current_music == music)
    {
        sl__audio.current_music = 0;

        // Stop the OpenAL source immediately
        alSourceStop(data->source);

        // Flush queue buffers
        sl__music_unqueue_all_buffers(data);

        // Reset the music to beginning
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        data->is_paused = false;

        // Pre-fill buffers for next playback
        sl__music_prepare_buffers(data);
    }

    SDL_UnlockMutex(sl__audio.music_mutex);
}

void sl_music_rewind(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to rewind invalid music [ID %d]", music);
        return;
    }

    if (!sl__audio.music_thread_initialized) {
        // If thread not initialized, just rewind the decoder silently
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        sl__music_prepare_buffers(data);
        return;
    }

    SDL_LockMutex(sl__audio.music_mutex);

    if (sl__audio.current_music == music) {
        bool wasPaused = data->is_paused;

        // Temporarily stop the source
        alSourceStop(data->source);

        // Flush queue buffers
        sl__music_unqueue_all_buffers(data);

        // Rewind the decoder
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;

        // Pre-fill the buffers again
        sl__music_prepare_buffers(data);

        // Resume playing if it was not paused
        if (!wasPaused) {
            alSourcePlay(data->source);
        }

        // Wake up thread
        SDL_SignalCondition(sl__audio.music_condition);
    }
    else {
        // Not currently playing, just rewind the decoder
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        sl__music_prepare_buffers(data);
    }

    SDL_UnlockMutex(sl__audio.music_mutex);
}

bool sl_music_is_playing(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        return false;
    }

    if (!sl__audio.music_thread_initialized) {
        return false;
    }

    SDL_LockMutex(sl__audio.music_mutex);
    bool is_current = (sl__audio.current_music == music);
    bool is_paused = data->is_paused;
    SDL_UnlockMutex(sl__audio.music_mutex);

    return is_current && !is_paused;
}

void sl_music_loop(sl_music_id music, bool loop)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to set loop on invalid music [ID %d]", music);
        return;
    }

    if (sl__audio.music_thread_initialized) {
        SDL_LockMutex(sl__audio.music_mutex);
        data->should_loop = loop;
        SDL_UnlockMutex(sl__audio.music_mutex);
    }
    else {
        data->should_loop = loop;
    }
}

void sl_music_set_volume(sl_music_id music, float volume)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to set volume on invalid music [ID %d]", music);
        return;
    }

    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (sl__audio.music_thread_initialized) {
        SDL_LockMutex(sl__audio.music_mutex);
        data->volume = volume;

        // Update OpenAL source volume immediately if this is the current music
        if (sl__audio.current_music == music) {
            float final_volume = sl__audio_calculate_final_music_volume(volume);
            alSourcef(data->source, AL_GAIN, final_volume);
        }

        SDL_UnlockMutex(sl__audio.music_mutex);
    }
    else {
        data->volume = volume;
    }
}

float sl_music_get_volume(sl_music_id music)
{
    sl__music_t* data = sl__registry_get(&sl__audio.reg_musics, music);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to get volume from invalid music [ID %d]", music);
        return 0.0f;
    }

    float volume;
    if (sl__audio.music_thread_initialized) {
        SDL_LockMutex(sl__audio.music_mutex);
        volume = data->volume;
        SDL_UnlockMutex(sl__audio.music_mutex);
    } else {
        volume = data->volume;
    }

    return volume;
}
