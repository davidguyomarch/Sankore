/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

/**
 * ToolButton — shared toolbar button (#351).
 *
 * One implementation of the Phosphor-icon toolbar button used by both
 * StylusPaletteV2 (board) and DesktopToolbar (desktop mode): themed colors,
 * Phosphor SVG + ColorOverlay, hover/active states, active indicator bar,
 * and a mouse-transparent tooltip (#247).
 *
 * The button is "dumb": the parent decides `active` and reacts to `clicked()`.
 * `primary` drives the blue (primary) highlight; when it differs from `active`
 * (e.g. the Shapes toggle: open palette = active tint, shape tool actually
 * selected = primary/blue) pass both.
 */
Rectangle {
    id: btn

    property string iconName: ""
    property string tooltip: ""
    property bool active: false          // softer highlight (hover-like tint)
    property bool primary: active        // blue (primary) highlight + indicator
    property int buttonSize: 40
    property bool isVertical: false
    property bool placeTooltipBelow: false

    signal clicked()

    width: buttonSize
    height: buttonSize
    radius: 8

    property bool isHovered: btnMouse.containsMouse

    color: primary ? themeManager.primary
         : active ? themeManager.surfaceHover
         : isHovered ? themeManager.surfaceHover
         : "transparent"

    // Icon (hidden source for ColorOverlay)
    Image {
        id: iconImg
        anchors.centerIn: parent
        width: 24
        height: 24
        source: "qrc:/icons/phosphor/" + btn.iconName + ".svg"
        sourceSize: Qt.size(24, 24)
        smooth: true
        mipmap: true
        visible: false
    }

    ColorOverlay {
        anchors.fill: iconImg
        source: iconImg
        color: btn.primary ? themeManager.onPrimary : themeManager.onSurface
        opacity: btn.active ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
    }

    // Active indicator bar (shown on primary highlight)
    Rectangle {
        visible: btn.primary
        color: themeManager.onPrimary
        radius: 1.5
        width: btn.isVertical ? 3 : parent.width * 0.45
        height: btn.isVertical ? parent.height * 0.45 : 3
        anchors {
            horizontalCenter: btn.isVertical ? undefined : parent.horizontalCenter
            verticalCenter: btn.isVertical ? parent.verticalCenter : undefined
            right: btn.isVertical ? parent.right : undefined
            rightMargin: btn.isVertical ? 2 : 0
            bottom: btn.isVertical ? undefined : parent.bottom
            bottomMargin: btn.isVertical ? 0 : 2
        }
    }

    MouseArea {
        id: btnMouse
        anchors.fill: parent
        hoverEnabled: true
        onClicked: btn.clicked()
    }

    // #247: mouse-transparent tooltip that never intercepts the click.
    TooltipLabel {
        anchor: btn
        text: btn.tooltip
        show: btnMouse.containsMouse && btn.tooltip !== ""
        placeBelow: btn.placeTooltipBelow
    }
}
