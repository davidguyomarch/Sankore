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

    // #512 TEMP diagnostics: dump the real geometry when the Line options are
    // shown, so we can tell on the VM why the board bar looks distorted while
    // the desktop one is fine. Logged (debounced) via the controller.
    Timer {
        id: diagTimer
        interval: 120
        repeat: false
        onTriggered: {
            toolController.logDiag(
                "root=" + Math.round(root.width) + "x" + Math.round(root.height)
                + " implicit=" + Math.round(root.implicitWidth) + "x" + Math.round(root.implicitHeight)
                + " propsRow=" + Math.round(propsRow.width) + "x" + Math.round(propsRow.height)
                + " propsRowImplicit=" + Math.round(propsRow.implicitWidth)
                + " propsRowX=" + Math.round(propsRow.x)
                + " colorsN=" + (toolController.currentColors ? toolController.currentColors.length : -1)
                + " activeTool=" + toolController.activeTool
                + " shape=" + toolController.currentShape
                + " showLine=" + toolController.showLineOptions
                + " isEraser=" + root.isEraser + " isFill=" + root.isFill)
        }
    }
    onWidthChanged: diagTimer.restart()
    Connections {
        target: toolController
        function onCurrentShapeChanged() { diagTimer.restart() }
        function onActiveToolChanged() { diagTimer.restart() }
    }

    // Is this eraser mode? (show only widths, no colors)
    property bool isEraser: toolController.activeTool === 1
    // #429-followup: fill mode (ChangeFill = 13) — show colors only, no widths
    // (stroke width is irrelevant to the fill bucket).
    property bool isFill: toolController.activeTool === 13

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
                        // #492: right-click (desktop) also edits the swatch.
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        // #475: explicit invokable (not a property write-back) so a
                        // Repeater delegate rebuild can't masquerade as a click and
                        // reset the custom colour.
                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton)
                                toolController.editColorSwatch(index)   // #492
                            else
                                toolController.selectColorIndex(index)
                        }
                        // #492: long-press (touch / TNI, where there is no right
                        // click) opens the editor for this fixed swatch.
                        onPressAndHold: toolController.editColorSwatch(index)
                    }
                }
            }
        }

        // === 5th "custom colour" swatch (#475) — opens the unified colour picker.
        // Shown wherever colours are (hidden for the eraser). At rest it is
        // INVISIBLE (filled with the bar's surface colour): no custom colour set.
        // Once a custom colour is picked it shows that colour, like a 5th swatch,
        // and gets the same selection ring when it is the active colour.
        Rectangle {
            visible: !root.isEraser
            width: 28; height: 28
            radius: 6
            anchors.verticalCenter: parent.verticalCenter
            // Active custom colour → show it as the fill; otherwise blend into
            // the bar. The palette icon stays ON TOP in both states so this
            // swatch always reads as "custom colour picker", distinct from the
            // 4 fixed swatches.
            property bool hasCustom: toolController.hasCurrentCustomColor
            property color fillColor: hasCustom ? toolController.currentCustomColor
                                     : customMouse.containsMouse ? themeManager.surfaceHover
                                     : themeManager.surface
            color: fillColor
            border.width: hasCustom ? 2 : 1
            border.color: hasCustom ? themeManager.onPrimary : themeManager.border

            // Palette icon, always visible, tinted for contrast against the
            // current fill (dark icon on a light fill, light icon on a dark one)
            // so it stays legible whatever custom colour is chosen.
            Image {
                id: customIcon
                anchors.centerIn: parent
                width: 16; height: 16
                source: "qrc:/icons/phosphor/palette.svg"
                sourceSize: Qt.size(16, 16)
                visible: false
            }
            ColorOverlay {
                anchors.fill: customIcon
                source: customIcon
                // Luminance of the fill decides black vs white icon. Use
                // onSurface when idle (no custom colour) to match the bar.
                color: !parent.hasCustom ? themeManager.onSurfaceDim
                     : (0.299 * parent.fillColor.r + 0.587 * parent.fillColor.g
                        + 0.114 * parent.fillColor.b) > 0.5 ? themeManager.onSurface
                                                            : themeManager.surface
            }

            // Selection ring, same as the 4 fixed swatches, when the custom
            // colour is the active one (currentColorIndex === -1).
            Rectangle {
                visible: parent.hasCustom && toolController.currentColorIndex === -1
                anchors.fill: parent
                anchors.margins: -3
                radius: 8
                color: "transparent"
                border.width: 2
                border.color: themeManager.primary
            }

            MouseArea {
                id: customMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: toolController.pickCustomColor()
            }

            TooltipLabel {
                anchor: parent
                text: "Couleur personnalisée"
                show: customMouse.containsMouse
                placeBelow: false
            }
        }

        // === Separator (hidden in fill mode — no widths there) ===
        Rectangle {
            visible: !root.isFill
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // === Widths (hidden in fill mode: #429-followup) ===
        Row {
            visible: !root.isFill
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

        // === Line options (#369) — only when drawing the Line shape ===
        // Stroke style (solid / dashed) + start/end arrowheads, reusing the
        // existing shape-factory arrow/dash infrastructure.

        // Separator before the line-style group.
        Rectangle {
            visible: toolController.showLineOptions
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // Stroke style: solid vs dashed.
        Row {
            visible: toolController.showLineOptions
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            LineOptionButton {
                icon: "line-segment"
                tooltip: "Trait plein"
                selected: !toolController.lineDashed
                onClicked: toolController.setLineDashed(false)
            }
            LineOptionButton {
                icon: "dots-three"
                tooltip: "Trait pointillé"
                selected: toolController.lineDashed
                onClicked: toolController.setLineDashed(true)
            }
        }

        // Separator before the arrows group.
        Rectangle {
            visible: toolController.showLineOptions
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // Start endpoint: none / arrow / round. (0=None, 1=Round, 2=Arrow)
        Row {
            visible: toolController.showLineOptions
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            LineOptionButton {
                icon: "minus"
                tooltip: "Début : aucun"
                selected: toolController.startArrowType === 0
                onClicked: toolController.setStartArrowType(0)
            }
            LineOptionButton {
                icon: "arrow-left"
                tooltip: "Début : flèche"
                selected: toolController.startArrowType === 2
                onClicked: toolController.setStartArrowType(2)
            }
            LineOptionButton {
                icon: "circle"
                tooltip: "Début : rond"
                selected: toolController.startArrowType === 1
                onClicked: toolController.setStartArrowType(1)
            }
        }

        // Separator between start and end groups.
        Rectangle {
            visible: toolController.showLineOptions
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // End endpoint: none / arrow / round.
        Row {
            visible: toolController.showLineOptions
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            LineOptionButton {
                icon: "minus"
                tooltip: "Fin : aucun"
                selected: toolController.endArrowType === 0
                onClicked: toolController.setEndArrowType(0)
            }
            LineOptionButton {
                icon: "arrow-right"
                tooltip: "Fin : flèche"
                selected: toolController.endArrowType === 2
                onClicked: toolController.setEndArrowType(2)
            }
            LineOptionButton {
                icon: "circle"
                tooltip: "Fin : rond"
                selected: toolController.endArrowType === 1
                onClicked: toolController.setEndArrowType(1)
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
        id: eaBtn
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
        // #247: mouse-transparent tooltip above the button.
        TooltipLabel {
            anchor: eaBtn
            text: eaBtn.tooltip
            show: eaMouse.containsMouse && eaBtn.tooltip !== ""
            placeBelow: false
        }
    }

    // === Reusable line-option toggle button (#369) ===
    // A 28x28 Phosphor-icon button that highlights (primary) when `selected`.
    component LineOptionButton: Rectangle {
        id: loBtn
        property string icon
        property string tooltip
        property bool selected: false
        signal clicked()

        width: 28; height: 28
        radius: 6
        color: selected ? themeManager.primary
             : loMouse.containsMouse ? themeManager.surfaceHover
             : "transparent"

        Image {
            id: loIcon
            anchors.centerIn: parent
            width: 18; height: 18
            source: "qrc:/icons/phosphor/" + parent.icon + ".svg"
            sourceSize: Qt.size(18, 18)
            visible: false
        }
        ColorOverlay {
            anchors.fill: loIcon
            source: loIcon
            color: loBtn.selected ? themeManager.onPrimary : themeManager.onSurface
        }
        MouseArea {
            id: loMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
        TooltipLabel {
            anchor: loBtn
            text: loBtn.tooltip
            show: loMouse.containsMouse && loBtn.tooltip !== ""
            placeBelow: false
        }
    }
}
