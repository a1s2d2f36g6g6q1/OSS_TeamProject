#pragma warning(disable : 4819)

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
    ITEM_SLOW_BALL,
    ITEM_EXTRA_LIFE,
    ITEM_TYPES_COUNT
} ItemType;

// 아이템 구조체
typedef struct {
    double x, y;
    ItemType type;
    bool active;
} Item;

// 게임 상태 구조체
typedef struct {
    double x, y;
    double dx, dy;
    GtkWidget* drawing_area;
    double paddle_x;
    int score;
    bool game_running;
    int bricks[BRICK_ROWS][BRICK_COLS];
    int lives;
    Item items[20];
    int active_items;
    double paddle_width;
    double ball_speed;
    int paddle_extend_time;
    int slow_ball_time;
} GameState;

// 전역 게임 상태 변수
static GameState game_state;

// 함수 선언
static void init_game(void);
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data);
static void switch_to_main_menu(GtkWidget* widget, gpointer data);
static void spawn_item(double x, double y);

// 외부 함수 선언
extern void send_game_score(const char* username, const char* game, int score);
extern const char* g_username;  // 전역 사용자 이름 변수

// 아이템 효과 적용 함수
static void apply_item_effect(ItemType type) {
    switch (type) {
        case ITEM_PADDLE_EXTEND:
            game_state.paddle_width = PADDLE_WIDTH * 1.5;
            game_state.paddle_extend_time = 600;
            break;

        case ITEM_SLOW_BALL:
            game_state.ball_speed = BALL_SPEED * 0.7;
            game_state.slow_ball_time = 600;
            break;

        case ITEM_EXTRA_LIFE:
            game_state.lives++;
            break;
    }
}

// update_game 함수의 아이템 이동 부분
static gboolean update_game(gpointer data) {
    if (!game_state.game_running) return TRUE;

    // 아이템 ��과 시간 감소
    if (game_state.paddle_extend_time > 0) {
        game_state.paddle_extend_time--;
        if (game_state.paddle_extend_time == 0) {
            game_state.paddle_width = PADDLE_WIDTH;
        }
    }

    if (game_state.slow_ball_time > 0) {
        game_state.slow_ball_time--;
        if (game_state.slow_ball_time == 0) {
            game_state.ball_speed = BALL_SPEED;
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
                // 아이템 획득 시 시각적 효과
                cairo_t* cr = gdk_cairo_create(gtk_widget_get_window(game_state.drawing_area));
                switch (game_state.items[i].type) {
                    case ITEM_PADDLE_EXTEND:
                        cairo_set_source_rgba(cr, 1.0, 0.8, 0.0, 0.5);  // 반투명 노란색
                        break;
                    case ITEM_SLOW_BALL:
                        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.5);  // 반투명 ��록색
                        break;
                    case ITEM_EXTRA_LIFE:
                        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);  // 반투명 빨간색
                        break;
                }
                cairo_arc(cr, game_state.items[i].x, game_state.items[i].y, 20, 0, 2 * M_PI);
                cairo_fill(cr);
                cairo_destroy(cr);

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

    // 공 이동
    game_state.x += game_state.dx;
    game_state.y += game_state.dy;

    // 벽 충돌 처리
    if (game_state.x <= 0 || game_state.x >= WINDOW_WIDTH) {
        game_state.dx = -game_state.dx;
    }
    if (game_state.y <= 0) {
        game_state.dy = -game_state.dy;
    }

    // 패들 충돌 처리
    if (game_state.y >= WINDOW_HEIGHT - 20 - BALL_SIZE / 2 &&
        game_state.x >= game_state.paddle_x &&
        game_state.x <= game_state.paddle_x + PADDLE_WIDTH) {
        // 패들의 어느 부분에 맞았는��에 따라 반사 각도 조정
        double relative_intersect_x = (game_state.paddle_x + (PADDLE_WIDTH / 2)) - game_state.x;
        double normalized_intersect = relative_intersect_x / (PADDLE_WIDTH / 2);
        double bounce_angle = normalized_intersect * M_PI / 3;  // 최대 60도

        game_state.dy = -BALL_SPEED * cos(bounce_angle);
        game_state.dx = BALL_SPEED * sin(bounce_angle);

        // 공이 패들 안으로 파고들지 않도록 위치 조정
        game_state.y = WINDOW_HEIGHT - 20 - BALL_SIZE / 2;
    }

    // 벽돌 충돌 처리
    double brick_start_x = (WINDOW_WIDTH - TOTAL_BRICK_WIDTH) / 2;
    double brick_start_y = 50;

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game_state.bricks[i][j] == 1) {
                double brick_x = brick_start_x + j * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
                double brick_y = brick_start_y + i * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING;

                // 공의 중심점
                double ball_center_x = game_state.x;
                double ball_center_y = game_state.y;

                // 벽돌의 각 면과의 충돌 검사
                if (ball_center_x >= brick_x - BALL_SIZE / 2 &&
                    ball_center_x <= brick_x + BRICK_WIDTH + BALL_SIZE / 2 &&
                    ball_center_y >= brick_y - BALL_SIZE / 2 &&
                    ball_center_y <= brick_y + BRICK_HEIGHT + BALL_SIZE / 2) {
                    // 충돌이 발생한 면 확인
                    double overlap_left = (brick_x + BRICK_WIDTH) - (ball_center_x - BALL_SIZE / 2);
                    double overlap_right = (ball_center_x + BALL_SIZE / 2) - brick_x;
                    double overlap_top = (brick_y + BRICK_HEIGHT) - (ball_center_y - BALL_SIZE / 2);
                    double overlap_bottom = (ball_center_y + BALL_SIZE / 2) - brick_y;

                    // 가장 작은 겹침을 찾아 해당 방향으로 반사
                    double min_overlap = fmin(fmin(overlap_left, overlap_right),
                                              fmin(overlap_top, overlap_bottom));

                    if (min_overlap == overlap_left || min_overlap == overlap_right) {
                        game_state.dx = -game_state.dx;
                    } else {
                        game_state.dy = -game_state.dy;
                    }

                    // 벽돌 제거 및 점수 추가
                    game_state.bricks[i][j] = 0;
                    game_state.score += 10;
                    spawn_item(brick_x + BRICK_WIDTH / 2, brick_y + BRICK_HEIGHT / 2);

                    // 충돌 후 즉시 루�� 탈출
                    goto collision_handled;
                }
            }
        }
    }
