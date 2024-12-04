#pragma warning(disable : 4819)

#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "games.h"

#define TILE_SIZE 100
#define TILE_MARGIN 8
#define M_PI 3.14159265358979323846

GtkLabel* score_label;
GtkWidget* drawing_area;
static int score = 0;
char score_text[50];

int** grid;
int game_size = 9;

float ball_x, ball_y;
float ball_dx, ball_dy;

float paddle_x;
const float paddle_width = TILE_SIZE * 1.5;
const float paddle_height = 20;

gboolean is_game_running = TRUE;

// 점수를 업데이트하고 레이블에 표시하는 함수
void show_score() {
    sprintf_s(score_text, sizeof(score_text), "Score: %d", score);
    gtk_label_set_text(score_label, score_text);
}

// 벽돌 초기화: 색상과 점수 부여
void init_game(int size) {
    grid = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        grid[i] = (int*)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            grid[i][j] = (i < 9) ? (9 - i) : 0;  // 벽돌 갯수를 8에서 9로 증가 (위아래로 늘리기)
        }
    }
    ball_x = (game_size * TILE_SIZE) / 2.0f;
    ball_y = (game_size * TILE_SIZE - TILE_MARGIN - paddle_height - 10);
    ball_dx = 1.0;
    ball_dy = -1.0;
    paddle_x = (game_size * TILE_SIZE) / 2 - paddle_width / 2;

    // 벽돌 높이 조정
    const float brick_height = (TILE_SIZE - TILE_MARGIN) / 4;  // 벽돌 높이를 1/4로 설정
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] > 0) {
                // 벽돌의 높이를 조정하는 로직 추가
                grid[i][j] = (i < 2) ? (3 - i) : 0;  // 기존 로직 유지
            }
        }
    }
}

// 게임 메모리 해제 함수
void free_game(int size) {
    for (int i = 0; i < size; i++) {
        free(grid[i]);
    }
    free(grid);
}

// 벽돌 높이와 간격을 설정하는 상수 추가
const float BRICK_HEIGHT_RATIO = 0.3;  // 벽돌 높이 비율
const float BRICK_GAP_RATIO = 0.1;     // 벽돌 간격 비율

// 공 이동 함수 (업데이트)
void move_ball() {
    if (!is_game_running) return;

    ball_x += ball_dx * 5;
    ball_y += ball_dy * 5;

    // 벽 충돌 처리
    if (ball_x < 0 || ball_x > game_size * TILE_SIZE) {
        ball_dx = -ball_dx;
    }
    if (ball_y < 0) {
        ball_dy = -ball_dy;
    }

    // 패들 충돌 처리
    if (ball_y > game_size * TILE_SIZE - TILE_MARGIN - paddle_height &&
        ball_x > paddle_x && ball_x < paddle_x + paddle_width) {
        ball_dy = -fabs(ball_dy);  // 패들에 닿으면 위로 반사
    }

    // 벽돌 충돌 처리
    for (int i = 0; i < game_size; i++) {
        for (int j = 0; j < game_size; j++) {
            if (grid[i][j] > 0) {
                float brick_height = (TILE_SIZE - TILE_MARGIN) * BRICK_HEIGHT_RATIO;  // 벽돌 높이
                float gap = brick_height * BRICK_GAP_RATIO;                           // 높이에 비례한 간격
                if (ball_x > j * TILE_SIZE && ball_x < (j + 1) * TILE_SIZE &&
                    ball_y > i * (brick_height + gap) && ball_y < (i * (brick_height + gap) + brick_height)) {  // 충돌 감지 로직 수정
                    score += grid[i][j] * 10;                                                                   // 벽돌 레벨에 따른 점수 추가
                    grid[i][j] = 0;                                                                             // 벽돌 제거
                    ball_dy = -ball_dy;
                    show_score();
                }
            }
        }
    }

    // 바닥에 닿으면 게임    료
    if (ball_y > game_size * TILE_SIZE) {
        is_game_running = FALSE;
        gtk_label_set_text(score_label, "Game Over!");
        return;
    }
}

