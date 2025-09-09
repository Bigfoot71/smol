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

#include "./sl__audio.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_timer.h>

/* === Global State === */

struct sl__audio sl__audio = { 0 };

/* === DR WAV Implementation === */

#define DR_WAV_IMPLEMENTATION

#define DRWAV_ASSERT(expression)           SDL_assert(expression)
#define DRWAV_MALLOC(sz)                   SDL_malloc((sz))
#define DRWAV_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRWAV_FREE(p)                      SDL_free((p))
#define DRWAV_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRWAV_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_wav.h>

/* === DR Flac Implementation === */

#define DR_FLAC_IMPLEMENTATION

#define DRFLAC_ASSERT(expression)           SDL_assert(expression)
#define DRFLAC_MALLOC(sz)                   SDL_malloc((sz))
#define DRFLAC_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRFLAC_FREE(p)                      SDL_free((p))
#define DRFLAC_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRFLAC_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_flac.h>

/* === DR MP3 Implementation === */

#define DR_MP3_IMPLEMENTATION

#define DRMP3_ASSERT(expression)           SDL_assert(expression)
#define DRMP3_MALLOC(sz)                   SDL_malloc((sz))
#define DRMP3_REALLOC(p, sz)               SDL_realloc((p), (sz))
#define DRMP3_FREE(p)                      SDL_free((p))
#define DRMP3_COPY_MEMORY(dst, src, sz)    SDL_memcpy((dst), (src), (sz))
#define DRMP3_ZERO_MEMORY(p, sz)           SDL_memset((p), 0, (sz))

#include <dr_mp3.h>

/* === STB Vorbis Implementation === */

#define STB_VORBIS_MALLOC(sz)   SDL_malloc(sz)
#define STB_VORBIS_FREE(p)      SDL_free(p)

#include <stb_vorbis.c>

/* === Module Functions === */

bool sl__audio_init(void)
{
    /* --- Initialize decode buffer pool --- */

    sl__audio.decode_buffer_pool_size = SL__stream_BUFFER_SIZE;

    if (!sl__audio_stream_ensure_buffer_pool_capacity(4)) {
        sl_loge("AUDIO: Failed to initialize decode buffer pool");
        return false;
    }

    /* --- Initialize active streams list --- */

    sl__audio.active_streams_capacity = 8;
    sl__audio.active_streams = SDL_malloc(sl__audio.active_streams_capacity * sizeof(sl_stream_id));

    if (sl__audio.active_streams == NULL) {
        sl_loge("AUDIO: Failed to initialize active streams list");
        return false;
    }

    /* --- Create registries --- */

    sl__audio.reg_samples = sl__registry_create(16, sizeof(sl__sample_t));
    sl__audio.reg_streams = sl__registry_create(8, sizeof(sl__stream_t));

    /* --- Create the OpenAL context and device --- */

    sl__audio.device = alcOpenDevice(NULL);
    if (sl__audio.device == NULL) {
        sl_loge("AUDIO: Failed to create OpenAL device; %s", SDL_GetError());
        return false;
    }

    sl__audio.context = alcCreateContext(sl__audio.device, NULL);
    if (sl__audio.context == NULL) {
        sl_loge("AUDIO: Failed to create OpenAL context; %s", SDL_GetError());
        return false;
    }

    alcMakeContextCurrent(sl__audio.context);

    /* --- Set default global parameters --- */

    sl__audio.volume_master = 1.0f;
    sl__audio.volume_sample = 1.0f;
    sl__audio.volume_stream = 1.0f;

    /* --- Initialize stream streaming state --- */

    sl__audio.stream_thread_initialized = false;
    sl__audio.stream_thread = NULL;
    sl__audio.stream_mutex = NULL;
    sl__audio.stream_condition = NULL;
    SDL_SetAtomicInt(&sl__audio.stream_thread_should_stop, 0);

    return true;
}

