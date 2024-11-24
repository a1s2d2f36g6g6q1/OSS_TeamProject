#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include "games.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BALL_RADIUS 10
static float paddle_width = 100;
#define PADDLE_HEIGHT 20
#define BRICK_WIDTH 80
#define BRICK_HEIGHT 30
#define NUM_BRICKS 24

// 구조체 정의
typedef struct
{
    float x, y;   // 공의 위치
    float dx, dy; // 공의 속도
} Ball;

typedef struct
{
    float x, y; // 패들의 위치
    float dx;   // 패들의 속도
} Paddle;

typedef struct
{
    float x, y;         // 벽돌의 위치
    gboolean destroyed; // 벽돌이 파괴됐는지 여부
    int durability;     // 벽돌의 내구도
    int points;         // 벽돌의 점수
} Brick;

// 타이머 콜백을 위한 구조체 정의
typedef struct
{
    GtkWidget *drawing_area;
    gboolean valid;
} TimerData;

// 전역 변수 선언
Ball ball;
Paddle paddle;
Brick bricks[NUM_BRICKS];
static gboolean is_fullscreen = FALSE;
static int score = 0;
static int lives = 3;
static float speed_multiplier = 1.0;
static time_t game_start_time;
static gboolean game_over = FALSE; // 게임 오버 상태
static gboolean game_won = FALSE;  // 승리 상태
static int current_level = 1;
static const int MAX_LEVEL = 3; // 최대 레벨 수
static const int BRICKS_PER_ROW = 8;
static const int MAX_ROWS = 5;

// 레벨별 벽돌 배치 구조체
typedef struct
{
    int num_rows;              // 행 수
    float strong_brick_chance; // 강화 벽돌 확률
    float speed_factor;        // 속도 계수
} LevelConfig;

// 레벨별 설정
static const LevelConfig level_configs[3] = {
    {2, 0.1, 1.0}, // 레벨 1: 2행, 10% 강화 벽돌, 기본 속도
    {3, 0.2, 1.2}, // 레벨 2: 3행, 20% 강화 벽돌, 20% 빠른 속도
    {5, 0.3, 1.5}  // 레벨 3: 5행, 30% 강화 벽돌, 50% 빠른 속도
};

// 아이템 종류 정의
typedef enum
{
    ITEM_LIFE,          // 생명 +1
    ITEM_PADDLE_EXPAND, // 패들 크기 증가
    ITEM_SLOW_BALL,     // 공 속도 감소
    ITEM_TYPES_COUNT    // 아이템 종류 수
} ItemType;

// 아이템 구조체
typedef struct
{
    float x, y;        // 위치
    float dy;          // 떨어지는 속도
    ItemType type;     // 아이템 종류
    gboolean active;   // 활성화 상태
    time_t spawn_time; // 생성 시간
} Item;

// 전역 변수 추가
#define MAX_ITEMS 5
static Item items[MAX_ITEMS];
static const float ITEM_DROP_SPEED = 3.0;
static const int ITEM_DURATION = 10; // 아이템 지속시간(초)
static const int ITEM_LIFETIME = 5;  // 아이템이 사라지기까지의 시간(초)
static float original_paddle_width;
static gboolean paddle_expanded = FALSE;
static time_t paddle_expand_time = 0;
static gboolean ball_slowed = FALSE;
static time_t ball_slow_time = 0;

// 함수 선언
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_timeout(gpointer user_data);
static void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean game_started = FALSE; // 게임 시작 여부
static gboolean paused = FALSE;       // 게임 일시 정지 상태
void init_game();
void move_ball();
void move_paddle();
void check_collisions();
void reset_ball();
void draw_bricks(cairo_t *cr);
void draw_paddle(cairo_t *cr);
void draw_ball(cairo_t *cr);
void draw_game_status(cairo_t *cr);
void draw_game_over(cairo_t *cr);
void advance_to_next_level();
void init_items();
void spawn_item(float x, float y);
void apply_item_effect(ItemType type);
void update_items();
void draw_items(cairo_t *cr);

