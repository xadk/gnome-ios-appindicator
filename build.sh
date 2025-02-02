#!/bin/bash

mkdir -p dist
gcc -o ./dist/iosindicator main.c device.c tray.c \
-Wl,-Bstatic \
./lib/libimobiledevice.a \
./lib/libusbmuxd.a \
./lib/libssl.a \
./lib/libcrypto.a \
./lib/libplist-2.0.a \
./lib/libayatana-appindicator3.a \
./lib/libayatana-indicator3.a \
./lib/libdbusmenu-gtk3.a \
./lib/libdbusmenu-glib.a \
-Wl,-Bdynamic \
-lgtk-3 \
$(pkg-config --cflags --libs gtk+-3.0) \
-lm