void sl__audio_quit(void)
{
    /* --- Shutdown the stream decoder thread first --- */

    sl__audio_stream_thread_shutdown();

    /* --- Release non destroyed objects --- */

    for (int i = 0; i < sl__audio.reg_streams.elements.count; i++) {
        if (((bool*)sl__audio.reg_streams.valid_flags.data)[i]) {
            sl_stream_destroy(i + 1);
        }
    }

    for (int i = 0; i < sl__audio.reg_samples.elements.count; i++) {
        if (((bool*)sl__audio.reg_samples.valid_flags.data)[i]) {
            sl_sample_destroy(i + 1);
        }
    }

    /* --- Release registries --- */

    sl__registry_destroy(&sl__audio.reg_streams);
    sl__registry_destroy(&sl__audio.reg_samples);

    /* --- Cleanup active streams list --- */

    if (sl__audio.active_streams) {
        SDL_free(sl__audio.active_streams);
        sl__audio.active_streams = NULL;
    }

    /* --- Cleanup decode buffer pool --- */

    if (sl__audio.decode_buffer_pool) {
        for (size_t i = 0; i < sl__audio.decode_buffer_count; i++) {
            if (sl__audio.decode_buffer_pool[i]) {
                SDL_free(sl__audio.decode_buffer_pool[i]);
            }
        }
        SDL_free(sl__audio.decode_buffer_pool);
        sl__audio.decode_buffer_pool = NULL;
    }
    
    if (sl__audio.decode_buffer_in_use) {
        SDL_free(sl__audio.decode_buffer_in_use);
        sl__audio.decode_buffer_in_use = NULL;
    }

    /* --- Close OpenAL device and context --- */

    if (sl__audio.context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(sl__audio.context);
        sl__audio.context = NULL;
    }

    if (sl__audio.device) {
        alcCloseDevice(sl__audio.device);
        sl__audio.device = NULL;
    }
}

/* === Helper Functions === */

const char* sl__audio_get_format_name(ALenum format)
{
    switch (format) {
    case AL_FORMAT_MONO8:
        return "Mono 8-Bit";
    case AL_FORMAT_MONO16:
        return "Mono 16-Bit";
    case AL_FORMAT_STEREO8:
        return "Stereo 8-Bit";
    case AL_FORMAT_STEREO16:
        return "Stereo 16-Bit";
    default:
        break;
    }

    return "Unknown";
}

size_t sl__audio_get_sample_count(size_t pcm_date_size, ALenum format)
{
    switch (format) {
    case AL_FORMAT_MONO8:
        return pcm_date_size;
    case AL_FORMAT_MONO16:
        return pcm_date_size / 2;
    case AL_FORMAT_STEREO8:
        return pcm_date_size / 2;
    case AL_FORMAT_STEREO16:
        return pcm_date_size / 4;
    default:
        break;
    }

    return 0;
}

sl__audio_format_t sl__audio_get_format(const uint8_t* data, size_t size)
{
    // Check for WAV format (RIFF + WAVE)
    if (size >= 12 && SDL_memcmp(data, "RIFF", 4) == 0 && SDL_memcmp(data + 8, "WAVE", 4) == 0) {
        return SL__AUDIO_WAV;
    }

    // Check for FLAC format
    if (size >= 4 && SDL_memcmp(data, "fLaC", 4) == 0) {
        return SL__AUDIO_FLAC;
    }

    // Check for MP3 format (ID3 tag or sync frame)
    if (size >= 3 && (SDL_memcmp(data, "ID3", 3) == 0 ||
        (size >= 2 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0))) {
        return SL__AUDIO_MP3;
    }

    // Check for OGG container format
    if (size >= 4 && SDL_memcmp(data, "OggS", 4) == 0) {
        // Look for Vorbis codec identifier in the first logical stream
        // Vorbis identification header starts with packet type 0x01 followed by "vorbis"
        for (size_t i = 28; i < size - 6; i++) {
            if (data[i] == 0x01 && SDL_memcmp(data + i + 1, "vorbis", 6) == 0) {
                return SL__AUDIO_OGG;
            }
        }

        // Check for other common OGG codecs and report them as unsupported
        for (size_t i = 28; i < size - 8; i++) {
            if (SDL_memcmp(data + i, "OpusHead", 8) == 0) {
                sl_loge("AUDIO: OGG Opus codec detected but not supported (only OGG Vorbis is supported)");
                return SL__AUDIO_UNKNOWN;
            }
            if (data[i] == 0x80 && SDL_memcmp(data + i + 1, "theora", 6) == 0) {
                sl_loge("AUDIO: OGG Theora codec detected but not supported (video codec, only OGG Vorbis audio is supported)");
                return SL__AUDIO_UNKNOWN;
            }
            if (data[i] == 0x7F && SDL_memcmp(data + i + 1, "FLAC", 4) == 0) {
                sl_loge("AUDIO: OGG FLAC codec detected but not supported (use native FLAC format instead)");
                return SL__AUDIO_UNKNOWN;
            }
            if (SDL_memcmp(data + i, "Speex   ", 8) == 0) {
                sl_loge("AUDIO: OGG Speex codec detected but not supported (only OGG Vorbis is supported)");
                return SL__AUDIO_UNKNOWN;
            }
        }

        sl_loge("AUDIO: OGG container detected but codec not recognized or supported (only OGG Vorbis is supported)");
        return SL__AUDIO_UNKNOWN;
    }

    return SL__AUDIO_UNKNOWN;
}