void start_breakout_game_BP(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "brickbreaker_screen");
}

// 메인 함수
GtkWidget* create_brickbreaker_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, WINDOW_WIDTH, WINDOW_HEIGHT);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 5);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw_event), NULL); // on_draw_event 연결

    // 키보드 이벤트 처리
    g_signal_connect(gtk_widget_get_toplevel(drawing_area), "key-press-event", G_CALLBACK(on_key_press), NULL);

    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 5);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    init_game(); // 벽돌깨기 초기화

    // 타이머 설정
    TimerData* timer_data = g_new(TimerData, 1);
    timer_data->drawing_area = drawing_area;
    timer_data->valid = TRUE;
    g_timeout_add_full(G_PRIORITY_DEFAULT, 16, on_timeout, timer_data, (GDestroyNotify)g_free);

    return vbox;
}

// 게임 초기화 함수
void init_game()
{
    // 공 초기화
    ball.x = WINDOW_WIDTH / 2;
    ball.y = WINDOW_HEIGHT - 100;

    // 레벨에 따른 초기 속도 설정
    float base_speed = 6.0 * level_configs[current_level - 1].speed_factor;
    ball.dx = base_speed * speed_multiplier;
    ball.dy = -base_speed * speed_multiplier;

    // 패들 초기화
    paddle.x = (WINDOW_WIDTH - paddle_width) / 2;
    paddle.y = WINDOW_HEIGHT - PADDLE_HEIGHT - 20;
    paddle.dx = 0;

    // 현재 레벨 설정 가져오기
    const LevelConfig *config = &level_configs[current_level - 1];
    int num_bricks = config->num_rows * BRICKS_PER_ROW;

    // 벽돌 초기화
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        // 현재 레벨의 행 수를 초과하는 벽돌은 숨김
        if (i / BRICKS_PER_ROW >= config->num_rows)
        {
            bricks[i].destroyed = TRUE;
            continue;
        }

        bricks[i].x = (i % BRICKS_PER_ROW) * (BRICK_WIDTH + 10) + 50;
        bricks[i].y = (i / BRICKS_PER_ROW) * (BRICK_HEIGHT + 5) + 50;
        bricks[i].destroyed = FALSE;

        // 레벨별 강화 벽돌 확률 적용
        if ((float)rand() / RAND_MAX < config->strong_brick_chance)
        {
            bricks[i].durability = 2 + rand() % 2;                        // 2-3의 내구도
            bricks[i].points = 20 * bricks[i].durability * current_level; // 레벨에 따른 점수 증가
        }
        else
        {
            bricks[i].durability = 1;
            bricks[i].points = 10 * current_level;
        }
    }

    game_start_time = time(NULL);
    speed_multiplier = 1.0;
    init_items();
}

// 게임 시작 화면 그리기
void draw_start_screen(cairo_t *cr)
{
    cairo_set_source_rgb(cr, 0, 0, 0); // 배경색 검정
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 1, 1, 1); // 흰색 텍스트
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 50);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2);
    cairo_show_text(cr, "Brick-out Game");

    cairo_set_font_size(cr, 30);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 145, WINDOW_HEIGHT / 2 + 60);
    cairo_show_text(cr, "Press 'Enter' to restart");
}

// 게임 화면을 그리는 함수
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    if (!game_started)
    {
        draw_start_screen(cr); // 게임 시작 전 화면 그리기
    }
    else
    {
        // 배경 그리기
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);

        // 게임 요소 그리기
        draw_game_status(cr);
        draw_ball(cr);
        draw_paddle(cr);
        draw_bricks(cr);
        draw_items(cr);

        if (game_over)
        {
            draw_game_over(cr);
        }
    }
    return FALSE;
}

// 공 그리기 함수
void draw_ball(cairo_t *cr)
{
    cairo_set_source_rgb(cr, 1, 0, 0); // 공 색 빨간색
    cairo_arc(cr, ball.x, ball.y, BALL_RADIUS, 0, 2 * G_PI);
    cairo_fill(cr);
}

