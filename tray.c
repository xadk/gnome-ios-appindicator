#include "tray.h"
#include <stdlib.h>

// Define the global tray variable
Tray *tray = NULL;

void update_menu_item_label(GtkMenuItem *menu_item, const char *new_label) {
    if (menu_item == NULL || new_label == NULL) {
        return; // Ensure valid input
    }

    // Get the child widget of the GtkMenuItem
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(menu_item));
    if (child == NULL) {
        return; // No child widget found
    }

    // Check if the child is a GtkLabel
    if (GTK_IS_LABEL(child)) {
        gtk_label_set_text(GTK_LABEL(child), new_label); // Update the label text
    }
}

// Function to update the menu
void generate_menu() {
    g_return_if_fail(tray != NULL);
    g_return_if_fail(tray->indicator != NULL);
    g_return_if_fail(tray->menu != NULL);
    g_return_if_fail(tray->widgets != NULL);

    // Clear existing menu items
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(tray->menu));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    /**
     * Info Menu Item
     */
    char *label = g_strdup_printf("%s %s (%s)", "NULL", "NULL", "NULL");
    if (label == NULL) {
        fprintf(stderr, "Failed to allocate memory for label\n");
        return;
    }
    tray->widgets->info = gtk_menu_item_new_with_label(label);
    if (tray->widgets->info == NULL) {
        fprintf(stderr, "Failed to create info menu item\n");
        g_free(label);
        return;
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(tray->menu), tray->widgets->info);
    gtk_widget_set_sensitive(tray->widgets->info, TRUE); // Make the item non-clickable
    gtk_widget_hide(tray->widgets->info);
    g_free(label);

    /**
     * Sep-line Menu Item
     */

    // Create a separator menu item (horizontal line)
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(tray->menu), separator);
    gtk_widget_show(separator);


    // Array of structs holding the label and corresponding widget pointer
    struct {
        const char *label;
        GtkWidget **widget;
    } menu_items[] = {
        {"battery", &tray->widgets->battery},
        {"storage", &tray->widgets->storage},
        {"meid", &tray->widgets->meid},
        {"imei", &tray->widgets->imei},
        {"color", &tray->widgets->color},
        {"msisdn", &tray->widgets->msisdn},
        {"is_activated", &tray->widgets->is_activated},
        {"is_passwd", &tray->widgets->is_passwd}
    };

    for (size_t i = 0; i < sizeof(menu_items) / sizeof(menu_items[0]); ++i) {
        // Create a new menu item with the corresponding label
        *menu_items[i].widget = gtk_menu_item_new_with_label(menu_items[i].label);
        if (*menu_items[i].widget == NULL) {
            fprintf(stderr, "Failed to create <%s> menu item\n", menu_items[i].label);
            return;
        }

        // Append the menu item to the menu shell
        gtk_menu_shell_append(GTK_MENU_SHELL(tray->menu), *menu_items[i].widget);

        // Set the item as non-clickable and hide it
        gtk_widget_set_sensitive(*menu_items[i].widget, FALSE);
        gtk_widget_hide(*menu_items[i].widget);
    }

    /**
     * Quit Menu Item
     */
    tray->widgets->quit = gtk_menu_item_new_with_label("Quit");
    if (tray->widgets->quit == NULL) {
        fprintf(stderr, "Failed to create quit menu item\n");
        return;
    }
    g_signal_connect(tray->widgets->quit, "activate", G_CALLBACK(on_menu_item_quit_clicked), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(tray->menu), tray->widgets->quit);
    gtk_widget_show(tray->widgets->quit);

    // Set the updated menu to the app indicator
    app_indicator_set_menu(tray->indicator, tray->menu);
}

void initialize_tray() {
    // Allocate memory for the Tray struct
    tray = (Tray *)malloc(sizeof(Tray));
    if (tray == NULL) {
        fprintf(stderr, "Failed to allocate memory for Tray\n");
        exit(EXIT_FAILURE);
    }

    // Initialize members to NULL
    tray->indicator = NULL;
    tray->menu = NULL;
    tray->widgets = NULL;

    // Initialize the GtkMenu
    tray->menu = GTK_MENU(gtk_menu_new()); // Create a new GtkMenu
    if (tray->menu == NULL) {
        fprintf(stderr, "Failed to create GtkMenu\n");
        free(tray);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for TrayWidgets
    tray->widgets = (TrayWidgets *)malloc(sizeof(TrayWidgets));
    if (tray->widgets == NULL) {
        fprintf(stderr, "Failed to allocate memory for TrayWidgets\n");
        g_clear_object(&tray->menu); // Free the GtkMenu
        free(tray);
        exit(EXIT_FAILURE);
    }

    // Initialize widget pointers to NULL
    tray->widgets->info = NULL;
    tray->widgets->battery = NULL;
    tray->widgets->storage = NULL;
    tray->widgets->meid = NULL;
    tray->widgets->imei = NULL;
    tray->widgets->color = NULL;
    tray->widgets->msisdn = NULL;
    tray->widgets->is_activated = NULL;
    tray->widgets->is_passwd = NULL;
    tray->widgets->quit = NULL;
}

static void destroy_widget(gpointer widget) {
    if (widget != NULL) {
        gtk_widget_destroy(GTK_WIDGET(widget));
    }
}

void free_tray() {
    if (tray != NULL) {
        // Free the GtkMenu if it exists
        if (tray->menu != NULL) {
            g_clear_pointer(&tray->menu, destroy_widget); // Properly release the GtkMenu
        }

        // Free TrayWidgets if it exists
        if (tray->widgets != NULL) {
            free(tray->widgets);
            tray->widgets = NULL;
        }

        // Free the AppIndicator if it exists
        if (tray->indicator != NULL) {
            g_clear_object(&tray->indicator); // Properly release the AppIndicator
        }

        // Free the Tray struct
        free(tray);
        tray = NULL;
    }
}

// Callback function for the "Quit" menu item
static void on_menu_item_quit_clicked(GtkWidget *widget, gpointer data) {
    free_tray(); // Free the tray before quitting
    gtk_main_quit();
    g_print("Exited.\n");
}