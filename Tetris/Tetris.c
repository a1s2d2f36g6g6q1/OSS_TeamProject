
#include <stdio.h>
#include "Tetris.h"
/*
#define ROWS 20
#define COLS 10

static char board[ROWS][COLS];

void initializeBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = '.';  // 빈 공간을 '.'으로 표시
        }
    }
}

void printBoard() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

void startTetris() {
    printf("Starting Tetris game...\n");

    initializeBoard();

    printBoard();
}
*/

#include <gtk/gtk.h>

void startTetris(int argc, char *argv[]) {
    GtkWidget *window;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}