// 패들 그리기 함수
void draw_paddle(cairo_t *cr)
{
    cairo_set_source_rgb(cr, 0, 0, 1); // 패들 색 파란색
    cairo_rectangle(cr, paddle.x, paddle.y, paddle_width, PADDLE_HEIGHT);
    cairo_fill(cr);
}

// 벽돌 그리기 함수
void draw_bricks(cairo_t *cr)
{
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (!bricks[i].destroyed)
        {
            // 내구도에 따른 벽돌 색상 설정
            switch (bricks[i].durability)
            {
            case 3:
                cairo_set_source_rgb(cr, 1, 0, 0); // 빨강 (가장 강한 벽돌)
                break;
            case 2:
                cairo_set_source_rgb(cr, 1, 0.5, 0); // 주황
                break;
            default:
                cairo_set_source_rgb(cr, 0, 1, 0); // 초록 (일반 벽돌)
            }

            cairo_rectangle(cr, bricks[i].x, bricks[i].y, BRICK_WIDTH, BRICK_HEIGHT);
            cairo_fill(cr);
        }
    }
}

// 게임 루프 (타이머 설정)
static gboolean on_timeout(gpointer user_data)
{
    TimerData *data = (TimerData *)user_data;

    if (!data || !data->valid || !GTK_IS_WIDGET(data->drawing_area))
    {
        return FALSE; // 타이머 중지
    }

    if (!paused)
    {
        move_ball();
        move_paddle();
        update_items(); // 아이템 업데이트
        check_collisions();
        gtk_widget_queue_draw(data->drawing_area);
    }
    return TRUE;
}

// 공 이동 함수
void move_ball()
{
    // 시간에 따른 속도 증가
    time_t current_time = time(NULL);
    float elapsed_time = difftime(current_time, game_start_time);
    speed_multiplier = 1.0 + (elapsed_time / 30.0) * 0.5; // 30초마다 50%씩 속도 증가

    // 속도에 multiplier 적용
    float current_dx = ball.dx > 0 ? 6 * speed_multiplier : -6 * speed_multiplier;
    float current_dy = ball.dy > 0 ? 6 * speed_multiplier : -6 * speed_multiplier;

    ball.x += current_dx;
    ball.y += current_dy;

    // 벽에 부딪히면 반사
    if (ball.x - BALL_RADIUS < 0 || ball.x + BALL_RADIUS > WINDOW_WIDTH)
    {
        ball.dx = -ball.dx;
    }

    // 위쪽 벽에 부딪히면 반사
    if (ball.y - BALL_RADIUS < 0)
    {
        ball.dy = -ball.dy;
    }

    // 바닥에 떨어지면 공 초기화
    if (ball.y + BALL_RADIUS > WINDOW_HEIGHT)
    {
        reset_ball();
    }
}

// 패들 이동 함수
void move_paddle()
{
    paddle.x += paddle.dx;

    // 패들이 화면 밖으로 나가지 않게 제한
    if (paddle.x < 0)
        paddle.x = 0;
    if (paddle.x + paddle_width > WINDOW_WIDTH)
        paddle.x = WINDOW_WIDTH - paddle_width;
}

