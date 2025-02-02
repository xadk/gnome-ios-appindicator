#ifndef DEVICE_H
#define DEVICE_H

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <plist/plist.h>

void* handle_device_thread(void *arg);
void device_event_callback(const idevice_event_t *event, void *user_data);

#endif // DEVICE_H