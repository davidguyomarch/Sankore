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
 * DrawingPropsBar — floating bar showing color/width for pen/marker/eraser.
 *
 * Binds to toolController (UBToolController).
 * Appears contextually when Pen, Marker, Line, or Eraser is active.
 * Uses tool-aware currentColors / currentColorIndex / currentWidthIndex
 * so the bar automatically adapts to Pen vs Marker vs Eraser.
 *
 * Issue #121 Step 5 / Issue #122 Bug 2.
 */
Rectangle {
    id: root
    visible: toolController.showDrawingProps
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 150 } }

    implicitWidth: propsRow.implicitWidth + 24
    implicitHeight: 44
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Is this eraser mode? (show only widths, no colors)
    property bool isEraser: toolController.activeTool === 1

    Row {
        id: propsRow
        anchors.centerIn: parent
        spacing: 8

        // === Colors (hidden for eraser) ===
        Row {
            visible: !root.isEraser
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: toolController.currentColors

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: modelData
                    border.width: 2
                    border.color: (index === toolController.currentColorIndex) ? themeManager.onPrimary : "transparent"

                    Rectangle {
                        visible: index === toolController.currentColorIndex
                        anchors.fill: parent
                        anchors.margins: -3
                        radius: 8
                        color: "transparent"
                        border.width: 2
                        border.color: themeManager.primary
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: toolController.currentColorIndex = index
                    }
                }
            }
        }

        // === Separator ===
        Rectangle {
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // === Widths ===
        Row {
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: 3

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: (index === toolController.currentWidthIndex) ? themeManager.primary
                         : widthMouse.containsMouse ? themeManager.surfaceHover
                         : "transparent"

                    Rectangle {
                        anchors.centerIn: parent
                        width: 4 + index * 4
                        height: 4 + index * 4
                        radius: width / 2
                        color: (index === toolController.currentWidthIndex) ? themeManager.onPrimary : themeManager.onSurface
                    }

                    MouseArea {
                        id: widthMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: toolController.currentWidthIndex = index
                    }
                }
            }
        }

        // === Eraser actions (issue #249) — only shown for the eraser ===
        Rectangle {
            visible: root.isEraser
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        Row {
            visible: root.isEraser
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            EraserActionButton {
                icon: "broom"
                tooltip: "Effacer toute l'encre"
                onClicked: toolController.eraseAllInk()
            }
            EraserActionButton {
                icon: "trash"
                tooltip: "Effacer toute la page"
                onClicked: toolController.eraseWholePage()
            }
        }
    }

    // === Reusable eraser action button ===
    component EraserActionButton: Rectangle {
        property string icon
        property string tooltip
        signal clicked()

        width: 28; height: 28
        radius: 6
        color: eaMouse.containsMouse ? themeManager.surfaceHover : "transparent"

        Image {
            id: eaIcon
            anchors.centerIn: parent
            width: 18; height: 18
            source: "qrc:/icons/phosphor/" + parent.icon + ".svg"
            sourceSize: Qt.size(18, 18)
            visible: false
        }
        ColorOverlay {
            anchors.fill: eaIcon
            source: eaIcon
            color: themeManager.onSurface
        }
        MouseArea {
            id: eaMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
        ToolTip {
            enabled: false  // #247: informational only — must not intercept clicks
            visible: eaMouse.containsMouse
            delay: 500
            text: parent.tooltip
        }
    }
}
