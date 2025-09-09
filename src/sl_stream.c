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

sl_stream_id sl_stream_load(const char* file_path)
{
    sl__stream_t stream = { 0 };

    /* --- Load file data --- */

    if (!file_path) {
        sl_loge("AUDIO: Invalid file path provided to sl_stream_load");
        return 0;
    }

    size_t file_size = 0;
    void* file_data = sl_file_load(file_path, &file_size);
    if (!file_data) {
        sl_loge("AUDIO: Failed to load stream file: %s", file_path);
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

    if (!sl__audio_stream_decoder_init(&stream.decoder, file_data, file_size, format)) {
        sl_loge("AUDIO: Failed to initialize decoder for file: %s", file_path);
        SDL_free(file_data);
        return 0;
    }

    SDL_free(file_data); // We can free file_data now because the decoders have copied the data

    /* --- Create OpenAL buffers --- */

    alGenBuffers(SL__stream_BUFFER_COUNT, stream.buffers);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to generate OpenAL buffers for stream");
        stream.decoder.close_func(stream.decoder.handle);
        return 0;
    }

    /* --- Create the OpenAL source --- */

    alGenSources(1, &stream.source);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to generate OpenAL source for stream");
        alDeleteBuffers(SL__stream_BUFFER_COUNT, stream.buffers);
        stream.decoder.close_func(stream.decoder.handle);
        return 0;
    }

    /* --- Initialize states --- */

    stream.is_paused = false;
    stream.should_loop = false;
    stream.volume = 1.0f;
    stream.assigned_buffer_index = -1;

    /* --- Pre-fill buffers for first playback --- */

    sl__audio_stream_prepare_buffers(&stream);

    /* --- Set initial volume --- */

    float volume = sl__audio_calculate_final_stream_volume(stream.volume);
    alSourcef(stream.source, AL_GAIN, volume);

    sl_stream_id stream_id = sl__registry_add(&sl__audio.reg_streams, &stream);
    if (stream_id == 0) {
        sl_loge("AUDIO: Failed to register stream in registry");
        alDeleteSources(1, &stream.source);
        alDeleteBuffers(SL__stream_BUFFER_COUNT, stream.buffers);
        stream.decoder.close_func(stream.decoder.handle);
        return 0;
    }

    return stream_id;
}

void sl_stream_destroy(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to destroy invalid stream [ID %d]", stream);
        return;
    }

    /* --- Make sure this stream is not currently playing --- */

    if (sl__audio.stream_thread_initialized) {
        SDL_LockMutex(sl__audio.stream_mutex);
        if (sl__audio_stream_is_active(stream)) {
            sl__audio_stream_remove_active(stream);
            SDL_SignalCondition(sl__audio.stream_condition);
        }
        SDL_UnlockMutex(sl__audio.stream_mutex);
    }

    /* --- Stop the OpenAL source --- */

    alSourceStop(data->source);

    /* --- Flush queue buffers --- */

    sl__audio_stream_unqueue_all_buffers(data);

    /* --- Clean OpenAL resources --- */

    alDeleteSources(1, &data->source);
    alDeleteBuffers(SL__stream_BUFFER_COUNT, data->buffers);

    /* --- Clean the decoder and associated data --- */

    if (data->decoder.handle) {
        data->decoder.close_func(data->decoder.handle);
        data->decoder.handle = NULL;
    }

    /* --- Release decode buffer if assigned --- */

    if (data->assigned_buffer_index >= 0) {
        sl__audio_stream_release_decode_buffer(data->assigned_buffer_index);
    }

    /* --- Remove from registry --- */

    sl__registry_remove(&sl__audio.reg_streams, stream);
}

void sl_stream_play(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to play invalid stream [ID %d]", stream);
        return;
    }

    if (!sl__audio_stream_thread_init()) {
        sl_loge("AUDIO: Failed to initialize stream streaming thread");
        return;
    }

    SDL_LockMutex(sl__audio.stream_mutex);

    // Resume if paused
    if (sl__audio_stream_is_active(stream) && data->is_paused) {
        data->is_paused = false;
        alSourcePlay(data->source);
        float volume = sl__audio_calculate_final_stream_volume(data->volume);
        alSourcef(data->source, AL_GAIN, volume);
    } else {
        // Add to active list if not already present
        if (!sl__audio_stream_is_active(stream)) {
            if (!sl__audio_stream_add_active(stream)) {
                sl_loge("AUDIO: Failed to add stream to active list");
                SDL_UnlockMutex(sl__audio.stream_mutex);
                return;
            }
        }

        data->is_paused = false;
        float volume = sl__audio_calculate_final_stream_volume(data->volume);
        alSourcef(data->source, AL_GAIN, volume);
        alSourcePlay(data->source);
    }

    SDL_SignalCondition(sl__audio.stream_condition);
    SDL_UnlockMutex(sl__audio.stream_mutex);
}

