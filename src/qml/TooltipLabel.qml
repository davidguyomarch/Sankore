/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * TooltipLabel — a tooltip that neither blocks clicks nor gets clipped (#247).
 *
 * Two earlier attempts failed:
 *  - a QtQuick ToolTip with the default popupType (Popup.Item) is drawn in the
 *    host window overlay and intercepts the click on the button underneath;
 *  - a plain Rectangle child is mouse-transparent but is clipped to the palette
 *    QQuickWidget bounds, so positioned outside the widget it was invisible.
 *
 * Solution (Qt 6.8+): a ToolTip with `popupType: Popup.Window` renders in its
 * own top-level window, so it is never clipped by the QQuickWidget and does not
 * sit in the button's event surface — the click passes through to the button.
 *
 * Usage: place it as a child of the button and set `anchor`, `text`, `show`.
 * `placeBelow` selects below (top bar) vs above (bottom bars) the anchor.
 */
ToolTip {
    id: tip

    property Item anchor: parent
    property bool show: false
    // true = below the anchor (top bar), false = above (bottom bars).
    property bool placeBelow: false

    visible: show && text !== ""
    delay: 400
    popupType: Popup.Window

    // Position relative to the anchor, clear of the button.
    x: anchor ? (anchor.width - width) / 2 : 0
    y: placeBelow ? (anchor ? anchor.height + 6 : 0)
                  : -height - 6

    padding: 8

    contentItem: Text {
        text: tip.text
        font.pixelSize: 12
        color: "#F2F2F2"
    }

    background: Rectangle {
        color: "#2A2A2E"
        border.color: themeManager.border
        border.width: 1
        radius: 6
    }
}
