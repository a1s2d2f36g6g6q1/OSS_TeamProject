#include <gtk/gtk.h>
#include "games.h"

// Mock Data (실제 데이터베이스에서 가져오도록 구현 가능)
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

// 점수 테이블 업데이트 함수
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

// 새로고침 버튼 핸들러
void on_refresh_button_clicked(GtkWidget* widget, gpointer grid) {
    update_score_table(GTK_WIDGET(grid));
}

// 점수판 페이지 생성
GtkWidget* create_scoreboard_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // 점수판 그리드
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    for (int g = 0; g < 4; g++) {
        // 게임 이름 추가
        GtkWidget* game_label = gtk_label_new(game_scores[g].game_name);
        gtk_grid_attach(GTK_GRID(grid), game_label, g * 3, 0, 3, 1); // 각 게임 이름을 3칸에 걸쳐 표시

        // 열 제목
        GtkWidget* rank_label = gtk_label_new("Rank");
        GtkWidget* username_label = gtk_label_new("Username");
        GtkWidget* score_label = gtk_label_new("Score");
        gtk_grid_attach(GTK_GRID(grid), rank_label, g * 3, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), username_label, g * 3 + 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), score_label, g * 3 + 2, 1, 1, 1);

        // 순위 데이터
        for (int i = 0; i < 10; i++) {
            char rank_text[4];
            snprintf(rank_text, sizeof(rank_text), "%d", i + 1);

            GtkWidget* rank = gtk_label_new(rank_text);
            GtkWidget* username = gtk_label_new(game_scores[g].scores[i].username);
            char score_text[16];
            snprintf(score_text, sizeof(score_text), "%d", game_scores[g].scores[i].score);
            GtkWidget* score = gtk_label_new(score_text);

            gtk_grid_attach(GTK_GRID(grid), rank, g * 3, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), username, g * 3 + 1, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), score, g * 3 + 2, i + 2, 1, 1);
        }
    }

    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 10);

    // 버튼 박스
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

    // 새로고침 버튼
    GtkWidget* refresh_button = gtk_button_new_with_label("Refresh Scores");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), grid);
    gtk_box_pack_start(GTK_BOX(button_box), refresh_button, FALSE, FALSE, 5);

    // 메인메뉴로 돌아가기 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);
    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 10);

    return vbox;
}