/* === Volume Functions === */

static float sl__audio_linear_to_log(float linear_volume)
{
    if (linear_volume <= 0.0f) return 0.0f;
    if (linear_volume >= 1.0f) return 1.0f;

    return powf(linear_volume, 3.0f);
}

static float sl__audio_log_to_linear(float log_volume)
{
    if (log_volume <= 0.0f) return 0.0f;
    if (log_volume >= 1.0f) return 1.0f;

    return powf(log_volume, 1.0f / 3.0f);
}

float sl__audio_calculate_final_sample_volume(void)
{
    float master_log = sl__audio_linear_to_log(sl__audio.volume_master);
    float sample_log = sl__audio_linear_to_log(sl__audio.volume_sample);
    return master_log * sample_log;
}

float sl__audio_calculate_final_stream_volume(float stream_volume)
{
    float master_log = sl__audio_linear_to_log(sl__audio.volume_master);
    float stream_global_log = sl__audio_linear_to_log(sl__audio.volume_stream);
    float stream_individual_log = sl__audio_linear_to_log(stream_volume);
    return master_log * stream_global_log * stream_individual_log;
}

void sl__audio_update_all_sample_volumes(void)
{
    float final_volume = sl__audio_calculate_final_sample_volume();

    for (size_t i = 0; i < sl__audio.reg_samples.elements.count; i++) {
        if (((bool*)sl__audio.reg_samples.valid_flags.data)[i]) {
            sl__sample_t* sample = &((sl__sample_t*)sl__audio.reg_samples.elements.data)[i];
            for (int j = 0; j < sample->source_count; j++) {
                alSourcef(sample->sources[j], AL_GAIN, final_volume);
            }
        }
    }
}

void sl__audio_update_all_stream_volumes(void)
{
    for (int i = 0; i < sl__audio.active_streams_count; i++) {
        sl__stream_t* stream = sl__registry_get(&sl__audio.reg_streams, sl__audio.active_streams[i]);
        float final_volume = sl__audio_calculate_final_stream_volume(stream->volume);
        alSourcef(stream->source, AL_GAIN, final_volume);
    }
}

/* === Sample Functions === */

bool sl__audio_sample_load_wav(sl__sample_raw_t* out, const void* data, size_t data_size)
{
    drwav wav;

    if (!drwav_init_memory(&wav, data, data_size, NULL)) {
        sl_loge("AUDIO: Failed to initialize WAV decoder");
        return false;
    }

    if (wav.channels == 1) {
        if (wav.bitsPerSample == 16) {
            out->format = AL_FORMAT_MONO16;
        }
        else {
            sl_loge("AUDIO: Unsupported WAV format for mono audio (bits per sample: %u)", wav.bitsPerSample);
            drwav_uninit(&wav);
            return false;
        }
    }
    else if (wav.channels == 2) {
        if (wav.bitsPerSample == 16) {
            out->format = AL_FORMAT_STEREO16;
        }
        else {
            sl_loge("AUDIO: Unsupported WAV format for stereo audio (bits per sample: %u)", wav.bitsPerSample);
            drwav_uninit(&wav);
            return false;
        }
    }
    else {
        sl_loge("AUDIO: Unsupported number of channels (%u) in WAV file", wav.channels);
        drwav_uninit(&wav);
        return false;
    }

    size_t total_frames = wav.totalPCMFrameCount;
    size_t bytes_per_frame = wav.channels * (wav.bitsPerSample / 8);
    size_t pcm_data_size = total_frames * bytes_per_frame;

    void* pcm_data = SDL_malloc(pcm_data_size);
    if (!pcm_data) {
        sl_loge("AUDIO: Failed to allocate memory for PCM data");
        drwav_uninit(&wav);
        return false;
    }

    size_t frames_read = drwav_read_pcm_frames(&wav, total_frames, pcm_data);
    if (frames_read != total_frames) {
        sl_loge("AUDIO: Failed to read all PCM frames (expected: %zu, read: %zu)", total_frames, frames_read);
        SDL_free(pcm_data);
        drwav_uninit(&wav);
        return false;
    }

    out->sample_rate = wav.sampleRate;
    out->pcm_data_size = pcm_data_size;
    out->pcm_data = pcm_data;

    drwav_uninit(&wav);
    return true;
}

