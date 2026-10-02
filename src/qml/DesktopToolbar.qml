/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * DesktopToolbar — desktop-annotation toolbar. Since #406 this is a one-line
 * shim over the unified MainToolbar (mode "desktop"); all model/highlight/click
 * logic lives there, shared with the board toolbar. Loaded by
 * UBDesktopAnnotationController via setSource("qrc:/qml/DesktopToolbar.qml"),
 * unchanged. Requires the `desktopController` context property for the
 * capture / return-to-board actions.
 */
MainToolbar {
    mode: "desktop"
}
