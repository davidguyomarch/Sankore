/*
 * Open-Sankoré Community Edition
 *
 * Copyright (C) 2026 David Guyomarch
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */

import QtQuick 2.15

/**
 * UBToolbar — the shared toolbar shell used by both the board (StylusPaletteV2)
 * and the desktop overlay (DesktopToolbar). It owns the themed Rectangle, the
 * Row/Repeater layout and the delegate that instantiates a UBToolButton or a
 * ToolbarSeparator per model row. It has NO knowledge of toolController /
 * desktopController: the caller supplies the model and reacts to buttonClicked.
 *
 * NB: named UBToolbar (not "Toolbar"/"ToolBar") to avoid colliding with the
 * QtQuick.Controls type, same rule as UBToolButton.
 *
 * Model: an array of row descriptors. Each row is one of:
 *   { kind: "tool",      id, icon, tooltip }      // a normal tool button
 *   { kind: "toggle",    id, icon, tooltip }      // a toggle button (e.g. Shapes)
 *   { kind: "action",    action, icon, tooltip }  // an action button (no tool id)
 *   { kind: "separator" }                         // a thin separator
 *
 * The caller decides highlighting and click behaviour by supplying:
 *   - isActive(row)   -> bool : softer (hover-like) highlight
 *   - isPrimary(row)  -> bool : blue (primary) highlight + active indicator
 *   - onButtonClicked(row)    : handle the click (set the tool, toggle, etc.)
 * Both function properties default to reading a `toolController` if present,
 * but callers are expected to override them for clarity.
 */
Rectangle {
    id: root

    // The row descriptors (see header).
    property var model: []

    // Layout.
    property bool isVertical: false
    property int buttonSize: 40
    property int padding: 6
    property int spacing: 2

    // Caller-provided predicates and click handler. `row` is the model entry.
    property var isActive: function(row) { return false; }
    property var isPrimary: function(row) { return false; }
    signal buttonClicked(var row)

    // Computed dimensions.
    readonly property int contentLength: toolRow.implicitWidth > 0
                                         ? toolRow.implicitWidth + padding * 2
                                         : buttonSize + padding * 2
    readonly property int thickness: buttonSize + padding * 2

    width: isVertical ? thickness : contentLength
    height: isVertical ? contentLength : thickness
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    Row {
        id: toolRow
        anchors.centerIn: parent
        spacing: root.spacing

        // Delegate: an Item holding a shared UBToolButton and a ToolbarSeparator
        // inline, fed the row via a QUALIFIED reference (del.modelData). A bare
        // reference does not resolve across the separate .qml boundary (#351),
        // hence the explicit del.modelData everywhere.
        Repeater {
            model: root.model

            delegate: Item {
                id: del
                required property var modelData
                readonly property bool isSeparator: del.modelData.kind === "separator"
                readonly property Item shown: isSeparator ? sep : toolBtn
                implicitWidth: shown.implicitWidth > 0 ? shown.implicitWidth : shown.width
                implicitHeight: shown.implicitHeight > 0 ? shown.implicitHeight : shown.height
                width: implicitWidth
                height: implicitHeight

                UBToolButton {
                    id: toolBtn
                    visible: !del.isSeparator
                    buttonSize: root.buttonSize
                    isVertical: root.isVertical
                    iconName: del.isSeparator ? "" : del.modelData.icon
                    tooltip: del.isSeparator ? "" : del.modelData.tooltip
                    active: !del.isSeparator && root.isActive(del.modelData)
                    primaryHighlight: !del.isSeparator && root.isPrimary(del.modelData)
                    onClicked: root.buttonClicked(del.modelData)
                }

                ToolbarSeparator {
                    id: sep
                    visible: del.isSeparator
                    buttonSize: root.buttonSize
                    isVertical: root.isVertical
                }
            }
        }
    }
}
