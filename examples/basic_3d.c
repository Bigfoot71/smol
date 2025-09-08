#include <smol.h>

/* === Include Resources === */

#include "./resources/cube.h"

/* === Constants === */

#define WIN_W 640
#define WIN_H 480

#define CAM_POS SL_VEC3(5.0f, 2.0f, 5.0f)

/* === Program === */

int main(void)
{
    sl_init("Smol - Basic 3D Example", WIN_W, WIN_H, 0);
    sl_frame_set_target_fps(60);

    /* --- Load cube mesh --- */

    sl_mesh_id mesh = sl_mesh_create(cube_vertices, 24, cube_indices, 36);

    /* --- Load and setup shader --- */

    sl_shader_id shader = sl_shader_load(RESOURCES_PATH "light.glsl");
    sl_render_set_shader(shader);

    sl_render_set_uniform3f(sl_shader_uniform(shader, "u_light_pos"), 10, 10, 10);
    sl_render_set_uniform_vec3(sl_shader_uniform(shader, "u_view_pos"), &CAM_POS, 1);

    /* --- Calculate view / projection --- */

    sl_mat4_t proj = sl_mat4_perspective(60 * SL_DEG2RAD, (float)WIN_W / WIN_H, 0.1f, 100.0f);
    sl_render_set_projection(&proj);

    sl_mat4_t view = sl_mat4_look_at(CAM_POS, SL_VEC3_ZERO, SL_VEC3_UP);
    sl_render_set_view(&view);

    /* --- Setup pipeline --- */

    sl_render_set_depth_test(true);
    sl_render_set_cull_face(SL_CULL_BACK);

    /* --- Main loop! --- */

    while (sl_frame_step())
    {
        sl_render_push();
        sl_render_rotate(SL_VEC3(0, sl_time(), sl_time()));
        sl_render_clear(SL_BLACK);
        sl_render_set_color(SL_GRAY);
        sl_render_mesh(mesh, 36);
        sl_render_present();
        sl_render_pop();
    }

    sl_quit();

    return 0;
}
