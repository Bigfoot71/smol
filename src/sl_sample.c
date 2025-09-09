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

/* === Public API === */

sl_sample_id sl_sample_load(const char* file_path, int channel_count)
{
    sl__sample_t sample = { 0 };

    if (channel_count <= 0) {
        sl_loge("AUDIO: Failed to load sample; Invalid channel count %d", channel_count);
        return 0;
    }

    /* --- Load file data --- */

    if (!file_path) {
        sl_loge("AUDIO: Failed to load sample; Null path");
        return 0;
    }

    size_t file_size = 0;
    void* file_data = sl_file_load(file_path, &file_size);
    if (file_data == NULL) {
        sl_loge("AUDIO: Failed load sample; Unable to load file '%s'", file_path);
        return 0;
    }

    /* --- Decode all sample data --- */

    sl__sample_raw_t raw = { 0 };

    switch (sl__audio_get_format(file_data, file_size))
    {
    case SL__AUDIO_UNKNOWN:
        sl_loge("AUDIO: Failed load sample; Unknown audio format for '%s'", file_path);
        SDL_free(file_data);
        return 0;
    case SL__AUDIO_WAV:
        if (!sl__audio_sample_load_wav(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sample; Unable to load WAV file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_FLAC:
        if (!sl__audio_sample_load_flac(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sample; Unable to decode FLAC file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_MP3:
        if (!sl__audio_sample_load_mp3(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sample; Unable to decode MP3 file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_OGG:
        if (!sl__audio_sample_load_ogg(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sample; Unable to decode OGG file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    }

    SDL_free(file_data);

    /* --- Create the OpenAL buffer --- */

    alGenBuffers(1, &sample.buffer);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sample; Could not generate OpenAL buffer");
        SDL_free(raw.pcm_data);
        return 0;
    }

    /* --- Load data into the OpenAL buffer --- */

    alBufferData(sample.buffer, raw.format, raw.pcm_data, raw.pcm_data_size, raw.sample_rate);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sample; Could not buffer data to OpenAL");
        alDeleteBuffers(1, &sample.buffer);
        SDL_free(raw.pcm_data);
        return 0;
    }

    /* --- Allocate arrays for channels --- */

    sample.source_count = channel_count;
    sample.sources = SDL_malloc(channel_count * sizeof(ALuint));

    if (!sample.sources) {
        sl_loge("AUDIO: Failed to load sample; Could not allocate memory for channels");
        alDeleteBuffers(1, &sample.buffer);
        SDL_free(raw.pcm_data);
        if (sample.sources) SDL_free(sample.sources);
        return 0;
    }

    /* --- Create all OpenAL sources --- */

    alGenSources(channel_count, sample.sources);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sample; Could not generate OpenAL sources");
        alDeleteBuffers(1, &sample.buffer);
        SDL_free(raw.pcm_data);
        SDL_free(sample.sources);
        return 0;
    }

    /* --- Attach the buffer to all sources --- */

    for (int i = 0; i < channel_count; i++) {
        alSourcei(sample.sources[i], AL_BUFFER, sample.buffer);
        if (alGetError() != AL_NO_ERROR) {
            sl_loge("AUDIO: Failed to load sample; Could not attach buffer to source %d", i);
            // Cleanup already created sources
            for (int j = 0; j < channel_count; j++) {
                if (alIsSource(sample.sources[j])) {
                    alDeleteSources(1, &sample.sources[j]);
                }
            }
            alDeleteBuffers(1, &sample.buffer);
            SDL_free(raw.pcm_data);
            SDL_free(sample.sources);
            return 0;
        }
        
        // Set initial volume for this source
        float final_volume = sl__audio_calculate_final_sample_volume();
        alSourcef(sample.sources[i], AL_GAIN, final_volume);
    }

    /* --- Push sample to the registry --- */

    return sl__registry_add(&sl__audio.reg_samples, &sample);
}

void sl_sample_destroy(sl_sample_id sample_id)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return;

    for (int i = 0; i < sample->source_count; i++) {
        if (alIsSource(sample->sources[i])) {
            ALint state;
            alGetSourcei(sample->sources[i], AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING || state == AL_PAUSED) {
                alSourceStop(sample->sources[i]);
            }
            alDeleteSources(1, &sample->sources[i]);
        }
    }

    if (alIsBuffer(sample->buffer)) {
        alDeleteBuffers(1, &sample->buffer);
    }

    if (sample->sources) {
        SDL_free(sample->sources);
    }

    sl__registry_remove(&sl__audio.reg_samples, sample_id);
}

int sl_sample_play(sl_sample_id sample_id, int channel)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return -1;

    /* --- Clamp given channel --- */

    channel = SL_MIN(channel, sample->source_count - 1);

    /* --- Select a free channel if necessary --- */

    if (channel < 0) {
        for (int i = 0; i < sample->source_count; i++) {
            ALint state = 0;
            alGetSourcei(sample->sources[i], AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                channel = i;
                break;
            }
        }
        if (channel < 0) {
            return -1;
        }
    }

    /* --- Stop current playback on this channel if any --- */

    ALint state;
    alGetSourcei(sample->sources[channel], AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING || state == AL_PAUSED) {
        alSourceRewind(sample->sources[channel]);
    }

    /* --- Play the sample on the specified channel --- */

    alSourcePlay(sample->sources[channel]);

    /* --- Return used channel --- */

    return channel;
}

void sl_sample_pause(sl_sample_id sample_id, int channel)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return;

    if (channel >= sample->source_count) return;
    if (channel >= 0) alSourcePause(sample->sources[channel]);
    else alSourcePausev(sample->source_count, sample->sources);
}

void sl_sample_stop(sl_sample_id sample_id, int channel)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return;

    if (channel >= sample->source_count) return;
    if (channel >= 0) alSourceStop(sample->sources[channel]);
    else alSourceStopv(sample->source_count, sample->sources);
}

void sl_sample_rewind(sl_sample_id sample_id, int channel)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return;

    if (channel >= sample->source_count) return;
    if (channel >= 0) alSourceRewind(sample->sources[channel]);
    else alSourceRewindv(sample->source_count, sample->sources);
}

bool sl_sample_is_playing(sl_sample_id sample_id, int channel)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return false;

    if (channel >= sample->source_count) {
        return false;
    }

    if (channel >= 0) {
        ALenum state = 0;
        alGetSourcei(sample->sources[channel], AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }

    for (int i = 0; i < sample->source_count; i++) {
        ALenum state = 0;
        alGetSourcei(sample->sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) return true;
    }

    return false;
}

int sl_sample_get_channel_count(sl_sample_id sample_id)
{
    sl__sample_t* sample = sl__registry_get(&sl__audio.reg_samples, sample_id);
    if (sample == NULL) return 0;

    return sample->source_count;
}
