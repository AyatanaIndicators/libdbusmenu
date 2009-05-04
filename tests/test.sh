#!/bin/sh

echo dbus-test-runner -d /usr/share/dbus-test-runner/session.conf --task ./test-glib-layout-client --task ./test-glib-layout-server
dbus-test-runner -d /usr/share/dbus-test-runner/session.conf --task ./test-glib-layout-client --task ./test-glib-layout-server