bool sl__audio_sample_load_flac(sl__sample_raw_t* out, const void* data, size_t data_size)
{
    drflac_uint32 channels;
    drflac_uint32 sample_rate;
    drflac_uint64 total_pcm_frame_count;

    drflac_int16* pcm_data = drflac_open_memory_and_read_pcm_frames_s16(
        data,
        data_size,
        &channels,
        &sample_rate,
        &total_pcm_frame_count,
        NULL
    );

    if (!pcm_data) {
        sl_loge("AUDIO: Failed to decode FLAC file");
        return false;
    }

    if (channels == 1) {
        out->format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        out->format = AL_FORMAT_STEREO16;
    }
    else {
        sl_loge("AUDIO: Unsupported number of channels (%u) in FLAC file", channels);
        drflac_free(pcm_data, NULL);
        return false;
    }

    out->sample_rate = sample_rate;
    out->pcm_data_size = total_pcm_frame_count * channels * sizeof(drflac_int16);
    out->pcm_data = pcm_data;

    return true;
}

bool sl__audio_sample_load_mp3(sl__sample_raw_t* out, const void* data, size_t data_size)
{
    drmp3_config config;
    drmp3_uint64 total_pcm_frame_count;

    drmp3_int16* pcm_data = drmp3_open_memory_and_read_pcm_frames_s16(
        data,
        data_size,
        &config,
        &total_pcm_frame_count,
        NULL
    );

    if (!pcm_data) {
        sl_loge("AUDIO: Failed to decode MP3 file");
        return false;
    }

    if (config.channels == 1) {
        out->format = AL_FORMAT_MONO16;
    }
    else if (config.channels == 2) {
        out->format = AL_FORMAT_STEREO16;
    }
    else {
        sl_loge("AUDIO: Unsupported number of channels (%u) in MP3 file", config.channels);
        drmp3_free(pcm_data, NULL);
        return false;
    }

    out->sample_rate = config.sampleRate;
    out->pcm_data_size = total_pcm_frame_count * config.channels * sizeof(drmp3_int16);
    out->pcm_data = pcm_data;

    return true;
}

bool sl__audio_sample_load_ogg(sl__sample_raw_t* out, const void* data, size_t data_size)
{
    int channels = 0;
    int sample_rate = 0;
    short* pcm_data = NULL;

    int total_samples = stb_vorbis_decode_memory(
        (uint8_t*)data,
        (int)data_size,
        &channels,
        &sample_rate,
        &pcm_data
    );

    if (total_samples == -1 || !pcm_data) {
        sl_loge("AUDIO: Failed to decode OGG file");
        return false;
    }

    if (channels == 1) {
        out->format = AL_FORMAT_MONO16;
    }
    else if (channels == 2) {
        out->format = AL_FORMAT_STEREO16;
    }
    else {
        sl_loge("AUDIO: Unsupported number of channels (%d) in OGG file", channels);
        SDL_free(pcm_data);
        return false;
    }

    out->sample_rate = sample_rate;
    out->pcm_data_size = total_samples * channels * sizeof(short);
    out->pcm_data = pcm_data;

    return true;
}

/* === Stream Functions === */

bool sl__audio_stream_thread_init(void)
{
    if (sl__audio.stream_thread_initialized) {
        return true;
    }

    /* --- Create mutex and condition variable --- */

    sl__audio.stream_mutex = SDL_CreateMutex();
    if (!sl__audio.stream_mutex) {
        sl_loge("AUDIO: Failed to create stream mutex");
        return false;
    }

    sl__audio.stream_condition = SDL_CreateCondition();
    if (!sl__audio.stream_condition) {
        sl_loge("AUDIO: Failed to create stream condition variable");
        SDL_DestroyMutex(sl__audio.stream_mutex);
        sl__audio.stream_mutex = NULL;
        return false;
    }

    /* --- Initialize atomic flag --- */

    SDL_SetAtomicInt(&sl__audio.stream_thread_should_stop, 0);

    /* --- Create and start the thread --- */

    sl__audio.stream_thread = SDL_CreateThread(sl__audio_stream_thread, "streamStreamThread", NULL);
    if (!sl__audio.stream_thread) {
        sl_loge("AUDIO: Failed to create stream streaming thread");
        SDL_DestroyCondition(sl__audio.stream_condition);
        SDL_DestroyMutex(sl__audio.stream_mutex);
        sl__audio.stream_condition = NULL;
        sl__audio.stream_mutex = NULL;
        return false;
    }

    /* --- Ready to dance! --- */

    sl__audio.stream_thread_initialized = true;

    return true;
}

