#include <gtk/gtk.h>

/*
#include "Brickbreaker.h"

// 메인 함수
int main(int argc, char *argv[]) {
    // GTK UI 초기화
    GtkWidget *window;
    GtkWidget *drawing_area;

    // GTK 라이브러리 초기화
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);                                  // 창 생성
    gtk_window_set_title(GTK_WINDOW(window), "벽돌깨기 게임");                     // 창 제목 설정
    gtk_window_set_default_size(GTK_WINDOW(window), WINDOW_WIDTH, WINDOW_HEIGHT);  // 창 크기 설정
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);          // 창 닫기 버튼을 누르면 프로그램 종료

    drawing_area = gtk_drawing_area_new();                                    // 그리기 영역 생성
    gtk_container_add(GTK_CONTAINER(window), drawing_area);                   // 창에 그리기 영역 추가
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw_event), NULL);  // 그리기 이벤트 처리

    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);  // 키보드 입력 이벤트 처리

    init_game();  // 게임 초기화

    // 타이머 설정 (게임 루프)
    g_timeout_add(16, on_timeout, drawing_area);  // 약 60 FPS

    gtk_widget_show_all(window);  // 모든 위젯을 표시
    gtk_main();                   // GTK 메인 루프 시작

    return 0;
}

// 게임 초기화 함수
void init_game() {
    // 공 초기화
    ball.x = WINDOW_WIDTH / 2;
    ball.y = WINDOW_HEIGHT - 100;
    ball.dx = 6;
    ball.dy = -6;

    // 패들 초기화
    paddle.x = (WINDOW_WIDTH - PADDLE_WIDTH) / 2;
    paddle.y = WINDOW_HEIGHT - PADDLE_HEIGHT - 20;
    paddle.dx = 0;

    // 벽돌 초기화
    for (int i = 0; i < NUM_BRICKS; i++) {
        bricks[i].x = (i % 8) * (BRICK_WIDTH + 10) + 50;  // x 위치 계산
        bricks[i].y = (i / 8) * (BRICK_HEIGHT + 5) + 50;  // y 위치 계산
        bricks[i].destroyed = FALSE;                      // 벽돌 파괴 여부 초기화
    }
}

// 게임 시작 화면 그리기
void draw_start_screen(cairo_t *cr) {
    cairo_set_source_rgb(cr, 0, 0, 0);  // 배경색 검정
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 1, 1, 1);  // 흰색 텍스트
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 50);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2);
    cairo_show_text(cr, "Brick-out Game");

    cairo_set_font_size(cr, 30);
    cairo_move_to(cr, WINDOW_WIDTH / 2 - 145, WINDOW_HEIGHT / 2 + 60);
    cairo_show_text(cr, "Press 'Enter' to restart");
}

// 게임 화면을 그리는 함수
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer data) {
    if (!game_started) {
        draw_start_screen(cr);  // 게임 시작 전 화면 그리기
    } else {
        cairo_set_source_rgb(cr, 0, 0, 0);  // 배경색 검정
        cairo_paint(cr);

        draw_ball(cr);    // 공 그리기
        draw_paddle(cr);  // 패들 그리기
        draw_bricks(cr);  // 벽돌 그리기

        // 게임 오버 메시지
        if (game_over) {
            cairo_set_source_rgb(cr, 1, 0, 0);  // 빨간색 텍스트
            cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 50);
            cairo_move_to(cr, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2);
            cairo_show_text(cr, "GAME OVER!");

            // 게임 재시작 메시지
            cairo_set_font_size(cr, 30);
            cairo_move_to(cr, WINDOW_WIDTH / 2 - 125, WINDOW_HEIGHT / 2 + 60);
            cairo_show_text(cr, "Press 'R' to restart");
        }
    }
    return FALSE;
}

// 공 그리기 함수
void draw_ball(cairo_t *cr) {
    cairo_set_source_rgb(cr, 1, 0, 0);  // 공 색 빨간색
    cairo_arc(cr, ball.x, ball.y, BALL_RADIUS, 0, 2 * G_PI);
    cairo_fill(cr);
}

// 패들 그리기 함수
void draw_paddle(cairo_t *cr) {
    cairo_set_source_rgb(cr, 0, 0, 1);  // 패들 색 파란색
    cairo_rectangle(cr, paddle.x, paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT);
    cairo_fill(cr);
}

// 벽돌 그리기 함수
void draw_bricks(cairo_t *cr) {
    cairo_set_source_rgb(cr, 0, 1, 0);  // 벽돌 색 초록색
    for (int i = 0; i < NUM_BRICKS; i++) {
        if (!bricks[i].destroyed) {
            cairo_rectangle(cr, bricks[i].x, bricks[i].y, BRICK_WIDTH, BRICK_HEIGHT);
            cairo_fill(cr);
        }
    }
}

// 게임 루프 (타이머 설정)
static gboolean on_timeout(gpointer data) {
    if (!paused) {
        move_ball();                              // 공 이동
        move_paddle();                            // 패들 이동
        check_collisions();                       // 충돌 검사
        gtk_widget_queue_draw(GTK_WIDGET(data));  // 화면 업데이트
    }
    return TRUE;
}

// 공 이동 함수
void move_ball() {
    ball.x += ball.dx;
    ball.y += ball.dy;

    // 벽에 부딪히면 반사
    if (ball.x - BALL_RADIUS < 0 || ball.x + BALL_RADIUS > WINDOW_WIDTH) {
        ball.dx = -ball.dx;
    }

    // 위쪽 벽에 부딪히면 반사
    if (ball.y - BALL_RADIUS < 0) {
        ball.dy = -ball.dy;
    }

    // 바닥에 떨어지면 공 초기화
    if (ball.y + BALL_RADIUS > WINDOW_HEIGHT) {
        reset_ball();
    }
}

// 패들 이동 함수
void move_paddle() {
    paddle.x += paddle.dx;

    // 패들이 화면 밖으로 나가지 않게 제한
    if (paddle.x < 0) paddle.x = 0;
    if (paddle.x + PADDLE_WIDTH > WINDOW_WIDTH) paddle.x = WINDOW_WIDTH - PADDLE_WIDTH;
}

// 충돌 검사 함수
void check_collisions() {
    // 공과 패들 충돌 검사
    if (ball.y + BALL_RADIUS > paddle.y && ball.y - BALL_RADIUS < paddle.y + PADDLE_HEIGHT &&
        ball.x > paddle.x && ball.x < paddle.x + PADDLE_WIDTH) {
        ball.dy = -ball.dy;
    }

    // 공과 벽돌 충돌 검사
    for (int i = 0; i < NUM_BRICKS; i++) {
        if (!bricks[i].destroyed) {
            if (ball.x > bricks[i].x && ball.x < bricks[i].x + BRICK_WIDTH &&
                ball.y - BALL_RADIUS < bricks[i].y + BRICK_HEIGHT && ball.y + BALL_RADIUS > bricks[i].y) {
                bricks[i].destroyed = TRUE;  // 벽돌 파괴
                ball.dy = -ball.dy;          // 공 반사
            }
        }
    }
}

// 공 초기화 함수
void reset_ball() {
    game_over = TRUE;  // 게임 오버 상태로 변경
    paused = TRUE;     // 게임 일시 정지 상태로 변경
}

// 키보드 입력 이벤트 처리 함수
static void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    // 키보드 입력 처리 (왼쪽, 오른쪽 방향키)
    if (event->keyval == GDK_KEY_Left) {
        paddle.dx = -7;  // 왼쪽으로 이동
    } else if (event->keyval == GDK_KEY_Right) {
        paddle.dx = 7;  // 오른쪽으로 이동
    }

    // Enter 키로 게임 시작
    if (!game_started && event->keyval == GDK_KEY_Return) {
        game_started = TRUE;  // 게임 시작
        init_game();          // 게임 초기화
    }

    // 게임 오버 상태에서 R키를 누르면 게임 재시작
    if (game_over && event->keyval == GDK_KEY_R || event->keyval == GDK_KEY_r) {
        game_over = FALSE;  // 게임 오버 상태 해제
        paused = FALSE;     // 게임 일시 정지 상태 해제
        init_game();        // 게임 초기화
    }
}
*/


void start_breakout_game_BP() {
    GtkWidget *window;
    gtk_init(NULL, NULL);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tetris Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}