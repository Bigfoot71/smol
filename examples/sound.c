#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Sound Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_sound_id sound = sl_sound_load(RESOURCES_PATH "coin.ogg", 4);

    sl_font_id font = sl_font_load(RESOURCES_PATH "font.ttf", SL_FONT_BITMAP, 64, 0, 0);
    sl_render_font(font);

    while (sl_run())
    {
        sl_render_clear(SL_DARK_GRAY);

        if (sl_key_just_pressed(SL_KEY_SPACE)) {
            sl_sound_play(sound, -1);
        }

        if (!sl_sound_is_playing(sound, -1)) {
            sl_render_color(SL_YELLOW);
            sl_render_text_centered("Press SPACE to play the sound!", SL_VEC2(320, 240), 32, SL_VEC2(2, 2));
        }
        else {
            sl_render_color(SL_GREEN);
            sl_render_text_centered("MONEY", SL_VEC2(320, 240), 64, SL_VEC2(2, 2));
        }

        sl_render_present();
    }

    sl_quit();

    return 0;
}
