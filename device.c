#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "device.h"
#include "tray.h" // To access the global indicator variable

// Struct to hold arguments for handle_device
struct device_args {
    char udid[64];
};

// Shared flag to signal handler to stop when needed
const unsigned int REFRESH_INTERVAL = 5;
volatile bool device_connected = true;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* handle_device_thread(void *arg) {
    struct device_args *args = (struct device_args *)arg;
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    plist_t device_info = NULL;

    // Connect to the device
    if (idevice_new(&device, args->udid) != IDEVICE_E_SUCCESS) {
        fprintf(stderr, "[UDID=%s][Thread] Failed to connect to device.\n", args->udid);
        free(args);
        return NULL;
    }

    // Start lockdown service
    if (lockdownd_client_new_with_handshake(device, &client, NULL) != LOCKDOWN_E_SUCCESS) {
        fprintf(stderr, "[UDID=%s][Thread] Failed to start lockdown service for device.\n", args->udid);
        idevice_free(device);
        free(args);
        return NULL;
    }

    // Get full device information
    if (lockdownd_get_value(client, NULL, NULL, &device_info) != LOCKDOWN_E_SUCCESS || device_info == NULL) {
        fprintf(stderr, "[UDID=%s][Thread] Failed to get device information for device.\n", args->udid);
        lockdownd_client_free(client);
        idevice_free(device);
        free(args);
        return NULL;
    }

    /**
     * Allocate and initialize device_info_params
     */
    struct {
        char *device_class;
        char *product_name;
        char *product_version;
        char *device_name;
        char *meid;
        char *imei;
        char *color;
        char *msisdn;
        char *is_activated;
        uint8_t is_passwd;
    } *device_info_params;

    device_info_params = malloc(sizeof(*device_info_params));
    if (device_info_params == NULL) {
        fprintf(stderr, "[UDID=%s][Thread] Failed to allocate memory for device_info_params\n", args->udid);
        plist_free(device_info);
        lockdownd_client_free(client);
        idevice_free(device);
        free(args);
        return NULL;
    }
    memset(device_info_params, 0, sizeof(*device_info_params)); // Initialize all pointers to NULL

    /**
     * Extract device information
     */
    printf("[UDID=%s][Thread] Getting device info\n", args->udid);
    plist_t node;
    if ((node = plist_dict_get_item(device_info, "DeviceClass")) != NULL) 
        plist_get_string_val(node, &device_info_params->device_class);
    if ((node = plist_dict_get_item(device_info, "ProductName")) != NULL) 
        plist_get_string_val(node, &device_info_params->product_name);
    if ((node = plist_dict_get_item(device_info, "ProductVersion")) != NULL) 
        plist_get_string_val(node, &device_info_params->product_version);
    if ((node = plist_dict_get_item(device_info, "DeviceName")) != NULL) 
        plist_get_string_val(node, &device_info_params->device_name);

    if (device_info_params->device_name && device_info_params->product_version) {
        char *info_label = g_strdup_printf("📱 %s (IOS %s)", device_info_params->device_name, device_info_params->product_version);
        if (info_label != NULL) {
            update_menu_item_label(GTK_MENU_ITEM(tray->widgets->info), info_label);
            gtk_widget_show(tray->widgets->info);
            g_free(info_label);
        }
    }

    /**
     * Extract sub-properties
     */
    printf("[UDID=%s][Thread] Getting sub-device info\n", args->udid);

    if ((node = plist_dict_get_item(device_info, "MobileEquipmentIdentifier")) != NULL) {
        plist_get_string_val(node, &device_info_params->meid);
        if (device_info_params->meid != NULL) {
            char *meid_label = g_strdup_printf(" MEID: %s", device_info_params->meid);
            if (meid_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->meid), meid_label);
                gtk_widget_show(tray->widgets->meid);
                g_free(meid_label);
            } else {
                gtk_widget_hide(tray->widgets->meid);
            }
        }
    }

    if ((node = plist_dict_get_item(device_info, "InternationalMobileEquipmentIdentity")) != NULL)  {
        plist_get_string_val(node, &device_info_params->imei);
        if (device_info_params->imei != NULL) {
            char *imei_label = g_strdup_printf(" IMEI: %s", device_info_params->imei);
            if (imei_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->imei), imei_label);
                gtk_widget_show(tray->widgets->imei);
                g_free(imei_label);
            } else {
                gtk_widget_hide(tray->widgets->imei);
            }
        }
    }

    if ((node = plist_dict_get_item(device_info, "DeviceColor")) != NULL)  {
        plist_get_string_val(node, &device_info_params->color);
        if (device_info_params->color != NULL) {
            char *color_label = g_strdup_printf(" Color: %s", device_info_params->color);
            if (color_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->color), color_label);
                gtk_widget_show(tray->widgets->color);
                g_free(color_label);
            } else {
                gtk_widget_hide(tray->widgets->color);
            }
        }
    }

    if ((node = plist_dict_get_item(device_info, "PhoneNumber")) != NULL) {
        plist_get_string_val(node, &device_info_params->msisdn);
        if (device_info_params->msisdn != NULL) {
            char *msisdn_label = g_strdup_printf(" Phone: %s", device_info_params->msisdn);
            if (msisdn_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->msisdn), msisdn_label);
                gtk_widget_show(tray->widgets->msisdn);
                g_free(msisdn_label);
            } else {
                gtk_widget_hide(tray->widgets->msisdn);
            }
        }
    }

    if ((node = plist_dict_get_item(device_info, "ActivationState")) != NULL) {
        plist_get_string_val(node, &device_info_params->is_activated);
        if (device_info_params->is_activated != NULL) {
            char *is_activated_label = g_strdup_printf(" Activation: %s", device_info_params->is_activated);
            if (is_activated_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->is_activated), is_activated_label);
                gtk_widget_show(tray->widgets->is_activated);
                g_free(is_activated_label);
            } else {
                gtk_widget_hide(tray->widgets->is_activated);
            }
        }
    }

    // Cleanup
    if (device_info_params->device_class) free(device_info_params->device_class);
    if (device_info_params->product_name) free(device_info_params->product_name);
    if (device_info_params->product_version) free(device_info_params->product_version);
    if (device_info_params->device_name) free(device_info_params->device_name);
    if (device_info_params->meid) free(device_info_params->meid);
    if (device_info_params->imei) free(device_info_params->imei);
    if (device_info_params->msisdn) free(device_info_params->msisdn);
    if (device_info_params->is_activated) free(device_info_params->is_activated);
    if (device_info_params->color) free(device_info_params->color);
    if (device_info != NULL) {
        plist_free(device_info);
        node = NULL;
    }

    /**
     * Storage information
     */
    printf("[UDID=%s][Thread] Getting storage info\n", args->udid);
    plist_t storage_info = NULL;
    if (lockdownd_get_value(client, "com.apple.disk_usage", NULL, &storage_info) == LOCKDOWN_E_SUCCESS && storage_info != NULL) {
        int64_t total_disk_capacity = 0;
        int64_t amount_data_available = 0;
        if ((node = plist_dict_get_item(storage_info, "TotalDiskCapacity")) != NULL) plist_get_int_val(node, &total_disk_capacity);
        if ((node = plist_dict_get_item(storage_info, "AmountDataAvailable")) != NULL) plist_get_int_val(node, &amount_data_available);
        if (total_disk_capacity > 0 && amount_data_available > 0) {
            double storage_used =  (total_disk_capacity - amount_data_available) / 1000000000.0;
            char *storage_label = g_strdup_printf(" Storage: %.1fGB / %ldGB used", storage_used, total_disk_capacity / 1000000000);
            if (storage_label != NULL) {
                update_menu_item_label(GTK_MENU_ITEM(tray->widgets->storage), storage_label);
                g_free(storage_label);
            }
        } else {
            fprintf(stderr, "[UDID=%s][Thread] Total disk capacity is zero or invalid\n", args->udid);
        }
    }
    if (storage_info != NULL) {
        gtk_widget_show(tray->widgets->storage);
        plist_free(storage_info);
        node = NULL;
    } else {
        gtk_widget_hide(tray->widgets->storage);
    }

    /**
     * Main loop for dynamic updates
     */
    printf("[UDID=%s][Thread] Monitoring started\n", args->udid);
    while (true) {
        pthread_mutex_lock(&lock);
        if (!device_connected) {
        printf("[UDID=%s][Thread] Monitoring stopped\n", args->udid);
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);

        // Password protection status
        if (lockdownd_get_value(client, NULL, NULL, &device_info) == LOCKDOWN_E_SUCCESS && device_info != NULL) {
            if ((node = plist_dict_get_item(device_info, "PasswordProtected")) != NULL) {
                plist_get_bool_val(node, &device_info_params->is_passwd);
                char *passwd_label = g_strdup_printf(" Password Protected: %s", device_info_params->is_passwd == 1 ? "yes" : "no");
                if (passwd_label != NULL) {
                    update_menu_item_label(GTK_MENU_ITEM(tray->widgets->is_passwd), passwd_label);
                    g_free(passwd_label);
                }
            }
        } else {
            fprintf(stderr, "[UDID=%s][Thread][Loop] Failed to get device information\n", args->udid);
        }

        if (device_info != NULL) {
            gtk_widget_show(tray->widgets->is_passwd);
            plist_free(device_info);
            node = NULL;
        } else {
            gtk_widget_hide(tray->widgets->is_passwd);
        }

        // Battery information
        plist_t battery_info = NULL;
        if (lockdownd_get_value(client, "com.apple.mobile.battery", NULL, &battery_info) == LOCKDOWN_E_SUCCESS && battery_info != NULL) {
            int64_t battery_level = -1;
            if ((node = plist_dict_get_item(battery_info, "BatteryCurrentCapacity")) != NULL) {
                plist_get_int_val(node, &battery_level);
                if (battery_level >= 0) {
                    char *battery_label = g_strdup_printf(" Battery: %ld%%", battery_level);
                    if (battery_label != NULL) {
                        update_menu_item_label(GTK_MENU_ITEM(tray->widgets->battery), battery_label);
                        g_free(battery_label);
                    }
                }
            }
        }

        if (battery_info != NULL) {
            gtk_widget_show(tray->widgets->battery);
            plist_free(battery_info);
            node = NULL;
        } else {
            gtk_widget_hide(tray->widgets->battery);
        }

        sleep(REFRESH_INTERVAL);
    }

    /**
     * Cleanup
     */
    gtk_widget_hide(tray->widgets->info);
    gtk_widget_hide(tray->widgets->battery);
    gtk_widget_hide(tray->widgets->storage);
    gtk_widget_hide(tray->widgets->meid);
    gtk_widget_hide(tray->widgets->imei);
    gtk_widget_hide(tray->widgets->color);
    gtk_widget_hide(tray->widgets->msisdn);
    gtk_widget_hide(tray->widgets->is_activated);
    gtk_widget_hide(tray->widgets->is_passwd);
    if (device_info_params) free(device_info_params);
    lockdownd_client_free(client);
    idevice_free(device);
    free(args);
    return NULL;
}