collision_handled:

    // 점수 레이블 업데이트 부분 수정
    GtkWidget* vbox = gtk_widget_get_parent(game_state.drawing_area);
    vbox = gtk_widget_get_parent(vbox);
    GList* children = gtk_container_get_children(GTK_CONTAINER(vbox));
    GtkWidget* score_label = GTK_WIDGET(children->data);

    char score_text[32];
    sprintf(score_text, "Score: %d    Lives: %d", game_state.score, game_state.lives);
    gtk_label_set_text(GTK_LABEL(score_label), score_text);

    g_list_free(children);

    // 게임 오버 체크
    if (game_state.y >= WINDOW_HEIGHT) {
        game_state.lives--;

        if (game_state.lives <= 0) {
            game_state.game_running = FALSE;
            extern void send_game_score(const char* username, const char* game, int score);
            send_game_score(g_username, "Breakout", game_state.score);
        } else {
            // 공 위치 초기화
            game_state.x = WINDOW_WIDTH / 2;
            game_state.y = WINDOW_HEIGHT - 50;

            // 초기 방향 재설정
            double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
            game_state.dx = BALL_SPEED * cos(angle);
            game_state.dy = -BALL_SPEED * sin(angle);
        }
    }

    gtk_widget_queue_draw(game_state.drawing_area);
    return TRUE;
}

// 마우스 이동 처리
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

// init_game 함수 구현 (이전에 있던 코드를 여기로 이동)
static void init_game(void) {
    game_state.x = WINDOW_WIDTH / 2;
    game_state.y = WINDOW_HEIGHT - 50;

    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;
    game_state.dx = BALL_SPEED * cos(angle);
    game_state.dy = -BALL_SPEED * sin(angle);

    game_state.paddle_x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;
    game_state.score = 0;
    game_state.game_running = true;
    game_state.lives = 3;

    game_state.paddle_width = PADDLE_WIDTH;
    game_state.ball_speed = BALL_SPEED;
    game_state.active_items = 0;
    game_state.paddle_extend_time = 0;
    game_state.slow_ball_time = 0;

    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            game_state.bricks[i][j] = 1;
        }
    }

    for (int i = 0; i < 20; i++) {
        game_state.items[i].active = false;
    }
}

// on_draw 함수 구현 추가
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 배경 그리기
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    // 패들 그리기
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, game_state.paddle_x, WINDOW_HEIGHT - 20,
                    game_state.paddle_width, PADDLE_HEIGHT);
    cairo_fill(cr);

    // 공 그리기
    cairo_arc(cr, game_state.x, game_state.y, BALL_SIZE / 2, 0, 2 * M_PI);
    cairo_fill(cr);

    // 벽돌 그리기
    double brick_start_x = (WINDOW_WIDTH - TOTAL_BRICK_WIDTH) / 2;
    double brick_start_y = 50;

    cairo_set_source_rgb(cr, 0.4, 0.6, 1.0);  // 파스텔 블루 색상
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (game_state.bricks[i][j] == 1) {
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
                case ITEM_SLOW_BALL:
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
