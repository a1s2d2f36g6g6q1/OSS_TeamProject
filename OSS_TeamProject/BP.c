#pragma warning(disable : 4819)

#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// 함수 선언 추가 (파일 상단의 다른 함수 선언들과 함께)
static void restart_game(GtkWidget* widget, gpointer data);

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
    int level;  // 레벨 추가
} GameState;

// 전역 게임 태 변수
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

        // 벽 돌
        if (ball->x <= 0 || ball->x >= WINDOW_WIDTH) ball->dx = -ball->dx;
        if (ball->y <= 0) ball->dy = -ball->dy;

        // 패들 충돌 수정
        if (ball->y >= WINDOW_HEIGHT - 20 &&
            ball->x >= game_state.paddle_x &&
            ball->x <= game_state.paddle_x + game_state.paddle_width) {
            // 패들의 중앙을 기준으로 충돌 위치의 상대적 위치 계산 (-1.0 ~ 1.0)
            double relative_intersect_x = (ball->x - (game_state.paddle_x + game_state.paddle_width / 2)) / (game_state.paddle_width / 2);

            // 반사각 계산 (최대 75도)
            double max_angle = 75.0 * M_PI / 180.0;
            double bounce_angle = relative_intersect_x * max_angle;

            // 현재 속도 계산
            double current_speed = sqrt(ball->dx * ball->dx + ball->dy * ball->dy);

            // 새로운 방향 설정
            ball->dx = current_speed * sin(bounce_angle);
            ball->dy = -current_speed * cos(bounce_angle);
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
                    // 새로운 공 생성 - 현재 레벨에 맞는 속도로
                    game_state.balls[0].x = WINDOW_WIDTH / 2;
                    game_state.balls[0].y = WINDOW_HEIGHT - 50;

                    double speed_multiplier = 1.0 + (game_state.level - 1) * 0.2;
                    if (speed_multiplier > 2.0) speed_multiplier = 2.0;

                    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
                    game_state.balls[0].dx = BALL_SPEED * speed_multiplier * cos(angle);
                    game_state.balls[0].dy = -BALL_SPEED * speed_multiplier * sin(angle);
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

                    // 공의 중심과 가장 가까운 벽돌 점 사이의 거리
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

    // 모든 벽돌이 파괴되었는지 확인
    bool all_bricks_destroyed = true;
    for (int i = 0; i < BRICK_ROWS && all_bricks_destroyed; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game_state.bricks[i][j] > 0) {
                all_bricks_destroyed = false;
                break;
            }
        }
    }

    // 모든 벽돌이 파괴되면 다음 레벨로
    if (all_bricks_destroyed) {
        game_state.level++;
        init_game();  // 다음 레벨 초기화
    }

    // 점수 표시 업데이트
    GtkWidget* vbox = gtk_widget_get_parent(game_state.drawing_area);
    vbox = gtk_widget_get_parent(vbox);
    GList* children = gtk_container_get_children(GTK_CONTAINER(vbox));
    GtkWidget* score_label = GTK_WIDGET(children->data);

    char score_text[50];
    sprintf(score_text, "Score: %d    Lives: %d    Level: %d",
            game_state.score, game_state.lives, game_state.level);
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

// 키 이벤트 핸들러 함수 수정
static gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    if (event->keyval == GDK_KEY_k || event->keyval == GDK_KEY_K) {  // 대소문자 모두 처리
        if (game_state.game_running) {                               // 게임이 실행 중일 때만 처리
            game_state.level++;
            init_game();
            return TRUE;
        }
    }
    return FALSE;
}

// restart_game 함수 수정
static void restart_game(GtkWidget* widget, gpointer data) {
    game_state.level = 0;            // 레벨을 0으로 설정
    game_state.game_running = true;  // 게임 상태를 실행 중으로 설정
    init_game();

    // drawing_area에 다시 포커스 설정
    gtk_widget_grab_focus(game_state.drawing_area);
}

