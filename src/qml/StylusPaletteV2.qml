/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * StylusPaletteV2 — Bottom toolbar for drawing tools.
 *
 * Binds directly to UBToolController (no QAction, no QActionGroup).
 * Uses Phosphor Icons SVGs from qrc:/icons/phosphor/.
 *
 * Issue #121 Step 2.
 */
Rectangle {
    id: root

    property bool isVertical: false
    property int buttonSize: 40
    property int padding: 6
    property int spacing: 2

    // Computed dimensions
    property int contentLength: toolRow.implicitWidth + padding * 2
    property int thickness: buttonSize + padding * 2

    width: isVertical ? thickness : contentLength
    height: isVertical ? contentLength : thickness
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Tool definitions — IDs match UBStylusTool::Enum
    // Pen=0 Eraser=1 Marker=2 Selector=3 Play=4 Hand=5 ZoomIn=6 ZoomOut=7
    // Pointer=8 Line=9 Text=10 Capture=11 RichText=12 ChangeFill=13 Drawing=14 Ocr=15
    readonly property var tools: [
        { id: -1,  icon: "shapes",                  tooltip: "Formes",          isToggle: true },
        { id: 0,   icon: "pen",                     tooltip: "Stylo" },
        { id: 1,   icon: "eraser",                  tooltip: "Gomme" },
        { id: 2,   icon: "highlighter-circle",      tooltip: "Marqueur" },
        { id: 3,   icon: "cursor",                  tooltip: "Sélection" },
        { id: -2,  icon: "",                        tooltip: "",                isSeparator: true },
        { id: 4,   icon: "play",                    tooltip: "Interagir — déclenche les actions et widgets (mode présentation)" },
        { id: 5,   icon: "hand",                    tooltip: "Déplacer la vue — fait défiler la page sans la modifier" },
        { id: 6,   icon: "magnifying-glass-plus",   tooltip: "Zoom +" },
        { id: 7,   icon: "magnifying-glass-minus",  tooltip: "Zoom -" },
        { id: 8,   icon: "crosshair",              tooltip: "Pointeur" },
        { id: -3,  icon: "",                        tooltip: "",                isSeparator: true },
        { id: 9,   icon: "line-segment",            tooltip: "Ligne" },
        { id: 10,  icon: "text-aa",                  tooltip: "Texte" },
        { id: 11,  icon: "selection",               tooltip: "Capture" },
        { id: 15,  icon: "magic-wand",              tooltip: "OCR" }
    ]

    Row {
        id: toolRow
        anchors.centerIn: parent
        spacing: root.spacing

        // #351 regression fix: the delegate is an Item that holds BOTH a shared
        // UBToolButton and a ToolbarSeparator inline, and passes the row data to
        // them via QUALIFIED references (del.modelData). The previous approach
        // let a Loader-hosted button read the Loader's `toolData` by bare scope
        // resolution, which does not reach into a separate .qml component:
        // `toolData` came out undefined ("Unable to assign [undefined] to
        // bool"), buttons failed to bind, the row collapsed (tool offset) and
        // the Shapes toggle never fired.
        Repeater {
            model: root.tools

            delegate: Item {
                id: del
                required property var modelData
                readonly property Item shown: (modelData.isSeparator === true) ? sep : toolBtn
                implicitWidth: shown.implicitWidth > 0 ? shown.implicitWidth : shown.width
                implicitHeight: shown.implicitHeight > 0 ? shown.implicitHeight : shown.height
                width: implicitWidth
                height: implicitHeight

                // `=== true` everywhere a model flag feeds a bool, so a missing
                // flag (undefined) becomes a real false instead of undefined
                // ("Unable to assign [undefined] to bool").
                readonly property bool isSeparator: del.modelData.isSeparator === true

                UBToolButton {
                    id: toolBtn
                    visible: !del.isSeparator
                    readonly property bool isToggle: del.modelData.isToggle === true
                    readonly property bool shapeToolActive:
                        isToggle && toolController.activeTool === 14
                    buttonSize: root.buttonSize
                    isVertical: root.isVertical
                    iconName: del.isSeparator ? "" : del.modelData.icon
                    tooltip: del.isSeparator ? "" : del.modelData.tooltip
                    active: isToggle ? (toolController.shapesVisible || shapeToolActive)
                                     : (toolController.activeTool === del.modelData.id)
                    primaryHighlight: isToggle ? shapeToolActive
                                               : (toolController.activeTool === del.modelData.id)
                    onClicked: {
                        if (isToggle)
                            toolController.toggleShapes()
                        else
                            toolController.activeTool = del.modelData.id
                    }
                }

                ToolbarSeparator {
                    id: sep
                    visible: del.isSeparator
                    buttonSize: root.buttonSize
                    isVertical: root.isVertical
                }
            }
        }
    }
}
