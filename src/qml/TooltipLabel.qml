/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * TooltipLabel — a purely visual tooltip bubble (#247).
 *
 * A QtQuick Controls ToolTip is a Popup: inside a QQuickWidget it draws over
 * the button and intercepts the click (setting enabled:false was not enough),
 * so tools could not be clicked while the tooltip showed. This is a plain
 * Rectangle with NO MouseArea — mouse-transparent by construction — positioned
 * clear of the anchor (below for the top bar, above for the bottom bars). It
 * can never block input on the button underneath.
 *
 * Usage: place it as a child of the button and set `anchor`, `text`, `show`.
 */
Rectangle {
    id: tip

    // The button this tooltip describes (used for positioning).
    property Item anchor: parent
    property string text: ""
    property bool show: false
    // true = below the anchor (top bar), false = above (bottom bars).
    property bool placeBelow: false

    visible: show && text !== ""
    z: 1000
    width: tipText.implicitWidth + 16
    height: tipText.implicitHeight + 10
    radius: 6
    color: "#2A2A2E"
    border.color: themeManager.border
    border.width: 1

    anchors.horizontalCenter: anchor ? anchor.horizontalCenter : undefined
    anchors.top: (placeBelow && anchor) ? anchor.bottom : undefined
    anchors.bottom: (!placeBelow && anchor) ? anchor.top : undefined
    anchors.topMargin: 6
    anchors.bottomMargin: 6

    Text {
        id: tipText
        anchors.centerIn: parent
        text: tip.text
        font.pixelSize: 12
        color: "#F2F2F2"
    }
}
