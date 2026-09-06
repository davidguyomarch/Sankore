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
 * ShapesPaletteV2 — floating shapes palette.
 *
 * Toggled by the Shapes button in the stylus palette.
 * Binds to toolController.shapesVisible.
 *
 * Issue #121 Step 5 / Issue #122 Bug 3.
 */
Rectangle {
    id: root
    visible: toolController.shapesVisible
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 150 } }

    implicitWidth: 3 * 42 + 2 * 4 + 20
    implicitHeight: contentCol.implicitHeight + 20
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    Column {
        id: contentCol
        anchors.centerIn: parent
        spacing: 4

        // Section: Shapes
        Text {
            text: "FORMES"
            font.pixelSize: 10
            font.weight: Font.DemiBold
            color: themeManager.onSurface
            opacity: 0.5
            leftPadding: 2
        }

        Grid {
            columns: 3
            spacing: 4

            Repeater {
                model: [
                    { icon: "circle",    tooltip: "Ellipse",    action: "ellipse" },
                    { icon: "rectangle", tooltip: "Rectangle",  action: "rectangle" },
                    { icon: "triangle",  tooltip: "Triangle",   action: "triangle" },
                    { icon: "diamond",   tooltip: "Losange",    action: "diamond" },
                    { icon: "hexagon",   tooltip: "Hexagone",   action: "hexagon" },
                    { icon: "star",      tooltip: "Étoile",     action: "star" }
                ]

                Rectangle {
                    width: 42; height: 42
                    radius: 6
                    color: shapeMouse.containsMouse ? themeManager.surfaceHover : "transparent"

                    Image {
                        id: shapeIcon
                        anchors.centerIn: parent
                        width: 22; height: 22
                        source: "qrc:/icons/phosphor/" + modelData.icon + ".svg"
                        sourceSize: Qt.size(22, 22)
                        visible: false
                    }
                    ColorOverlay {
                        anchors.fill: shapeIcon
                        source: shapeIcon
                        color: themeManager.onSurface
                    }
                    MouseArea {
                        id: shapeMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolController.createShape(modelData.action)
                        }
                    }
                    ToolTip {
                        enabled: false  // #247: informational only — must not intercept clicks
                        visible: shapeMouse.containsMouse
                        delay: 500
                        text: modelData.tooltip
                    }
                }
            }
        }

        // Section: Properties
        Text {
            text: "PROPRIÉTÉS"
            font.pixelSize: 10
            font.weight: Font.DemiBold
            color: themeManager.onSurface
            opacity: 0.5
            leftPadding: 2
            topPadding: 4
        }

        Grid {
            columns: 3
            spacing: 4

            Repeater {
                model: [
                    { icon: "paint-bucket",             tooltip: "Remplissage",  action: "fill" },
                    { icon: "pencil-line",              tooltip: "Contour",      action: "stroke" },
                    { icon: "align-center-horizontal",  tooltip: "Aligner",      action: "align" }
                ]

                Rectangle {
                    width: 42; height: 42
                    radius: 6
                    color: propMouse.containsMouse ? themeManager.surfaceHover : "transparent"

                    Image {
                        id: propIcon
                        anchors.centerIn: parent
                        width: 22; height: 22
                        source: "qrc:/icons/phosphor/" + modelData.icon + ".svg"
                        sourceSize: Qt.size(22, 22)
                        visible: false
                    }
                    ColorOverlay {
                        anchors.fill: propIcon
                        source: propIcon
                        color: themeManager.onSurface
                    }
                    MouseArea {
                        id: propMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (modelData.action === "fill")
                                toolController.activateFillTool()
                            else if (modelData.action === "stroke")
                                toolController.applyStrokeToSelection()
                            else if (modelData.action === "align")
                                toolController.alignSelection()
                        }
                    }
                    ToolTip {
                        enabled: false  // #247: informational only — must not intercept clicks
                        visible: propMouse.containsMouse
                        delay: 500
                        text: modelData.tooltip
                    }
                }
            }
        }
    }
}
