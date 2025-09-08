#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Text Example", WIN_W, WIN_H, 0);
    sl_frame_set_target_fps(60);

    sl_font_id font = sl_font_load(RESOURCES_PATH "font.ttf", SL_FONT_BITMAP, 32, 0, 0);
    sl_render_set_font(font);

    while (sl_frame_step())
    {
        sl_render_clear(SL_DARK_GRAY);

        sl_render_set_color(SL_YELLOW);
        sl_render_text_centered("Hello World!", sl_vec2_scale(SL_VEC2(WIN_W, WIN_H), 0.5f), 128, SL_VEC2(2, 2));

        sl_render_present();
    }

    sl_quit();

    return 0;
}
