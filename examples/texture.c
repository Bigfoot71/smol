#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Texture Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_texture_id texture = sl_texture_load(RESOURCES_PATH "emoji.png", NULL, NULL);
    sl_texture_parameters(texture, SL_FILTER_BILINEAR, SL_WRAP_CLAMP);
    sl_render_sampler(0, texture);

    sl_render_blend(SL_BLEND_ALPHA);

    while (sl_frame_step())
    {
        sl_render_clear(SL_DARK_GRAY);

        sl_render_push();

        float s = 1.0f + sinf(sl_time()) * 0.5f;
        sl_render_rectangle_ex(SL_VEC2(320, 240), sl_vec2_scale(SL_VEC2(256, 256), s), sl_time());

        sl_render_pop();

        sl_render_present();
    }

    sl_quit();

    return 0;
}
