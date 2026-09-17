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
 * DesktopToolbar — V2 toolbar for the Desktop annotation mode (issue #336).
 *
 * Replaces the legacy UBDesktopPalette. Modeled on StylusPaletteV2.qml:
 * Phosphor icons + ColorOverlay, all colors from themeManager.
 *
 * Two kinds of buttons:
 *  - tool buttons bound to UBToolController.activeTool (ids match
 *    UBStylusTool::Enum: Pen=0 Eraser=1 Marker=2 Selector=3 Pointer=8),
 *  - action buttons that call slots on the desktopController context object
 *    (UBDesktopAnnotationController): customCapture / screenCapture / goToUniboard.
 *
 * Hosted in a QQuickWidget parented to mTransparentDrawingView, with
 * toolController, themeManager and desktopController as context properties.
 */
Rectangle {
    id: root

    property int buttonSize: 40
    property int padding: 6
    property int spacing: 2

    // Computed dimensions (horizontal bar)
    property int contentLength: toolRow.implicitWidth + padding * 2
    property int thickness: buttonSize + padding * 2

    width: contentLength
    height: thickness
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Button descriptors.
    // kind "tool"    → toggles toolController.activeTool to `id`
    // kind "action"  → invokes desktopController[`action`]()
    // kind "separator"
    readonly property var buttons: [
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

    Row {
        id: toolRow
        anchors.centerIn: parent
        spacing: root.spacing

        Repeater {
            model: root.buttons

            Loader {
                sourceComponent: modelData.kind === "separator" ? separatorComp : buttonComp
                property var btnData: modelData
            }
        }
    }

    // --- Button Component (shared ToolButton, #351) ---
    Component {
        id: buttonComp

        ToolButton {
            required property var btnData

            readonly property bool isTool: btnData.kind === "tool"

            buttonSize: root.buttonSize
            iconName: btnData.icon
            tooltip: btnData.tooltip
            // tool buttons highlight (blue) when they are the active tool
            active: isTool && toolController.activeTool === btnData.id
            primaryHighlight: active
            onClicked: {
                if (isTool) {
                    toolController.activeTool = btnData.id
                } else if (btnData.action === "customCapture") {
                    desktopController.customCapture()
                } else if (btnData.action === "screenCapture") {
                    desktopController.screenCapture()
                } else if (btnData.action === "goToUniboard") {
                    desktopController.goToUniboard()
                }
            }
        }
    }

    // --- Separator Component (shared, #351) ---
    Component {
        id: separatorComp
        ToolbarSeparator { buttonSize: root.buttonSize }
    }
}
