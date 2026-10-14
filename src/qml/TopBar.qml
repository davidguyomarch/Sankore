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
 * TopBar — top toolbar with mode tabs, undo/redo, page nav, backgrounds.
 *
 * Binds to appController (UBAppController) and pageController (UBPageController).
 * Uses Phosphor Icons.
 *
 * Issue #121 Step 3.
 */
Rectangle {
    id: root
    height: 48
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 0

    // Bottom border only
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: themeManager.border
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 4

        // === Group VUE — full-screen views (exclusive): Tableau / Documents ===
        Rectangle {
            Layout.preferredHeight: 34
            Layout.preferredWidth: modeTabs.implicitWidth + 4
            radius: 6
            color: Qt.darker(themeManager.surface, 1.3)

            Row {
                id: modeTabs
                anchors.centerIn: parent
                spacing: 2

                Repeater {
                    // #407: "Bureau" is no longer a mode tab — it is a page
                    // background kind (see-through) chosen from the Fond d'écran
                    // menu below. Only Tableau / Documents remain as modes.
                    model: [
                        { mode: 0, icon: "chalkboard-teacher", label: "Tableau",   enabled: true },
                        { mode: 1, icon: "folders",            label: "Documents", enabled: true }
                    ]

                    Rectangle {
                        width: modeRow.implicitWidth + 20
                        height: 30
                        radius: 4
                        color: appController.activeMode === modelData.mode ? themeManager.primary : "transparent"
                        opacity: modelData.enabled ? 1.0 : 0.4

                        Row {
                            id: modeRow
                            anchors.centerIn: parent
                            spacing: 5

                            Image {
                                id: modeIcon
                                width: 16; height: 16
                                source: "qrc:/icons/phosphor/" + modelData.icon + ".svg"
                                sourceSize: Qt.size(16, 16)
                                visible: false
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            ColorOverlay {
                                width: 16; height: 16
                                source: modeIcon
                                color: appController.activeMode === modelData.mode ? themeManager.onPrimary : themeManager.onSurface
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.label
                                font.pixelSize: 12
                                color: appController.activeMode === modelData.mode ? themeManager.onPrimary : themeManager.onSurface
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: if (modelData.enabled) appController.activeMode = modelData.mode
                        }
                    }
                }
            }
        }

        // === Separator (VUE | ACTIONS) ===
        ToolbarSeparator {}

        // === Group ACTIONS — undo/redo (global one-shot actions) ===
        ToolbarButton { icon: "arrow-counter-clockwise"; tooltip: "Annuler"; enabled: appController.canUndo; onClicked: appController.undo() }
        ToolbarButton { icon: "arrow-clockwise"; tooltip: "Refaire"; enabled: appController.canRedo; onClicked: appController.redo() }

        // === Separator (ACTIONS | PAGE) ===
        ToolbarSeparator {}

        // === Group PAGE — navigate + quick add (works with the sidebar closed) ===
        ToolbarButton { icon: "caret-left"; tooltip: "Page précédente"; enabled: pageController.canGoBack; onClicked: pageController.previousPage() }

        Text {
            text: pageController.currentPage + " / " + pageController.pageCount
            font.pixelSize: 12
            color: themeManager.onSurface
            opacity: 0.7
            Layout.leftMargin: 4
            Layout.rightMargin: 4
        }

        ToolbarButton { icon: "caret-right"; tooltip: "Page suivante"; enabled: pageController.canGoForward; onClicked: pageController.nextPage() }
        ToolbarButton { icon: "plus"; tooltip: "Nouvelle page"; onClicked: pageController.addPage() }
        ToolbarButton { icon: "copy"; tooltip: "Dupliquer"; onClicked: pageController.duplicatePage() }

        // === Separator (PAGE | FOND) ===
        ToolbarSeparator {}

        // === Group FOND — page background (persisted page property) ===
        // #389/#434: two separate menus for the two orthogonal axes (ADR-0009):
        //  - "Fond"    = the background KIND (exclusive): Jour / Nuit / Bureau / Image.
        //  - "Lignage" = the RULING (exclusive): Aucun / Quadrillage / Séyès / ...
        // Splitting them removes the "Image + Quadrillage both checked" ambiguity
        // of the old single menu, and the sun/moon buttons move into Fond.
        ToolbarButton {
            id: kindButton
            icon: "image"
            tooltip: "Fond de page"
            active: appController.isDarkBackground || appController.isSeeThrough
                    || appController.isImageBackground
            onClicked: kindMenu.popup(kindButton, 0, kindButton.height + 4)
        }
        BackgroundKindMenu { id: kindMenu }

        ToolbarButton {
            id: gridButton
            icon: "grid-four"
            tooltip: "Lignage"
            active: appController.gridType !== 0
            onClicked: gridMenu.popup(gridButton, 0, gridButton.height + 4)
        }
        RulingMenu { id: gridMenu }

        // === Spacer (pushes the app actions to the right edge) ===
        Item { Layout.fillWidth: true }

        // === Separator (| APPLICATION) ===
        ToolbarSeparator {}

        // === Group APPLICATION — preferences / quit ===
        ToolbarButton { icon: "gear"; tooltip: "Préférences"; onClicked: appController.openPreferences() }
        ToolbarButton { icon: "sign-out"; tooltip: "Quitter"; onClicked: appController.quit() }
    }

    // === Reusable ToolbarButton ===
    component ToolbarButton: Rectangle {
        id: parentBtn
        property string icon
        property string tooltip
        property bool active: false
        property bool enabled: true
        signal clicked()

        Layout.preferredWidth: 34
        Layout.preferredHeight: 34
        radius: 6
        color: active ? themeManager.primary
             : btnMa.containsMouse ? themeManager.surfaceHover
             : "transparent"
        opacity: enabled ? 1.0 : 0.4

        Image {
            id: btnIcon
            anchors.centerIn: parent
            width: 20; height: 20
            source: "qrc:/icons/phosphor/" + parent.icon + ".svg"
            sourceSize: Qt.size(20, 20)
            visible: false
        }
        ColorOverlay {
            anchors.fill: btnIcon
            source: btnIcon
            color: parent.active ? themeManager.onPrimary : themeManager.onSurface
        }
        MouseArea {
            id: btnMa
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: if (parent.enabled) parent.clicked()
        }
        // #247: a QtQuick ToolTip is a Popup that draws over the button and
        // swallows the click inside a QQuickWidget (enabled:false was not
        // enough). Use a plain label with NO MouseArea instead — mouse-
        // transparent by construction — positioned below the top bar button.
        TooltipLabel {
            text: parentBtn.tooltip
            show: btnMa.containsMouse && parentBtn.tooltip !== ""
            placeBelow: true
        }
    }

    // === Reusable Separator ===
    component ToolbarSeparator: Rectangle {
        Layout.preferredWidth: 1
        Layout.preferredHeight: 28
        Layout.alignment: Qt.AlignVCenter
        color: themeManager.border
    }



    // === Ruling type dropdown (#289) ===
    // Native QtQuick Menu so the dropdown renders in its own popup window and
    // is not clipped to the 48px TopBar QQuickWidget (#292).
    // Native QtQuick Menu so the dropdown renders in its own popup window and
    // is not clipped to the 48px TopBar QQuickWidget (#292). popupType window
    // avoids the clipping; explicit MenuItems (not a Repeater) so they lay out
    // vertically. Each row is a reusable RulingMenuItem.

    // #389/#434: RULING axis only (ADR-0009) — exclusive choice among rulings.
    component RulingMenu: Menu {
        id: rulingMenuRoot
        popupType: Popup.Window
        width: 240
        background: Rectangle {
            color: themeManager.surface
            border.color: themeManager.border
            border.width: 1
            radius: 8
        }
        RulingMenuItem { menuRoot: rulingMenuRoot; rulingType: 0; rulingIcon: "circle";           rulingLabel: "Aucun" }
        RulingMenuItem { menuRoot: rulingMenuRoot; rulingType: 1; rulingIcon: "grid-four";        rulingLabel: "Quadrillage" }
        RulingMenuItem { menuRoot: rulingMenuRoot; rulingType: 2; rulingIcon: "rows-plus-top";    rulingLabel: "Séyès" }
        RulingMenuItem { menuRoot: rulingMenuRoot; rulingType: 3; rulingIcon: "rows-plus-bottom"; rulingLabel: "Séyès agrandi" }
        RulingMenuItem { menuRoot: rulingMenuRoot; rulingType: 4; rulingIcon: "list";             rulingLabel: "Double lignage 3 mm" }
    }

    // #389/#434: background KIND axis (ADR-0009) — exclusive choice: Jour / Nuit
    // (opaque light/dark) / Bureau (see-through) / Image. Replaces the sun/moon
    // TopBar buttons (moved here).
    component BackgroundKindMenu: Menu {
        id: kindMenuRoot
        popupType: Popup.Window
        width: 240
        background: Rectangle {
            color: themeManager.surface
            border.color: themeManager.border
            border.width: 1
            radius: 8
        }
        RulingMenuItem { menuRoot: kindMenuRoot; dayKind: true;   rulingIcon: "sun";     rulingLabel: "Jour (fond clair)" }
        RulingMenuItem { menuRoot: kindMenuRoot; nightKind: true; rulingIcon: "moon";    rulingLabel: "Nuit (fond sombre)" }
        MenuSeparator {}
        RulingMenuItem { menuRoot: kindMenuRoot; seeThrough: true; rulingIcon: "desktop"; rulingLabel: "Bureau (transparent)" }
        RulingMenuItem { menuRoot: kindMenuRoot; imagePick: true;  rulingIcon: "image";   rulingLabel: "Image…" }
    }

    // One row of a background dropdown: leading themed icon, label, trailing
    // check mark when it is the active choice.
    component RulingMenuItem: MenuItem {
        id: item
        // The owning Menu, so we can dismiss() it before a modal dialog.
        property Menu menuRoot: null
        property int rulingType: 0
        property string rulingIcon: ""
        property string rulingLabel: ""
        // #407: when true this entry is the see-through "Bureau" background, not
        // a ruling — it enters desktop mode and its check follows isSeeThrough.
        property bool seeThrough: false
        // #389: when true this entry opens the background-image picker.
        property bool imagePick: false
        // #434: opaque Jour / Nuit (background kind, light/dark).
        property bool dayKind: false
        property bool nightKind: false

        text: rulingLabel
        implicitHeight: 34

        onTriggered: {
            if (item.imagePick) {
                // #389: close the menu FIRST, then open the (modal) file dialog
                // on the next event loop turn — otherwise the modal dialog blocks
                // the loop while this QML popup is still up, leaving the menu
                // stuck open behind it (VM bug).
                if (item.menuRoot) item.menuRoot.dismiss()
                Qt.callLater(function() { appController.pickBackgroundImage() })
            } else if (item.dayKind) {
                appController.setBackgroundLight()
            } else if (item.nightKind) {
                appController.setBackgroundDark()
            } else if (item.seeThrough) {
                appController.setBackgroundSeeThrough()
            } else {
                appController.setGridType(rulingType)
            }
        }

        contentItem: Item {
            Image {
                id: rowIcon
                x: 8
                width: 18; height: 18
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/icons/phosphor/" + item.rulingIcon + ".svg"
                sourceSize: Qt.size(18, 18)
                visible: false
            }
            ColorOverlay {
                width: 18; height: 18
                x: 8
                anchors.verticalCenter: parent.verticalCenter
                source: rowIcon
                color: themeManager.onSurface
            }
            Text {
                x: 34
                anchors.verticalCenter: parent.verticalCenter
                text: item.rulingLabel
                font.pixelSize: 13
                color: themeManager.onSurface
            }

            // Trailing check mark for the active ruling.
            Image {
                id: checkIcon
                width: 16; height: 16
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/icons/phosphor/check.svg"
                sourceSize: Qt.size(16, 16)
                visible: false
            }
            ColorOverlay {
                anchors.fill: checkIcon
                source: checkIcon
                color: themeManager.onSurface
                // #407/#389/#434: each entry's check follows the axis it belongs
                // to. Day/Night check only for an opaque page (not see-through /
                // image); Bureau on isSeeThrough; Image on isImageBackground; a
                // ruling on its type regardless of kind (orthogonal axes).
                readonly property bool opaquePage: !appController.isSeeThrough
                                                   && !appController.isImageBackground
                visible: item.imagePick
                         ? appController.isImageBackground
                         : item.seeThrough
                           ? appController.isSeeThrough
                           : item.dayKind
                             ? (opaquePage && !appController.isDarkBackground)
                             : item.nightKind
                               ? (opaquePage && appController.isDarkBackground)
                               : (appController.gridType === item.rulingType)
            }
        }
    }
}
