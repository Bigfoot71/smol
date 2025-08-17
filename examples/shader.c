#include <smol.h>

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    sl_init("Smol - Shader Example", WIN_W, WIN_H, 0);

    sl_frame_target(60);

    sl_shader_id shader = sl_shader_load(RESOURCES_PATH "raymarch.glsl");
    int loc_time = sl_shader_uniform(shader, "u_time");
    sl_render_shader(shader);

    while (sl_run()) {
        sl_render_uniform1f(loc_time, sl_time());
        sl_render_rectangle(0, 0, WIN_W, WIN_H);
        sl_render_present();
    }

    sl_quit();

    return 0;
}
