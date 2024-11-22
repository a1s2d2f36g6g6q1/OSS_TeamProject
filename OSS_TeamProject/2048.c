#include <stdio.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "games.h"

#define TILE_SIZE 100
#define TILE_MARGINE 10

//위젯 and 데이터구조설계
GtkWidget* drawing_area;
int** grid;
int grid_size = 4;
int score = 0;

//그리드 초기화 함수
void initialize_grid(int size){
    grid = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        grid[i] = (int*)malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            grid[i][j] = 0;  // 초기화
        }
    }
}

// 새로운 타일 생성
void add_random_tile(){
    int** empty_tiles = (int**)malloc((grid_size * grid_size) * sizeof(int*));
int empty_count = 0;

for (int i = 0; i < grid_size; i++) {
    for (int j = 0; j < grid_size; j++) {
        if (grid[i][j] == 0) {
            empty_tiles[empty_count] = (int*)malloc(2 * sizeof(int));
            empty_tiles[empty_count][0] = i;
            empty_tiles[empty_count][1] = j;
            empty_count++;
        }
    }
}

if (empty_count > 0) {
    int* tile = empty_tiles[rand() % empty_count];
    grid[tile[0]][tile[1]] = (rand() % 2 + 1) * 2;  // 2 또는 4 추가
}
    // 동적 메모리 해제
for (int i = 0; i < empty_count; i++) {
    free(empty_tiles[i]);
}
free(empty_tiles);
}

//타일 이동 및 합치기 함수
int move_tiles(int dx, int dy){
    int moved = 0;
}

void start_2048_game() {

    initialize_grid(grid_size);
    add_random_tile();
    add_random_tile();

    GtkWidget *window;
    gtk_init(NULL, NULL);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "2048 Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}
