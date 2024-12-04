#pragma warning(disable : 4819)

#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "games.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BRICK_ROWS 5
#define BRICK_COLUMNS 10
#define BRICK_WIDTH 60
#define BRICK_HEIGHT 20

GtkLabel* score_label;
GtkWidget* drawing_area;
int game_size;

// 공 구조체
typedef struct {
    int x, y;
    int dx, dy;
} Ball;

// 패들 구조체
typedef struct {
    int x, y;
} Paddle;

// 벽돌 구조체
typedef struct {
    int x, y;
    bool destroyed;
} Brick;

// 상태 구조체
typedef struct {
    int score;
    int lives;
    int level;
} State;

Ball ball;      // 공 구조체 변수
Paddle paddle;  // 패들 구조체 변수
Brick bricks[BRICK_ROWS][BRICK_COLUMNS];  // 벽돌 구조체 배열
State state;    // 게임 상태 변수

// 함수 선언
void reset_game();
void reset_ball();
void init_bricks();
static void check_collision();

/*
GtkWidget* create_scoreboard_screen(GtkStack* stack);
GtkWidget* create_setting_screen(GtkStack* stack);
*/

// 게임 초기화 함수
void init_game(int game_size) {
    state.score = 0;
    state.lives = 3;
    state.level = 1;
    reset_ball();
    init_bricks();
}

// 벽돌 초기화 함수
void init_bricks() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
            bricks[i][j].x = j * BRICK_WIDTH + 50;
            bricks[i][j].y = i * BRICK_HEIGHT + 50;
            bricks[i][j].destroyed = false;
        }
    }
}

// 타이머 이벤트 핸들러 (게임 상태 업데이트 코드)
gboolean on_timer(gpointer data) {
    // Validate the drawing area widget before updating
    if (GTK_IS_WIDGET(drawing_area) && gtk_widget_get_visible(drawing_area)) {
        // 공 위치
        ball.x += ball.dx;  // 공의 x 위치
        ball.y += ball.dy;  // 공의 y 위치

        // 벽 충돌 감지
        if (ball.x < 0 || ball.x > WINDOW_WIDTH) {
            ball.dx = -ball.dx;  // x 방향 반전
        }

        if (ball.y < 0) {
            ball.dy = -ball.dy;  // y 방향 반전
        }

        if (ball.y > WINDOW_WIDTH) {
            // 공이 바닥에 닿았을 때
            state.lives--;

            if (state.lives <= 0) {
                reset_game();  // 게임 초기화 함수 호출
            }
            else {
                reset_ball();  // 공 초기화 함수 호출
            }
        }

        // 패들 충돌 감지
        if (ball.y + 10 >= paddle.y && ball.x >= paddle.x && ball.x <= paddle.x + 100) {
            ball.dy = -ball.dy;  // y 방향 반전
        }

        // 레벨 업 조건
        if (state.score >= 100) {  // 점수가 100 이상일 때 레벨 업
            state.level++;
            state.score = 0;  // 점수 초기화
            init_bricks();
        }

        // Queue a redraw only if the widget is realized and valid
        gtk_widget_queue_draw(drawing_area);  // 화면 갱신
    }

    return TRUE;  // 계속 호출되도록 TRUE 반환
}

// 벽돌 충돌 감지 함수
void check_collision() {
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
            if (!bricks[i][j].destroyed) {
                if (ball.x >= bricks[i][j].x && ball.x <= bricks[i][j].x + BRICK_WIDTH &&
                    ball.y >= bricks[i][j].y && ball.y <= bricks[i][j].y + BRICK_HEIGHT) {
                    ball.dy = -ball.dy;  // y 방향 반전
                    bricks[i][j].destroyed = true;
                    state.score += 10;
                }
            }
        }
    }
}

void reset_game() {
    state.score = 0;
    state.lives = 3;
    state.level = 1;
    reset_ball();
    init_bricks();
}

void reset_ball() {
    ball.x = WINDOW_WIDTH / 2;
    ball.y = WINDOW_HEIGHT - 50;               // 패들 위쪽에 초기화
    ball.dx = (rand() % 2 == 0 ? 1 : -1) * 5;  // 임의의 x 방향
    ball.dy = -5;                              // 위쪽으로 이동
}