void sl__audio_stream_thread_shutdown(void)
{
    if (!sl__audio.stream_thread_initialized) {
        return;
    }

    /* --- Signal the thread to stop --- */

    SDL_SetAtomicInt(&sl__audio.stream_thread_should_stop, 1);

    /* --- Wake up the thread if it's waiting --- */

    if (sl__audio.stream_condition) {
        SDL_LockMutex(sl__audio.stream_mutex);
        sl__audio.active_streams_count = 0;
        SDL_SignalCondition(sl__audio.stream_condition);
        SDL_UnlockMutex(sl__audio.stream_mutex);
    }

    /* --- Wait for thread to finish --*/

    if (sl__audio.stream_thread) {
        SDL_WaitThread(sl__audio.stream_thread, NULL);
        sl__audio.stream_thread = NULL;
    }

    /* --- Clean up synchronization primitives --- */

    if (sl__audio.stream_condition) {
        SDL_DestroyCondition(sl__audio.stream_condition);
        sl__audio.stream_condition = NULL;
    }

    if (sl__audio.stream_mutex) {
        SDL_DestroyMutex(sl__audio.stream_mutex);
        sl__audio.stream_mutex = NULL;
    }

    /* --- Completed!! --- */

    sl__audio.stream_thread_initialized = false;
}

int sl__audio_stream_thread(void* data)
{
    (void)data; // Unused parameter

    while (!SDL_GetAtomicInt(&sl__audio.stream_thread_should_stop))
    {
        SDL_LockMutex(sl__audio.stream_mutex);

        /* --- Wait for work or shutdown signal --- */
        while (sl__audio.active_streams_count == 0 && !SDL_GetAtomicInt(&sl__audio.stream_thread_should_stop)) {
            SDL_WaitCondition(sl__audio.stream_condition, sl__audio.stream_mutex);
        }

        /* --- Check if we should exit --- */
        if (SDL_GetAtomicInt(&sl__audio.stream_thread_should_stop)) {
            SDL_UnlockMutex(sl__audio.stream_mutex);
            break;
        }

        /* --- Process all active streams --- */
        size_t streams_to_remove[sl__audio.active_streams_count];
        size_t remove_count = 0;

        for (size_t i = 0; i < sl__audio.active_streams_count; i++) {
            sl_stream_id stream_id = sl__audio.active_streams[i];
            sl__stream_t* stream = sl__registry_get(&sl__audio.reg_streams, stream_id);

            if (!stream) {
                streams_to_remove[remove_count++] = i;
                continue;
            }

            /* --- Check if stream is paused --- */
            if (stream->is_paused) {
                continue;
            }

            /* --- Ensure decode buffer is assigned --- */
            if (stream->assigned_buffer_index < 0) {
                stream->assigned_buffer_index = sl__audio_stream_acquire_decode_buffer();
                if (stream->assigned_buffer_index < 0) {
                    sl_loge("AUDIO: Failed to acquire decode buffer for stream ID %d", stream_id);
                    continue;
                }
            }

            int16_t* decode_buffer = sl__audio.decode_buffer_pool[stream->assigned_buffer_index];

            /* --- Check OpenAL source state --- */
            ALint state;
            alGetSourcei(stream->source, AL_SOURCE_STATE, &state);

            /* --- Check how many buffers have been processed --- */
            ALint processed = 0;
            alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);

            /* --- Process completed buffers --- */
            while (processed > 0 && !SDL_GetAtomicInt(&sl__audio.stream_thread_should_stop)) {
                ALuint buffer;
                alSourceUnqueueBuffers(stream->source, 1, &buffer);

                if (!stream->decoder.is_finished) {
                    // Decode more data
                    size_t samples_to_read = SL__stream_BUFFER_SIZE / (stream->decoder.channels * sizeof(int16_t));
                    size_t sample_read = stream->decoder.decode_func(stream->decoder.handle, decode_buffer, samples_to_read);

                    if (sample_read > 0) {
                        size_t data_size = sample_read * stream->decoder.channels * sizeof(int16_t);
                        alBufferData(buffer, stream->decoder.format, decode_buffer, 
                                    data_size, stream->decoder.sample_rate);
                        alSourceQueueBuffers(stream->source, 1, &buffer);
                        stream->decoder.current_sample += sample_read;
                    } else {
                        // End of file
                        if (stream->should_loop) {
                            // Start from the beginning
                            stream->decoder.seek_func(stream->decoder.handle, 0);
                            stream->decoder.current_sample = 0;
                            stream->decoder.is_finished = false;

                            // Decode again
                            sample_read = stream->decoder.decode_func(
                                stream->decoder.handle,
                                decode_buffer,
                                samples_to_read
                            );

                            if (sample_read > 0) {
                                size_t data_size = sample_read * stream->decoder.channels * sizeof(int16_t);
                                alBufferData(
                                    buffer, stream->decoder.format, decode_buffer, 
                                    data_size, stream->decoder.sample_rate
                                );
                                alSourceQueueBuffers(stream->source, 1, &buffer);
                                stream->decoder.current_sample += sample_read;
                            }
                        } else {
                            stream->decoder.is_finished = true;
                        }
                    }
                }

                processed--;
            }

            /* --- Check if the source has stopped naturally --- */
            ALint queued = 0;
            alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);

            if (queued == 0 && stream->decoder.is_finished) {
                // No more buffers and end of file reached - stop this stream
                streams_to_remove[remove_count++] = i;

                // Release the decode buffer since we're stopping
                if (stream->assigned_buffer_index >= 0) {
                    sl__audio_stream_release_decode_buffer(stream->assigned_buffer_index);
                    stream->assigned_buffer_index = -1;
                }

                // Rewind the stream for potential future playback
                stream->decoder.seek_func(stream->decoder.handle, 0);
                stream->decoder.current_sample = 0;
                stream->decoder.is_finished = false;

                // Pre-fill buffers for next playback
                sl__audio_stream_prepare_buffers(stream);
            }
            else if (state != AL_PLAYING && !stream->is_paused && queued > 0) {
                // Check if the source has stopped unexpectedly and restart it
                sl_logw("AUDIO: Stream ID %d source stopped unexpectedly, restarting...", stream_id);
                alSourcePlay(stream->source);
            }
        }

        /* --- Remove finished streams from active list (in reverse order to maintain indices) --- */
        for (int i = (int)remove_count - 1; i >= 0; i--) {
            size_t index_to_remove = streams_to_remove[i];
            sl_stream_id stream_id = sl__audio.active_streams[index_to_remove];
            
            // Shift remaining elements
            for (size_t j = index_to_remove; j < sl__audio.active_streams_count - 1; j++) {
                sl__audio.active_streams[j] = sl__audio.active_streams[j + 1];
            }
            sl__audio.active_streams_count--;
        }

        SDL_UnlockMutex(sl__audio.stream_mutex);

        /* --- Sleep for a short time to avoid busy waiting --- */
        SDL_Delay(16); // ~60 FPS
    }

    return 0;
}

