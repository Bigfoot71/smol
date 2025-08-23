#include <smol.h>

#define WIN_W 640
#define WIN_H 480

typedef struct {
    sl_texture_id texture;
    int cols, rows, count;
    int fw, fh;
} sprite_t;

sprite_t load_sprite(const char* path, int cols, int rows) 
{
    sprite_t s = {0};
    int w, h;

    s.texture = sl_texture_load(path, &w, &h);
    s.cols = cols;
    s.rows = rows; 
    s.count = cols * rows;
    s.fw = w / cols;
    s.fh = h / rows;

    return s;
}

void draw_sprite(sprite_t s, int frame, float x, float y, float scale) 
{
    frame = (int)sl_wrap(frame, 0, s.count);

    int col = frame % s.cols;
    int row = frame / s.cols;

    float u = (float)col / s.cols;
    float v = (float)row / s.rows; 
    float du = 1.0f / s.cols;
    float dv = 1.0f / s.rows;

    float hw = s.fw * scale * 0.5f;
    float hh = s.fh * scale * 0.5f;

    sl_vertex_t q[] = {
        {{x-hw, y-hh}, {u,    v   }, {0, 0, 1}, SL_WHITE},
        {{x-hw, y+hh}, {u,    v+dv}, {0, 0, 1}, SL_WHITE}, 
        {{x+hw, y+hh}, {u+du, v+dv}, {0, 0, 1}, SL_WHITE},
        {{x+hw, y-hh}, {u+du, v   }, {0, 0, 1}, SL_WHITE}
    };

    sl_render_sampler(0, s.texture);
    sl_render_quad_list(q, 1);
    sl_render_sampler(0, 0);
}

// Just another way to draw sprites
void draw_sprite_ex(sprite_t s, int frame, float x, float y, float rot, float scale) 
{
    frame = (int)sl_wrap(frame, 0, s.count);

    int col = frame % s.cols;
    int row = frame / s.cols;

    float u = (float)col / s.cols;
    float v = (float)row / s.rows; 
    float du = 1.0f / s.cols;
    float dv = 1.0f / s.rows;

    sl_render_texture_scale(SL_VEC2(du, dv));
    sl_render_texture_translate(SL_VEC2(u, v));
    sl_render_sampler(0, s.texture);

    sl_render_rectangle_ex(SL_VEC2(x, y), SL_VEC2(s.fw * scale, s.fh * scale), rot);

    sl_render_sampler(0, 0);
    sl_render_texture_identity();
}

int main(void)
{
    sl_init("Smol - Sprite Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sprite_t sprite = load_sprite(RESOURCES_PATH "spritesheet.png", 8, 3);

    sl_render_blend(SL_BLEND_ALPHA);

    while (sl_run())
    {
        sl_render_clear(SL_DARK_GRAY);
        draw_sprite_ex(sprite, 5.0f * sl_time(), WIN_W * 0.5f, WIN_H * 0.5f, 0.0f, 16.0f);
        sl_render_present();
    }

    sl_quit();

    return 0;
}
