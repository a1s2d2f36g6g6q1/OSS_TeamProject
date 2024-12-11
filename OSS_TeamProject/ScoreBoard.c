#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>

#include "games.h"

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void update_score_table(GtkWidget* grid) {
    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        const char* server_url = "http://localhost:5000/auth/get-all-scores";
        curl_easy_setopt(curl, CURLOPT_URL, server_url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            printf("Server response: %s\n", chunk.memory);

            struct json_object* parsed_json = json_tokener_parse(chunk.memory);
            if (parsed_json == NULL) {
                fprintf(stderr, "Failed to parse JSON response\n");
                return;
            }

            struct json_object* game_scores;
            if (json_object_object_get_ex(parsed_json, "gameScores", &game_scores)) {
                int num_scores = json_object_array_length(game_scores);

                for (int i = 0; i < num_scores; i++) {
                    struct json_object* score_entry = json_object_array_get_idx(game_scores, i);
                    struct json_object* game;
                    struct json_object* username;
                    struct json_object* high_score;
                    struct json_object* rank;

                    if (json_object_object_get_ex(score_entry, "game", &game) &&
                        json_object_object_get_ex(score_entry, "username", &username) &&
                        json_object_object_get_ex(score_entry, "high_score", &high_score) &&
                        json_object_object_get_ex(score_entry, "rank", &rank)) {
                        const char* game_name = json_object_get_string(game);
                        const char* user_name = json_object_get_string(username);
                        int score_value = json_object_get_int(high_score);
                        int rank_value = json_object_get_int(rank);

                        int game_column = 0;
                        if (strcmp(game_name, "tetris") == 0) {
                            game_column = 0;
                        } else if (strcmp(game_name, "2048") == 0) {
                            game_column = 4;
                        } else if (strcmp(game_name, "bp") == 0) {
                            game_column = 8;
                        } else if (strcmp(game_name, "mine") == 0) {
                            game_column = 12;
                        }

                        int row_index = rank_value + 2;
                        GtkWidget* rank_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column, row_index);
                        GtkWidget* username_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 1, row_index);
                        GtkWidget* score_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 2, row_index);

                        if (GTK_IS_LABEL(rank_label)) {
                            char rank_text[4];
                            snprintf(rank_text, sizeof(rank_text), "%d", rank_value);
                            gtk_label_set_text(GTK_LABEL(rank_label), rank_text);
                        }

                        if (GTK_IS_LABEL(username_label)) {
                            gtk_label_set_text(GTK_LABEL(username_label), user_name);
                        }

                        if (GTK_IS_LABEL(score_label)) {
                            char score_text[16];
                            snprintf(score_text, sizeof(score_text), "%d", score_value);
                            gtk_label_set_text(GTK_LABEL(score_label), score_text);
                        }
                    }
                }
            }

            json_object_put(parsed_json);
        } else {
            fprintf(stderr, "Failed to fetch scores: %s\n", curl_easy_strerror(res));
        }

        char user_url[256];
        snprintf(user_url, sizeof(user_url), "http://localhost:5000/auth/get-user-scores?username=%s", username);
        printf("Requesting user scores with URL: %s\n", user_url);

        chunk.memory = malloc(1);
        chunk.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, user_url);
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            printf("User Scores Response: %s\n", chunk.memory);

            struct json_object* parsed_json = json_tokener_parse(chunk.memory);
            if (parsed_json == NULL) {
                fprintf(stderr, "Failed to parse JSON response\n");
                return;
            }

            struct json_object* user_scores;
            if (json_object_object_get_ex(parsed_json, "userScores", &user_scores)) {
                int num_games = json_object_array_length(user_scores);

                for (int i = 0; i < num_games; i++) {
                    struct json_object* score_entry = json_object_array_get_idx(user_scores, i);
                    struct json_object* game;
                    struct json_object* high_score;

                    if (json_object_object_get_ex(score_entry, "game", &game) &&
                        json_object_object_get_ex(score_entry, "high_score", &high_score)) {
                        const char* game_name = json_object_get_string(game);
                        int score_value = json_object_get_int(high_score);

                        int game_column = 0;
                        if (strcmp(game_name, "tetris") == 0) {
                            game_column = 0;
                        } else if (strcmp(game_name, "2048") == 0) {
                            game_column = 4;
                        } else if (strcmp(game_name, "bp") == 0) {
                            game_column = 8;
                        } else if (strcmp(game_name, "mine") == 0) {
                            game_column = 12;
                        }

                        GtkWidget* user_score_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 2, 14);
                        if (GTK_IS_LABEL(user_score_label)) {
                            char user_score_text[16];
                            snprintf(user_score_text, sizeof(user_score_text), "%d", score_value);
                            gtk_label_set_text(GTK_LABEL(user_score_label), user_score_text);
                        }
                    }
                }
            }

            json_object_put(parsed_json);
        } else {
            fprintf(stderr, "Failed to fetch user scores: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        if (chunk.memory) {
            free(chunk.memory);
        }
    }
}

void on_refresh_button_clicked(GtkWidget* widget, gpointer grid) {
    update_score_table(GTK_WIDGET(grid));
}

