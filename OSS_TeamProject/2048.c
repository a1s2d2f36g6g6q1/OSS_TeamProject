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
// 타일 그리기
gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    
    
}
// 키보드 입력 처리
gboolean on_key_press(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    
}



void start_2048_game() {
    GtkWidget *window;
    gtk_init(NULL, NULL);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "2048 Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}
