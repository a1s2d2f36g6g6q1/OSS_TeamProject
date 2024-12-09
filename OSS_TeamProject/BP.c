﻿#pragma warning(disable : 4819)

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// 상수 정의
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 800
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 10
#define BALL_SIZE 15
#define BRICK_ROWS 7
#define BRICK_COLS 10
#define BRICK_WIDTH 80
#define BRICK_HEIGHT 30
#define BRICK_PADDING 5
#define BALL_SPEED 8.0
#define ITEM_FALL_SPEED 3.5  // 아이템 떨어지는 속도 증가
#define TOTAL_BRICK_WIDTH ((BRICK_WIDTH + BRICK_PADDING) * BRICK_COLS + BRICK_PADDING)
#define TOTAL_BRICK_HEIGHT ((BRICK_HEIGHT + BRICK_PADDING) * BRICK_ROWS + BRICK_PADDING)

// 아이템 타입 정의
typedef enum {
    ITEM_PADDLE_EXTEND,
    ITEM_MULTI_BALL,
    ITEM_EXTRA_LIFE,
    ITEM_TYPES_COUNT
} ItemType;

// 아이템 구조체
typedef struct {
    double x, y;
    ItemType type;
    bool active;
} Item;

// 공 구조체
typedef struct {
    double x, y;
    double dx, dy;
    bool active;
} Ball;

// 게임 상태 구조체
typedef struct {
    Ball balls[10];
    int active_balls;
    GtkWidget* drawing_area;
    double paddle_x;
    int score;
    bool game_running;
    int bricks[BRICK_ROWS][BRICK_COLS];
    int lives;
    Item items[20];
    int active_items;
    double paddle_width;
    int paddle_extend_time;
} GameState;

// 전역 게임 상태 변수
static GameState game_state;

// 함수 선언
static void init_game(void);
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data);
static void switch_to_main_menu(GtkWidget* widget, gpointer data);
static void spawn_item(double x, double y);
static void add_new_ball(void);
static gboolean update_game(gpointer data);

// 외부 함수 선언
extern void send_game_score(const char* username, const char* game, int score);
extern const char* g_username;

// 아이템 효과 적용 함수
static void apply_item_effect(ItemType type) {
    switch (type) {
        case ITEM_PADDLE_EXTEND:
            game_state.paddle_width = PADDLE_WIDTH * 1.5;
            game_state.paddle_extend_time = 600;
            break;

        case ITEM_MULTI_BALL:
            add_new_ball();
            break;

        case ITEM_EXTRA_LIFE:
            game_state.lives++;
            break;
    }
}

