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

sl_sound_id sl_sound_load(const char* file_path, int channel_count)
{
    sl__sound_t sound = { 0 };

    if (channel_count <= 0) {
        sl_loge("AUDIO: Failed to load sound; Invalid channel count %d", channel_count);
        return 0;
    }

    /* --- Load file data --- */

    if (!file_path) {
        sl_loge("AUDIO: Failed to load sound; Null path");
        return 0;
    }

    size_t file_size = 0;
    void* file_data = sl_file_load(file_path, &file_size);
    if (file_data == NULL) {
        sl_loge("AUDIO: Failed load sound; Unable to load file '%s'", file_path);
        return 0;
    }

    /* --- Decode all sound data --- */

    sl__sound_raw_t raw = { 0 };

    switch (sl__audio_get_format(file_data, file_size))
    {
    case SL__AUDIO_UNKNOWN:
        sl_loge("AUDIO: Failed load sound; Unknown audio format for '%s'", file_path);
        SDL_free(file_data);
        return 0;
    case SL__AUDIO_WAV:
        if (!sl__sound_load_wav(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sound; Unable to load WAV file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_FLAC:
        if (!sl__sound_load_flac(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sound; Unable to decode FLAC file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_MP3:
        if (!sl__sound_load_mp3(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sound; Unable to decode MP3 file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    case SL__AUDIO_OGG:
        if (!sl__sound_load_ogg(&raw, file_data, file_size)) {
            sl_loge("AUDIO: Failed to load sound; Unable to decode OGG file '%s'", file_path);
            SDL_free(file_data);
            return 0;
        }
        break;
    }

    SDL_free(file_data);

    /* --- Create the OpenAL buffer --- */

    alGenBuffers(1, &sound.buffer);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sound; Could not generate OpenAL buffer");
        SDL_free(raw.pcm_data);
        return 0;
    }

    /* --- Load data into the OpenAL buffer --- */

    alBufferData(sound.buffer, raw.format, raw.pcm_data, raw.pcm_data_size, raw.sample_rate);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sound; Could not buffer data to OpenAL");
        alDeleteBuffers(1, &sound.buffer);
        SDL_free(raw.pcm_data);
        return 0;
    }

    /* --- Allocate arrays for channels --- */

    sound.source_count = channel_count;
    sound.sources = SDL_malloc(channel_count * sizeof(ALuint));

    if (!sound.sources) {
        sl_loge("AUDIO: Failed to load sound; Could not allocate memory for channels");
        alDeleteBuffers(1, &sound.buffer);
        SDL_free(raw.pcm_data);
        if (sound.sources) SDL_free(sound.sources);
        return 0;
    }

    /* --- Create all OpenAL sources --- */

    alGenSources(channel_count, sound.sources);
    if (alGetError() != AL_NO_ERROR) {
        sl_loge("AUDIO: Failed to load sound; Could not generate OpenAL sources");
        alDeleteBuffers(1, &sound.buffer);
        SDL_free(raw.pcm_data);
        SDL_free(sound.sources);
        return 0;
    }

    /* --- Attach the buffer to all sources --- */

    for (int i = 0; i < channel_count; i++) {
        alSourcei(sound.sources[i], AL_BUFFER, sound.buffer);
        if (alGetError() != AL_NO_ERROR) {
            sl_loge("AUDIO: Failed to load sound; Could not attach buffer to source %d", i);
            // Cleanup already created sources
            for (int j = 0; j < channel_count; j++) {
                if (alIsSource(sound.sources[j])) {
                    alDeleteSources(1, &sound.sources[j]);
                }
            }
            alDeleteBuffers(1, &sound.buffer);
            SDL_free(raw.pcm_data);
            SDL_free(sound.sources);
            return 0;
        }
        
        // Set initial volume for this source
        float final_volume = sl__audio_calculate_final_sound_volume();
        alSourcef(sound.sources[i], AL_GAIN, final_volume);
    }

    /* --- Push sound to the registry --- */

    return sl__registry_add(&sl__audio.reg_sounds, &sound);
}

void sl_sound_destroy(sl_sound_id sound_id)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return;

    for (int i = 0; i < sound->source_count; i++) {
        if (alIsSource(sound->sources[i])) {
            ALint state;
            alGetSourcei(sound->sources[i], AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING || state == AL_PAUSED) {
                alSourceStop(sound->sources[i]);
            }
            alDeleteSources(1, &sound->sources[i]);
        }
    }

    if (alIsBuffer(sound->buffer)) {
        alDeleteBuffers(1, &sound->buffer);
    }

    if (sound->sources) {
        SDL_free(sound->sources);
    }

    sl__registry_remove(&sl__audio.reg_sounds, sound_id);
}

int sl_sound_play(sl_sound_id sound_id, int channel)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return -1;

    /* --- Clamp given channel --- */

    channel = SL_MIN(channel, sound->source_count - 1);

    /* --- Select a free channel if necessary --- */

    if (channel < 0) {
        for (int i = 0; i < sound->source_count; i++) {
            ALint state = 0;
            alGetSourcei(sound->sources[i], AL_SOURCE_STATE, &state);
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
    alGetSourcei(sound->sources[channel], AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING || state == AL_PAUSED) {
        alSourceRewind(sound->sources[channel]);
    }

    /* --- Play the sound on the specified channel --- */

    alSourcePlay(sound->sources[channel]);

    /* --- Return used channel --- */

    return channel;
}

void sl_sound_pause(sl_sound_id sound_id, int channel)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return;

    if (channel >= sound->source_count) return;
    if (channel >= 0) alSourcePause(sound->sources[channel]);
    else alSourcePausev(sound->source_count, sound->sources);
}

void sl_sound_stop(sl_sound_id sound_id, int channel)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return;

    if (channel >= sound->source_count) return;
    if (channel >= 0) alSourceStop(sound->sources[channel]);
    else alSourceStopv(sound->source_count, sound->sources);
}

void sl_sound_rewind(sl_sound_id sound_id, int channel)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return;

    if (channel >= sound->source_count) return;
    if (channel >= 0) alSourceRewind(sound->sources[channel]);
    else alSourceRewindv(sound->source_count, sound->sources);
}

bool sl_sound_is_playing(sl_sound_id sound_id, int channel)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return false;

    if (channel >= sound->source_count) {
        return false;
    }

    if (channel >= 0) {
        ALenum state = 0;
        alGetSourcei(sound->sources[channel], AL_SOURCE_STATE, &state);
        return (state == AL_PLAYING);
    }

    for (int i = 0; i < sound->source_count; i++) {
        ALenum state = 0;
        alGetSourcei(sound->sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) return true;
    }

    return false;
}

int sl_sound_get_channel_count(sl_sound_id sound_id)
{
    sl__sound_t* sound = sl__registry_get(&sl__audio.reg_sounds, sound_id);
    if (sound == NULL) return 0;

    return sound->source_count;
}
