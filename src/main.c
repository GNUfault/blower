/*
 * blower - GNOME ThinkPad Fan Speed Controller 
 * Copyright (C) 2025 Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <adwaita.h>

GtkLabel *status_label;

static void button_clicked(GtkWidget *row, gpointer user_data) {
    const char *level = (const char *)user_data;
    g_print("%s clicked\n", level);
    gchar *command_string = g_strdup_printf("echo level %s | tee /proc/acpi/ibm/fan", level);
    gchar *envp_display = g_strdup_printf("DISPLAY=%s", g_getenv("DISPLAY"));
    gchar *envp_xdg_runtime_dir = g_strdup_printf("XDG_RUNTIME_DIR=%s", g_getenv("XDG_RUNTIME_DIR"));
    gchar *envp[] = { envp_display, envp_xdg_runtime_dir, NULL };
    gchar *argv[] = { "/bin/bash", "-c", command_string, NULL };
    g_autoptr(GError) error = NULL;

    if (!g_spawn_async_with_pipes(NULL, argv, envp, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL, NULL, NULL, &error)) {
        gchar *error_feedback = g_strdup_printf("Error: %s", error->message);
        gtk_label_set_text(status_label, error_feedback);
        g_free(error_feedback);
    } else {
        g_print("Command launched successfully.\n");
    }

    g_free(command_string);
    g_free(envp_display);
    g_free(envp_xdg_runtime_dir);
}

static gboolean update_status(gpointer data) {
    gchar *fan_contents = NULL, *temp_contents = NULL;
    gchar *cpu_temp = NULL, *level = NULL, *speed = NULL;

    if (g_file_get_contents("/proc/acpi/ibm/fan", &fan_contents, NULL, NULL)) {
        gchar **lines = g_strsplit(fan_contents, "\n", -1);
        for (int i = 0; lines[i]; i++) {
            if (g_str_has_prefix(lines[i], "speed:")) speed = g_strdup(lines[i] + 7);
            if (g_str_has_prefix(lines[i], "level:")) level = g_strdup(lines[i] + 7);
        }
        g_strfreev(lines);
        g_free(fan_contents);
    }

    if (g_file_get_contents("/sys/class/thermal/thermal_zone0/temp", &temp_contents, NULL, NULL)) {
        int temp_int = atoi(temp_contents);
        cpu_temp = g_strdup_printf(" %d°C", temp_int / 1000);
        g_free(temp_contents);
    }

    gchar *summary = g_strdup_printf("temp : %-6s\nlevel: %-6s\nspeed: %-6s",
                                     cpu_temp ? cpu_temp : "?",
                                     level ? level : "?",
                                     speed ? speed : "?");

    gtk_label_set_text(status_label, summary);

    g_free(cpu_temp);
    g_free(level);
    g_free(speed);
    g_free(summary);

    return G_SOURCE_CONTINUE;
}

static void on_activate(AdwApplication *app, gpointer user_data) {
    GtkWidget *win = GTK_WIDGET(adw_application_window_new(GTK_APPLICATION(app)));
    gtk_window_set_title(GTK_WINDOW(win), "Blower");
    gtk_window_set_resizable(GTK_WINDOW(win), FALSE);

    GtkWidget *header = adw_header_bar_new();
    gtk_widget_add_css_class(header, "flat");
    GtkWidget *title = gtk_label_new("Blower");
    gtk_widget_add_css_class(title, "title");

    adw_header_bar_set_title_widget(ADW_HEADER_BAR(header), title);
    adw_header_bar_set_show_start_title_buttons(ADW_HEADER_BAR(header), TRUE);
    adw_header_bar_set_show_end_title_buttons(ADW_HEADER_BAR(header), TRUE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(main_box), header);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(win), main_box);

    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(content_box, 0);
    gtk_widget_set_margin_bottom(content_box, 20);
    gtk_widget_set_margin_start(content_box, 20);
    gtk_widget_set_margin_end(content_box, 20);
    gtk_box_append(GTK_BOX(main_box), content_box);

    const gchar *levels[] = {
        "0", "1", "2", "3",
        "4", "5", "6", "7",
        "disengaged", "auto"
    };

    const gchar *labels[] = {
        "Off", "One", "Two", "Three",
        "Four", "Five", "Six", "Seven",
        "Full Speed", "Automatic"
    };

    AdwPreferencesGroup *fan_group = ADW_PREFERENCES_GROUP(adw_preferences_group_new());
    gtk_box_append(GTK_BOX(content_box), GTK_WIDGET(fan_group));

    for (int i = 0; i < 9; i++) {
        AdwButtonRow *row = ADW_BUTTON_ROW(adw_button_row_new());
        g_object_set(row, "title", labels[i], NULL);
        gtk_widget_set_tooltip_text(GTK_WIDGET(row), levels[i]);
        gtk_widget_add_css_class(GTK_WIDGET(row), "flat");
        g_signal_connect(row, "activated", G_CALLBACK(button_clicked), (gpointer)levels[i]);
        adw_preferences_group_add(fan_group, GTK_WIDGET(row));
    }

    AdwButtonRow *auto_row = ADW_BUTTON_ROW(adw_button_row_new());
    g_object_set(auto_row, "title", labels[9], NULL);
    gtk_widget_add_css_class(GTK_WIDGET(auto_row), "suggested-action");
    gtk_widget_add_css_class(GTK_WIDGET(auto_row), "pill");
    g_signal_connect(auto_row, "activated", G_CALLBACK(button_clicked), (gpointer)levels[9]);
    adw_preferences_group_add(fan_group, GTK_WIDGET(auto_row));

    GtkWidget *status_widget = gtk_label_new("Fan status loading...");
    gtk_widget_set_size_request(status_widget, 280, 55);
    gtk_widget_add_css_class(status_widget, "dim-label");
    gtk_widget_add_css_class(status_widget, "monospace");
    gtk_widget_set_halign(status_widget, GTK_ALIGN_CENTER);
    gtk_label_set_wrap(GTK_LABEL(status_widget), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(status_widget), 30);
    gtk_box_append(GTK_BOX(content_box), status_widget);
    status_label = GTK_LABEL(status_widget);

    g_timeout_add(80, update_status, NULL);
    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char *argv[]) {
    AdwApplication *app = adw_application_new("org.bluMATRIKZ.Blower", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}

