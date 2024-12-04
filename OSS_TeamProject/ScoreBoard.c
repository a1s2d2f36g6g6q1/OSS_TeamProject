#include <gtk/gtk.h>
#include <curl.h>
#include <json.h>
#include "games.h"

// ���� ���� �����͸� ������ ����ü
struct MemoryStruct {
    char* memory;
    size_t size;
};

// ���� ������ �����ϱ� ���� �ݹ� �Լ�
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // �޸� �Ҵ� ����
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// ���� ���̺� ������Ʈ �Լ�
void update_score_table(GtkWidget* grid) {
    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  // �ʱ� �޸� �Ҵ�
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        const char* server_url = "http://localhost:5000/auth/get-all-scores";

        curl_easy_setopt(curl, CURLOPT_URL, server_url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L); // GET ��û ����
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            printf("Server response: %s\n", chunk.memory); // ���� �����

            // JSON ������ �Ľ�
            struct json_object* parsed_json = json_tokener_parse(chunk.memory);
            if (parsed_json == NULL) {
                fprintf(stderr, "Failed to parse JSON response\n");
                return;
            }

            struct json_object* game_scores;
            if (json_object_object_get_ex(parsed_json, "gameScores", &game_scores)) {
                int num_scores = json_object_array_length(game_scores);

                // �� ���� �����͸� ������Ʈ
                for (int i = 0; i < num_scores; i++) {
                    struct json_object* score_entry = json_object_array_get_idx(game_scores, i);
                    struct json_object* game;
                    struct json_object* username;
                    struct json_object* high_score;
                    struct json_object* rank;

                    // JSON �Ӽ� ��������
                    if (json_object_object_get_ex(score_entry, "game", &game) &&
                        json_object_object_get_ex(score_entry, "username", &username) &&
                        json_object_object_get_ex(score_entry, "high_score", &high_score) &&
                        json_object_object_get_ex(score_entry, "rank", &rank)) {

                        // ���� �̸� ��ȯ
                        const char* game_name = json_object_get_string(game);
                        const char* user_name = json_object_get_string(username);
                        int score_value = json_object_get_int(high_score);
                        int rank_value = json_object_get_int(rank);

                        const char* formatted_game_name;
                        if (strcmp(game_name, "mine") == 0) {
                            formatted_game_name = "MineSweeper";
                        }
                        else if (strcmp(game_name, "tetris") == 0) {
                            formatted_game_name = "Tetris";
                        }
                        else if (strcmp(game_name, "2048") == 0) {
                            formatted_game_name = "2048";
                        }
                        else if (strcmp(game_name, "bp") == 0) {
                            formatted_game_name = "BrickBreak";
                        }
                        else {
                            formatted_game_name = "Unknown";
                        }

                        // �׸����� �� ��ġ ��� (���Ӻ� ���� ����)
                        int game_column = 0;
                        if (strcmp(game_name, "tetris") == 0) {
                            game_column = 3;
                        }
                        else if (strcmp(game_name, "2048") == 0) {
                            game_column = 6;
                        }
                        else if (strcmp(game_name, "bp") == 0) {
                            game_column = 9;
                        }

                        // ���� �̸� ������Ʈ (0�࿡ ǥ��)
                        GtkWidget* game_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column, 0);
                        if (game_label) gtk_label_set_text(GTK_LABEL(game_label), formatted_game_name);

                        // ���� ������ ������Ʈ
                        GtkWidget* rank_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column, rank_value + 1);
                        GtkWidget* username_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 1, rank_value + 1);
                        GtkWidget* score_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 2, rank_value + 1);

                        if (rank_label) {
                            char rank_text[4];
                            snprintf(rank_text, sizeof(rank_text), "%d", rank_value);
                            gtk_label_set_text(GTK_LABEL(rank_label), rank_text);
                        }

                        if (username_label) {
                            gtk_label_set_text(GTK_LABEL(username_label), user_name);
                        }

                        if (score_label) {
                            char score_text[16];
                            snprintf(score_text, sizeof(score_text), "%d", score_value);
                            gtk_label_set_text(GTK_LABEL(score_label), score_text);
                        }
                    }
                }
            }
            else {
                fprintf(stderr, "Invalid JSON structure: 'gameScores' not found\n");
            }

            json_object_put(parsed_json); // JSON ��ü �޸� ����
        }
        else {
            fprintf(stderr, "Failed to fetch scores: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        if (chunk.memory) {
            free(chunk.memory);
        }
    }
    else {
        fprintf(stderr, "Failed to initialize CURL\n");
    }
}


// ���ΰ�ħ ��ư �ڵ鷯
void on_refresh_button_clicked(GtkWidget* widget, gpointer grid) {
    update_score_table(GTK_WIDGET(grid));
}

// ������ ������ ����
GtkWidget* create_scoreboard_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // ������ �׸���
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    for (int g = 0; g < 4; g++) {
        // ���� �̸� �߰�
        GtkWidget* game_label = gtk_label_new("game_name_placeholder");
        gtk_grid_attach(GTK_GRID(grid), game_label, g * 3, 0, 3, 1); // �� ���� �̸��� 3ĭ�� ���� ǥ��

        // �� ����
        GtkWidget* rank_label = gtk_label_new("Rank");
        GtkWidget* username_label = gtk_label_new("Username");
        GtkWidget* score_label = gtk_label_new("Score");
        gtk_grid_attach(GTK_GRID(grid), rank_label, g * 3, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), username_label, g * 3 + 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), score_label, g * 3 + 2, 1, 1, 1);

        // ���� ������ �ʱ�ȭ (�÷��̽�Ȧ��)
        for (int i = 0; i < 10; i++) {
            char rank_text[4];
            snprintf(rank_text, sizeof(rank_text), "%d", i + 1);

            GtkWidget* rank = gtk_label_new(rank_text);
            GtkWidget* username = gtk_label_new("username_placeholder");
            char score_text[16];
            snprintf(score_text, sizeof(score_text), "%d", 0); // ���� ������ �������� �޾ƿ�
            GtkWidget* score = gtk_label_new(score_text);

            gtk_grid_attach(GTK_GRID(grid), rank, g * 3, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), username, g * 3 + 1, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), score, g * 3 + 2, i + 2, 1, 1);
        }
    }

    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 10);

    // ��ư �ڽ�
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

    // ���ΰ�ħ ��ư
    GtkWidget* refresh_button = gtk_button_new_with_label("Refresh Scores");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), grid);
    gtk_box_pack_start(GTK_BOX(button_box), refresh_button, FALSE, FALSE, 5);

    // ���θ޴��� ���ư��� ��ư
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);
    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 10);

    return vbox;
}
