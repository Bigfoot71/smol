#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Basic Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    float dir = (SL_PI / 4) + (SL_PI / 2) * sl_randi_range(0, 4);

    sl_vec2_t p = SL_VEC2(WIN_W * 0.5f, WIN_H * 0.5f);
    sl_vec2_t v = SL_VEC2(cosf(dir), sinf(dir));

    const float radius = 32.0f;

    while (sl_run())
    {
        float dt = sl_frame_time();

        p = sl_vec2_add(p, sl_vec2_scale(v, 256 * dt));

        if (p.x + radius > WIN_W) p.x = WIN_W - radius, v.x *= -1;
        else if (p.x - radius < 0) p.x = radius, v.x *= -1;

        if (p.y + radius > WIN_H) p.y = WIN_H - radius, v.y *= -1;
        else if (p.y - radius < 0) p.y = radius, v.y *= -1;

        sl_render_clear(SL_BLACK);

        sl_render_color(SL_RED);
        sl_render_circle(p, radius, 24);

        sl_render_present();
    }

    sl_quit();

    return 0;
}
