#include <smol.h>

#define WIN_W 640
#define WIN_H 480

static const sl_vertex_t triangle[3] =
{
    SL_VERTEX(SL_VEC3(320, 0, 0), SL_VEC2(0, 0), SL_VEC3(0, 0, 1), SL_RED),
    SL_VERTEX(SL_VEC3(0, 480, 0), SL_VEC2(0, 0), SL_VEC3(0, 0, 1), SL_GREEN),
    SL_VERTEX(SL_VEC3(640, 480, 0), SL_VEC2(0, 0), SL_VEC3(0, 0, 1), SL_BLUE),
};

int main(void)
{
    sl_init("Smol - Mesh Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_mesh_id mesh = sl_mesh_create(triangle, 3, NULL, 0);

    while (sl_frame_step()) {
        sl_render_clear(SL_BLACK);
        sl_render_color(SL_RED);
        sl_render_mesh(mesh, 3);
        sl_render_present();
    }

    sl_quit();

    return 0;
}
