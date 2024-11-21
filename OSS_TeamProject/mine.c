#include <stdio.h>
#include <gtk/gtk.h>
#include <time.h>
#include <stdbool.h>

#define FIELD_SIZE 16
#define MINE -1
#define TOTAL_MINES 40

int mines_left = TOTAL_MINES;

int field[FIELD_SIZE][FIELD_SIZE];
bool revealed[FIELD_SIZE][FIELD_SIZE];
bool flagged[FIELD_SIZE][FIELD_SIZE];

GtkWidget* mine_label, * timer_label;
GtkWidget* buttons[FIELD_SIZE][FIELD_SIZE];

int elapsed_time = 0;
guint timer_id;

gboolean update_timer(gpointer data) {
    elapsed_time++;
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Time: %d", elapsed_time);
    gtk_label_set_text(GTK_LABEL(timer_label), buffer);

    return TRUE;
}

void update_mine_counter() {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "Mines: %d", mines_left);
    gtk_label_set_text(GTK_LABEL(mine_label), buffer);
}

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
            g_print("Mine placed at (%d, %d)\n", x, y);
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

void reveal_cell(int x, int y);
void reveal_neighbors(int x, int y) {
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < FIELD_SIZE && ny >= 0 && ny < FIELD_SIZE &&
                !revealed[nx][ny] && !flagged[nx][ny] && field[nx][ny] != MINE) {
                reveal_cell(nx, ny);
            }
        }
    }
}

void reveal_cell(int x, int y) {
    if (revealed[x][y] || flagged[x][y])  return;

    revealed[x][y] = true;
    gtk_widget_set_sensitive(buttons[x][y], FALSE);

    if (field[x][y] == MINE) {
        set_button_label(GTK_BUTTON(buttons[x][y]), "B");
        g_print("Game Over! You hit a mine at (%d, %d).\n", x, y);
        return;
    }

    if (field[x][y] > 0) {
        char buffer[2];
        snprintf(buffer, sizeof(buffer), "%d", field[x][y]);
        set_button_label(GTK_BUTTON(buttons[x][y]), buffer);
    }
    else {
        set_button_label(GTK_BUTTON(buttons[x][y]), "");
        reveal_neighbors(x, y);
    }
}

void on_button_clicked(GtkWidget* widget, GdkEventButton* event, gpointer data) {

    int* coords = (int*)data;
    int x = coords[0];
    int y = coords[1];

    if (event->button == GDK_BUTTON_PRIMARY && !flagged[x][y]) {
        reveal_cell(x, y);
    }
    else if (event->button == GDK_BUTTON_SECONDARY) {
        if (!flagged[x][y] && !revealed[x][y]) {
            set_button_label(GTK_BUTTON(buttons[x][y]), "F");
            flagged[x][y] = true;
            mines_left--;
        }
        else if (flagged[x][y]) {
            set_button_label(GTK_BUTTON(buttons[x][y]), "");
            flagged[x][y] = false;
            mines_left++;
        }
        update_mine_counter();
    }
}

void lose() {

}

void win() {

}

void start_minesweeper_game() {
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minesweeper");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    GtkWidget* grid = gtk_grid_new();
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 5);

    mine_label = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(hbox), mine_label, FALSE, FALSE, 5);

    timer_label = gtk_label_new(NULL);
    gtk_box_pack_end(GTK_BOX(hbox), timer_label, FALSE, FALSE, 5);

    initialize_field();

    for (int i = 0; i < FIELD_SIZE; i++) {
        for (int j = 0; j < FIELD_SIZE; j++) {
            int* coords = malloc(2 * sizeof(int));
            coords[0] = i;
            coords[1] = j;

            buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(buttons[i][j], 50, 50);
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j], j, i, 1, 1);
            g_signal_connect(buttons[i][j], "button-press-event", G_CALLBACK(on_button_clicked), coords);
        }
    }

    update_mine_counter();
    timer_id = g_timeout_add(1000, update_timer, NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);
}