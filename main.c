    #include <gtk/gtk.h>
    #include <stdlib.h>
    #include <time.h>
    #include <math.h>
    #include <stdbool.h>
    #include "games.h"

    void show_main_menu() {
        GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Select Game");
        gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
        g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

        GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 20);
        gtk_container_add(GTK_CONTAINER(window), vbox);

        GtkWidget* tetris_button = gtk_button_new_with_label("Play Tetris");
        g_signal_connect(tetris_button, "clicked", G_CALLBACK(start_tetris_game), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), tetris_button, TRUE, TRUE, 5);

        GtkWidget* game2048_button = gtk_button_new_with_label("Play 2048");
        g_signal_connect(game2048_button, "clicked", G_CALLBACK(start_2048_game), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), game2048_button, TRUE, TRUE, 5);

        GtkWidget* gamebreak_button = gtk_button_new_with_label("Play break");
        g_signal_connect(gamebreak_button, "clicked", G_CALLBACK(start_breakout_game_BP), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), gamebreak_button, TRUE, TRUE, 5);

        GtkWidget* minesweeper_button = gtk_button_new_with_label("Play Minesweeper");
        g_signal_connect(minesweeper_button, "clicked", G_CALLBACK(start_minesweeper_game), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), minesweeper_button, TRUE, TRUE, 5);

        GtkWidget* exit_button = gtk_button_new_with_label("Exit");
        g_signal_connect(exit_button, "clicked", G_CALLBACK(gtk_main_quit), NULL);
        gtk_box_pack_start(GTK_BOX(vbox), exit_button, TRUE, TRUE, 5);

        gtk_widget_show_all(window);
    }

    int main(int argc, char* argv[]) {
        gtk_init(&argc, &argv);
        srand(time(NULL));

        show_main_menu(); 
        gtk_main();

        return 0;
    }
