/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * DesktopToolbar — desktop-annotation toolbar (issue #336). A thin instance of
 * the shared UBToolbar (same shell/rendering as the board StylusPaletteV2): it
 * only supplies the desktop button set and the click routing.
 *
 * Buttons:
 *  - "tool"   → toolController.activeTool = id (ids match UBStylusTool::Enum:
 *               Pen=0 Eraser=1 Marker=2 Selector=3 Pointer=8),
 *  - "action" → a slot on the desktopController context object
 *               (UBDesktopAnnotationController): customCapture / screenCapture /
 *               goToUniboard,
 *  - "separator".
 *
 * Hosted in a top-level QQuickWidget transient-parented to the overlay, with
 * toolController, themeManager and desktopController as context properties.
 */
UBToolbar {
    id: root

    model: [
        { kind: "tool",   id: 0,  icon: "pen",                tooltip: "Stylo" },
        { kind: "tool",   id: 1,  icon: "eraser",             tooltip: "Gomme" },
        { kind: "tool",   id: 2,  icon: "highlighter-circle", tooltip: "Marqueur" },
        { kind: "tool",   id: 3,  icon: "cursor",             tooltip: "Sélection" },
        { kind: "tool",   id: 8,  icon: "crosshair",          tooltip: "Pointeur" },
        { kind: "separator" },
        { kind: "action", action: "customCapture", icon: "selection", tooltip: "Capturer une zone de l'écran" },
        { kind: "action", action: "screenCapture", icon: "desktop",   tooltip: "Capturer tout l'écran" },
        { kind: "separator" },
        { kind: "action", action: "goToUniboard", icon: "chalkboard-teacher", tooltip: "Retour au tableau" }
    ]

    isActive: function(row) {
        return row.kind === "tool" && toolController.activeTool === row.id
    }
    isPrimary: function(row) {
        return row.kind === "tool" && toolController.activeTool === row.id
    }

    onButtonClicked: function(row) {
        if (row.kind === "tool")
            toolController.activeTool = row.id
        else if (row.action === "customCapture")
            desktopController.customCapture()
        else if (row.action === "screenCapture")
            desktopController.screenCapture()
        else if (row.action === "goToUniboard")
            desktopController.goToUniboard()
    }
}
