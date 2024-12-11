#define _CRT_SECURE_NO_WARNINGS
#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>

#include "games.h"

size_t write_response(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total_size = size * nmemb;
    if (strlen(userdata) + total_size >= 1024) {
        g_print("Error: Response buffer overflow.\n");
        return 0;
    }
    strcat(userdata, (char*)ptr);
    return total_size;
}

void on_check_username_button_clicked(GtkWidget* widget, gpointer data) {
    const char* username = gtk_entry_get_text(GTK_ENTRY(data));

    if (strlen(username) == 0) {
        g_print("Error: Username cannot be empty.\n");
        return;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        char url[256];
        sprintf(url, "http://localhost:5000/auth/check-username?username=%s", username);

        char response[1024] = "";

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        g_print("Sending request to: %s\n", url);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            g_print("Error: Failed to check username. CURLcode: %d\n", res);
        } else {
            g_print("Server response: %s\n", response);
            struct json_object* parsed_json = json_tokener_parse(response);
            struct json_object* success;
            struct json_object* message;

            if (parsed_json != NULL) {
                json_object_object_get_ex(parsed_json, "success", &success);
                json_object_object_get_ex(parsed_json, "message", &message);

                if (json_object_get_boolean(success)) {
                    g_print("%s\n", json_object_get_string(message));
                } else {
                    g_print("%s\n", json_object_get_string(message));
                }

                json_object_put(parsed_json);
            } else {
                g_print("Error: Failed to parse server response.\n");
            }
        }

        curl_easy_cleanup(curl);
    }
}

void send_register_request(const char* input_username, const char* password, GtkWidget* result_label, GtkStack* stack) {
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
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:5000/auth/register");
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
                gtk_label_set_text(GTK_LABEL(result_label), "Registration successful!");
                strncpy(username, input_username, sizeof(username) - 1);
                username[sizeof(username) - 1] = '\0';  // Null-terminate

                printf("Registered username: %s\n", username);
                gtk_stack_set_visible_child_name(stack, "login_screen");
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

void on_register_button_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget** widgets = (GtkWidget**)data;
    GtkEntry* username_entry = GTK_ENTRY(widgets[0]);
    GtkEntry* password_entry = GTK_ENTRY(widgets[1]);
    GtkEntry* confirm_entry = GTK_ENTRY(widgets[2]);
    GtkLabel* result_label = GTK_LABEL(widgets[3]);
    GtkStack* stack = GTK_STACK(widgets[4]);

    const char* username = gtk_entry_get_text(username_entry);
    const char* password = gtk_entry_get_text(password_entry);
    const char* confirm_password = gtk_entry_get_text(confirm_entry);

    if (strlen(username) == 0 || strlen(password) == 0 || strlen(confirm_password) == 0) {
        gtk_label_set_text(result_label, "All fields are required!");
        return;
    }

    if (strcmp(password, confirm_password) != 0) {
        gtk_label_set_text(result_label, "Passwords do not match!");
        return;
    }

    send_register_request(username, password, GTK_WIDGET(result_label), stack);
}

GtkWidget* create_signup_screen(GtkStack* stack) {
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(outer_box, GTK_ALIGN_CENTER);

    // 흰색 배경의 컨테이너 생성
    GtkWidget* white_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(white_container, 460, -1);
    gtk_widget_set_margin_start(white_container, 30);
    gtk_widget_set_margin_end(white_container, 30);

    // REGISTER 타이틀
    GtkWidget* title_label = gtk_label_new(NULL);
    const char* markup = "<span font_desc='18' weight='bold'>Register</span>";
    gtk_label_set_markup(GTK_LABEL(title_label), markup);
    gtk_widget_set_margin_bottom(title_label, 80);

    // 입력 필드들을 담을 박스
    GtkWidget* entries_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_bottom(entries_box, 25);

    // 입력 필드들
    GtkWidget* username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");
    gtk_widget_set_size_request(username_entry, -1, 40);
    gtk_widget_set_margin_bottom(username_entry, 15);
    gtk_widget_set_margin_start(username_entry, 30);
    gtk_widget_set_margin_end(username_entry, 30);

    GtkWidget* password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_widget_set_size_request(password_entry, -1, 40);
    gtk_widget_set_margin_bottom(password_entry, 15);
    gtk_widget_set_margin_start(password_entry, 30);
    gtk_widget_set_margin_end(password_entry, 30);

    GtkWidget* confirm_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_entry), "Confirm Password");
    gtk_entry_set_visibility(GTK_ENTRY(confirm_entry), FALSE);
    gtk_widget_set_size_request(confirm_entry, -1, 40);
    gtk_widget_set_margin_bottom(confirm_entry, 15);
    gtk_widget_set_margin_start(confirm_entry, 30);
    gtk_widget_set_margin_end(confirm_entry, 30);

    // Sign up 버튼
    GtkWidget* register_button = gtk_button_new_with_label("Sign up");
    gtk_widget_set_size_request(register_button, -1, 45);
    gtk_widget_set_margin_bottom(register_button, 10);
    gtk_widget_set_margin_start(register_button, 30);
    gtk_widget_set_margin_end(register_button, 30);

    // Back to Login 버튼
    GtkWidget* back_button = gtk_button_new_with_label("Back to Login");
    gtk_widget_set_size_request(back_button, -1, 45);
    gtk_widget_set_margin_start(back_button, 30);
    gtk_widget_set_margin_end(back_button, 30);

    // 결과 라벨
    GtkWidget* result_label = gtk_label_new("");
    gtk_widget_set_margin_top(result_label, 15);

    // 입력 필드들을 entries_box에 추가
    gtk_box_pack_start(GTK_BOX(entries_box), username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(entries_box), password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(entries_box), confirm_entry, FALSE, FALSE, 0);

    // 위젯들을 컨테이너에 추가
    gtk_box_pack_start(GTK_BOX(white_container), title_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), entries_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), register_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), back_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(white_container), result_label, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(outer_box), white_container, FALSE, FALSE, 0);

    // CSS 스타일 적용
    GtkCssProvider* provider = gtk_css_provider_new();
    GError* error = NULL;
    const gchar* css_data =
        ".error-message { "
        "   color: #ff0000; "
        "   font-size: 18px; "
        "}"
        "button { "
        "   border-radius: 8px; "
        "   background: white; "
        "   border: 1px solid #e0e0e0; "
        "   font-size: 18px; "
        "   min-height: 45px; "
        "}"
        "button:hover { background: #f8f9fa; }"
        "#register-button { "
        "   background: #1a1a1a; "
        "   color: white; "
        "   border: none; "
        "   font-weight: bold; "
        "}"
        "#register-button:hover { background: #333; }"
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
        "}";

    gtk_css_provider_load_from_data(provider, css_data, -1, &error);

    if (error != NULL) {
        g_warning("CSS 로딩 실패: %s", error->message);
        g_error_free(error);
    }

    // 스타일 적용
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(white_container);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(context, "white-container");

    context = gtk_widget_get_style_context(register_button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_widget_set_name(register_button, "register-button");

    // 결과 라벨에 스타일 적용
    GtkStyleContext* result_context = gtk_widget_get_style_context(result_label);
    gtk_style_context_add_provider(result_context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_class(result_context, "error-message");

    g_object_unref(provider);

    // 시그널 연결
    GtkWidget** widgets = g_new(GtkWidget*, 5);
    widgets[0] = username_entry;
    widgets[1] = password_entry;
    widgets[2] = confirm_entry;
    widgets[3] = result_label;
    widgets[4] = GTK_WIDGET(stack);

    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), widgets);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_login), stack);

    return outer_box;
}