// 충돌 검사 함수
void check_collisions()
{
    // 공과 패들 충돌 검사
    if (ball.y + BALL_RADIUS > paddle.y &&
        ball.y - BALL_RADIUS < paddle.y + PADDLE_HEIGHT &&
        ball.x > paddle.x &&
        ball.x < paddle.x + paddle_width)
    {
        ball.dy = -ball.dy;
    }

    // 공과 벽돌 충돌 검사
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (!bricks[i].destroyed)
        {
            if (ball.x > bricks[i].x &&
                ball.x < bricks[i].x + BRICK_WIDTH &&
                ball.y - BALL_RADIUS < bricks[i].y + BRICK_HEIGHT &&
                ball.y + BALL_RADIUS > bricks[i].y)
            {

                bricks[i].durability--;

                if (bricks[i].durability <= 0)
                {
                    bricks[i].destroyed = TRUE;
                    score += bricks[i].points; // 점수 추가

                    // 벽돌이 파괴될 때 아이템 생성
                    spawn_item(bricks[i].x + BRICK_WIDTH / 2,
                               bricks[i].y + BRICK_HEIGHT);
                }

                ball.dy = -ball.dy; // 공 반사

                // 벽돌 파괴 시 속도 약간 증가 (추가 난이도)
                speed_multiplier += 0.05;
                break; // 한 번에 하나의 벽돌만 처리
            }
        }
    }

    // 모든 벽돌이 파괴되었는지 확인
    gboolean all_destroyed = TRUE;
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        if (!bricks[i].destroyed)
        {
            all_destroyed = FALSE;
            break;
        }
    }

    if (all_destroyed)
    {
        if (current_level < MAX_LEVEL)
        {
            advance_to_next_level();
        }
        else
        {
            game_won = TRUE;
            game_over = TRUE;
            paused = TRUE;
        }
    }
}

// 공 초기화 함수
void reset_ball()
{
    lives--;

    if (lives <= 0)
    {
        game_over = TRUE;
        game_won = FALSE;
        paused = TRUE;
    }
    else
    {
        ball.x = WINDOW_WIDTH / 2;
        ball.y = WINDOW_HEIGHT - 100;
        ball.dx = 6 * speed_multiplier; // 현재 속도 유지
        ball.dy = -6 * speed_multiplier;
    }
}

// 키보드 입력 이벤트 처리 함수
static void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    GtkWindow *window = GTK_WINDOW(widget);

    switch (event->keyval)
    {
    case GDK_KEY_Left:
        paddle.dx = -7;
        break;
    case GDK_KEY_Right:
        paddle.dx = 7;
        break;
    case GDK_KEY_F11: // F11 키로 전체화면 전환
        is_fullscreen = !is_fullscreen;
        if (is_fullscreen)
        {
            gtk_window_fullscreen(window);
        }
        else
        {
            gtk_window_unfullscreen(window);
        }
        break;
    case GDK_KEY_Return:
        if (!game_started)
        {
            game_started = TRUE;
            init_game();
        }
        break;
    case GDK_KEY_r:
    case GDK_KEY_R:
        if (game_over)
        {
            game_over = FALSE;
            game_won = FALSE;
            paused = FALSE;
            score = 0;
            lives = 3;
            init_game();
        }
        break;
    }
}

// 게임 상태 표시 함수 수정
void draw_game_status(cairo_t *cr)
{
    char score_text[32];
    char lives_text[32];
    char level_text[32];

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);

    // 점수 표시
    snprintf(score_text, sizeof(score_text), "Score: %d", score);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, score_text);

    // 레벨 표시
    snprintf(level_text, sizeof(level_text), "Level: %d", current_level);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 40, 30);
    cairo_show_text(cr, level_text);

    // 생명 표시
    snprintf(lives_text, sizeof(lives_text), "Lives: %d", lives);
    cairo_move_to(cr, WINDOW_WIDTH - 100, 30);
    cairo_show_text(cr, lives_text);
}

// 게임 오버 화면 그리기 함수 수정
void draw_game_over(cairo_t *cr)
{
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    // 승리/패배 메시지
    cairo_set_font_size(cr, 50);
    if (game_won)
    {
        cairo_set_source_rgb(cr, 0, 1, 0); // 초록색 (승리)
        cairo_move_to(cr, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50);
        cairo_show_text(cr, "YOU WIN!");
    }
    else
    {
        cairo_set_source_rgb(cr, 1, 0, 0); // 빨간색 (패배)
        cairo_move_to(cr, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50);
        cairo_show_text(cr, "GAME OVER");
    }

    // 최종 점수 표시
    cairo_set_font_size(cr, 30);
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Final Score: %d", score);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 20);
    cairo_show_text(cr, score_text);

    // 재시작 안내
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 125, WINDOW_HEIGHT / 2 + 70);
    cairo_show_text(cr, "Press 'R' to restart");
}

