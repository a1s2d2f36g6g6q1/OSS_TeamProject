#ifndef GAMES_H
#define GAMES_H

void start_2048_game();
void start_tetris_game();
void start_breakout_game_BP();
void start_minesweeper_game();

void switch_to_main_menu(GtkWidget* widget, gpointer data);

GtkWidget* create_minesweeper_screen(GtkStack* stack);
GtkWidget* create_brickbreaker_screen(GtkStack* stack);

#endif
