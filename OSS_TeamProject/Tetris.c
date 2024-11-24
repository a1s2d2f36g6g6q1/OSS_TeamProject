#include <gtk/gtk.h>
#include <cairo.h>

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 10x20 게임판 데이터

// 게임판을 그리는 함수
gboolean draw_game_board(GtkWidget *widget, cairo_t *cr, gpointer data) {
    double cell_size = 30; // 셀 크기 (픽셀)

    // 게임판 그리기
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == 0) {
                cairo_set_source_rgb(cr, 0.9, 0.9, 0.9); // 빈 칸 (연한 회색)
            } else {
                cairo_set_source_rgb(cr, 0.2, 0.6, 0.8); // 블록 칸 (파란색)
            }

            // 셀 그리기 (사각형)
            cairo_rectangle(cr, x * cell_size, y * cell_size, cell_size - 1, cell_size - 1);
            cairo_fill(cr);
        }
    }

    return FALSE; // 기본 처리 후 종료
}

// 테트리스 시작 함수
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

    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    gtk_widget_show_all(window);

    gtk_main();
}