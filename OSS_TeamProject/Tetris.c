#include <gtk/gtk.h>
#include <cairo.h>
#include <stdlib.h>
#include <time.h>

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

// 게임판과 현재 도형 정보
int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 게임판 데이터
int current_tetromino[4][4];            // 현재 도형
int current_x = GRID_WIDTH / 2 - 2;     // 도형의 X 위치
int current_y = 0;                      // 도형의 Y 위치

// 테트리스 도형 정의
int tetrominos[7][4][4] = {
    // I 블록
    {
        {0, 0, 0, 0},
        {1, 1, 1, 1},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // O 블록
    {
        {0, 1, 1, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // T 블록
    {
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // S 블록
    {
        {0, 1, 1, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // Z 블록
    {
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // L 블록
    {
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    },
    // J 블록
    {
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    }
};

// 새로운 도형 생성
void spawn_new_tetromino() {
    int random_index = rand() % 7; // 0~6 사이의 랜덤 값
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            current_tetromino[y][x] = tetrominos[random_index][y][x];
        }
    }
    current_x = GRID_WIDTH / 2 - 2;
    current_y = 0;
}

// 게임판을 그리는 함수
gboolean draw_game_board(GtkWidget *widget, cairo_t *cr, gpointer data) {
    double cell_size = 30;

    // 게임판 그리기
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == 0) {
                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); // 빈 칸
            } else {
                cairo_set_source_rgb(cr, 0.3, 0.3, 0.3); // 고정된 블록
            }

            cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
            cairo_fill(cr);
        }
    }

    // 현재 도형 그리기
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (current_tetromino[y][x] == 1) {
                cairo_set_source_rgb(cr, 0.2, 0.6, 0.8); // 현재 블록 색상
                cairo_rectangle(cr, (current_x + x) * cell_size, (current_y + y) * cell_size, cell_size - 1, cell_size - 1);
                cairo_fill(cr);
            }
        }
    }

    return FALSE;
}

// 게임 루프
gboolean game_loop(GtkWidget *widget) {
    current_y++;

    // 충돌 검사
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (current_tetromino[y][x] == 1) {
                int new_y = current_y + y;
                int new_x = current_x + x;

                if (new_y >= GRID_HEIGHT || (new_y >= 0 && grid[new_y][new_x] != 0)) {
                    current_y--; // 충돌 시 원래 위치로 되돌림

                    // 현재 도형을 게임판에 고정
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            if (current_tetromino[i][j] == 1) {
                                grid[current_y + i][current_x + j] = 1;
                            }
                        }
                    }

                    // 새로운 도형 생성
                    spawn_new_tetromino();
                    return TRUE;
                }
            }
        }
    }

    gtk_widget_queue_draw(widget);
    return TRUE;
}

// 키 입력 처리
gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    switch (event->keyval) {
    case GDK_KEY_Left: // 왼쪽 이동
        current_x = (current_x > 0) ? current_x - 1 : current_x;
        break;
    case GDK_KEY_Right: // 오른쪽 이동
        current_x = (current_x < GRID_WIDTH - 4) ? current_x + 1 : current_x;
        break;
    case GDK_KEY_Down: // 아래로 빠르게
        current_y++;
        break;
    }
    gtk_widget_queue_draw(widget);
    return TRUE;
}

// 테트리스 게임 시작
void start_tetris_game() {
    GtkWidget *window;
    GtkWidget *drawing_area;

    gtk_init(NULL, NULL);
    srand(time(NULL)); // 랜덤 시드 설정

    spawn_new_tetromino(); // 첫 번째 도형 생성

    // 창 생성
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tetris Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // 게임판 위젯
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 600);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_game_board), NULL);

    // 키보드 이벤트 연결
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // 게임 루프 타이머 설정
    g_timeout_add(500, (GSourceFunc)game_loop, drawing_area);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    gtk_widget_show_all(window);

    gtk_main();
}