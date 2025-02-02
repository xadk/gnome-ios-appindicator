#ifndef TRAY_H
#define TRAY_H

#include <gtk/gtk.h>
#include <libayatana-appindicator3-0.1/libayatana-appindicator/app-indicator.h>
#include <plist/plist.h>

// Define the TrayWidgets struct
typedef struct {
    GtkWidget *info;
    GtkWidget *battery;
    GtkWidget *storage;
    GtkWidget *meid;
    GtkWidget *imei;
    GtkWidget *color;
    GtkWidget *msisdn;
    GtkWidget *is_activated;
    GtkWidget *is_passwd;
    GtkWidget *quit;
} TrayWidgets;

// Define the Tray struct
typedef struct {
    AppIndicator *indicator; // AppIndicator object
    GtkMenu *menu;           // GtkMenu for AppIndicator
    TrayWidgets *widgets;    // Pointer to TrayWidgets
} Tray;

// Declare the global tray variable as extern
extern Tray *tray;

// Function prototypes
void update_menu_item_label(GtkMenuItem *menu_item, const char *new_label);
void initialize_tray();
void free_tray();
void generate_menu();
static void on_menu_item_quit_clicked(GtkWidget *widget, gpointer data);

#endif // TRAY_H