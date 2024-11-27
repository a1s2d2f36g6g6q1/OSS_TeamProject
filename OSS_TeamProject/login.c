#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include <stdlib.h>
#include <curl.h>
#include <json.h>
#include <stdbool.h>
#include <string.h>
#include "games.h"

bool is_guest_mode = false;
char username[50];

void handle_login_success(GtkStack* stack) {
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

void on_guest_button_clicked(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);

    // 게스트 모드 활성화
    is_guest_mode = true;
    strncpy(username, "Guest", sizeof(username) - 1); // 게스트 유저 이름 설정
    username[sizeof(username) - 1] = '\0'; // Null-terminate

    printf("Guest mode activated. Username: %s\n", username);

    // 메인 메뉴로 전환
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

size_t write_callback(void* data, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    struct Memory* mem = (struct Memory*)userp;

    char* ptr = realloc(mem->response, mem->size + real_size + 1);
    if (ptr == NULL) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, real_size);
    mem->size += real_size;
    mem->response[mem->size] = '\0';

    return real_size;
}

void switch_to_login(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "login_screen");
}

void send_login_request(const char* input_username, const char* password, GtkWidget* result_label, GtkStack* stack) {
    CURL* curl;
    CURLcode res;
    struct curl_slist* headers = NULL;
    struct Memory chunk = { NULL, 0 };

    curl = curl_easy_init();
    if (curl) {
        // JSON 데이터 생성
        struct json_object* json_data = json_object_new_object();
        json_object_object_add(json_data, "username", json_object_new_string(input_username));
        json_object_object_add(json_data, "password", json_object_new_string(password));
        const char* json_string = json_object_to_json_string(json_data);

        printf("JSON Sent: %s\n", json_string); // 디버깅: 전송되는 JSON 출력

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.137.1:5000/auth/login");
        // 경로 확인
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            gtk_label_set_text(GTK_LABEL(result_label), "Network error!");
        }
        else {
            printf("Server Response: %s\n", chunk.response); // 디버깅: 서버 응답 출력

            struct json_object* parsed_json = json_tokener_parse(chunk.response);
            struct json_object* success;
            struct json_object* message;

            if (json_object_object_get_ex(parsed_json, "success", &success) &&
                json_object_get_boolean(success)) {
                gtk_label_set_text(GTK_LABEL(result_label), "Login successful!");
                //is_guest_mode = false;

                // 유저네임을 전역 변수에 저장
                strncpy(username, input_username, sizeof(username) - 1);
                username[sizeof(username) - 1] = '\0'; // Null-terminate
                printf("Logged in username: %s\n", username);

                handle_login_success(stack);
            }
            else if (json_object_object_get_ex(parsed_json, "message", &message)) {
                gtk_label_set_text(GTK_LABEL(result_label), json_object_get_string(message));
            }
            else {
                gtk_label_set_text(GTK_LABEL(result_label), "Invalid response from server!");
            }
            json_object_put(parsed_json);
        }

        json_object_put(json_data);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.response);
    }
}

void on_login_button_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget** widgets = (GtkWidget**)data;
    GtkEntry* username_entry = GTK_ENTRY(widgets[0]);
    GtkEntry* password_entry = GTK_ENTRY(widgets[1]);
    GtkLabel* result_label = GTK_LABEL(widgets[2]);
    GtkStack* stack = GTK_STACK(widgets[3]);

    char* username = gtk_entry_get_text(username_entry);
    const char* password = gtk_entry_get_text(password_entry);

    if (strlen(username) == 0 || strlen(password) == 0) {
        gtk_label_set_text(result_label, "Username and password required!");
        return;
    }

    send_login_request(username, password, GTK_WIDGET(result_label), stack);
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

    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), widgets);
    g_signal_connect(guest_button, "clicked", G_CALLBACK(on_guest_button_clicked), stack);

    return outer_box;
}