// 다음 레벨로 진행하는 함수 추가
void advance_to_next_level()
{
    current_level++;
    if (current_level <= MAX_LEVEL)
    {
        score += 100 * (current_level - 1); // 레벨 클리어 보너스
        init_game();
        game_won = FALSE;
        game_over = FALSE;
        paused = FALSE;
    }
    else
    {
        // 모든 레벨 클리어
        game_won = TRUE;
        game_over = TRUE;
        paused = TRUE;
    }
}

// 아이템 초기화 함수
void init_items()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        items[i].active = FALSE;
    }
    original_paddle_width = paddle_width;
}

// 아이템 생성 함수
void spawn_item(float x, float y)
{
    // 30% 확률로 아이템 생성
    if ((float)rand() / RAND_MAX > 0.3)
        return;

    // 비활성 아이템 슬롯 찾기
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (!items[i].active)
        {
            items[i].x = x;
            items[i].y = y;
            items[i].dy = ITEM_DROP_SPEED * level_configs[current_level - 1].speed_factor;
            items[i].type = rand() % ITEM_TYPES_COUNT;
            items[i].active = TRUE;
            items[i].spawn_time = time(NULL);
            break;
        }
    }
}

// 아이템 효과 적용 함수
void apply_item_effect(ItemType type)
{
    time_t current_time = time(NULL);

    switch (type)
    {
    case ITEM_LIFE:
        lives++;
        break;

    case ITEM_PADDLE_EXPAND:
        if (!paddle_expanded)
        {
            paddle_width = original_paddle_width * 1.5f;
            paddle_expanded = TRUE;
            paddle_expand_time = current_time;
        }
        break;

    case ITEM_SLOW_BALL:
        if (!ball_slowed)
        {
            speed_multiplier *= 0.7f;
            ball_slowed = TRUE;
            ball_slow_time = current_time;
        }
        break;
    }
}

// 아이템 업데이트 함수
void update_items()
{
    time_t current_time = time(NULL);

    // 아이템 이동 및 제거
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (items[i].active)
        {
            // 아이템 이동
            items[i].y += items[i].dy;

            // 패들과 충돌 검사
            if (items[i].y + 10 > paddle.y &&
                items[i].y < paddle.y + PADDLE_HEIGHT &&
                items[i].x > paddle.x &&
                items[i].x < paddle.x + paddle_width)
            {

                apply_item_effect(items[i].type);
                items[i].active = FALSE;
            }

            // 화면 밖으로 나가거나 일정 시간 경과 시 제거
            if (items[i].y > WINDOW_HEIGHT ||
                difftime(current_time, items[i].spawn_time) > ITEM_LIFETIME)
            {
                items[i].active = FALSE;
            }
        }
    }

    // 아이템 효과 시간 체크
    if (paddle_expanded &&
        difftime(current_time, paddle_expand_time) > ITEM_DURATION)
    {
        paddle_width = original_paddle_width;
        paddle_expanded = FALSE;
    }

    if (ball_slowed &&
        difftime(current_time, ball_slow_time) > ITEM_DURATION)
    {
        speed_multiplier /= 0.7f;
        ball_slowed = FALSE;
    }
}

// 아이템 그리기 함수
void draw_items(cairo_t *cr)
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (items[i].active)
        {
            // 아이템 종류별 색상 설정
            switch (items[i].type)
            {
            case ITEM_LIFE:
                cairo_set_source_rgb(cr, 1, 0, 0); // 빨강
                break;
            case ITEM_PADDLE_EXPAND:
                cairo_set_source_rgb(cr, 0, 1, 0); // 초록
                break;
            case ITEM_SLOW_BALL:
                cairo_set_source_rgb(cr, 0, 0, 1); // 파랑
                break;
            }

            // 아이템 그리기 (작은 원으로 표시)
            cairo_arc(cr, items[i].x, items[i].y, 8, 0, 2 * G_PI);
            cairo_fill(cr);
        }
    }
}
