#include <gtk/gtk.h>
#include <curl.h>
#include <json.h>
#include "games.h"

// 서버 응답 데이터를 저장할 구조체
struct MemoryStruct {
    char* memory;
    size_t size;
};

// 서버 응답을 저장하기 위한 콜백 함수
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // 메모리 할당 실패
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void update_score_table(GtkWidget * grid) {
    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);  // 초기 메모리 할당
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        // 1등부터 10등까지의 점수 가져오기
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
                            game_column = 3;
                        }
                        else if (strcmp(game_name, "2048") == 0) {
                            game_column = 6;
                        }
                        else if (strcmp(game_name, "bp") == 0) {
                            game_column = 9;
                        }

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

            json_object_put(parsed_json);
        }
        else {
            fprintf(stderr, "Failed to fetch scores: %s\n", curl_easy_strerror(res));
        }

        // 사용자 자신의 최고 점수 가져오기
        char user_url[256];
        snprintf(user_url, sizeof(user_url), "http://localhost:5000/auth/get-user-scores?username=%s", username);
        printf("Requesting user scores with URL: %s\n", user_url);

        chunk.memory = malloc(1);  // 메모리 재할당
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
                            game_column = 3;
                        }
                        else if (strcmp(game_name, "2048") == 0) {
                            game_column = 6;
                        }
                        else if (strcmp(game_name, "bp") == 0) {
                            game_column = 9;
                        }

                        GtkWidget* user_score_label = gtk_grid_get_child_at(GTK_GRID(grid), game_column + 2, 13);
                        if (user_score_label) {
                            char user_score_text[16];
                            snprintf(user_score_text, sizeof(user_score_text), "%d", score_value);
                            gtk_label_set_text(GTK_LABEL(user_score_label), user_score_text);
                        }
                        else {
                            printf("User score label not found for game: %s\n", game_name);
                        }
                    }
                }
            }

            json_object_put(parsed_json);
        }
        else {
            fprintf(stderr, "Failed to fetch user scores: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        if (chunk.memory) {
            free(chunk.memory);
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
        GtkWidget* game_label = gtk_label_new("game_name_placeholder");
        gtk_grid_attach(GTK_GRID(grid), game_label, g * 3, 0, 3, 1); // 각 게임 이름을 3칸에 걸쳐 표시

        // 열 제목
        GtkWidget* rank_label = gtk_label_new("Rank");
        GtkWidget* username_label = gtk_label_new("Username");
        GtkWidget* score_label = gtk_label_new("Score");
        gtk_grid_attach(GTK_GRID(grid), rank_label, g * 3, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), username_label, g * 3 + 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), score_label, g * 3 + 2, 1, 1, 1);

        // 순위 데이터 초기화 (플레이스홀더)
        for (int i = 0; i < 10; i++) {
            char rank_text[4];
            snprintf(rank_text, sizeof(rank_text), "%d", i + 1);

            GtkWidget* rank = gtk_label_new(rank_text);
            GtkWidget* username = gtk_label_new("username_placeholder");
            char score_text[16];
            snprintf(score_text, sizeof(score_text), "%d", 0); // 실제 점수는 서버에서 받아옴
            GtkWidget* score = gtk_label_new(score_text);

            gtk_grid_attach(GTK_GRID(grid), rank, g * 3, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), username, g * 3 + 1, i + 2, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), score, g * 3 + 2, i + 2, 1, 1);
        }

        // 사용자 점수 추가 공간
        GtkWidget* your_score_label = gtk_label_new("Your Best Score:");
        GtkWidget* your_score_value = gtk_label_new("0");

        gtk_grid_attach(GTK_GRID(grid), your_score_label, g * 3, 13, 2, 1); // 사용자 점수 레이블
        gtk_grid_attach(GTK_GRID(grid), your_score_value, g * 3 + 2, 13, 1, 1); // 사용자 점수 값
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
