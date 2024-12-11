#define _CRT_SECURE_NO_WARNINGS
#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "games.h"

bool is_guest_mode = false;
char username[50];

void switch_to_register(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);
    gtk_stack_set_visible_child_name(stack, "signup_screen");
}

void handle_login_success(GtkStack* stack) {
    gtk_stack_set_visible_child_name(stack, "main_menu");
}

void on_guest_button_clicked(GtkWidget* widget, gpointer data) {
    GtkStack* stack = GTK_STACK(data);

    is_guest_mode = true;
    strncpy(username, "Guest", sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';

    printf("Guest mode activated. Username: %s\n", username);

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
    struct Memory chunk = {NULL, 0};

    curl = curl_easy_init();
    if (curl) {
        struct json_object* json_data = json_object_new_object();
        json_object_object_add(json_data, "username", json_object_new_string(input_username));
        json_object_object_add(json_data, "password", json_object_new_string(password));
        const char* json_string = json_object_to_json_string(json_data);

        printf("JSON Sent: %s\n", json_string);

        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/auth/login");

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            gtk_label_set_text(GTK_LABEL(result_label), "Network error!");
        } else {
            printf("Server Response: %s\n", chunk.response);

            struct json_object* parsed_json = json_tokener_parse(chunk.response);
            struct json_object* success;
            struct json_object* message;

            if (json_object_object_get_ex(parsed_json, "success", &success) &&
                json_object_get_boolean(success)) {
                gtk_label_set_text(GTK_LABEL(result_label), "Login successful!");
                // is_guest_mode = false;

                strncpy(username, input_username, sizeof(username) - 1);
                username[sizeof(username) - 1] = '\0';  // Null-terminate
                printf("Logged in username: %s\n", username);

                handle_login_success(stack);
            } else if (json_object_object_get_ex(parsed_json, "message", &message)) {
                gtk_label_set_text(GTK_LABEL(result_label), json_object_get_string(message));
            } else {
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

    const char* username = gtk_entry_get_text(username_entry);
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

    // 흰색 배경의 컨테이너 생성
    GtkWidget* white_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);  // 컨테이너 내 여백 0으로 설정
    gtk_widget_set_size_request(white_container, 460, -1);                  // 컨테이너 크기 설정
    gtk_widget_set_margin_start(white_container, 30);                       // 왼쪽 여백 추가
    gtk_widget_set_margin_end(white_container, 30);                         // 오른쪽 여백 추가

    // LOGIN 타이틀
    GtkWidget* title_label = gtk_label_new(NULL);
    const char* markup = "<span font_desc='18' weight='bold'>LOGIN</span>";
    gtk_label_set_markup(GTK_LABEL(title_label), markup);
    gtk_widget_set_margin_bottom(title_label, 80);

    // 입력 필드들
    GtkWidget* username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "ID");
    gtk_widget_set_size_request(username_entry, -1, 40);
    gtk_widget_set_margin_bottom(username_entry, 15);
    // 입력 필드 좌우 여백 추가
    gtk_widget_set_margin_start(username_entry, 30);
    gtk_widget_set_margin_end(username_entry, 30);

    GtkWidget* password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_widget_set_size_request(password_entry, -1, 40);
    gtk_widget_set_margin_bottom(password_entry, 40);
    // 입력 필드 좌우 여백 추가
    gtk_widget_set_margin_start(password_entry, 30);
    gtk_widget_set_margin_end(password_entry, 30);

    // Sign in 버튼
    GtkWidget* login_button = gtk_button_new_with_label("Sign in");
    gtk_widget_set_size_request(login_button, -1, 45);
    gtk_widget_set_margin_bottom(login_button, 10);
    // 버튼 좌우 여백 추가
    gtk_widget_set_margin_start(login_button, 30);
    gtk_widget_set_margin_end(login_button, 30);

    // Guest와 Sign up 버튼을 위한 수평 박스
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    // 버튼 좌우 여백 추가
    gtk_widget_set_margin_start(button_box, 30);
    gtk_widget_set_margin_end(button_box, 30);

    GtkWidget* guest_button = gtk_button_new_with_label("Guest");
    GtkWidget* sign_up_button = gtk_button_new_with_label("Sign up");

    // 버튼 크기 조정
    gtk_widget_set_size_request(guest_button, 175, 45);
    gtk_widget_set_size_request(sign_up_button, 175, 45);

    gtk_box_pack_start(GTK_BOX(button_box), guest_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), sign_up_button, TRUE, TRUE, 0);

    // 결과 라벨
    GtkWidget* result_label = gtk_label_new("");
    gtk_widget_set_margin_top(result_label, 15);

    // 위젯들을 컨테이너에 추가
    gtk_box_pack_start(GTK_BOX(white_container), title_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), login_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), button_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), result_label, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(outer_box), white_container, FALSE, FALSE, 0);

    // CSS 스타일 적용
    GtkCssProvider* provider = gtk_css_provider_new();
    GError* error = NULL;
    const gchar* css_data =
        "button { "
        "   border-radius: 8px; "
        "   background: white; "
        "   border: 1px solid #e0e0e0; "
        "   font-size: 18px; "
        "   min-height: 45px; "
        "}"
        "button:hover { background: #f8f9fa; }"
        "#login-button { "
        "   background: #1a1a1a; "
        "   color: white; "
        "   border: none; "
        "   font-weight: bold; "
        "}"
        "#login-button:hover { background: #333; }"
        "entry { "
        "   border-radius: 8px; "
        "   border: 1px solid #e0e0e0; "
        "   padding: 12px; "
        "   font-size: 14px; "
        "   min-height: 20px; "
        "   margin: 5px 0; "
        "}"
        "entry:focus { "
        "   border-color: #999; "
        "   outline: none; "
        "}"
        ".white-container { "
        "   background: white; "
        "   border-radius: 5px; "
        "   box-shadow: 0 1px 4px rgba(0,0,0,0.15); "  // 그림자 크기와 투명도
        "   padding-top: 30px; "                       // 위 여백 추가
        "   padding-bottom: 30px; "                    // 아래 여백 추가
        "}"
        ".error-message { "
        "   color: #ff0000; "  // 빨간색으로 설정
        "   font-size: 14px; "
        "}";

    // 결과 라벨에 스타일 적용
    GtkStyleContext* result_context = gtk_widget_get_style_context(result_label);
    gtk_style_context_add_provider(result_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(result_context, "error-message");

    gtk_css_provider_load_from_data(provider, css_data, -1, &error);

    if (error != NULL) {
        g_warning("CSS 로딩 실패: %s", error->message);
        g_error_free(error);
    }

    GtkStyleContext* context;
    context = gtk_widget_get_style_context(white_container);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "white-container");

    context = gtk_widget_get_style_context(login_button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_widget_set_name(login_button, "login-button");

    g_object_unref(provider);

    // 시그널 연결
    GtkWidget** widgets = g_new(GtkWidget*, 4);
    widgets[0] = username_entry;
    widgets[1] = password_entry;
    widgets[2] = result_label;
    widgets[3] = GTK_WIDGET(stack);

    g_signal_connect(login_button, "clicked", G_CALLBACK(on_login_button_clicked), widgets);
    g_signal_connect(guest_button, "clicked", G_CALLBACK(on_guest_button_clicked), stack);
    g_signal_connect(sign_up_button, "clicked", G_CALLBACK(switch_to_register), stack);

    return outer_box;
}
