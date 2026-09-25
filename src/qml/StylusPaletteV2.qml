/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * StylusPaletteV2 — board drawing toolbar. A thin instance of the shared
 * UBToolbar: it only supplies the board tool set, the highlight predicates and
 * the click routing to UBToolController. All layout/rendering lives in
 * UBToolbar (shared with the desktop toolbar). Issue #121 Step 2.
 *
 * Tool ids match UBStylusTool::Enum: Pen=0 Eraser=1 Marker=2 Selector=3 Play=4
 * Hand=5 ZoomIn=6 ZoomOut=7 Pointer=8 Line=9 Text=10 Capture=11 Drawing=14 Ocr=15.
 * The Shapes button is a "toggle" (opens the shapes palette); it reads as
 * primary (blue) while the shape tool is actually active (activeTool === 14).
 */
UBToolbar {
    id: root

    // isVertical is inherited from UBToolbar; UBBoardPaletteManager sets it.

    model: [
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

    // Softer highlight: a toggle shows it while its palette is open OR the shape
    // tool is active; a normal tool shows it when it is the active tool.
    isActive: function(row) {
        if (row.kind === "toggle")
            return toolController.shapesVisible || toolController.activeTool === 14
        return toolController.activeTool === row.id
    }
    // Blue (primary) highlight: for the Shapes toggle only while the shape tool
    // is actually active (activeTool === 14); for a normal tool when active.
    isPrimary: function(row) {
        if (row.kind === "toggle")
            return toolController.activeTool === 14
        return toolController.activeTool === row.id
    }

    onButtonClicked: function(row) {
        if (row.kind === "toggle")
            toolController.toggleShapes()
        else
            toolController.activeTool = row.id
    }
}