// create_breakout_screen 함수 (기존과 동일)
GtkWidget* create_breakout_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // 점수와 생명 표시를 위한 레이블
    char initial_text[50];
    sprintf(initial_text, "Score: 0    Lives: 3    Level: 1");
    GtkWidget* score_label = gtk_label_new(initial_text);
    gtk_widget_set_name(score_label, "score_label");
    gtk_widget_set_halign(score_label, GTK_ALIGN_CENTER);  // 중앙 정렬 추가
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
    g_signal_connect(restart_button, "clicked", G_CALLBACK(restart_game), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), restart_button, FALSE, FALSE, 5);

    // 이벤트 연결
    g_signal_connect(game_state.drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    gtk_widget_add_events(game_state.drawing_area,
                          GDK_POINTER_MOTION_MASK |
                              GDK_KEY_PRESS_MASK |
                              GDK_FOCUS_CHANGE_MASK);  // 포커스 변경 이벤트 추가
    g_signal_connect(game_state.drawing_area, "motion-notify-event",
                     G_CALLBACK(on_motion_notify), NULL);
    g_signal_connect(game_state.drawing_area, "key-press-event",
                     G_CALLBACK(on_key_press), NULL);

    // drawing_area가 키 이벤트를 받을 수 있도록 설정
    gtk_widget_set_can_focus(game_state.drawing_area, TRUE);
    gtk_widget_grab_focus(game_state.drawing_area);

    return vbox;
}

// 게임 시작 함수
void start_breakout_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "breakout_screen");

    // 게임 처음 시작할 때는 레벨을 0으로 설정하여 init_game에서 1로 초기화되도록 함
    game_state.level = 0;
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

    game_state.paddle_x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;

    // 게임이 처음 시작될 때만 점수와 레벨을 초기화
    if (game_state.level <= 0) {
        game_state.score = 0;
        game_state.level = 1;
    }

    game_state.game_running = true;
    game_state.lives = 3;
    game_state.paddle_width = PADDLE_WIDTH;
    game_state.active_items = 0;
    game_state.paddle_extend_time = 0;

    // 벽돌 초기화 수정 - 레벨에 따른 내구도 설정
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game_state.level == 1) {
                // 벨 1에서는 90%의 확률로 내구도 1, 10%의 확률로 내구도 2
                game_state.bricks[i][j] = (rand() % 100 < 90) ? 1 : 2;
            } else {
                // 레벨 2부터는 기존 로직 유지
                int max_durability = game_state.level + 1;  // 최대 내구도 조정
                if (max_durability > 5) max_durability = 5;

                int min_durability = (game_state.level) / 2;  // 최소 내구도 조정
                if (min_durability < 1) min_durability = 1;
                if (min_durability > 3) min_durability = 3;

                game_state.bricks[i][j] = min_durability + (rand() % (max_durability - min_durability + 1));
            }
        }
    }

    // 공 속도 설정 - 레벨에 따라 증가
    double speed_multiplier = 1.0 + (game_state.level - 1) * 0.2;  // 레벨당 20% 증가
    if (speed_multiplier > 2.0) speed_multiplier = 2.0;            // 최대 2배로 제한

    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
    game_state.balls[0].dx = BALL_SPEED * speed_multiplier * cos(angle);
    game_state.balls[0].dy = -BALL_SPEED * speed_multiplier * sin(angle);
    game_state.balls[0].active = true;
    game_state.active_balls = 1;

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

                // 현재 레벨에 맞는 속도 계산
                double speed_multiplier = 1.0 + (game_state.level - 1) * 0.2;
                if (speed_multiplier > 2.0) speed_multiplier = 2.0;

                // 약간 다른 각도로 발사
                double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
                game_state.balls[i].dx = BALL_SPEED * speed_multiplier * cos(angle);
                game_state.balls[i].dy = -BALL_SPEED * speed_multiplier * sin(angle);
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
                // 벽돌 위치 계산
                double brick_x = brick_start_x + j * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
                double brick_y = brick_start_y + i * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING;

                // 내구도에 따른 색상 설정
                switch (game_state.bricks[i][j]) {
                    case 1:
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
                    case 5:
                        cairo_set_source_rgb(cr, 1.0, 0.4, 0.4);  // 파스텔 레드
                        break;
                }

                // 벽돌 그리기
                cairo_rectangle(cr, brick_x, brick_y, BRICK_WIDTH, BRICK_HEIGHT);
                cairo_fill(cr);

                // 내구도 텍스트 표시
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);  // 검정색
                cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
                cairo_set_font_size(cr, 20.0);

                // 텍스트 중앙 정렬을 위한 계산
                char durability_text[2];
                sprintf(durability_text, "%d", game_state.bricks[i][j]);

                cairo_text_extents_t extents;
                cairo_text_extents(cr, durability_text, &extents);

                double text_x = brick_x + (BRICK_WIDTH - extents.width) / 2;
                double text_y = brick_y + (BRICK_HEIGHT + extents.height) / 2;

                cairo_move_to(cr, text_x, text_y);
                cairo_show_text(cr, durability_text);
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
