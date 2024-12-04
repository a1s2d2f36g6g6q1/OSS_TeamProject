#include <gtk/gtk.h>
#include "games.h"

// Mock Data (���� �����ͺ��̽����� ���������� ���� ����)
typedef struct {
    char username[32];
    int score;
} Score;

typedef struct {
    const char* game_name;
    Score scores[10];
} GameScore;

GameScore game_scores[] = {
    {"Tetris", {{"Alice", 1000}, {"Bob", 900}, {"Charlie", 800}, {"David", 700}, {"Eve", 600},
                {"Frank", 500}, {"Grace", 400}, {"Hank", 300}, {"Ivy", 200}, {"Jack", 100}}},
    {"Brick Breaker", {{"Alice", 1500}, {"Bob", 1400}, {"Charlie", 1300}, {"David", 1200}, {"Eve", 1100},
                       {"Frank", 1000}, {"Grace", 900}, {"Hank", 800}, {"Ivy", 700}, {"Jack", 600}}},
    {"2048", {{"Alice", 2000}, {"Bob", 1900}, {"Charlie", 1800}, {"David", 1700}, {"Eve", 1600},
              {"Frank", 1500}, {"Grace", 1400}, {"Hank", 1300}, {"Ivy", 1200}, {"Jack", 1100}}},
    {"Minesweeper", {{"Alice", 50}, {"Bob", 60}, {"Charlie", 70}, {"David", 80}, {"Eve", 90},
                     {"Frank", 100}, {"Grace", 110}, {"Hank", 120}, {"Ivy", 130}, {"Jack", 140}}}
};

// ���� ���̺� ������Ʈ �Լ�
void update_score_table(GtkWidget* grid) {
    for (int g = 0; g < 4; g++) {
        for (int i = 0; i < 10; i++) {
            GtkWidget* label_username = gtk_grid_get_child_at(GTK_GRID(grid), 1, g * 11 + i + 1);
            GtkWidget* label_score = gtk_grid_get_child_at(GTK_GRID(grid), 2, g * 11 + i + 1);
            gtk_label_set_text(GTK_LABEL(label_username), game_scores[g].scores[i].username);
            char score_text[16];
            snprintf(score_text, sizeof(score_text), "%d", game_scores[g].scores[i].score);
            gtk_label_set_text(GTK_LABEL(label_score), score_text);
        }
    }
}

// ���ΰ�ħ ��ư �ڵ鷯
void on_refresh_button_clicked(GtkWidget* widget, gpointer grid) {
    update_score_table(GTK_WIDGET(grid));
}

// ������ ������ ����
GtkWidget* create_scoreboard_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // ��ü �������� ���η� ������ �� �ִ� �׸���
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 30);

    for (int g = 0; g < 4; g++) {
        // �� ���Ӻ� ������ ���� VBox ����
        GtkWidget* game_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        // ���� �̸� ��
        GtkWidget* game_label = gtk_label_new(game_scores[g].game_name);
        gtk_widget_set_margin_bottom(game_label, 10);
        gtk_box_pack_start(GTK_BOX(game_box), game_label, FALSE, FALSE, 5);

        // �� ���� ��
        GtkWidget* rank_label = gtk_label_new("Rank");
        GtkWidget* username_label = gtk_label_new("Username");
        GtkWidget* score_label = gtk_label_new("Score");
        GtkWidget* header_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        gtk_box_pack_start(GTK_BOX(header_hbox), rank_label, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(header_hbox), username_label, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(header_hbox), score_label, FALSE, FALSE, 5);
        gtk_box_pack_start(GTK_BOX(game_box), header_hbox, FALSE, FALSE, 5);

        // ���� ������
        for (int i = 0; i < 10; i++) {
            GtkWidget* row_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            char rank_text[4];
            snprintf(rank_text, sizeof(rank_text), "%d", i + 1);

            GtkWidget* rank = gtk_label_new(rank_text);
            GtkWidget* username = gtk_label_new(game_scores[g].scores[i].username);
            char score_text[16];
            snprintf(score_text, sizeof(score_text), "%d", game_scores[g].scores[i].score);
            GtkWidget* score = gtk_label_new(score_text);

            gtk_box_pack_start(GTK_BOX(row_hbox), rank, FALSE, FALSE, 5);
            gtk_box_pack_start(GTK_BOX(row_hbox), username, FALSE, FALSE, 5);
            gtk_box_pack_start(GTK_BOX(row_hbox), score, FALSE, FALSE, 5);

            gtk_box_pack_start(GTK_BOX(game_box), row_hbox, FALSE, FALSE, 2);
        }

        // �� ���� ������ Grid�� ���� �߰�
        gtk_grid_attach(GTK_GRID(grid), game_box, g, 0, 1, 1);
    }

    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 10);

    // ���ΰ�ħ ��ư
    GtkWidget* refresh_button = gtk_button_new_with_label("Refresh Scores");
    gtk_widget_set_margin_top(refresh_button, 20);
    gtk_widget_set_halign(refresh_button, GTK_ALIGN_CENTER);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), grid);
    gtk_box_pack_end(GTK_BOX(vbox), refresh_button, FALSE, FALSE, 5);

    return vbox;
}
