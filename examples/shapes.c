#include <smol.h>

int main(void)
{
    sl_init("Smol - Shapes Example", 640, 480, SL_FLAG_MSAA_X4 | SL_FLAG_WINDOW_RESIZABLE);
    sl_frame_target(60);

    while (sl_frame_step())
    {
        sl_vec2_t size = sl_window_get_size();

        sl_render_clear(SL_BLACK);

        sl_render_color(SL_DARK_GRAY);
        sl_render_grid(0, 0, size.x, size.y, 16, 16, 1.0f);

        sl_render_color(SL_RED);
        sl_render_line(SL_VEC2(0, 0), SL_VEC2(size.x, size.y), 10.0f);
        sl_render_line(SL_VEC2(size.x, 0), SL_VEC2(0, size.y), 10.0f);

        sl_render_color(SL_PURPLE);
        sl_render_circle(sl_vec2_scale(size, 0.5f), 64, 16);

        sl_render_color(SL_YELLOW);
        sl_render_star(sl_vec2_scale(size, 0.5f), 32, 64, 8);

        sl_render_color(SL_GREEN);
        sl_render_arrow(SL_VEC2(0, 0), SL_VEC2(64, 64), 32, 8);
        sl_render_arrow(SL_VEC2(size.x, 0), SL_VEC2(size.x - 64, 64), 32, 8);
        sl_render_arrow(size, SL_VEC2(size.x - 64, size.y - 64), 32, 8);
        sl_render_arrow(SL_VEC2(0, size.y), SL_VEC2(64, size.y - 64), 32, 8);

        sl_render_present();
    }

    sl_quit();

    return 0;
}
