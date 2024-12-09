#pragma warning(disable : 4819)

#define _USE_MATH_DEFINES        // M_PI를 사용하기 위해 필요
#define _CRT_SECURE_NO_WARNINGS  // sprintf 경고 제거

#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>  // bool 타입을 위해 추가
#include <stdlib.h>
#include <time.h>

// g_username 외부 선언 추가
extern char* g_username;

// switch_to_main_menu 함수 선언 추가
extern void switch_to_main_menu(GtkWidget* widget, gpointer data);

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

// 전체 벽돌 영역의 너비와 높이 계산
#define TOTAL_BRICK_WIDTH ((BRICK_WIDTH + BRICK_PADDING) * BRICK_COLS + BRICK_PADDING)
#define TOTAL_BRICK_HEIGHT ((BRICK_HEIGHT + BRICK_PADDING) * BRICK_ROWS + BRICK_PADDING)

// 아이템 타입 정의
typedef enum {
    ITEM_PADDLE_EXTEND,  // 패들 크기 증가
    ITEM_SLOW_BALL,      // 공 속도 감소
    ITEM_EXTRA_LIFE,     // 추가 생명
    ITEM_TYPES_COUNT     // 아이템 종류 수
} ItemType;

// 아이템 구조체
typedef struct {
    double x, y;
    ItemType type;
    bool active;
} Item;

// 게임 상태 구조체
typedef struct {
    double x, y;    // 공의 위치
    double dx, dy;  // 공의 이동 방향
    GtkWidget* drawing_area;
    double paddle_x;                     // 패들 위치
    int score;                           // 점수
    bool game_running;                   // 게임 상태
    int bricks[BRICK_ROWS][BRICK_COLS];  // 벽돌 상태 배열
    int lives;                           // 생명 추가
    Item items[20];                      // 최대 20개의 아이템
    int active_items;                    // 현재 활성화된 아이템 수
    double paddle_width;                 // 현재 패들 너비
    double ball_speed;                   // 현재 공 속도
    int paddle_extend_time;              // 패들 확장 지속 시간
    int slow_ball_time;                  // 공 감속 지속 시간
} GameState;

// 전역 게임 상태 변수
static GameState game_state;

// 게임 초기화 함수
static void init_game() {
    game_state.x = WINDOW_WIDTH / 2;
    game_state.y = WINDOW_HEIGHT - 50;

    // 초기 방향을 위쪽으로 고정하고, 약간의 랜덤성 추가
    double angle = M_PI / 2 + (rand() % 40 - 20) * M_PI / 180.0;  // ±20도
    game_state.dx = BALL_SPEED * cos(angle);
    game_state.dy = -BALL_SPEED * sin(angle);

    game_state.paddle_x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;
    game_state.score = 0;
    game_state.game_running = TRUE;
    game_state.lives = 3;  // 초기 생명 3개

    // 벽돌 초기화
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            game_state.bricks[i][j] = 1;
        }
    }

    game_state.paddle_width = PADDLE_WIDTH;
    game_state.ball_speed = BALL_SPEED;
    game_state.active_items = 0;
    game_state.paddle_extend_time = 0;
    game_state.slow_ball_time = 0;

    // 아이템 초기화
    for (int i = 0; i < 20; i++) {
        game_state.items[i].active = false;
    }
}

// 아이템 생성 함수
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

// 아이템 효과 적용 함수
static void apply_item_effect(ItemType type) {
    switch (type) {
        case ITEM_PADDLE_EXTEND:
            game_state.paddle_width = PADDLE_WIDTH * 1.5;
            game_state.paddle_extend_time = 600;  // 약 10초
            break;
        case ITEM_SLOW_BALL:
            game_state.ball_speed = BALL_SPEED * 0.7;
            game_state.slow_ball_time = 600;  // 약 10초
            break;
        case ITEM_EXTRA_LIFE:
            game_state.lives++;
            break;
    }
}

// 그리기 함수
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    // 배경 그리기
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    // 패들 그리기
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_rectangle(cr, game_state.paddle_x, WINDOW_HEIGHT - 20,
                    PADDLE_WIDTH, PADDLE_HEIGHT);
    cairo_fill(cr);

    // 공 그리기
    cairo_arc(cr, game_state.x, game_state.y, BALL_SIZE / 2, 0, 2 * M_PI);
    cairo_fill(cr);

    // 벽돌 그리기 - 단일 색상으로 통일
    double brick_start_x = (WINDOW_WIDTH - TOTAL_BRICK_WIDTH) / 2;
    double brick_start_y = 50;

    cairo_set_source_rgb(cr, 0.4, 0.6, 1.0);  // 파스텔 블루 색상으로 통일
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
            cairo_rectangle(cr, game_state.items[i].x, game_state.items[i].y, 15, 15);
            cairo_fill(cr);
        }
    }

    return FALSE;
}

// 충돌 감지 및 게임 업데이트
static gboolean update_game(gpointer data) {
    if (!game_state.game_running) return TRUE;

    // 아이템 효과 시간 감소
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
            game_state.items[i].y += 2;  // 아이템 낙하 속도

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
        // 패들의 어느 부분에 맞았는지에 따라 반사 각도 조정
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

                    // 충돌 후 즉시 루프 탈출
                    goto collision_handled;
                }
            }
        }
    }
collision_handled:

    // 게임 오버 체크
    if (game_state.y >= WINDOW_HEIGHT) {
        game_state.lives--;  // 생명 감소

        if (game_state.lives <= 0) {
            // 게임 오버
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

    // 점수와 생명 레이블 업데이트
    GtkWidget* info_box = gtk_widget_get_parent(game_state.drawing_area);
    info_box = gtk_widget_get_parent(info_box);
    info_box = gtk_container_get_children(GTK_CONTAINER(info_box))->data;

    GList* children = gtk_container_get_children(GTK_CONTAINER(info_box));
    GtkWidget* score_label = children->data;
    GtkWidget* lives_label = children->next->data;

    char score_text[32], lives_text[32];
    sprintf(score_text, "Score: %d", game_state.score);
    sprintf(lives_text, "Lives: %d", game_state.lives);

    gtk_label_set_text(GTK_LABEL(score_label), score_text);
    gtk_label_set_text(GTK_LABEL(lives_label), lives_text);

    g_list_free(children);

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

    // 상단 정보 표시를 위한 수평 박스
    GtkWidget* info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(info_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), info_box, FALSE, FALSE, 10);

    // 점수 표시 레이블
    GtkWidget* score_label = gtk_label_new("");
    gtk_widget_set_name(score_label, "score_label");
    gtk_box_pack_start(GTK_BOX(info_box), score_label, FALSE, FALSE, 0);

    // 생명 표시 레이블
    GtkWidget* lives_label = gtk_label_new("");
    gtk_widget_set_name(lives_label, "lives_label");
    gtk_box_pack_start(GTK_BOX(info_box), lives_label, FALSE, FALSE, 0);

    // 게임 영역을 중앙에 배치하기 위한 컨테이너
    GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), center_box, TRUE, TRUE, 0);

    // 게임 영역
    game_state.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(game_state.drawing_area, WINDOW_WIDTH, WINDOW_HEIGHT);
    gtk_box_pack_start(GTK_BOX(center_box), game_state.drawing_area, FALSE, FALSE, 0);

    // 컨트롤 버튼을 위한 수평 박스 (하단에 배치, 여백 조정)
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 15);  // 상단 여백을 15로 조정

    // 메뉴로 돌아가기 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);
    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);

    // 재시작 버튼 추가
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
