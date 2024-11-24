#include <gtk/gtk.h>
#include <cairo.h>

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 10x20 게임판 데이터
int current_x = GRID_WIDTH / 2 - 1; // 블록 시작 위치 (가로 중앙)
int current_y = 0;                 // 블록 시작 위치 (맨 위)

gboolean game_loop(GtkWidget *widget) {
    // 블록이 아래로 한 칸 이동
    current_y++;

    // 충돌 검사 (나중에 추가 예정)
    if (current_y >= GRID_HEIGHT || grid[current_y][current_x] != 0) {
        current_y--; // 충돌했으므로 원래 위치로 되돌림
        // 블록을 고정 (나중에 추가)
        return FALSE; // 타이머 종료 (블록 고정 후 새 블록 생성)
    }

    // 게임판 다시 그리기
    gtk_widget_queue_draw(widget);
    return TRUE; // 계속 호출
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    switch (event->keyval) {
    case GDK_KEY_Left:  // 왼쪽 이동
        if (current_x > 0) current_x--;
        break;
    case GDK_KEY_Right: // 오른쪽 이동
        if (current_x < GRID_WIDTH - 1) current_x++;
        break;
    case GDK_KEY_Down:  // 아래로 빠르게
        if (current_y < GRID_HEIGHT - 1) current_y++;
        break;
    }

    // 게임판 다시 그리기
    gtk_widget_queue_draw(widget);
    return TRUE;
}

gboolean draw_game_board(GtkWidget *widget, cairo_t *cr, gpointer data) {
    double cell_size = 30; // 셀 크기

    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == 0 && (y == current_y && x == current_x)) {
                // 현재 블록 (파란색)
                cairo_set_source_rgb(cr, 0.2, 0.6, 0.8);
            } else if (grid[y][x] == 0) {
                // 빈 칸 (연한 회색)
                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
            } else {
                // 고정된 블록 (어두운 회색)
                cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);
            }

            cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
            cairo_fill(cr);
        }
    }

    return FALSE;
}

void start_tetris_game() {
    GtkWidget *window;
    GtkWidget *drawing_area;

    gtk_init(NULL, NULL);

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