// update_game 함수 수정
static gboolean update_game(gpointer data) {
    if (!game_state.game_running) return TRUE;

    // 모든 공 업데이트
    for (int ball_idx = 0; ball_idx < 10; ball_idx++) {
        if (!game_state.balls[ball_idx].active) continue;

        Ball* ball = &game_state.balls[ball_idx];

        // 공 이동
        ball->x += ball->dx;
        ball->y += ball->dy;

        // 벽 충돌
        if (ball->x <= 0 || ball->x >= WINDOW_WIDTH) ball->dx = -ball->dx;
        if (ball->y <= 0) ball->dy = -ball->dy;

        // 패들 충돌
        if (ball->y >= WINDOW_HEIGHT - 20 &&
            ball->x >= game_state.paddle_x &&
            ball->x <= game_state.paddle_x + game_state.paddle_width) {
            ball->dy = -ball->dy;
        }

        // 바닥에 닿았을 때
        if (ball->y >= WINDOW_HEIGHT) {
            ball->active = false;
            game_state.active_balls--;

            // 모든 공이 없어졌을 때만 생명 감소
            if (game_state.active_balls == 0) {
                game_state.lives--;

                if (game_state.lives <= 0) {
                    game_state.game_running = FALSE;
                    send_game_score(g_username, "Breakout", game_state.score);
                } else {
                    // 새로운 공만 생성
                    game_state.balls[0].x = WINDOW_WIDTH / 2;
                    game_state.balls[0].y = WINDOW_HEIGHT - 50;

                    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
                    game_state.balls[0].dx = BALL_SPEED * cos(angle);
                    game_state.balls[0].dy = -BALL_SPEED * sin(angle);
                    game_state.balls[0].active = true;
                    game_state.active_balls = 1;
                }
            }
        }

        // 벽돌 충돌 검사
        double brick_start_x = (WINDOW_WIDTH - ((BRICK_WIDTH + BRICK_PADDING) * BRICK_COLS + BRICK_PADDING)) / 2;
        double brick_start_y = 50;

        bool collision_detected = false;

        for (int i = 0; i < BRICK_ROWS && !collision_detected; i++) {
            for (int j = 0; j < BRICK_COLS && !collision_detected; j++) {
                if (game_state.bricks[i][j] > 0) {
                    double brick_x = brick_start_x + j * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
                    double brick_y = brick_start_y + i * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING;

                    // 공의 중심과 벽돌 각 모서리 사이의 거리 계산
                    double closest_x = fmax(brick_x, fmin(ball->x, brick_x + BRICK_WIDTH));
                    double closest_y = fmax(brick_y, fmin(ball->y, brick_y + BRICK_HEIGHT));

                    // 공의 중심과 가장 가까운 벽돌 지점 사이의 거리
                    double distance_x = ball->x - closest_x;
                    double distance_y = ball->y - closest_y;
                    double distance = sqrt(distance_x * distance_x + distance_y * distance_y);

                    // 공의 반지름(BALL_SIZE/2)보다 거리가 작으면 충돌
                    if (distance < BALL_SIZE / 2) {
                        int original_durability = game_state.bricks[i][j];
                        game_state.bricks[i][j]--;

                        // 벽돌이 완전히 부서졌을 때
                        if (game_state.bricks[i][j] == 0) {
                            game_state.score += original_durability * 10;
                        } else {
                            game_state.score += 5;
                        }

                        // 모든 벽돌 타격 시 30% 확률로 아이템 생성 (파괴 및 타격 포함)
                        if (rand() % 100 < 30) {
                            spawn_item(brick_x + BRICK_WIDTH / 2, brick_y + BRICK_HEIGHT / 2);
                        }

                        // 충돌 방향에 따른 반사 처리
                        if (fabs(distance_x) > fabs(distance_y)) {
                            ball->dx = -ball->dx;  // 좌우 충돌
                        } else {
                            ball->dy = -ball->dy;  // 상하 충돌
                        }

                        collision_detected = true;
                        break;
                    }
                }
            }
        }
    }

    // 아이템 이동 및 충돌 검사
    for (int i = 0; i < 20; i++) {
        if (game_state.items[i].active) {
            game_state.items[i].y += ITEM_FALL_SPEED;

            // 패들과 충돌 검사
            if (game_state.items[i].y >= WINDOW_HEIGHT - 20 &&
                game_state.items[i].x >= game_state.paddle_x &&
                game_state.items[i].x <= game_state.paddle_x + game_state.paddle_width) {
                apply_item_effect(game_state.items[i].type);
                game_state.items[i].active = false;
                game_state.active_items--;
            }

            // 화면 밖으로 나갔는지 검사
            if (game_state.items[i].y >= WINDOW_HEIGHT) {
                game_state.items[i].active = false;
                game_state.active_items--;
            }
        }
    }

    // 점수 표시 업데이트
    GtkWidget* vbox = gtk_widget_get_parent(game_state.drawing_area);
    vbox = gtk_widget_get_parent(vbox);
    GList* children = gtk_container_get_children(GTK_CONTAINER(vbox));
    GtkWidget* score_label = GTK_WIDGET(children->data);

    char score_text[32];
    sprintf(score_text, "Score: %d    Lives: %d", game_state.score, game_state.lives);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    g_list_free(children);

    gtk_widget_queue_draw(game_state.drawing_area);
    return TRUE;
}

