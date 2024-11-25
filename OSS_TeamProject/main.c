#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <curl.h>
#include <json.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
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

    GtkWidget* BP_button = gtk_button_new_with_label("Play BrickBreak");
    gtk_box_pack_start(GTK_BOX(vbox), BP_button, FALSE, FALSE, 5);
    g_signal_connect(BP_button, "clicked", G_CALLBACK(start_breakout_game_BP), NULL);

    GtkWidget* minesweeper_button = gtk_button_new_with_label("Play Minesweeper");
    gtk_box_pack_start(GTK_BOX(vbox), minesweeper_button, FALSE, FALSE, 5);
    g_signal_connect(minesweeper_button, "clicked", G_CALLBACK(start_minesweeper_game), stack);

    GtkWidget* logout_button = gtk_button_new_with_label("Logout");
    gtk_box_pack_start(GTK_BOX(vbox), logout_button, FALSE, FALSE, 5);
    g_signal_connect(logout_button, "clicked", G_CALLBACK(switch_to_login), stack);


    return vbox;
}

void send_game_score(const char* username, const char* game, int score) {
    if (is_guest_mode) {
        printf("Guest mode: Score not sent to server. Username: %s, Game: %s, Score: %d\n", username, game, score);
        return; // 서버와 통신하지 않음
    }

    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;
    struct Memory chunk = { NULL, 0 };

    curl = curl_easy_init();
    if (curl) {
        struct json_object* json_data = json_object_new_object();
        json_object_object_add(json_data, "username", json_object_new_string(username));
        json_object_object_add(json_data, "game", json_object_new_string(game));
        json_object_object_add(json_data, "score", json_object_new_int(score));
        const char* json_string = json_object_to_json_string(json_data);

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.103:5000/auth/save-score");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("Failed to send score to server!\n");
        }
        else {
            printf("Score sent to server successfully. Response: %s\n", chunk.response);
        }

        json_object_put(json_data);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.response);
    }
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