void sl__audio_stream_unqueue_all_buffers(sl__stream_t* stream)
{
    ALuint buffers_to_remove[SL__stream_BUFFER_COUNT];

    /* --- Unqueue all processed buffers --- */

    ALint processed = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);
    SDL_assert(processed <= SL__stream_BUFFER_COUNT); // should never happen
    if (processed > 0) {
        alSourceUnqueueBuffers(stream->source, processed, buffers_to_remove);
    }

    /*  --- Unqueue any remaining buffer --- */

    ALint queued = 0;
    alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
    SDL_assert(queued <= SL__stream_BUFFER_COUNT); // should never happen
    if (queued > 0) {
        alSourceUnqueueBuffers(stream->source, queued, buffers_to_remove);
    }
}

void sl__audio_stream_prepare_buffers(sl__stream_t* stream)
{
    /* --- Ensure decode buffer is assigned --- */

    if (stream->assigned_buffer_index < 0) {
        stream->assigned_buffer_index = sl__audio_stream_acquire_decode_buffer();
        if (stream->assigned_buffer_index < 0) {
            sl_loge("AUDIO: Failed to acquire decode buffer for stream preparation");
            return;
        }
    }

    int16_t* decode_buffer = sl__audio.decode_buffer_pool[stream->assigned_buffer_index];

    /* --- Fill buffers with decoded audio data --- */

    for (int i = 0; i < SL__stream_BUFFER_COUNT; i++)
    {
        size_t samples_to_read = SL__stream_BUFFER_SIZE / (stream->decoder.channels * sizeof(int16_t));
        size_t sample_read = stream->decoder.decode_func(stream->decoder.handle, decode_buffer, samples_to_read);
        if (sample_read == 0) {
            break;
        }

        size_t data_size = sample_read * stream->decoder.channels * sizeof(int16_t);
        alBufferData(
            stream->buffers[i], stream->decoder.format,
            decode_buffer, data_size,
            stream->decoder.sample_rate
        );
        alSourceQueueBuffers(stream->source, 1, &stream->buffers[i]);
        stream->decoder.current_sample += sample_read;
    }
}

static size_t sl__decoder_wav_decode_samples(void* handle, void* buffer, size_t samples)
{
    drwav* wav = (drwav*)handle;
    return drwav_read_pcm_frames_s16(wav, samples, (drwav_int16*)buffer);
}

static void sl__decoder_wav_seek_sample(void* handle, size_t sample)
{
    drwav* wav = (drwav*)handle;
    drwav_seek_to_pcm_frame(wav, sample);
}

static void sl__decoder_wav_close(void* handle)
{
    drwav* wav = (drwav*)handle;
    drwav_uninit(wav);
    SDL_free(wav);
}