// 마우스 이동 리
static gboolean on_motion_notify(GtkWidget* widget, GdkEventMotion* event,
                                 gpointer data) {
    game_state.paddle_x = event->x - PADDLE_WIDTH / 2;

    // 패들이 화면 밖으로 나가지 않도록 제한
    if (game_state.paddle_x < 0) game_state.paddle_x = 0;
    if (game_state.paddle_x > WINDOW_WIDTH - PADDLE_WIDTH)
        game_state.paddle_x = WINDOW_WIDTH - PADDLE_WIDTH;

    return TRUE;
}

// 게임 화면 생성 함수
GtkWidget* create_breakout_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 점수와 생명 표시를 위한 레이블
    char initial_text[32];
    sprintf(initial_text, "Score: 0    Lives: 3");
    GtkWidget* score_label = gtk_label_new(initial_text);
    gtk_widget_set_name(score_label, "score_label");
    gtk_box_pack_start(GTK_BOX(vbox), score_label, FALSE, FALSE, 10);

    // 게임 영역을 중앙에 배치하기 위한 컨테이너
    GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), center_box, TRUE, TRUE, 0);

    // 게임 영역
    game_state.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(game_state.drawing_area, WINDOW_WIDTH, WINDOW_HEIGHT);
    gtk_box_pack_start(GTK_BOX(center_box), game_state.drawing_area, FALSE, FALSE, 0);

    // 컨트롤 버튼을 위한 수평 박스
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 15);

    // 메뉴로 돌아가기 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);  // stack을 직접 전달
    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);

    // 재시작 버튼
    GtkWidget* restart_button = gtk_button_new_with_label("Restart Game");
    g_signal_connect(restart_button, "clicked", G_CALLBACK(init_game), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), restart_button, FALSE, FALSE, 5);

    // 이벤트 연결
    g_signal_connect(game_state.drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_widget_add_events(game_state.drawing_area, GDK_POINTER_MOTION_MASK);
    g_signal_connect(game_state.drawing_area, "motion-notify-event",
                     G_CALLBACK(on_motion_notify), NULL);

    return vbox;
}

// 게임 시작 함수
void start_breakout_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "breakout_screen");

    // 게임 상태 초기화
    init_game();

    // 이전 타이머가 있다면 제거
    static guint timer_id = 0;
    if (timer_id > 0) {
        g_source_remove(timer_id);
    }

    // 새로운 타이머 시작
    timer_id = g_timeout_add(16, update_game, NULL);  // 약 60 FPS
}

// CSS 스타일 적용을 위한 함수 추가 (main.c에 추가)
void apply_css(void) {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    "#score_label {"
                                    "    font-size: 20px;"
                                    "    font-weight: bold;"
                                    "    color: #ffffff;"
                                    "    margin: 10px;"
                                    "}",
                                    -1, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(provider);
}

// spawn_item 함수 구현 추가
static void spawn_item(double x, double y) {
    if (game_state.active_items >= 20) return;
    if (rand() % 100 < 30) {  // 30% 확률로 아이템 생성
        for (int i = 0; i < 20; i++) {
            if (!game_state.items[i].active) {
                game_state.items[i].active = true;
                game_state.items[i].x = x;
                game_state.items[i].y = y;
                game_state.items[i].type = rand() % ITEM_TYPES_COUNT;
                game_state.active_items++;
                break;
            }
        }
    }
}

// switch_to_main_menu 함수 수정
static void switch_to_main_menu(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    game_state.game_running = false;                       // 게임 중지
    gtk_stack_set_visible_child_name(stack, "main_menu");  // "main_menu"로 수정
}

// init_game 함수 수정
static void init_game(void) {
    // 첫 번째 공 초기화
    game_state.balls[0].x = WINDOW_WIDTH / 2;
    game_state.balls[0].y = WINDOW_HEIGHT - 50;

    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
    game_state.balls[0].dx = BALL_SPEED * cos(angle);
    game_state.balls[0].dy = -BALL_SPEED * sin(angle);
    game_state.balls[0].active = true;
    game_state.active_balls = 1;

    game_state.paddle_x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;
    game_state.score = 0;
    game_state.game_running = true;
    game_state.lives = 3;

    game_state.paddle_width = PADDLE_WIDTH;
    game_state.active_items = 0;
    game_state.paddle_extend_time = 0;

    // 벽돌 초기화 수정 - 랜덤하게 다양한 내구도의 벽돌 배치
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            // 1~5 사이의 내구도를 가진 벽돌 생성
            int durability = rand() % 5 + 1;
            game_state.bricks[i][j] = durability;
        }
    }

    // 나머지 공들 비활성화
    for (int i = 1; i < 10; i++) {
        game_state.balls[i].active = false;
    }

    // 아이템 초기화
    for (int i = 0; i < 20; i++) {
        game_state.items[i].active = false;
    }
}

