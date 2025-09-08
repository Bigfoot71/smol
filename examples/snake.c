#include <smol.h>

#define WIN_W 640
#define WIN_H 480

#define GRID_SIZE 20
#define MAX_LENGTH 400

#define GRID_W (WIN_W / GRID_SIZE)
#define GRID_H (WIN_H / GRID_SIZE)

typedef struct { int x, y; } point_t;

static point_t snake[MAX_LENGTH];
static int length = 3, score = 0, dx = 1, dy = 0;
static point_t food;
static bool game_over = false;
static float timer = 0.0f;

void spawn_food()
{
    do {
        food.x = sl_randi_range(0, GRID_W);
        food.y = sl_randi_range(0, GRID_H);
        for (int i = 0; i < length; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                goto retry;
            }
        }
        break;
        retry:;
    } while (true);
}

void init()
{
    for (int i = 0; i < 3; i++) {
        snake[i] = (point_t){GRID_W/2 - i, GRID_H/2};
    }
    length = 3; score = 0; dx = 1; dy = 0;
    game_over = false; timer = 0;
    spawn_food();
}

void update()
{
    if (game_over) {
        if (sl_key_just_pressed(SL_KEY_SPACE)) init();
        return;
    }
    
    if (sl_key_just_pressed(SL_KEY_UP) && dy != 1) { dx = 0; dy = -1; }
    if (sl_key_just_pressed(SL_KEY_DOWN) && dy != -1) { dx = 0; dy = 1; }
    if (sl_key_just_pressed(SL_KEY_LEFT) && dx != 1) { dx = -1; dy = 0; }
    if (sl_key_just_pressed(SL_KEY_RIGHT) && dx != -1) { dx = 1; dy = 0; }
    
    timer += sl_frame_get_delta();
    if (timer < 0.15f) return;
    timer = 0;
    
    point_t head = {snake[0].x + dx, snake[0].y + dy};
    
    if (head.x < 0 || head.x >= GRID_W || head.y < 0 || head.y >= GRID_H) {
        game_over = true; return;
    }
    
    for (int i = 0; i < length; i++) {
        if (snake[i].x == head.x && snake[i].y == head.y) {
            game_over = true; return;
        }
    }
    
    bool ate = (head.x == food.x && head.y == food.y);
    if (ate) { score++; length++; spawn_food(); }
    
    for (int i = length - 1; i > 0; i--) snake[i] = snake[i-1];
    snake[0] = head;
}

void render()
{
    sl_render_clear(SL_BLACK);
    
    if (game_over) {
        sl_render_set_color(SL_RED);
        sl_render_text_centered("GAME OVER", SL_VEC2(320, 200), 32, SL_VEC2(2, 2));
        sl_render_set_color(SL_WHITE);
        sl_render_text_centered(sl_text_format("Score: %d - SPACE to restart", score), SL_VEC2(320, 250), 20, SL_VEC2(1, 1));
    }
    else {
        sl_render_set_color(SL_COLOR(31, 31, 31, 255));
        sl_render_grid(0, 0, 640, 480, 640 / GRID_SIZE, 480 / GRID_SIZE, 1.0f);

        sl_render_set_color(SL_GREEN);
        for (int i = 0; i < length; i++) {
            if (i == 0) sl_render_set_color(SL_WHITE);
            sl_render_rectangle(snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE-1, GRID_SIZE-1);
            if (i == 0) sl_render_set_color(SL_GREEN);
        }
        
        sl_render_set_color(SL_RED);
        sl_render_rectangle(food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE-1, GRID_SIZE-1);
        
        sl_render_set_color(SL_YELLOW);
        sl_render_text(sl_text_format("Score: %d", score), SL_VEC2(10, 10), 24, SL_VEC2(1, 1));
    }
    
    sl_render_present();
}

int main(void)
{
    sl_init("Smol - Snake Example", 640, 480, 0);
    sl_frame_set_target_fps(60);

    sl_font_id font = sl_font_load(RESOURCES_PATH "font.ttf", SL_FONT_PIXEL, 32, 0, 0);
    sl_render_set_font(font);

    init();

    while (sl_frame_step()) {
        if (sl_key_just_pressed(SL_KEY_ESCAPE)) break;
        update();
        render();
    }

    sl_quit();

    return 0;
}
