#include <stdio.h>
#include <gtk/gtk.h>
#include <time.h>
#include <stdbool.h>

#define FIELD_SIZE 16
#define MINE -1
#define TOTAL_MINES 40

int field[FIELD_SIZE][FIELD_SIZE];
bool revealed[FIELD_SIZE][FIELD_SIZE];
bool flagged[FIELD_SIZE][FIELD_SIZE];

GtkWidget* mine_label, * timer_label;
GtkWidget* buttons[FIELD_SIZE][FIELD_SIZE];

void set_button_label(GtkButton* button, const char* label) {
    gtk_button_set_label(button, label);
}

void initialize_field() {
    for (int i = 0; i < FIELD_SIZE; i++) {
        for (int j = 0; j < FIELD_SIZE; j++) {
            field[i][j] = 0;
            revealed[i][j] = false;
            flagged[i][j] = false;
        }
    }

    int mines = TOTAL_MINES;
    while (mines > 0) {
        int x = rand() % FIELD_SIZE;
        int y = rand() % FIELD_SIZE;
        if (field[x][y] != MINE) {
            field[x][y] = MINE;
            mines--;
        }
    }

    for (int x = 0; x < FIELD_SIZE; x++) {
        for (int y = 0; y < FIELD_SIZE; y++) {
            if (field[x][y] == MINE) continue;

            int count = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < FIELD_SIZE && ny >= 0 && ny < FIELD_SIZE && field[nx][ny] == MINE) {
                        count++;
                    }
                }
            }
            field[x][y] = count;
        }
    }
}

void start_minesweeper_game() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minesweeper");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 상단 라벨 박스
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    // 그리드
    GtkWidget* grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 5);

    initialize_field();

    for (int i = 0; i < FIELD_SIZE; i++) {
        for (int j = 0; j < FIELD_SIZE; j++) {
            int* coords = malloc(2 * sizeof(int));
            coords[0] = i;
            coords[1] = j;

            buttons[i][j] = gtk_button_new();
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j], j, i, 1, 1);
        }
    }

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
}