static size_t sl__decoder_flac_decode_samples(void* handle, void* buffer, size_t samples)
{
    drflac* flac = (drflac*)handle;
    return drflac_read_pcm_frames_s16(flac, samples, (drflac_int16*)buffer);
}

static void sl__decoder_flac_seek_sample(void* handle, size_t sample)
{
    drflac* flac = (drflac*)handle;
    drflac_seek_to_pcm_frame(flac, sample);
}

static void sl__decoder_flac_close(void* handle)
{
    drflac* flac = (drflac*)handle;
    drflac_close(flac);
}

static size_t sl__decoder_mp3_decode_samples(void* handle, void* buffer, size_t samples)
{
    drmp3* mp3 = (drmp3*)handle;
    return drmp3_read_pcm_frames_s16(mp3, samples, (drmp3_int16*)buffer);
}

static void sl__decoder_mp3_seek_sample(void* handle, size_t sample)
{
    drmp3* mp3 = (drmp3*)handle;
    drmp3_seek_to_pcm_frame(mp3, sample);
}

static void sl__decoder_mp3_close(void* handle)
{
    drmp3* mp3 = (drmp3*)handle;
    drmp3_uninit(mp3);
    SDL_free(mp3);
}

static size_t sl__decoder_ogg_decode_samples(void* handle, void* buffer, size_t samples)
{
    stb_vorbis* vorbis = (stb_vorbis*)handle;
    int sample_read = stb_vorbis_get_samples_short_interleaved(
        vorbis, vorbis->channels, buffer,
        samples * vorbis->channels
    );
    return sample_read;
}

static void sl__decoder_ogg_seek_sample(void* handle, size_t sample)
{
    stb_vorbis* vorbis = (stb_vorbis*)handle;
    stb_vorbis_seek(vorbis, sample);
}

static void sl__decoder_ogg_close(void* handle)
{
    stb_vorbis* vorbis = (stb_vorbis*)handle;
    stb_vorbis_close(vorbis);
}

bool sl__audio_stream_decoder_init(sl__decoder_t* decoder, const void* data, size_t data_size, sl__audio_format_t format)
{
    void* dataCopy = SDL_malloc(data_size);
    if (!dataCopy) return false;

    SDL_memcpy(dataCopy, data, data_size);

    switch (format) {
    case SL__AUDIO_WAV:
        {
            drwav* wav = (drwav*)SDL_malloc(sizeof(drwav));
            if (!drwav_init_memory(wav, dataCopy, data_size, NULL)) {
                SDL_free(wav);
                SDL_free(dataCopy);
                return false;
            }

            decoder->handle = wav;
            decoder->channels = wav->channels;
            decoder->sample_rate = wav->sampleRate;
            decoder->total_samples = wav->totalPCMFrameCount;
            decoder->decode_func = sl__decoder_wav_decode_samples;
            decoder->seek_func = sl__decoder_wav_seek_sample;
            decoder->close_func = sl__decoder_wav_close;
        }
        break;
    case SL__AUDIO_FLAC:
        {
            drflac* flac = drflac_open_memory(dataCopy, data_size, NULL);
            if (!flac) {
                SDL_free(dataCopy);
                return false;
            }

            decoder->handle = flac;
            decoder->channels = flac->channels;
            decoder->sample_rate = flac->sampleRate;
            decoder->total_samples = flac->totalPCMFrameCount;
            decoder->decode_func = sl__decoder_flac_decode_samples;
            decoder->seek_func = sl__decoder_flac_seek_sample;
            decoder->close_func = sl__decoder_flac_close;
        }
        break;
        
    case SL__AUDIO_MP3:
        {
            drmp3* mp3 = (drmp3*)SDL_malloc(sizeof(drmp3));
            if (!drmp3_init_memory(mp3, dataCopy, data_size, NULL)) {
                SDL_free(mp3);
                SDL_free(dataCopy);
                return false;
            }

            decoder->handle = mp3;
            decoder->channels = mp3->channels;
            decoder->sample_rate = mp3->sampleRate;
            decoder->total_samples = drmp3_get_pcm_frame_count(mp3);
            decoder->decode_func = sl__decoder_mp3_decode_samples;
            decoder->seek_func = sl__decoder_mp3_seek_sample;
            decoder->close_func = sl__decoder_mp3_close;
        }
        break;
        
    case SL__AUDIO_OGG:
        {
            int error;
            stb_vorbis* vorbis = stb_vorbis_open_memory(dataCopy, data_size, &error, NULL);
            if (!vorbis) {
                SDL_free(dataCopy);
                return false;
            }

            decoder->handle = vorbis;
            decoder->channels = vorbis->channels;
            decoder->sample_rate = vorbis->sample_rate;
            decoder->total_samples = stb_vorbis_stream_length_in_samples(vorbis);
            decoder->decode_func = sl__decoder_ogg_decode_samples;
            decoder->seek_func = sl__decoder_ogg_seek_sample;
            decoder->close_func = sl__decoder_ogg_close;
        }
        break;
        
    default:
        SDL_free(dataCopy);
        return false;
    }

    if (decoder->channels == 1) {
        decoder->format = AL_FORMAT_MONO16;
    }
    else if (decoder->channels == 2) {
        decoder->format = AL_FORMAT_STEREO16;
    }
    else {
        decoder->close_func(decoder->handle);
        SDL_free(dataCopy);
        return false;
    }

    decoder->current_sample = 0;
    decoder->is_finished = false;

    return true;
}

