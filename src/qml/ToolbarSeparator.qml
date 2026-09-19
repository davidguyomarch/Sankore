/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * ToolbarSeparator — shared thin separator for the V2 toolbars (#351).
 * Used by StylusPaletteV2 and DesktopToolbar.
 */
Rectangle {
    property int buttonSize: 40
    property bool isVertical: false

    width: isVertical ? buttonSize * 0.6 : 1
    height: isVertical ? 1 : buttonSize * 0.6
    color: themeManager.border
    anchors.verticalCenter: (!isVertical && parent) ? parent.verticalCenter : undefined
    anchors.horizontalCenter: (isVertical && parent) ? parent.horizontalCenter : undefined
}
