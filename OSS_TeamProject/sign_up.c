#define _CRT_SECURE_NO_WARNINGS
#include <gtk/gtk.h>
#include "games.h"
#include <json.h>
#include <curl.h>

// 서버 응답 처리 함수
size_t write_response(void* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total_size = size * nmemb;
    if (strlen(userdata) + total_size >= 1024) {
        g_print("Error: Response buffer overflow.\n");
        return 0;
    }
    strcat(userdata, (char*)ptr);
    return total_size;
}

// 닉네임 중복 확인 버튼 클릭 핸들러
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
        }
        else {
            g_print("Server response: %s\n", response);
            struct json_object* parsed_json = json_tokener_parse(response);
            struct json_object* success;
            struct json_object* message;

            if (parsed_json != NULL) {
                json_object_object_get_ex(parsed_json, "success", &success);
                json_object_object_get_ex(parsed_json, "message", &message);

                if (json_object_get_boolean(success)) {
                    g_print("%s\n", json_object_get_string(message));
                }
                else {
                    g_print("%s\n", json_object_get_string(message));
                }

                json_object_put(parsed_json);
            }
            else {
                g_print("Error: Failed to parse server response.\n");
            }
        }

        curl_easy_cleanup(curl);
    }
}

void send_register_request(const char* input_username, const char* password, GtkWidget* result_label, GtkStack* stack) {

}


void on_register_button_clicked(GtkWidget* widget, gpointer data) {

}



GtkWidget* create_signup_screen(GtkStack* stack) {
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(outer_box, GTK_ALIGN_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_spacing(GTK_BOX(vbox), 10);
    gtk_box_pack_start(GTK_BOX(outer_box), vbox, FALSE, FALSE, 0);

    GtkWidget* title_label = gtk_label_new("REGISTER");
    gtk_widget_set_halign(title_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    GtkWidget* username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Username");
    gtk_box_pack_start(GTK_BOX(vbox), username_entry, FALSE, FALSE, 5);

    GtkWidget* password_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Password");
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), password_entry, FALSE, FALSE, 5);

    GtkWidget* confirm_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_entry), "Confirm Password");
    gtk_entry_set_visibility(GTK_ENTRY(confirm_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), confirm_entry, FALSE, FALSE, 5);

    GtkWidget* register_button = gtk_button_new_with_label("Sign up");
    gtk_box_pack_start(GTK_BOX(vbox), register_button, FALSE, FALSE, 5);

    GtkWidget* back_button = gtk_button_new_with_label("Back to Login");
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 5);

    GtkWidget* result_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), result_label, FALSE, FALSE, 5);

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