// 창 크기와 맞춰 drawing_area 크기 설정
void set_drawing_area_size(GtkWidget* widget) {
    int width = game_size * TILE_SIZE;
    int height = game_size * TILE_SIZE;
    gtk_widget_set_size_request(widget, width, height);
}

// 벽돌 색상 설정
void draw_bricks(cairo_t* cr) {
    for (int i = 0; i < game_size; i++) {
        for (int j = 0; j < game_size; j++) {
            if (grid[i][j] > 0) {  // 벽돌이 존재할 때
                switch (grid[i][j]) {
                case 3:
                    cairo_set_source_rgb(cr, 1, 0, 0);  // 빨간색
                    break;
                case 2:
                    cairo_set_source_rgb(cr, 1, 1, 0);  // 노란색
                    break;
                case 1:
                    cairo_set_source_rgb(cr, 0, 1, 0);  // 초록색
                    break;
                }
                float brick_height = (TILE_SIZE - TILE_MARGIN) * BRICK_HEIGHT_RATIO;  // 벽돌 높이
                float gap = brick_height * BRICK_GAP_RATIO;                           // 높이에 비례한 간격
                cairo_rectangle(cr, j * TILE_SIZE, i * (brick_height + gap), TILE_SIZE - TILE_MARGIN, brick_height);
                cairo_fill(cr);
            }
        }
    }
}

// 그리기 이벤트 핸들러 (업데이트)
gboolean event_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 배경색 설정 (검은색)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    // 벽돌 그리기
    draw_bricks(cr);  // 벽돌 그리기 호출

    // 공 그리기
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_arc(cr, ball_x, ball_y, 10, 0, 2 * M_PI);
    cairo_fill(cr);

    // 패들 그리기
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_rectangle(cr, paddle_x, game_size * TILE_SIZE - TILE_MARGIN - paddle_height, paddle_width, paddle_height);
    cairo_fill(cr);

    return FALSE;
}

// 키 입력 이벤트 핸들러
gboolean on_key(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    switch (event->keyval) {
    case GDK_KEY_Left:
        paddle_x -= 20;
        if (paddle_x < 0) paddle_x = 0;
        break;
    case GDK_KEY_Right:
        paddle_x += 20;
        if (paddle_x > game_size * TILE_SIZE - paddle_width) {
            paddle_x = game_size * TILE_SIZE - paddle_width;
        }
        break;
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

// 주기적    이동 및 화면 갱신
gboolean on_timer(gpointer data) {
    move_ball();
    gtk_widget_queue_draw(drawing_area);
    return is_game_running;
}

// 게임 화면 생성 함수
GtkWidget* create_breakout_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    score_label = GTK_LABEL(gtk_label_new("Score: 0"));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(score_label), FALSE, FALSE, 0);

    drawing_area = gtk_drawing_area_new();
    set_drawing_area_size(drawing_area);  // 창 크기 설정
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(event_draw), NULL);
    g_signal_connect(drawing_area, "key-press-event", G_CALLBACK(on_key), NULL);

    gtk_widget_set_can_focus(drawing_area, TRUE);
    gtk_widget_grab_focus(drawing_area);

    init_game(game_size);
    g_timeout_add(16, on_timer, NULL);

    // 메인 메뉴로 돌아가는 버튼 추가
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 0);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    return vbox;
}

// 게임 시작 함수
void start_breakout_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);

    score = 0;                                                   // 점수 초기화
    show_score();                                                // 점수 레이블 업데이트
    is_game_running = TRUE;                                      // 게임 상태 설정
    free_game(game_size);                                        // 게임 메모리 해제 (초기화)
    init_game(game_size);                                        // 게임 초기화
    gtk_stack_set_visible_child_name(stack, "breakout_screen");  // 게임 화면 표시
}