GtkWidget* create_scoreboard_screen(GtkStack* stack) {
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(outer_box, GTK_ALIGN_CENTER);

    // 흰색 배경의 컨테이너 생성
    GtkWidget* white_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(white_container, 1000, -1);
    gtk_widget_set_margin_start(white_container, 30);
    gtk_widget_set_margin_end(white_container, 30);

    // CSS 스타일 적용
    GtkCssProvider* provider = gtk_css_provider_new();
    const char* css_data =
        ".white-container {"
        "   background: white;"
        "   border-radius: 5px;"
        "   box-shadow: 0 1px 4px rgba(0,0,0,0.15);"
        "   padding: 20px;"
        "}"
        "separator { background: #e0e0e0; }";

    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);

    // 스타일 적용
    GtkStyleContext* context = gtk_widget_get_style_context(white_container);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "white-container");

    g_object_unref(provider);

    // 타이틀
    GtkWidget* title_label = gtk_label_new(NULL);
    const char* markup = "<span font_desc='18' weight='bold'>Game Leaderboards</span>";
    gtk_label_set_markup(GTK_LABEL(title_label), markup);
    gtk_widget_set_margin_top(title_label, 30);
    gtk_widget_set_margin_bottom(title_label, 40);

    // 게임 그리드
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
    gtk_widget_set_margin_start(grid, 30);
    gtk_widget_set_margin_end(grid, 30);
    gtk_widget_set_margin_bottom(grid, 30);

    const char* game_names[] = {"Tetris", "2048", "Brick out", "Minesweeper"};
    const char* game_images[] = {
        "images/tetris.png",
        "images/2048.png",
        "images/breakout.png",
        "images/minesweeper.png"};

    // 게임별 섹션 생성
    for (int g = 0; g < 4; g++) {
        // 게임 이미지와 제목
        GtkWidget* game_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

        GtkWidget* image = gtk_image_new_from_file(game_images[g]);
        gtk_widget_set_size_request(image, 80, 80);

        GtkWidget* game_label = gtk_label_new(NULL);
        char* game_markup = g_markup_printf_escaped("<span font_desc='14' weight='bold'>%s</span>", game_names[g]);
        gtk_label_set_markup(GTK_LABEL(game_label), game_markup);
        g_free(game_markup);

        gtk_box_pack_start(GTK_BOX(game_box), image, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(game_box), game_label, FALSE, FALSE, 5);
        gtk_grid_attach(GTK_GRID(grid), game_box, g * 4, 0, 3, 2);

        // 헤더 (순위, 사용자명, 점수)
        const char* headers[] = {"Rank", "Username", "Score"};
        for (int h = 0; h < 3; h++) {
            GtkWidget* header = gtk_label_new(NULL);
            char* header_markup = g_markup_printf_escaped("<span weight='bold'>%s</span>", headers[h]);
            gtk_label_set_markup(GTK_LABEL(header), header_markup);
            g_free(header_markup);
            gtk_grid_attach(GTK_GRID(grid), header, g * 4 + h, 2, 1, 1);
        }

        // 순위표 항목들
        for (int i = 0; i < 10; i++) {
            char rank_text[4];
            snprintf(rank_text, sizeof(rank_text), "%d", i + 1);

            GtkWidget* rank = gtk_label_new(rank_text);
            GtkWidget* username = gtk_label_new("-");
            GtkWidget* score = gtk_label_new("0");

            gtk_grid_attach(GTK_GRID(grid), rank, g * 4, i + 3, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), username, g * 4 + 1, i + 3, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), score, g * 4 + 2, i + 3, 1, 1);
        }

        // 구분선
        GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_grid_attach(GTK_GRID(grid), separator, g * 4, 13, 3, 1);

        // 개인 최고 점수
        GtkWidget* best_score_label = gtk_label_new("Your Best Score:");
        gtk_widget_set_margin_top(best_score_label, 10);
        GtkWidget* best_score_value = gtk_label_new("0");
        gtk_widget_set_margin_top(best_score_value, 10);

        gtk_grid_attach(GTK_GRID(grid), best_score_label, g * 4, 14, 2, 1);
        gtk_grid_attach(GTK_GRID(grid), best_score_value, g * 4 + 2, 14, 1, 1);

        // 수직 구분선
        if (g < 3) {
            GtkWidget* vsep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
            gtk_grid_attach(GTK_GRID(grid), vsep, g * 4 + 3, 0, 1, 15);
        }
    }

    // 버튼 박스
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(button_box, 20);
    gtk_widget_set_margin_bottom(button_box, 30);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    GtkWidget* refresh_button = gtk_button_new_with_label("Refresh Scores");

    gtk_widget_set_size_request(back_button, 175, 45);
    gtk_widget_set_size_request(refresh_button, 175, 45);

    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_box), refresh_button, FALSE, FALSE, 5);

    // 컨테이너에 위젯 추가
    gtk_box_pack_start(GTK_BOX(white_container), title_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), grid, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), button_box, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(outer_box), white_container, FALSE, FALSE, 0);

    // 시그널 연결
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), grid);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);

    return outer_box;
}
