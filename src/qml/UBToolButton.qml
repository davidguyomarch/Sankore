/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * UBToolButton — shared toolbar button (#351).
 *
 * NB: named UBToolButton (not "ToolButton") to avoid colliding with
 * QtQuick.Controls' ToolButton type — that collision made the palettes resolve
 * the wrong type and fail to load (blank toolbars).
 *
 * One implementation of the Phosphor-icon toolbar button used by both
 * StylusPaletteV2 (board) and DesktopToolbar (desktop mode): themed colors,
 * Phosphor SVG recolored via MultiEffect, hover/active states, active
 * indicator bar, and a mouse-transparent tooltip (#247).
 *
 * #352: icon recoloring uses MultiEffect (QtQuick.Effects), NOT ColorOverlay
 * (Qt5Compat.GraphicalEffects). ColorOverlay renders through an offscreen
 * ShaderEffectSource/FBO that does not composite on a translucent top-level
 * QQuickWidget (the desktop-mode toolbar), so the icons were invisible there
 * while the plain Rectangles still painted. MultiEffect applies the tint
 * directly on the source and composites correctly on the translucent surface.
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

    // Themed tint for the icon, as a color value.
    readonly property color iconColor: btn.primaryHighlight ? themeManager.onPrimary
                                                             : themeManager.onSurface
    // 6-hex-digit form WITHOUT '#', for the image://phosphor/...?c= URL.
    // NB: a QML `color` is not a JS string — String(color) yields "#rrggbb"
    // (or "#aarrggbb"); take the last 6 chars so an alpha prefix is dropped.
    // (An earlier version called color.slice(), which threw a TypeError, left
    //  the Image source empty and never hit the provider — #351.)
    readonly property string iconColorHex: String(iconColor).slice(-6)

    // Phosphor icon, already recolored by the C++ image provider
    // (image://phosphor/<name>?c=RRGGBB). See UBIconImageProvider / ADR 0007.
    //
    // #351/#352 saga: every QML-side GPU recolor (ColorOverlay, MultiEffect,
    // layer.effect) left the icons invisible on the Windows test VM, which runs
    // the software Qt Quick backend (no GPU under x64 emulation) where shader/
    // layer effects don't paint. The provider rasterizes and tints the SVG on
    // the CPU, so the Image needs NO effect and renders on any backend.
    //
    // The tint color is the themed on-surface / on-primary color, passed as a
    // 6-hex-digit string without '#'. themeChanged / primaryHighlight changes
    // rebuild the source URL, so the icon recolors on theme switch and on
    // selection.
    Image {
        id: iconImg
        anchors.centerIn: parent
        width: 24
        height: 24
        source: "image://phosphor/" + btn.iconName + "?c=" + btn.iconColorHex
        sourceSize: Qt.size(24, 24)
        smooth: true
        mipmap: true
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
