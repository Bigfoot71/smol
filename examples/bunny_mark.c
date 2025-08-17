#include <smol.h>

#define WIN_W 640
#define WIN_H 480

typedef struct {
    sl_vec2_t position;
    sl_vec2_t velocity;
    sl_color_t color;
    float rotation;
} wabbit_t;

#define MAX_WABBIT 250000

static wabbit_t wabbits[MAX_WABBIT] = { 0 };
static int wabbit_count = 0;

int main(void)
{
    sl_init("Smol - Bunny Mark Example", WIN_W, WIN_H, 0);
    sl_frame_target(60);

    sl_texture_id texture = sl_texture_load(RESOURCES_PATH "wabbit.png", NULL, NULL);
    sl_render_sampler(0, texture);

    sl_render_blend(SL_BLEND_ALPHA);

    while (sl_run())
    {
        sl_window_set_title(sl_text_format("Smol - Bunny Mark Example - Count: %i - FPS: %i", wabbit_count, sl_frame_per_second()));

        float dt = sl_frame_time();

        sl_render_clear(SL_GRAY);

        if (sl_mouse_button_is_pressed(SL_MOUSE_BUTTON_LEFT)) {
            if (wabbit_count + 100 <= MAX_WABBIT) {
                for (int i = 0; i < 100; i++) {
                    float da = SL_TAU * sl_randf();
                    wabbits[wabbit_count++] = (wabbit_t) {
                        .position = sl_mouse_get_position(),
                        .velocity = SL_VEC2(cosf(da), sinf(da)),
                        .color = sl_color_from_hsv_vec(SL_VEC3(360.0f * sl_randf(), 1, 1)),
                        .rotation = SL_TAU * sl_randf()
                    };
                }
            }
        }

        for (int i = 0; i < wabbit_count; i++)
        {
            wabbit_t* wabbit = &wabbits[i];

            wabbit->rotation += 0.1f * wabbit->velocity.x;
            wabbit->position = sl_vec2_add(wabbit->position, sl_vec2_scale(wabbit->velocity, 64.0f * dt));

            if (wabbit->position.x < 0) {
                wabbit->velocity.x *= -1.0f;
                wabbit->position.x = 0;
            }
            else if (wabbit->position.x > WIN_W) {
                wabbit->velocity.x *= -1.0f;
                wabbit->position.x = WIN_W;
            }

            if (wabbit->position.y < 0) {
                wabbit->velocity.y *= -1.0f;
                wabbit->position.y = 0;
            }
            else if (wabbit->position.y > WIN_H) {
                wabbit->velocity.y *= -1.0f;
                wabbit->position.y = WIN_H;
            }

            sl_render_color(wabbit->color);
            sl_render_rectangle_ex(wabbit->position, SL_VEC2(32, 32), wabbit->rotation);
        }

        sl_render_present();
    }

    sl_quit();

    return 0;
}