int sl__audio_stream_acquire_decode_buffer(void)
{
    // Find available buffer
    for (size_t i = 0; i < sl__audio.decode_buffer_count; i++) {
        if (!sl__audio.decode_buffer_in_use[i]) {
            sl__audio.decode_buffer_in_use[i] = true;
            return (int)i;
        }
    }
    
    // Need to create new buffer
    if (sl__audio.decode_buffer_count >= sl__audio.decode_buffer_capacity) {
        if (!sl__audio_stream_ensure_buffer_pool_capacity(sl__audio.decode_buffer_capacity * 2)) {
            return -1;
        }
    }
    
    // Allocate new buffer
    int16_t* new_buffer = SDL_malloc(sl__audio.decode_buffer_pool_size);
    if (!new_buffer) return -1;
    
    int index = (int)sl__audio.decode_buffer_count;
    sl__audio.decode_buffer_pool[index] = new_buffer;
    sl__audio.decode_buffer_in_use[index] = true;
    sl__audio.decode_buffer_count++;
    
    return index;
}

void sl__audio_stream_release_decode_buffer(int buffer_index)
{
    if (buffer_index >= 0 && buffer_index < (int)sl__audio.decode_buffer_count) {
        sl__audio.decode_buffer_in_use[buffer_index] = false;
    }
}

bool sl__audio_stream_ensure_buffer_pool_capacity(size_t needed_capacity)
{
    if (needed_capacity <= sl__audio.decode_buffer_capacity) {
        return true;
    }
    
    // Reallocate arrays
    int16_t** new_pool = SDL_realloc(sl__audio.decode_buffer_pool, 
                                     needed_capacity * sizeof(int16_t*));
    if (!new_pool) return false;
    
    bool* new_in_use = SDL_realloc(sl__audio.decode_buffer_in_use, 
                                   needed_capacity * sizeof(bool));
    if (!new_in_use) return false;
    
    sl__audio.decode_buffer_pool = new_pool;
    sl__audio.decode_buffer_in_use = new_in_use;
    
    // Initialize new slots
    for (size_t i = sl__audio.decode_buffer_capacity; i < needed_capacity; i++) {
        sl__audio.decode_buffer_pool[i] = NULL;
        sl__audio.decode_buffer_in_use[i] = false;
    }
    
    sl__audio.decode_buffer_capacity = needed_capacity;
    return true;
}

bool sl__audio_stream_add_active(sl_stream_id stream_id)
{
    // Vérifier si déjà présente
    for (size_t i = 0; i < sl__audio.active_streams_count; i++) {
        if (sl__audio.active_streams[i] == stream_id) {
            return true; // Déjà active
        }
    }
    
    // Agrandir si nécessaire
    if (sl__audio.active_streams_count >= sl__audio.active_streams_capacity) {
        size_t new_capacity = sl__audio.active_streams_capacity * 2;
        sl_stream_id* new_array = SDL_realloc(sl__audio.active_streams, 
                                             new_capacity * sizeof(sl_stream_id));
        if (!new_array) return false;
        
        sl__audio.active_streams = new_array;
        sl__audio.active_streams_capacity = new_capacity;
    }
    
    sl__audio.active_streams[sl__audio.active_streams_count++] = stream_id;
    return true;
}

void sl__audio_stream_remove_active(sl_stream_id stream_id)
{
    for (size_t i = 0; i < sl__audio.active_streams_count; i++) {
        if (sl__audio.active_streams[i] == stream_id) {
            // Décaler les éléments suivants
            for (size_t j = i; j < sl__audio.active_streams_count - 1; j++) {
                sl__audio.active_streams[j] = sl__audio.active_streams[j + 1];
            }
            sl__audio.active_streams_count--;
            break;
        }
    }
}

bool sl__audio_stream_is_active(sl_stream_id stream_id)
{
    for (size_t i = 0; i < sl__audio.active_streams_count; i++) {
        if (sl__audio.active_streams[i] == stream_id) {
            return true;
        }
    }
    return false;
}
