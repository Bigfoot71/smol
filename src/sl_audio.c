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

float sl_audio_get_volume_master(void)
{
    return sl__audio.volume_master;
}

float sl_aduio_get_volume_stream(void)
{
    return sl__audio.volume_stream;
}

float sl_audio_get_volume_sample(void)
{
    return sl__audio.volume_sample;
}

void sl_audio_set_volume_master(float volume)
{
    sl__audio.volume_master = SL_CLAMP(volume, 0.0f, 1.0f);

    sl__audio_update_all_sample_volumes();

    SDL_LockMutex(sl__audio.stream_mutex);
    sl__audio_update_all_stream_volumes();
    SDL_UnlockMutex(sl__audio.stream_mutex);
}

void sl_audio_set_volume_stream(float volume)
{
    sl__audio.volume_stream = SL_CLAMP(volume, 0.0f, 1.0f);

    SDL_LockMutex(sl__audio.stream_mutex);
    sl__audio_update_all_stream_volumes();
    SDL_UnlockMutex(sl__audio.stream_mutex);
}

void sl_audio_set_volume_sample(float volume)
{
    sl__audio.volume_sample = SL_CLAMP(volume, 0.0f, 1.0f);

    sl__audio_update_all_sample_volumes();
}
