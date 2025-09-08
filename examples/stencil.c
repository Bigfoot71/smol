#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Stencil Example", WIN_W, WIN_H, 0);
    sl_frame_set_target_fps(60);

    while (sl_frame_step())
    {
        sl_render_clear(SL_BLACK);

        sl_render_set_stencil(SL_STENCIL_ALWAYS, 1, 0xFF, SL_STENCIL_REPLACE, SL_STENCIL_REPLACE, SL_STENCIL_REPLACE);
        sl_render_set_color(SL_RED);

        sl_render_rectangle(200, 150, 240, 180);

        sl_render_set_stencil(SL_STENCIL_EQUAL, 1, 0xFF, SL_STENCIL_KEEP, SL_STENCIL_KEEP, SL_STENCIL_KEEP);
        sl_render_set_color(SL_BLUE);

        sl_render_circle((sl_vec2_t){320, 240}, 100, 64);

        sl_render_set_stencil(SL_STENCIL_DISABLE, 0, 0, SL_STENCIL_KEEP, SL_STENCIL_KEEP, SL_STENCIL_KEEP);
        sl_render_present();
    }

    sl_quit();

    return 0;
}
