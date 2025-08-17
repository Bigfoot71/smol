#include <smol.h>

#define WIN_W 640
#define WIN_H 480

static const sl_vertex_t cube[6 * 4] =
{
    // Front (Z = +1)
    { { -1, -1,  1 }, { 0, 0 }, { 0, 0,  1 }, SL_BLUE },
    { {  1, -1,  1 }, { 1, 0 }, { 0, 0,  1 }, SL_BLUE },
    { {  1,  1,  1 }, { 1, 1 }, { 0, 0,  1 }, SL_BLUE },
    { { -1,  1,  1 }, { 0, 1 }, { 0, 0,  1 }, SL_BLUE },

    // Back (Z = -1)
    { {  1, -1, -1 }, { 0, 0 }, { 0, 0, -1 }, SL_BLUE },
    { { -1, -1, -1 }, { 1, 0 }, { 0, 0, -1 }, SL_BLUE },
    { { -1,  1, -1 }, { 1, 1 }, { 0, 0, -1 }, SL_BLUE },
    { {  1,  1, -1 }, { 0, 1 }, { 0, 0, -1 }, SL_BLUE },

    // Left (X = -1)
    { { -1, -1, -1 }, { 0, 0 }, { -1, 0, 0 }, SL_RED },
    { { -1, -1,  1 }, { 1, 0 }, { -1, 0, 0 }, SL_RED },
    { { -1,  1,  1 }, { 1, 1 }, { -1, 0, 0 }, SL_RED },
    { { -1,  1, -1 }, { 0, 1 }, { -1, 0, 0 }, SL_RED },

    // Right (X = +1)
    { {  1, -1,  1 }, { 0, 0 }, { 1, 0, 0 }, SL_RED },
    { {  1, -1, -1 }, { 1, 0 }, { 1, 0, 0 }, SL_RED },
    { {  1,  1, -1 }, { 1, 1 }, { 1, 0, 0 }, SL_RED },
    { {  1,  1,  1 }, { 0, 1 }, { 1, 0, 0 }, SL_RED },

    // Up (Y = +1)
    { { -1,  1,  1 }, { 0, 0 }, { 0, 1, 0 }, SL_GREEN },
    { {  1,  1,  1 }, { 1, 0 }, { 0, 1, 0 }, SL_GREEN },
    { {  1,  1, -1 }, { 1, 1 }, { 0, 1, 0 }, SL_GREEN },
    { { -1,  1, -1 }, { 0, 1 }, { 0, 1, 0 }, SL_GREEN },

    // Down (Y = -1)
    { { -1, -1, -1 }, { 0, 0 }, { 0, -1, 0 }, SL_GREEN },
    { {  1, -1, -1 }, { 1, 0 }, { 0, -1, 0 }, SL_GREEN },
    { {  1, -1,  1 }, { 1, 1 }, { 0, -1, 0 }, SL_GREEN },
    { { -1, -1,  1 }, { 0, 1 }, { 0, -1, 0 }, SL_GREEN },
};

int main(void)
{
    sl_init("Smol - Basic 3D Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    const sl_vec3_t cam_pos = SL_VEC3(5.0f, 2.0f, 5.0f);

    sl_shader_id shader = sl_shader_load(RESOURCES_PATH "light.glsl");
    sl_render_shader(shader);

    sl_render_uniform3f(sl_shader_uniform(shader, "u_light_pos"), 10, 10, 10);
    sl_render_uniform_vec3(sl_shader_uniform(shader, "u_view_pos"), &cam_pos, 1);

    sl_mat4_t proj = sl_mat4_perspective(60 * SL_DEG2RAD, (float)WIN_W / WIN_H, 0.1f, 100.0f);
    sl_render_projection(&proj);

    sl_mat4_t view = sl_mat4_look_at(cam_pos, SL_VEC3_ZERO, SL_VEC3_UP);
    sl_render_view(&view);

    sl_render_depth_test(true);
    sl_render_cull_face(SL_CULL_BACK);

    while (sl_run())
    {
        sl_render_push();
        sl_render_rotate(SL_VEC3(0, sl_time(), sl_time()));

        sl_render_clear(SL_BLACK);

        sl_render_color(SL_GRAY);
        sl_render_quad_list(cube, 6);

        sl_render_present();

        sl_render_pop();
    }

    sl_quit();

    return 0;
}
