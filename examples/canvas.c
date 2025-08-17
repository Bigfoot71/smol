#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Canvas Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_canvas_id canvas = sl_canvas_create(320, 240, SL_PIXEL_FORMAT_RGB8, false);

    sl_texture_id color_target = 0;
    sl_canvas_query(canvas, &color_target, NULL, NULL, NULL);
    sl_texture_parameters(color_target, SL_FILTER_BILINEAR, SL_WRAP_CLAMP);

    sl_shader_id shader = sl_shader_load(RESOURCES_PATH "raymarch.glsl");
    int loc_time = sl_shader_uniform(shader, "u_time");

    while (sl_run()) {
        sl_render_canvas(canvas);
        {
            sl_render_sampler(0, 0);
            sl_render_shader(shader);

            sl_render_uniform1f(loc_time, sl_time());
            sl_render_rectangle(0, 0, 320, 240);
        }
        sl_render_canvas(0);
        {
            sl_render_sampler(0, color_target);
            sl_render_shader(0);

            float s = 1.0f + sinf(2.0f * sl_time()) * 0.5f;

            sl_render_clear(SL_BLACK);
            sl_render_rectangle_ex(SL_VEC2(320, 240), sl_vec2_scale(SL_VEC2(320, 240), s), sl_time());
        }
        sl_render_present();
    }

    sl_quit();

    return 0;
}
