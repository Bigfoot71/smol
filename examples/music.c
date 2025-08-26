#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Music Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_music_id music = sl_music_load(RESOURCES_PATH "awesomeness.ogg");

    sl_font_id font = sl_font_load(RESOURCES_PATH "font.ttf", SL_FONT_BITMAP, 64, 0, 0);
    sl_render_font(font);

    while (sl_run())
    {
        sl_render_clear(SL_DARK_GRAY);

        if (!sl_music_is_playing(music))
        {
            sl_render_color(SL_YELLOW);
            sl_render_text_centered("Press SPACE to play the music!", SL_VEC2(320, 240), 32, SL_VEC2(2, 2));
            if (sl_key_just_pressed(SL_KEY_SPACE)) {
                sl_music_play(music);
            }
        }
        else
        {
            sl_audio_set_volume_music(sl_audio_get_volume_music() + sl_mouse_get_wheel().y * 0.05f);

            sl_render_color(SL_GREEN);
            sl_render_text_centered("DANCE!", SL_VEC2(320, 240), 64, SL_VEC2(2, 2));

            sl_render_color(SL_YELLOW);
            sl_render_text("Music by mrpoly", SL_VEC2(10, 438), 32, SL_VEC2(2, 2));

            sl_render_color(SL_YELLOW);
            sl_render_text(sl_text_format("Volume: %i%%", (int)roundf(100 * sl_audio_get_volume_music())), SL_VEC2(10, 10), 24, SL_VEC2(2,2));

            if (sl_key_just_pressed(SL_KEY_SPACE)) {
                sl_music_pause(music);
            }
            else if (sl_key_just_pressed(SL_KEY_S)) {
                sl_music_stop(music);
            }
        }

        sl_render_present();
    }

    sl_quit();

    return 0;
}
