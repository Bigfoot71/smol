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

float sl_audio_get_volume_music(void)
{
    return sl__audio.volume_music;
}

float sl_audio_get_volume_sound(void)
{
    return sl__audio.volume_sound;
}

void sl_audio_set_volume_master(float volume)
{
    sl__audio.volume_master = SL_CLAMP(volume, 0.0f, 1.0f);

    sl__audio_update_all_sound_volumes();

    SDL_LockMutex(sl__audio.music_mutex);
    if (sl__audio.current_music != 0) {
        sl__audio_update_music_volume(sl__audio.current_music);
    }
    SDL_UnlockMutex(sl__audio.music_mutex);
}

void sl_audio_set_volume_music(float volume)
{
    sl__audio.volume_music = SL_CLAMP(volume, 0.0f, 1.0f);

    SDL_LockMutex(sl__audio.music_mutex);
    if (sl__audio.current_music != 0) {
        sl__audio_update_music_volume(sl__audio.current_music);
    }
    SDL_UnlockMutex(sl__audio.music_mutex);
}

void sl_audio_set_volume_sound(float volume)
{
    sl__audio.volume_sound = SL_CLAMP(volume, 0.0f, 1.0f);

    sl__audio_update_all_sound_volumes();
}
