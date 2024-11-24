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

// 메인 함수
void start_breakout_game_BP()
{
    // GTK UI 초기화
    GtkWidget *window;
    GtkWidget *drawing_area;

    // GTK 라이브러리 초기화
    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);                                 // 창 생성
    gtk_window_set_title(GTK_WINDOW(window), "Breakout Game");                    // 창 제목 설정
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT); // 창 크기 설정
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);         // 창 닫기 버튼을 누르면 프로그램 종료

    drawing_area = gtk_drawing_area_new();                                   // 그리기 영역 생성
    gtk_container_add(GTK_CONTAINER(window), drawing_area);                  // 창에 그리기 영역 추가
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw_event), NULL); // 그리기 이벤트 처리

    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL); // 키보드 입력 이벤트 처리

    init_game(); // 게임 초기화

    // TimerData 구조체 초기화
    TimerData *timer_data = g_new(TimerData, 1);
    timer_data->drawing_area = drawing_area;
    timer_data->valid = TRUE;

    // 타이머 설정 (게임 루프)
    g_timeout_add_full(G_PRIORITY_DEFAULT, 16, on_timeout, timer_data, (GDestroyNotify)g_free);

    gtk_widget_show_all(window); // 모든 위젯을 표시
    gtk_main();                  // GTK 메인 루프 시작

    return 0;
}

// 게임 초기화 함수
void init_game()
{
    // 공 초기화
    ball.x = WINDOW_WIDTH / 2;
    ball.y = WINDOW_HEIGHT - 100;
    ball.dx = 6 * speed_multiplier;
    ball.dy = -6 * speed_multiplier;

    // 패들 초기화
    paddle.x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;
    paddle.y = WINDOW_HEIGHT - PADDLE_HEIGHT - 20;
    paddle.dx = 0;

    // 벽돌 초기화
    for (int i = 0; i < NUM_BRICKS; i++)
    {
        bricks[i].x = (i % 8) * (BRICK_WIDTH + 10) + 50;
        bricks[i].y = (i / 8) * (BRICK_HEIGHT + 5) + 50;
        bricks[i].destroyed = FALSE;

        // 랜덤하게 강화 벽돌 생성 (20% 확률)
        if (rand() % 5 == 0)
        {
            bricks[i].durability = 2 + rand() % 2;        // 2-3의 내구도
            bricks[i].points = 20 * bricks[i].durability; // 내구도에 따른 점수
        }
        else
        {
            bricks[i].durability = 1; // 일반 벽돌
            bricks[i].points = 10;    // 기본 점수
        }
    }

    // 게임 시작 시간 초기화
    game_start_time = time(NULL);
    speed_multiplier = 1.0;
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

static gboolean game_over = FALSE; // 게임 오버 상태

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

        // 게임 상태 표시
        draw_game_status(cr);

        // 게임 요소 그리기
        draw_ball(cr);
        draw_paddle(cr);
        draw_bricks(cr);

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
    cairo_rectangle(cr, paddle.x, paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
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
    if (paddle.x + PADDLE_WIDTH > WINDOW_WIDTH)
        paddle.x = WINDOW_WIDTH - PADDLE_WIDTH;
}

// 충돌 검사 함수
void check_collisions()
{
    // 공과 패들 충돌 검사
    if (ball.y + BALL_RADIUS > paddle.y &&
        ball.y - BALL_RADIUS < paddle.y + PADDLE_HEIGHT &&
        ball.x > paddle.x &&
        ball.x < paddle.x + PADDLE_WIDTH)
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
                }

                ball.dy = -ball.dy; // 공 반사

                // 벽돌 파괴 시 속도 약간 증가 (추가 난이도)
                speed_multiplier += 0.05;
                break; // 한 번에 하나의 벽돌만 처리
            }
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

    // 점수 표시
    snprintf(score_text, sizeof(score_text), "Score: %d", score); // 한글 제거
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 10, 30);
    cairo_show_text(cr, score_text);

    // 생명 표시
    snprintf(lives_text, sizeof(lives_text), "Lives: %d", lives); // 한글 제거
    cairo_move_to(cr, WINDOW_WIDTH - 100, 30);
    cairo_show_text(cr, lives_text);
}

// 게임 오버 화면 그리기 함수 추가
void draw_game_over(cairo_t *cr)
{
    cairo_set_source_rgb(cr, 1, 0, 0); // 빨간색 텍스트
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 50);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2);
    cairo_show_text(cr, "GAME OVER!");

    // 게임 재시작 메시지
    cairo_set_font_size(cr, 30);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 125, WINDOW_HEIGHT / 2 + 60);
    cairo_show_text(cr, "Press 'R' to restart");
}
