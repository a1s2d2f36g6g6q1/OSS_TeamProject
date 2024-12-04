#pragma warning(disable : 4819)

#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "games.h"

#define TILE_SIZE 100
#define TILE_MARGIN 10

GtkLabel* score_label;  // 점수 표시 라벨
int score = 0;          // 전역 변수로 score 선언

void update_score() {
    if (GTK_IS_LABEL(score_label)) {
        char score_text[50];
        sprintf(score_text, "Score: %d", score);
        gtk_label_set_text(score_label, score_text);
        while (g_main_context_iteration(NULL, FALSE));  // Force the update to be drawn immediately
    }
}


// 위젯 and 데이터구조설계
GtkWidget* drawing_area;
int** grid;
int grid_size = 4;

// 그리드 초기화 함수
void initialize_grid(int size) {
    grid = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        grid[i] = (int*)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            grid[i][j] = 0;  // 초기화
        }
    }
}

// 동적 그리드 메모리 해제
void free_grid(int size) {
    for (int i = 0; i < size; i++) {
        free(grid[i]);
    }
    free(grid);
}

// 새로운 타일 생성
void add_random_tile() {
    int** empty_tiles = (int**)malloc(grid_size * grid_size * sizeof(int*));
    int empty_count = 0;

    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) {
                empty_tiles[empty_count] = malloc(2 * sizeof(int));
                empty_tiles[empty_count][0] = i;
                empty_tiles[empty_count][1] = j;
                empty_count++;
            }
        }
    }

    if (empty_count > 0) {
        int* tile = empty_tiles[rand() % empty_count];
        int random_value = (rand() % 10 < 9) ? 2 : 4;
        grid[tile[0]][tile[1]] = random_value;
    }
    // 동적 메모리 해제
    for (int i = 0; i < empty_count; i++) {
        free(empty_tiles[i]);
    }
    free(empty_tiles);
}

// 타일 색 설정
void set_tile_color(cairo_t* cr, int value) {
    switch (value) {
        case 2:
            cairo_set_source_rgb(cr, 0.93, 0.89, 0.85);
            break;
        case 4:
            cairo_set_source_rgb(cr, 0.93, 0.87, 0.78);
            break;
        case 8:
            cairo_set_source_rgb(cr, 0.96, 0.64, 0.38);
            break;
        case 16:
            cairo_set_source_rgb(cr, 0.96, 0.58, 0.38);
            break;
        case 32:
            cairo_set_source_rgb(cr, 0.96, 0.48, 0.38);
            break;
        case 64:
            cairo_set_source_rgb(cr, 0.96, 0.38, 0.28);
            break;
        case 128:
            cairo_set_source_rgb(cr, 0.93, 0.81, 0.45);
            break;
        case 256:
            cairo_set_source_rgb(cr, 0.93, 0.80, 0.38);
            break;
        case 512:
            cairo_set_source_rgb(cr, 0.93, 0.78, 0.28);
            break;
        case 1024:
            cairo_set_source_rgb(cr, 0.93, 0.76, 0.18);
            break;
        case 2048:
            cairo_set_source_rgb(cr, 0.93, 0.75, 0.08);
            break;
        default:
            cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
            break;
    }
}
// 타일 그리기
gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);  // 배경색
    cairo_paint(cr);

    // 타일 그리기
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            int value = grid[i][j];
            set_tile_color(cr, value);

            cairo_rectangle(cr, TILE_MARGIN + j * (TILE_SIZE + TILE_MARGIN),
                            TILE_MARGIN + i * (TILE_SIZE + TILE_MARGIN),
                            TILE_SIZE, TILE_SIZE);
            cairo_fill(cr);

            if (value != 0) {
                cairo_set_source_rgb(cr, 0, 0, 0);  // 글자 색상 (검정색)
                cairo_set_font_size(cr, 24);
                cairo_move_to(cr, TILE_MARGIN + j * (TILE_SIZE + TILE_MARGIN) + TILE_SIZE / 3,
                              TILE_MARGIN + i * (TILE_SIZE + TILE_MARGIN) + TILE_SIZE / 1.5);
                cairo_show_text(cr, g_strdup_printf("%d", value));
            }
        }
    }
    return FALSE;
}

