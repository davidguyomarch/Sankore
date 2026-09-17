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

    // --- Button Component (tool + action share the same look) ---
    Component {
        id: buttonComp

        Rectangle {
            id: btn
            width: root.buttonSize
            height: root.buttonSize
            radius: 8

            readonly property bool isTool: btnData.kind === "tool"
            // Tool buttons highlight when they are the active tool.
            property bool isActive: isTool && toolController.activeTool === btnData.id
            property bool isHovered: btnMouse.containsMouse

            color: isActive ? themeManager.primary
                 : isHovered ? themeManager.surfaceHover
                 : "transparent"

            // Icon (hidden source for ColorOverlay)
            Image {
                id: iconImg
                anchors.centerIn: parent
                width: 24
                height: 24
                source: "qrc:/icons/phosphor/" + btnData.icon + ".svg"
                sourceSize: Qt.size(24, 24)
                smooth: true
                mipmap: true
                visible: false
            }

            ColorOverlay {
                anchors.fill: iconImg
                source: iconImg
                color: btn.isActive ? themeManager.onPrimary : themeManager.onSurface
                opacity: btn.isActive ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
            }

            // Active indicator bar (tool buttons only)
            Rectangle {
                visible: btn.isActive
                color: themeManager.onPrimary
                radius: 1.5
                width: parent.width * 0.45
                height: 3
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 2
                }
            }

            MouseArea {
                id: btnMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (btn.isTool) {
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

            // #247: mouse-transparent tooltip above the button.
            TooltipLabel {
                anchor: btn
                text: btnData.tooltip
                show: btnMouse.containsMouse && btnData.tooltip !== ""
                placeBelow: false
            }
        }
    }

    // --- Separator Component ---
    Component {
        id: separatorComp

        Rectangle {
            width: 1
            height: root.buttonSize * 0.6
            color: themeManager.border
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        }
    }
}
