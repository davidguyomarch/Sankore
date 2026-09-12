/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

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

        Repeater {
            model: root.tools

            Loader {
                sourceComponent: modelData.isSeparator ? separatorComp : toolButtonComp
                property var toolData: modelData
            }
        }
    }

    // --- Tool Button Component ---
    Component {
        id: toolButtonComp

        Rectangle {
            id: btn
            width: root.buttonSize
            height: root.buttonSize
            radius: 8

            // #318 regression: the Shapes button is a toggle (opens the shapes
            // palette), but it must also read as "selected" (blue) while the
            // shape tool is the active tool — activeTool === Drawing (14) — so
            // the user sees which tool is in use, like every other tool button.
            readonly property bool shapeToolActive:
                toolData.isToggle && toolController.activeTool === 14

            property bool isActive: {
                if (toolData.isToggle)
                    return toolController.shapesVisible || shapeToolActive
                return toolController.activeTool === toolData.id
            }
            // Blue (primary) highlight when this is the active tool. For the
            // Shapes toggle that means the shape tool is active; merely having
            // the palette open (without drawing) keeps the softer hover tint.
            readonly property bool primaryHighlight:
                toolData.isToggle ? shapeToolActive : isActive
            property bool isHovered: btnMouse.containsMouse

            color: primaryHighlight ? themeManager.primary
                 : isActive ? themeManager.surfaceHover
                 : isHovered ? themeManager.surfaceHover
                 : "transparent"

            // Icon (hidden source for ColorOverlay)
            Image {
                id: iconImg
                anchors.centerIn: parent
                width: 24
                height: 24
                source: "qrc:/icons/phosphor/" + toolData.icon + ".svg"
                sourceSize: Qt.size(24, 24)
                smooth: true
                mipmap: true
                visible: false
            }

            // Colored icon overlay
            ColorOverlay {
                anchors.fill: iconImg
                source: iconImg
                color: btn.primaryHighlight ? themeManager.onPrimary : themeManager.onSurface
                opacity: btn.isActive ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
            }

            // Active indicator bar
            Rectangle {
                visible: btn.primaryHighlight
                color: themeManager.onPrimary
                radius: 1.5
                width: root.isVertical ? 3 : parent.width * 0.45
                height: root.isVertical ? parent.height * 0.45 : 3
                anchors {
                    horizontalCenter: root.isVertical ? undefined : parent.horizontalCenter
                    verticalCenter: root.isVertical ? parent.verticalCenter : undefined
                    right: root.isVertical ? parent.right : undefined
                    rightMargin: root.isVertical ? 2 : 0
                    bottom: root.isVertical ? undefined : parent.bottom
                    bottomMargin: root.isVertical ? 0 : 2
                }
            }

            MouseArea {
                id: btnMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (toolData.isToggle) {
                        toolController.toggleShapes()
                    } else {
                        toolController.activeTool = toolData.id
                    }
                }
            }

            // #247: mouse-transparent tooltip above the button (bottom bar), so
            // it never intercepts the click like the QtQuick ToolTip Popup did.
            TooltipLabel {
                anchor: btn
                text: toolData.tooltip
                show: btnMouse.containsMouse && toolData.tooltip !== ""
                placeBelow: false
            }
        }
    }

    // --- Separator Component ---
    Component {
        id: separatorComp

        Rectangle {
            width: root.isVertical ? root.buttonSize * 0.6 : 1
            height: root.isVertical ? 1 : root.buttonSize * 0.6
            color: themeManager.border
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        }
    }
}