void sl_stream_pause(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to pause invalid stream [ID %d]", stream);
        return;
    }

    if (!sl__audio.stream_thread_initialized) {
        sl_logw("AUDIO: Cannot pause stream [ID %d], streaming thread not initialized", stream);
        return;
    }

    SDL_LockMutex(sl__audio.stream_mutex);

    if (sl__audio_stream_is_active(stream) && !data->is_paused) {
        data->is_paused = true;
        alSourcePause(data->source);
    }
    else {
        sl_logw("AUDIO: Cannot pause stream [ID %d] (not currently active or already paused)", stream);
    }

    SDL_UnlockMutex(sl__audio.stream_mutex);
}

void sl_stream_stop(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to stop invalid stream [ID %d]", stream);
        return;
    }

    if (!sl__audio.stream_thread_initialized) {
        sl_logw("AUDIO: Cannot stop stream [ID %d], streaming thread not initialized", stream);
        return;
    }

    SDL_LockMutex(sl__audio.stream_mutex);

    if (sl__audio_stream_is_active(stream)) {
        sl__audio_stream_remove_active(stream);
        
        alSourceStop(data->source);
        sl__audio_stream_unqueue_all_buffers(data);
        
        // Release decode buffer
        if (data->assigned_buffer_index >= 0) {
            sl__audio_stream_release_decode_buffer(data->assigned_buffer_index);
            data->assigned_buffer_index = -1;
        }
        
        // Reset stream state
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        data->is_paused = false;
        
        sl__audio_stream_prepare_buffers(data);
    }

    SDL_UnlockMutex(sl__audio.stream_mutex);
}

void sl_stream_rewind(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to rewind invalid stream [ID %d]", stream);
        return;
    }

    if (!sl__audio.stream_thread_initialized) {
        // If thread not initialized, just rewind the decoder silently
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        sl__audio_stream_prepare_buffers(data);
        return;
    }

    SDL_LockMutex(sl__audio.stream_mutex);

    if (sl__audio_stream_is_active(stream)) {
        bool wasPaused = data->is_paused;

        // Temporarily stop the source
        alSourceStop(data->source);

        // Flush queue buffers
        sl__audio_stream_unqueue_all_buffers(data);

        // Rewind the decoder
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;

        // Pre-fill the buffers again
        sl__audio_stream_prepare_buffers(data);

        // Resume playing if it was not paused
        if (!wasPaused) {
            alSourcePlay(data->source);
        }

        // Wake up thread
        SDL_SignalCondition(sl__audio.stream_condition);
    }
    else {
        // Not currently active, just rewind the decoder
        data->decoder.seek_func(data->decoder.handle, 0);
        data->decoder.current_sample = 0;
        data->decoder.is_finished = false;
        sl__audio_stream_prepare_buffers(data);
    }

    SDL_UnlockMutex(sl__audio.stream_mutex);
}

bool sl_stream_is_playing(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        return false;
    }

    if (!sl__audio.stream_thread_initialized) {
        return false;
    }

    SDL_LockMutex(sl__audio.stream_mutex);
    bool is_active = sl__audio_stream_is_active(stream);
    bool is_paused = data->is_paused;
    SDL_UnlockMutex(sl__audio.stream_mutex);

    return is_active && !is_paused;
}

void sl_stream_loop(sl_stream_id stream, bool loop)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to set loop on invalid stream [ID %d]", stream);
        return;
    }

    if (sl__audio.stream_thread_initialized) {
        SDL_LockMutex(sl__audio.stream_mutex);
        data->should_loop = loop;
        SDL_UnlockMutex(sl__audio.stream_mutex);
    }
    else {
        data->should_loop = loop;
    }
}

void sl_stream_set_volume(sl_stream_id stream, float volume)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to set volume on invalid stream [ID %d]", stream);
        return;
    }

    // Clamp volume to valid range
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    if (sl__audio.stream_thread_initialized) {
        SDL_LockMutex(sl__audio.stream_mutex);
        data->volume = volume;

        // Update OpenAL source volume immediately if this stream is currently active
        if (sl__audio_stream_is_active(stream)) {
            float final_volume = sl__audio_calculate_final_stream_volume(volume);
            alSourcef(data->source, AL_GAIN, final_volume);
        }

        SDL_UnlockMutex(sl__audio.stream_mutex);
    }
    else {
        data->volume = volume;
    }
}

float sl_stream_get_volume(sl_stream_id stream)
{
    sl__stream_t* data = sl__registry_get(&sl__audio.reg_streams, stream);
    if (data == NULL) {
        sl_logw("AUDIO: Attempted to get volume from invalid stream [ID %d]", stream);
        return 0.0f;
    }

    float volume;
    if (sl__audio.stream_thread_initialized) {
        SDL_LockMutex(sl__audio.stream_mutex);
        volume = data->volume;
        SDL_UnlockMutex(sl__audio.stream_mutex);
    } else {
        volume = data->volume;
    }

    return volume;
}
