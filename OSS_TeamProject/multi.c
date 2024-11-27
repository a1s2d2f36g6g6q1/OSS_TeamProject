#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <WinSock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include <curl.h>
#include <gtk/gtk.h>
#include <json.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "games.h"

// 글로벌 변수
SOCKET global_socket = INVALID_SOCKET;
GtkWidget* response_label;

// 소켓 초기화 및 종료
void initialize_sockets() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        exit(1);
    }
#endif
}

void cleanup_sockets() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// 소켓 생성 및 연결
SOCKET initialize_socket(const char* server_ip, int server_port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        perror("Socket creation failed");
        return INVALID_SOCKET;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        closesocket(s);
        return INVALID_SOCKET;
    }

    printf("Socket initialized and connected successfully.\n");
    return s;
}

// JSON 요청 전송 및 응답 처리
int send_json_request(SOCKET s, const char* endpoint, const char* json_data) {
    char request[2048];
    snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        endpoint, strlen(json_data), json_data);

    return send(s, request, strlen(request), 0);
}

void receive_response(SOCKET s, char* response, size_t response_size) {
    int recv_size = recv(s, response, response_size - 1, 0);
    if (recv_size <= 0) {
        perror("Receive failed");
        return;
    }

    response[recv_size] = '\0';
    printf("Server response: %s\n", response);
}

// 버튼 콜백 함수
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
    char response[4096];
    GtkEntry* join_entry = GTK_ENTRY(data);
    const char* room_id_str = gtk_entry_get_text(join_entry);
    int room_id = atoi(room_id_str); // 입력된 Room ID를 정수로 변환

    struct json_object* json = json_object_new_object();
    json_object_object_add(json, "action", json_object_new_string("join"));
    json_object_object_add(json, "roomId", json_object_new_int(room_id));

    const char* json_str = json_object_to_json_string(json);
    send_json_request(global_socket, "/game/join", json_str);

    receive_response(global_socket, response, sizeof(response));
    gtk_label_set_text(GTK_LABEL(response_label), response); // 응답을 GTK 라벨에 출력
    json_object_put(json);
}

void on_send_message(GtkWidget* widget, gpointer data) {
    // Send message logic
}

// 화면 생성 함수
GtkWidget* create_multi_screen(GtkStack* stack) {
    global_socket = initialize_socket("192.168.137.1", 5000);
    if (global_socket == INVALID_SOCKET) {
        printf("Failed to connect to server.\n");
        return NULL;
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

    GtkWidget* join_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(join_entry), "Enter Room ID");
    gtk_box_pack_start(GTK_BOX(vbox), join_entry, FALSE, FALSE, 0);

    GtkWidget* join_button = gtk_button_new_with_label("Join Room");
    g_signal_connect(join_button, "clicked", G_CALLBACK(on_join_room), join_entry);
    gtk_box_pack_start(GTK_BOX(vbox), join_button, FALSE, FALSE, 0);

    GtkWidget* message_entry = gtk_entry_new();
    GtkWidget* room_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Enter Message");
    gtk_entry_set_placeholder_text(GTK_ENTRY(room_entry), "Enter Room ID");
    gtk_box_pack_start(GTK_BOX(vbox), message_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), room_entry, FALSE, FALSE, 0);

    response_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(vbox), response_label, FALSE, FALSE, 0);

    return outer_box;
}