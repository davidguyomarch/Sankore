/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15
import Qt5Compat.GraphicalEffects

/**
 * UBToolButton — shared toolbar button (#351).
 *
 * NB: named UBToolButton (not "ToolButton") to avoid colliding with
 * QtQuick.Controls' ToolButton type — that collision made the palettes resolve
 * the wrong type and fail to load (blank toolbars).
 *
 * One implementation of the Phosphor-icon toolbar button used by both
 * StylusPaletteV2 (board) and DesktopToolbar (desktop mode): themed colors,
 * Phosphor SVG recolored via ColorOverlay, hover/active states, active
 * indicator bar, and a mouse-transparent tooltip (#247).
 *
 * Icon recoloring uses the EXACT mechanism StylusPaletteV2 used before #351 —
 * a hidden `Image` + `ColorOverlay` — because that is proven to render on the
 * Windows test VM. #351/#352 tried to "improve" this (MultiEffect,
 * layer.effect, a CPU image provider) to also fix the desktop overlay icons,
 * but every variant broke the board toolbar that already worked. Rule: keep the
 * board icon rendering identical to the known-good original; the desktop
 * overlay icons are a separate concern.
 *
 * The button is "dumb": the parent decides `active` and reacts to `clicked()`.
 * `primaryHighlight` drives the blue (primary) highlight; when it differs from
 * `active` (e.g. the Shapes toggle: open palette = active tint, shape tool
 * actually selected = blue) pass both.
 */
Rectangle {
    id: btn

    property string iconName: ""
    property string tooltip: ""
    property bool active: false               // softer highlight (hover-like tint)
    // NB: not named "primary" — that clashes with a QML-reserved/attached name
    // and makes the whole palette fail to load ("Cannot assign to non-existent
    // property \"primary\"").
    property bool primaryHighlight: active    // blue highlight + active indicator
    property int buttonSize: 40
    property bool isVertical: false
    property bool placeTooltipBelow: false

    signal clicked()

    width: buttonSize
    height: buttonSize
    radius: 8

    property bool isHovered: btnMouse.containsMouse

    color: primaryHighlight ? themeManager.primary
         : active ? themeManager.surfaceHover
         : isHovered ? themeManager.surfaceHover
         : "transparent"

    // Icon (hidden source for ColorOverlay) — identical to the pre-#351 original.
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
        color: btn.primaryHighlight ? themeManager.onPrimary : themeManager.onSurface
        opacity: btn.active ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
    }

    // Active indicator bar (shown on primary highlight)
    Rectangle {
        visible: btn.primaryHighlight
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
