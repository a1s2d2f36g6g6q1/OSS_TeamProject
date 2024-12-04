#pragma warning(disable : 4819)

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
// macOS 및 Linux 환경에서는 필요 없음
#endif

#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "games.h"

void switch_to_main_menu(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "main_menu");
}
void switch_to_scoreboard(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "scoreboard_screen");
}

void switch_to_setting(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "setting_screen");
}

GtkWidget* create_main_menu(GtkStack* stack) {

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(vbox, 40);
    gtk_widget_set_margin_end(vbox, 40);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);

    GtkWidget* title = gtk_label_new("Retro Game");
    PangoAttrList* attr_list = pango_attr_list_new();
    pango_attr_list_insert(attr_list, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attr_list, pango_attr_scale_new(2.0));
    gtk_label_set_attributes(GTK_LABEL(title), attr_list);
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 10);

    GtkWidget* grid_center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget* grid = gtk_grid_new();
    gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 20);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_box_pack_start(GTK_BOX(grid_center), grid, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), grid_center, TRUE, TRUE, 0);

    struct GameButton {
        const char* label;
        const char* image_path;
        GCallback callback;
    } games[] = {
        {"Tetris", "images/tetris.png", G_CALLBACK(start_tetris_game)},
        {"2048", "images/2048.png", G_CALLBACK(start_2048_game)},
        {"Break out", "images/breakout.png", G_CALLBACK(start_breakout_game)},
        {"Minesweeper", "images/minesweeper.png", G_CALLBACK(start_minesweeper_game)},
        {"Ranking", "images/ranking.png", G_CALLBACK(switch_to_scoreboard)},
        {"Setting", "images/setting.png", G_CALLBACK(switch_to_setting)}};

    for (int i = 0; i < 6; i++) {
        GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        GtkWidget* button = gtk_button_new();

        gtk_widget_set_size_request(button, 150, 150);
        gtk_style_context_add_class(gtk_widget_get_style_context(button), "game-button");

        GtkWidget* image = gtk_image_new_from_file(games[i].image_path);
        gtk_image_set_pixel_size(GTK_IMAGE(image), 64);

        GtkWidget* label = gtk_label_new(games[i].label);

        gtk_box_pack_start(GTK_BOX(button_box), image, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(button_box), label, FALSE, FALSE, 0);

        gtk_container_add(GTK_CONTAINER(button), button_box);

        if (games[i].callback) {
            g_signal_connect(button, "clicked", games[i].callback, stack);
        }

        gtk_grid_attach(GTK_GRID(grid), button, i % 2, i / 2, 1, 1);
    }

    GtkWidget* logout_button = gtk_button_new_with_label("Logout");
    gtk_widget_set_size_request(logout_button, 320, 50);
    g_signal_connect(logout_button, "clicked", G_CALLBACK(switch_to_login), stack);
    gtk_box_pack_start(GTK_BOX(vbox), logout_button, FALSE, FALSE, 20);

    return vbox;
}

void send_game_score(const char* username, const char* game, int score) {
    if (is_guest_mode) {
        printf("Guest mode: Score not sent to server. Username: %s, Game: %s, Score: %d\n", username, game, score);
        return;  
    }

    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;
    struct Memory chunk = {NULL, 0};

    curl = curl_easy_init();
    if (curl) {
        struct json_object* json_data = json_object_new_object();
        json_object_object_add(json_data, "username", json_object_new_string(username));
        json_object_object_add(json_data, "game", json_object_new_string(game));
        json_object_object_add(json_data, "score", json_object_new_int(score));
        const char* json_string = json_object_to_json_string(json_data);

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.137.1:5000/auth/save-score");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("Failed to send score to server!\n");
        } else {
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

    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
                                    ".game-button { "
                                    "    border-radius: 10px;"
                                    "    background: linear-gradient(to bottom, #ffffff, #f0f0f0);"
                                    "    border: 1px solid #cccccc;"
                                    "    padding: 10px;"
                                    "}"
                                    ".game-button:hover {"
                                    "    background: linear-gradient(to bottom, #f0f0f0, #e0e0e0);"
                                    "}",
                                    -1, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Retro");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkStack* stack = GTK_STACK(gtk_stack_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(stack));

    GtkWidget* login_screen = create_login_screen(stack);
    GtkWidget* main_menu = create_main_menu(stack);
    GtkWidget* screen_2048 = create_2048_screen(stack);
    GtkWidget* minesweeper_screen = create_minesweeper_screen(stack);
    GtkWidget* breakout_screen = create_breakout_screen(stack);
    GtkWidget* scoreboard_screen = create_scoreboard_screen(stack);
    GtkWidget* setting_screen = create_setting_screen(stack);

    gtk_stack_add_named(stack, login_screen, "login_screen");
    gtk_stack_add_named(stack, main_menu, "main_menu");
    gtk_stack_add_named(stack, minesweeper_screen, "minesweeper_screen");
    gtk_stack_add_named(stack, screen_2048, "2048_screen");
	gtk_stack_add_named(stack, breakout_screen, "breakout_screen");
	gtk_stack_add_named(stack, scoreboard_screen, "scoreboard_screen");
	gtk_stack_add_named(stack, setting_screen, "setting_screen");

    gtk_stack_set_visible_child_name(stack, "login_screen");

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}