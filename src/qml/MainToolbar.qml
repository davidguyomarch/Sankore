/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * MainToolbar — the single drawing toolbar shared by both the board and the
 * desktop-annotation overlay (issue #406, step A of the board/desktop unification).
 *
 * It is a thin instance of the shared UBToolbar shell (layout/rendering) and
 * owns the ONE place where the button model, the highlight predicates and the
 * click routing live for both modes. The mode is selected with the `mode`
 * property:
 *   - mode === "board"   : full board tool set (StylusPaletteV2 shim)
 *   - mode === "desktop" : desktop-annotation tool set + capture/return actions
 *                          (DesktopToolbar shim)
 *
 * This replaces the two previously-divergent copies (StylusPaletteV2.qml and
 * DesktopToolbar.qml), which are now one-line shims setting `mode`. Keeping them
 * as shims means the two C++ load sites (UBBoardPaletteManager and
 * UBDesktopAnnotationController) keep their existing setSource() URLs unchanged.
 *
 * Tool ids match UBStylusTool::Enum: Pen=0 Eraser=1 Marker=2 Selector=3 Play=4
 * Hand=5 ZoomIn=6 ZoomOut=7 Pointer=8 Line=9 Text=10 Capture=11 Drawing=14 Ocr=15.
 *
 * Context properties: both modes need `themeManager` and `toolController`.
 * The desktop mode additionally needs `desktopController` for the capture /
 * return-to-board actions (customCapture / screenCapture / goToUniboard).
 *
 * Model schema (see UBToolbar): each row is one of
 *   { kind: "tool",      id, icon, tooltip }
 *   { kind: "toggle",    id, icon, tooltip }   // board only: Shapes
 *   { kind: "action",    action, icon, tooltip } // desktop only: capture/return
 *   { kind: "separator" }
 */
UBToolbar {
    id: root

    // "board" (default) or "desktop". The two shims set this.
    property string mode: "board"
    readonly property bool isDesktop: mode === "desktop"

    // isVertical is inherited from UBToolbar; UBBoardPaletteManager sets it for
    // the board. The desktop overlay keeps the default (horizontal).

    readonly property var boardModel: [
        { kind: "toggle", id: -1, icon: "shapes",                 tooltip: "Formes" },
        { kind: "tool",   id: 0,  icon: "pen",                    tooltip: "Stylo" },
        { kind: "tool",   id: 1,  icon: "eraser",                 tooltip: "Gomme" },
        { kind: "tool",   id: 2,  icon: "highlighter-circle",     tooltip: "Marqueur" },
        { kind: "tool",   id: 3,  icon: "cursor",                 tooltip: "Sélection" },
        { kind: "separator" },
        { kind: "tool",   id: 4,  icon: "play",                   tooltip: "Interagir — déclenche les actions et widgets (mode présentation)" },
        { kind: "tool",   id: 5,  icon: "hand",                   tooltip: "Déplacer la vue — fait défiler la page sans la modifier" },
        { kind: "tool",   id: 6,  icon: "magnifying-glass-plus",  tooltip: "Zoom +" },
        { kind: "tool",   id: 7,  icon: "magnifying-glass-minus", tooltip: "Zoom -" },
        { kind: "tool",   id: 8,  icon: "crosshair",              tooltip: "Pointeur" },
        { kind: "separator" },
        { kind: "tool",   id: 9,  icon: "line-segment",           tooltip: "Ligne" },
        { kind: "tool",   id: 10, icon: "text-aa",                tooltip: "Texte" },
        { kind: "tool",   id: 11, icon: "selection",              tooltip: "Capture" },
        { kind: "tool",   id: 15, icon: "magic-wand",             tooltip: "OCR" }
    ]

    readonly property var desktopModel: [
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

    model: isDesktop ? desktopModel : boardModel

    // Softer highlight: a toggle shows it while its palette is open OR the shape
    // tool is active; a normal tool shows it when it is the active tool. Actions
    // never highlight.
    isActive: function(row) {
        if (row.kind === "toggle")
            return toolController.shapesVisible || toolController.activeTool === 14
        if (row.kind === "tool")
            return toolController.activeTool === row.id
        return false
    }
    // Blue (primary) highlight: for the Shapes toggle only while the shape tool
    // is actually active (activeTool === 14); for a normal tool when active.
    isPrimary: function(row) {
        if (row.kind === "toggle")
            return toolController.activeTool === 14
        if (row.kind === "tool")
            return toolController.activeTool === row.id
        return false
    }

    onButtonClicked: function(row) {
        if (row.kind === "toggle")
            toolController.toggleShapes()
        else if (row.kind === "tool")
            toolController.activeTool = row.id
        else if (row.kind === "action") {
            if (row.action === "customCapture")
                desktopController.customCapture()
            else if (row.action === "screenCapture")
                desktopController.screenCapture()
            else if (row.action === "goToUniboard")
                desktopController.goToUniboard()
        }
    }
}
