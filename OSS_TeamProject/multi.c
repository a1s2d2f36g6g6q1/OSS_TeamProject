#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WinSock2.h>
#include "games.h"

#pragma comment(lib, "ws2_32.lib")

typedef struct {
    GtkWidget* room_entry;
    GtkWidget* message_entry;
} EntryData;

SOCKET global_socket;
GtkWidget* response_label;

int send_json_request(SOCKET s, const char* endpoint, const char* json_data) {
    char request[2048];
    snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s",
        endpoint, strlen(json_data), json_data);

    return send(s, request, strlen(request), 0);
}

void receive_response(SOCKET s, char* response, size_t response_size) {
    if (s == INVALID_SOCKET) {
        printf("Socket is not valid. Please reinitialize the socket.\n");
        return;
    }

    int recv_size = recv(s, response, response_size - 1, 0);
    if (recv_size == 0) {
        printf("Server closed the connection.\n");
        return;
    }
    else if (recv_size == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        printf("Receive failed. Error Code: %d\n", error_code);

        if (error_code == WSAENOTSOCK) {
            printf("Socket is not a valid socket. Please check initialization.\n");
        }
        return;
    }

    response[recv_size] = '\0';
    printf("Server response: %s\n", response);
}

SOCKET initialize_socket(const char* server_ip, int server_port) {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(s);
        return INVALID_SOCKET;
    }

    printf("Socket initialized and connected successfully.\n");
    return s;
}



void on_create_room(GtkWidget* widget, gpointer data) {
    char response[4096];

    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "action", json_object_new_string("create"));
    json_object_object_add(json, "name", json_object_new_string("My Room"));

    const char* json_str = json_object_to_json_string(json);
    send_json_request(global_socket, "/game/create", json_str);

    receive_response(global_socket, response, sizeof(response));
    gtk_label_set_text(GTK_LABEL(response_label), response);
    json_object_put(json);
}


void on_join_room(GtkWidget* widget, gpointer data) {
  
}

void on_send_message(GtkWidget* widget, gpointer data) {
   
}



GtkWidget* create_multi_screen(GtkStack* stack) {
    global_socket = initialize_socket("172.30.152.50", 5000);
    if (global_socket == INVALID_SOCKET) {
        printf("Failed to connect to server.\n");
        return NULL; // NULL 반환으로 잘못된 값 방지
    }

    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(outer_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(outer_box, GTK_ALIGN_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_box_set_spacing(GTK_BOX(vbox), 10);
    gtk_box_pack_start(GTK_BOX(outer_box), vbox, FALSE, FALSE, 0);

    GtkWidget* create_button = gtk_button_new_with_label("Create Room");
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_room), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), create_button, FALSE, FALSE, 0);

    // 방 참여
    GtkWidget* join_entry = gtk_entry_new();
    if (!GTK_IS_ENTRY(join_entry)) {
        printf("Error: Join entry is not valid.\n");
        return NULL;
    }
    gtk_entry_set_placeholder_text(GTK_ENTRY(join_entry), "Enter Room ID");
    gtk_box_pack_start(GTK_BOX(vbox), join_entry, FALSE, FALSE, 0);

    GtkWidget* join_button = gtk_button_new_with_label("Join Room");
    g_signal_connect(join_button, "clicked", G_CALLBACK(on_join_room), join_entry);
    gtk_box_pack_start(GTK_BOX(vbox), join_button, FALSE, FALSE, 0);

    // 메시지 전송
    GtkWidget* message_entry = gtk_entry_new();
    GtkWidget* room_entry = gtk_entry_new();
    if (!GTK_IS_ENTRY(message_entry) || !GTK_IS_ENTRY(room_entry)) {
        printf("Error: One or both message entries are not valid.\n");
        return NULL;
    }

    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Enter Message");
    gtk_entry_set_placeholder_text(GTK_ENTRY(room_entry), "Enter Room ID");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), room_entry, FALSE, FALSE, 0);

    EntryData* entry_data = g_new(EntryData, 1);
    entry_data->room_entry = room_entry;
    entry_data->message_entry = message_entry;

    GtkWidget* send_button = gtk_button_new_with_label("Send Message");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message), entry_data);
    gtk_box_pack_start(GTK_BOX(vbox), send_button, FALSE, FALSE, 0);

    response_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), response_label, FALSE, FALSE, 0);

    return outer_box;
}
