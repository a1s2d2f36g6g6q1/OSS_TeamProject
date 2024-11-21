    #include <gtk/gtk.h>
    #include <stdlib.h>
    #include <time.h>
    #include <math.h>
    #include <stdbool.h>
    #include "games.h"

void switch_to_main_menu(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

GtkWidget* create_main_menu(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget* tetris_button = gtk_button_new_with_label("Play Tetris");
    gtk_box_pack_start(GTK_BOX(vbox), tetris_button, FALSE, FALSE, 5);
    g_signal_connect(tetris_button, "clicked", G_CALLBACK(start_tetris_game), NULL);

    GtkWidget* game2048_button = gtk_button_new_with_label("Play 2048");
    gtk_box_pack_start(GTK_BOX(vbox), game2048_button, FALSE, FALSE, 5);
    g_signal_connect(game2048_button, "clicked", G_CALLBACK(start_2048_game), NULL);

    GtkWidget* gamebreak_button = gtk_button_new_with_label("Play brick");
    gtk_box_pack_start(GTK_BOX(vbox), gamebreak_button, FALSE, FALSE, 5);
    g_signal_connect(gamebreak_button, "clicked", G_CALLBACK(start_breakout_game_BP), NULL);

    GtkWidget* minesweeper_button = gtk_button_new_with_label("Play Minesweeper");
    gtk_box_pack_start(GTK_BOX(vbox), minesweeper_button, FALSE, FALSE, 5);
    g_signal_connect(minesweeper_button, "clicked", G_CALLBACK(start_minesweeper_game), NULL);

    return vbox;
}

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Retro");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkStack* stack = GTK_STACK(gtk_stack_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(stack));

    GtkWidget* main_menu = create_main_menu(stack);

    gtk_stack_add_named(stack, main_menu, "main_menu");


    gtk_stack_set_visible_child_name(stack, "main_menu");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
