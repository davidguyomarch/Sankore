/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * StylusPaletteV2 — board drawing toolbar. Since #406 this is a one-line shim
 * over the unified MainToolbar (mode "board"); all model/highlight/click logic
 * lives there, shared with the desktop toolbar. Loaded by UBBoardPaletteManager
 * via setSource("qrc:/qml/StylusPaletteV2.qml"), unchanged.
 */
MainToolbar {
    mode: "board"
}
