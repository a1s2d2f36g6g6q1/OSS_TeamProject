#include <gtk/gtk.h>
#include <curl.h>
#include "games.h"

// Mock: Global variables for effect and BGM volume
int effect_volume = 50; // Default effect volume
int bgm_volume = 50;    // Default BGM volume

// Callback for effect volume change
void on_effect_volume_changed(GtkRange* range, gpointer data) {
    effect_volume = gtk_range_get_value(range);
    g_print("Effect Volume: %d\n", effect_volume);
    // Additional effect volume change handling can be added here
}

// Callback for BGM volume change
void on_bgm_volume_changed(GtkRange* range, gpointer data) {
    bgm_volume = gtk_range_get_value(range);
    g_print("BGM Volume: %d\n", bgm_volume);
    // Additional BGM volume change handling can be added here
}

// Callback for delete account button
void on_delete_account_clicked(GtkButton* button, gpointer data) {
    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        char url[256];
        snprintf(url, sizeof(url), "http://localhost:5000/auth/delete-account?username=%s", username);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            g_print("Failed to delete account: %s\n", curl_easy_strerror(res));
        }
        else {
            g_print("Account deleted successfully.\n");
            // Optional: Redirect to login screen or exit the app
            exit(0);
        }
        curl_easy_cleanup(curl);
    }
}

// Function to create the settings screen
GtkWidget* create_setting_screen(GtkStack* stack) {
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(vbox, GTK_ALIGN_CENTER);

    // Title label
    GtkWidget* title_label = gtk_label_new("Settings");
    gtk_widget_set_margin_bottom(title_label, 20);
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    // Effect Volume slider
    GtkWidget* effect_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* effect_label = gtk_label_new("Effect Volume:");
    GtkWidget* effect_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(effect_scale), effect_volume);
    gtk_widget_set_size_request(effect_scale, 300, -1);
    g_signal_connect(effect_scale, "value-changed", G_CALLBACK(on_effect_volume_changed), NULL);
    gtk_box_pack_start(GTK_BOX(effect_box), effect_label, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(effect_box), effect_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), effect_box, FALSE, FALSE, 5);

    // BGM Volume slider
    GtkWidget* bgm_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* bgm_label = gtk_label_new("BGM Volume:");
    GtkWidget* bgm_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(bgm_scale), bgm_volume);
    gtk_widget_set_size_request(bgm_scale, 300, -1);
    g_signal_connect(bgm_scale, "value-changed", G_CALLBACK(on_bgm_volume_changed), NULL);
    gtk_box_pack_start(GTK_BOX(bgm_box), bgm_label, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(bgm_box), bgm_scale, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), bgm_box, FALSE, FALSE, 5);

    // Delete Account button
    GtkWidget* delete_button = gtk_button_new_with_label("Delete Account");
    gtk_widget_set_margin_top(delete_button, 20);
    gtk_widget_set_halign(delete_button, GTK_ALIGN_CENTER);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_account_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), delete_button, FALSE, FALSE, 5);

    // Back to Main Menu button
    GtkWidget* back_button = gtk_button_new_with_label("Back to Main Menu");
    gtk_widget_set_margin_top(back_button, 20);
    gtk_widget_set_halign(back_button, GTK_ALIGN_CENTER);
    g_signal_connect(back_button, "clicked", G_CALLBACK(switch_to_main_menu), stack);
    gtk_box_pack_start(GTK_BOX(vbox), back_button, FALSE, FALSE, 5);

    return vbox;
}
