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

void switch_to_login(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "login_screen");
}

void on_guest_button_clicked(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

void on_login_button_clicked(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

GtkWidget* create_login_screen(GtkStack* stack) {
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(outer_box, GTK_ALIGN_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_spacing(GTK_BOX(vbox), 10);
    gtk_box_pack_start(GTK_BOX(outer_box), vbox, FALSE, FALSE, 0);

    GtkWidget* title_label = gtk_label_new("LOGIN");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    GtkWidget* username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "ID");
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 5);

    GtkWidget* password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 5);

    GtkWidget* login_button = gtk_button_new_with_label("Sign in");
    gtk_box_pack_start(GTK_BOX(vbox), login_button, FALSE, FALSE, 5);

    GtkWidget* guest_button = gtk_button_new_with_label("Guest");
    gtk_box_pack_start(GTK_BOX(vbox), guest_button, FALSE, FALSE, 5);

    GtkWidget* result_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), result_label, FALSE, FALSE, 5);

    GtkWidget** widgets = g_new(GtkWidget*, 4);
    widgets[0] = username_entry;
    widgets[1] = password_entry;
    widgets[2] = result_label;
    widgets[3] = stack;

   // g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), widgets);
    g_signal_connect(guest_button, "clicked", G_CALLBACK(on_guest_button_clicked), stack);

    return outer_box;
}

GtkWidget* create_main_menu(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    GtkWidget* tetris_button = gtk_button_new_with_label("Play Tetris");
    gtk_box_pack_start(GTK_BOX(vbox), tetris_button, FALSE, FALSE, 5);

    GtkWidget* game2048_button = gtk_button_new_with_label("Play 2048");
    gtk_box_pack_start(GTK_BOX(vbox), game2048_button, FALSE, FALSE, 5);

    GtkWidget* minesweeper_button = gtk_button_new_with_label("Play Minesweeper");
    gtk_box_pack_start(GTK_BOX(vbox), minesweeper_button, FALSE, FALSE, 5);
    g_signal_connect(minesweeper_button, "clicked", G_CALLBACK(start_minesweeper_game), stack);

    GtkWidget* logout_button = gtk_button_new_with_label("Logout");
    gtk_box_pack_start(GTK_BOX(vbox), logout_button, FALSE, FALSE, 5);
    g_signal_connect(logout_button, "clicked", G_CALLBACK(switch_to_login), stack);


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

    GtkWidget* login_screen = create_login_screen(stack);
    GtkWidget* main_menu = create_main_menu(stack);
    GtkWidget* minesweeper_screen = create_minesweeper_screen(stack);

    gtk_stack_add_named(stack, login_screen, "login_screen");
    gtk_stack_add_named(stack, main_menu, "main_menu");
    gtk_stack_add_named(stack, minesweeper_screen, "minesweeper_screen");

    gtk_stack_set_visible_child_name(stack, "login_screen");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}