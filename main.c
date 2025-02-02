#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <libayatana-appindicator3-0.1/libayatana-appindicator/app-indicator.h>
#include <gtk/gtk.h>
#include "device.h"
#include "tray.h"

int main(int argc, char *argv[]) {
    // To flush buffer instantly
    setvbuf(stdout, NULL, _IONBF, 0);

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Initialize tray
    initialize_tray();

    // Create a new app indicator (tray icon)
    tray->indicator = app_indicator_new("com.exvous.apps.gnome-ios-appindicator", "phone-apple-iphone", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    // Initialize a dummy menu
    generate_menu();

    // Initially set the app indicator to hidden
    app_indicator_set_status(tray->indicator, APP_INDICATOR_STATUS_PASSIVE);

    // Initialize libimobiledevice
    idevice_error_t ret = idevice_event_subscribe(device_event_callback, NULL);
    if (ret != IDEVICE_E_SUCCESS) {
        fprintf(stderr, "Error subscribing to device events: %d\n", ret);
        return EXIT_FAILURE;
    }

    // Enter the GTK main loop
    gtk_main();

    // Unsubscribe from device events
    idevice_event_unsubscribe();

    // Clears tray incl. appindicator
    free_tray();

    return 0;
}