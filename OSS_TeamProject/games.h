#ifndef GAMES_H
#define GAMES_H

#include <gtk/gtk.h>  // GtkWidget, gpointer 등 GTK+ 관련 타입 사용
#include <stdbool.h>  // bool 타입 사용


void start_2048_game(GtkWidget* widget, gpointer data);
void start_tetris_game(GtkWidget* widget, gpointer data);
void start_breakout_game(GtkWidget* widget, gpointer data);
void start_minesweeper_game(GtkWidget* widget, gpointer data);  // 함수 시그니처 수정

void switch_to_main_menu(GtkWidget* widget, gpointer data);
void switch_to_login(GtkWidget* widget, gpointer data);
GtkWidget* create_minesweeper_screen(GtkStack* stack);
GtkWidget* create_login_screen(GtkStack* stack);
GtkWidget* create_2048_screen(GtkStack* stack);
GtkWidget* create_breakout_screen(GtkStack* stack);
GtkWidget* create_scoreboard_screen(GtkStack* stack);
GtkWidget* create_setting_screen(GtkStack* stack);
GtkWidget* create_signup_screen(GtkStack* stack);
GtkWidget* create_tetris_screen(GtkStack* stack);

extern bool is_guest_mode;
extern char username[50];

struct Memory {
    char* response;
    size_t size;
};

size_t write_callback(void* data, size_t size, size_t nmemb, void* userp);
void send_game_score(const char* username, const char* game, int score);

#endif
