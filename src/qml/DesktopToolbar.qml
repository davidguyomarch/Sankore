/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * DesktopToolbar — V2 toolbar for the Desktop annotation mode (issue #336).
 *
 * Replaces the legacy UBDesktopPalette. Modeled on StylusPaletteV2.qml:
 * shared UBToolButton (Phosphor icons recolored via MultiEffect), all colors
 * from themeManager.
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

        // #351 regression fix: same pattern as StylusPaletteV2 — the delegate is
        // an Item holding a shared UBToolButton and a ToolbarSeparator inline,
        // fed the row data via QUALIFIED references (del.modelData). A bare
        // `btnData` resolved through a Loader did not reach into the separate
        // UBToolButton .qml (undefined), so the buttons failed to bind.
        Repeater {
            model: root.buttons

            delegate: Item {
                id: del
                required property var modelData
                readonly property Item shown: (modelData.kind === "separator") ? sep : toolBtn
                implicitWidth: shown.implicitWidth > 0 ? shown.implicitWidth : shown.width
                implicitHeight: shown.implicitHeight > 0 ? shown.implicitHeight : shown.height
                width: implicitWidth
                height: implicitHeight

                UBToolButton {
                    id: toolBtn
                    visible: del.modelData.kind !== "separator"
                    readonly property bool isTool: del.modelData.kind === "tool"
                    buttonSize: root.buttonSize
                    iconName: (del.modelData.kind === "separator") ? "" : del.modelData.icon
                    tooltip: (del.modelData.kind === "separator") ? "" : del.modelData.tooltip
                    active: isTool && toolController.activeTool === del.modelData.id
                    primaryHighlight: active
                    onClicked: {
                        if (isTool) {
                            toolController.activeTool = del.modelData.id
                        } else if (del.modelData.action === "customCapture") {
                            desktopController.customCapture()
                        } else if (del.modelData.action === "screenCapture") {
                            desktopController.screenCapture()
                        } else if (del.modelData.action === "goToUniboard") {
                            desktopController.goToUniboard()
                        }
                    }
                }

                ToolbarSeparator {
                    id: sep
                    visible: del.modelData.kind === "separator"
                    buttonSize: root.buttonSize
                }
            }
        }
    }
}