// 타일 이동 및 합치기 함수
int move_tiles(int dx, int dy) {
    int moved = 0;
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            int x = (dx == 1) ? (grid_size - 1 - i) : i;
            int y = (dy == 1) ? (grid_size - 1 - j) : j;

            if (grid[x][y] != 0) {
                int nx = x, ny = y;
                while (nx + dx >= 0 && nx + dx < grid_size && ny + dy >= 0 && ny + dy < grid_size && grid[nx + dx][ny + dy] == 0) {
                    nx += dx;
                    ny += dy;
                }

                if (nx + dx >= 0 && nx + dx < grid_size && ny + dy >= 0 && ny + dy < grid_size && grid[nx + dx][ny + dy] == grid[x][y]) {
                    grid[nx + dx][ny + dy] *= 2;
                    score += grid[nx + dx][ny + dy];
                    grid[x][y] = 0;
                    moved = 1;
                } else if (nx != x || ny != y) {
                    grid[nx][ny] = grid[x][y];
                    grid[x][y] = 0;
                    moved = 1;
                }
            }
        }
    }

    if (moved) {
        update_score();
    }
    return moved;
}

// 키보드 입력 처리
gboolean on_key_press1(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    int moved = 0;

    switch (event->keyval) {
    case GDK_KEY_Left:
        moved = move_tiles(0, -1);
        break;
    case GDK_KEY_Right:
        moved = move_tiles(0, 1);
        break;
    case GDK_KEY_Up:
        moved = move_tiles(-1, 0);
        break;
    case GDK_KEY_Down:
        moved = move_tiles(1, 0);
        break;
    }

    if (moved) {
        add_random_tile();
        if (GTK_IS_WIDGET(drawing_area) && gtk_widget_get_visible(drawing_area)) {
            gtk_widget_queue_draw(drawing_area);  // Queue a redraw of the drawing area
            while (g_main_context_iteration(NULL, FALSE));  // Force the redraw immediately
        }
        if (GTK_IS_LABEL(score_label)) {
            update_score();  // Update the score label if it's valid
            while (g_main_context_iteration(NULL, FALSE));  // Force the update to be drawn immediately
        }
    }

    return TRUE;
}



// 게임 오버 상태 체크 함수
bool is_game_over() {
    // 빈 칸 확인
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (grid[i][j] == 0) {
                return false;  // 빈 칸 존재
            }

            // 인접 타일 합칠 수 있는지 확인
            if (i > 0 && grid[i][j] == grid[i - 1][j]) return false;              // 위
            if (i < grid_size - 1 && grid[i][j] == grid[i + 1][j]) return false;  // 아래
            if (j > 0 && grid[i][j] == grid[i][j - 1]) return false;              // 왼쪽
            if (j < grid_size - 1 && grid[i][j] == grid[i][j + 1]) return false;  // 오른쪽
        }
    }
    return true;  // 더 이상 움직일 수 없음
}

GtkWidget* create_2048_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // 점수 표시 라벨
    score_label = GTK_LABEL(gtk_label_new("Score: 0"));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(score_label), FALSE, FALSE, 0);

    // 2048 게임 보드 (DrawingArea)
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, grid_size * TILE_SIZE + (grid_size + 1) * TILE_MARGIN,
        grid_size * TILE_SIZE + (grid_size + 1) * TILE_MARGIN);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, TRUE, TRUE, 0);

    // 포커스를 받을 수 있도록 설정하고 이벤트 마스크 추가
    gtk_widget_set_can_focus(drawing_area, TRUE);
    gtk_widget_add_events(drawing_area, GDK_KEY_PRESS_MASK);

    // DrawingArea의 이벤트 연결
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(drawing_area, "key-press-event", G_CALLBACK(on_key_press1), stack);
    g_signal_connect(drawing_area, "realize", G_CALLBACK(gtk_widget_grab_focus), NULL);

    // 뒤로가기 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 0);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    // 2048 게임 초기화
    initialize_grid(grid_size);
    add_random_tile();
    add_random_tile();

    return vbox;
}


void start_2048_game(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);

    // 2048 게임 초기화
    score = 0;
    initialize_grid(grid_size);
    add_random_tile();
    add_random_tile();

    // 2048 화면으로 전환
    gtk_stack_set_visible_child_name(stack, "2048_screen");

    // Drawing Area에 포커스 설정
    if (GTK_IS_WIDGET(drawing_area)) {
        gtk_widget_grab_focus(drawing_area);
    }

    // 화면 업데이트 요청
    if (GTK_IS_WIDGET(drawing_area)) {
        gtk_widget_queue_draw(drawing_area);
    }
}