// 그리기 이벤트 핸들러 (게임 화면 그리기 코드)
gboolean event_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 배경색 검정
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);

    // 공 그리기
    cairo_set_source_rgb(cr, 1, 0, 0);               // 빨간색
    cairo_arc(cr, ball.x, ball.y, 10, 0, 2 * G_PI);  // 공의 위치와 반지름
    cairo_fill(cr);                                  // 공 그리기

    // 벽돌 그리기
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
            if (!bricks[i][j].destroyed) {
                cairo_set_source_rgb(cr, 0, 1, 0);  // 초록색
                cairo_rectangle(cr, bricks[i][j].x, bricks[i][j].y, BRICK_WIDTH, BRICK_HEIGHT);
                cairo_fill(cr);  // 벽돌 그리기
            }
        }
    }

    // 패들 그리기
    cairo_set_source_rgb(cr, 0, 0, 1);                 // 파란색
    cairo_rectangle(cr, paddle.x, paddle.y, 100, 20);  // 패들의 위치와 크기
    cairo_fill(cr);                                    // 패들 그리기

    // 현재 레벨 표시
    cairo_set_font_size(cr, 20);
    cairo_set_source_rgb(cr, 1, 1, 1);  // 흰색
    cairo_move_to(cr, 10, 20);
    cairo_show_text(cr, g_strdup_printf("Level: %d", state.level));

    return FALSE;  // GTK가 추가적으로 그리도록 FALSE 반환
}

// 키보드 입력 핸들러
gboolean on_key(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    // 레벨에 따라 패들 속도 조정
    int paddle_speed = 10 + (state.level * 2);

    // 패들 이동
    switch (event->keyval) {
    case GDK_KEY_Left:             // 왼쪽 화살표 키
        paddle.x -= paddle_speed;  // 패들을 왼쪽으로 이동
        break;
    case GDK_KEY_Right:            // 오른쪽 화살표 키
        paddle.x += paddle_speed;  // 패들을 오른쪽으로 이동
        break;
    }

    return FALSE;  // GTK가 추가적으로 처리하도록 FALSE 반환
}

void set_drawing_area_size(GtkWidget* drawing_area) {
    gtk_widget_set_size_request(drawing_area, WINDOW_WIDTH, WINDOW_HEIGHT);
}

// 게임 화면 생성 함수
GtkWidget* create_breakout_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 점수 표시 라벨 생성 및 컨테이너에 추가
    GtkLabel* breakout_score_label = GTK_LABEL(gtk_label_new("Score: 0"));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(breakout_score_label), FALSE, FALSE, 0);

    // 게임 화면을 그릴 영역 생성
    GtkWidget* breakout_drawing_area = gtk_drawing_area_new();
    set_drawing_area_size(breakout_drawing_area);  // 화면 크기 설정 함수 호출
    gtk_box_pack_start(GTK_BOX(vbox), breakout_drawing_area, TRUE, TRUE, 0);

    // 그리기 이벤트 핸들러 등록
    g_signal_connect(breakout_drawing_area, "draw", G_CALLBACK(event_draw), NULL);

    // 키보드 입력 이벤트 핸들러 등록
    g_signal_connect(breakout_drawing_area, "key-press-event", G_CALLBACK(on_key), NULL);

    // drawing_area가 포커스를 받을 수 있도록 설정
    gtk_widget_set_can_focus(breakout_drawing_area, TRUE);
    g_signal_connect(breakout_drawing_area, "realize", G_CALLBACK(gtk_widget_grab_focus), NULL);

    // 게임 초기화 (공, 벽돌, 패들 상태 초기 설정)
    init_game(game_size);

    // 16ms마다 on_timer 함수 호출 (60FPS로 게임 상태 갱신)
    g_timeout_add(16, on_timer, NULL);

    // 메인 메뉴로 돌아가는 버튼 생성 및 추가
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 0);

    // 버튼 클릭 시 메인 메뉴 화면으로 전환
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    // 구성된 화면 반환
    return vbox;
}

// 게임 시작 함수
void start_breakout_game(GtkWidget* widget, gpointer data) {
    // GtkStack을 통해 여러 화면 관리
    GtkStack* stack = GTK_STACK(data);

    // "breakout_screen" 이름의 게임 화면 표시
    gtk_stack_set_visible_child_name(stack, "breakout_screen");  // 게임 화면 표시
}