// 새로운 공 추가 함수
static void add_new_ball(void) {
    if (game_state.active_balls >= 10) return;

    for (int i = 0; i < 10; i++) {
        if (!game_state.balls[i].active) {
            // 현재 활성화된 첫 번째 공의 위치에서 새 공 생성
            Ball* first_ball = NULL;
            for (int j = 0; j < 10; j++) {
                if (game_state.balls[j].active) {
                    first_ball = &game_state.balls[j];
                    break;
                }
            }

            if (first_ball) {
                game_state.balls[i].x = first_ball->x;
                game_state.balls[i].y = first_ball->y;

                // 약간 다른 각도로 발사
                double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
                game_state.balls[i].dx = BALL_SPEED * cos(angle);
                game_state.balls[i].dy = -BALL_SPEED * sin(angle);
                game_state.balls[i].active = true;
                game_state.active_balls++;
                break;
            }
        }
    }
}

// on_draw 함수 수정
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 배경 그리기
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    // 패들 그리기
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, game_state.paddle_x, WINDOW_HEIGHT - 20,
                    game_state.paddle_width, PADDLE_HEIGHT);
    cairo_fill(cr);

    // 모든 활성화된 공 그리기
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for (int i = 0; i < 10; i++) {
        if (game_state.balls[i].active) {
            cairo_arc(cr, game_state.balls[i].x, game_state.balls[i].y,
                      BALL_SIZE / 2, 0, 2 * M_PI);
            cairo_fill(cr);
        }
    }

    // 벽돌 그리기 수정
    double brick_start_x = (WINDOW_WIDTH - TOTAL_BRICK_WIDTH) / 2;
    double brick_start_y = 50;

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game_state.bricks[i][j] > 0) {
                // 내구도에 따른 색상 설정
                switch (game_state.bricks[i][j]) {
                    case 1:                                       // 가장 약한 벽돌
                        cairo_set_source_rgb(cr, 0.4, 0.6, 1.0);  // 파스텔 블루
                        break;
                    case 2:
                        cairo_set_source_rgb(cr, 0.4, 1.0, 0.4);  // 파스텔 그린
                        break;
                    case 3:
                        cairo_set_source_rgb(cr, 1.0, 1.0, 0.4);  // 파스텔 옐로우
                        break;
                    case 4:
                        cairo_set_source_rgb(cr, 1.0, 0.6, 0.4);  // 파스텔 오렌지
                        break;
                    case 5:                                       // 가장 강한 벽돌
                        cairo_set_source_rgb(cr, 1.0, 0.4, 0.4);  // 파스텔 레드
                        break;
                }

                cairo_rectangle(cr,
                                brick_start_x + j * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING,
                                brick_start_y + i * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING,
                                BRICK_WIDTH, BRICK_HEIGHT);
                cairo_fill(cr);
            }
        }
    }

    // 아이템 그리기
    for (int i = 0; i < 20; i++) {
        if (game_state.items[i].active) {
            switch (game_state.items[i].type) {
                case ITEM_PADDLE_EXTEND:
                    cairo_set_source_rgb(cr, 1.0, 0.8, 0.0);  // 노란색
                    break;
                case ITEM_MULTI_BALL:
                    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);  // 초록색
                    break;
                case ITEM_EXTRA_LIFE:
                    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);  // 빨간색
                    break;
            }
            cairo_rectangle(cr,
                            game_state.items[i].x - 7.5, game_state.items[i].y - 7.5,
                            15, 15);  // 15x15 크기의 아이템
            cairo_fill(cr);
        }
    }

    return FALSE;
}
