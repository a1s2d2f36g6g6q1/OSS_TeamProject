#ifndef BRICKBREAKER_H
#define BRICKBREAKER_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BALL_RADIUS 10
#define BRICK_WIDTH 80
#define BRICK_HEIGHT 30
#define NUM_BRICKS 24

// 구조체 정의
typedef struct {
    float x, y;    // 공의 위치
    float dx, dy;  // 공의 속도
} Ball;

typedef struct {
    float x, y;  // 패들의 위치
    float dx;    // 패들의 속도
} Paddle;

typedef struct {
    float x, y;          // 벽돌의 위치
    gboolean destroyed;  // 벽돌이 파괴됐는지 여부
} Brick;

// 전역 변수 선언
static Ball ball;
static Paddle paddle;
static Brick bricks[NUM_BRICKS];

// 함수 선언
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean on_timeout(gpointer data);
static void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);

void init_game();
void move_ball();
void move_paddle();
void check_collisions();
void reset_ball();
void draw_bricks(cairo_t *cr);
void draw_paddle(cairo_t *cr);
void draw_ball(cairo_t *cr);

static gboolean game_started = FALSE;  // 게임 시작 여부
static gboolean paused = FALSE;        // 게임 일시 정지 상태
static gboolean game_over = FALSE;     // 게임 오버 상태

void startBrickbreaker();

#endif  // BRICKBREAKER_H