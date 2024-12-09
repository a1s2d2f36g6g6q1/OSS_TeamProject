#include <gtk/gtk.h>
#include "games.h"

// ȸ������ ��ư Ŭ�� �ڵ鷯
void on_register_button_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget** entries = (GtkWidget**)data;
    const char* username = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const char* password = gtk_entry_get_text(GTK_ENTRY(entries[1]));
    const char* confirm_password = gtk_entry_get_text(GTK_ENTRY(entries[2]));

    // �Է� ����
    if (strlen(username) == 0) {
        g_print("Error: Username cannot be empty.\n");
        return;
    }
    if (strlen(password) == 0) {
        g_print("Error: Password cannot be empty.\n");
        return;
    }
    if (strcmp(password, confirm_password) != 0) {
        g_print("Error: Passwords do not match.\n");
        return;
    }

    // ȸ������ ���� ó�� (��: ����� ������ ����)
    g_print("Registration Successful! Username: %s\n", username);
    // �����δ� �����ͺ��̽� ���� ���� �߰�
}

// ȸ������ ������ ���� �Լ�
GtkWidget* create_signup_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // ������ ����
    GtkWidget* title_label = gtk_label_new("Register");
    gtk_widget_set_margin_bottom(title_label, 20);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    // ����� �̸� �Է¶�
    GtkWidget* username_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* username_label = gtk_label_new("Username:");
    GtkWidget* username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter your username");
    gtk_box_pack_start(GTK_BOX(username_box), username_label, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(username_box), username_entry, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), username_box, FALSE, FALSE, 5);

    // ��й�ȣ �Է¶�
    GtkWidget* password_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* password_label = gtk_label_new("Password:");
    GtkWidget* password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE); // ��й�ȣ ����
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter your password");
    gtk_box_pack_start(GTK_BOX(password_box), password_label, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(password_box), password_entry, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), password_box, FALSE, FALSE, 5);

    // ��й�ȣ Ȯ�� �Է¶�
    GtkWidget* confirm_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* confirm_label = gtk_label_new("Confirm Password:");
    GtkWidget* confirm_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(confirm_entry), FALSE); // ��й�ȣ ����
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_entry), "Re-enter your password");
    gtk_box_pack_start(GTK_BOX(confirm_box), confirm_label, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(confirm_box), confirm_entry, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), confirm_box, FALSE, FALSE, 5);

    // ��ư �ڽ�
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);

    // ȸ������ ��ư
    GtkWidget* register_button = gtk_button_new_with_label("Register");
    GtkWidget* entries[3] = { username_entry, password_entry, confirm_entry };
    g_signal_connect(register_button, "clicked", G_CALLBACK(on_register_button_clicked), entries);
    gtk_box_pack_start(GTK_BOX(button_box), register_button, FALSE, FALSE, 5);

    // ���� �޴��� ���ư��� ��ư
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);
    gtk_box_pack_start(GTK_BOX(button_box), back_button, FALSE, FALSE, 5);

    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 10);

    return vbox;
}