void device_event_callback(const idevice_event_t *event, void *user_data) {
    if (event->event == IDEVICE_DEVICE_ADD) {
        printf("[UDID=%s][MainCb] Device connected\n", event->udid);

        pthread_mutex_lock(&lock);
        device_connected = true;
        pthread_mutex_unlock(&lock);

        // Allocate memory for the struct
        struct device_args *args = (struct device_args *)malloc(sizeof(struct device_args));
        if (!args) {
            perror("Failed to allocate memory for device_args");
            return;
        }

        // Copy the UDID into the struct
        strncpy(args->udid, event->udid, sizeof(args->udid) - 1);
        args->udid[sizeof(args->udid) - 1] = '\0'; // Ensure null-termination

        // Create the thread
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_device_thread, args) != 0) {
            perror("Failed to create thread");
            free(args); // Free the memory if thread creation fails
        } else {
            pthread_detach(thread); // Detach the thread to avoid resource leaks
        }

        // Show the indicator
        if (tray->indicator != NULL) {
            app_indicator_set_status(tray->indicator, APP_INDICATOR_STATUS_ACTIVE);
        }

    } else if (event->event == IDEVICE_DEVICE_REMOVE) {
        printf("[UDID=%s][MainCb] Device disconnected\n", event->udid);

        // Hide the indicator
        if (tray->indicator != NULL) {
            app_indicator_set_status(tray->indicator, APP_INDICATOR_STATUS_PASSIVE);
        }

        // Signal the thread to stop as device is disconnected.
        pthread_mutex_lock(&lock);
        device_connected = false;
        pthread_mutex_unlock(&lock);
    }
}