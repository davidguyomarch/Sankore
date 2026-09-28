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

        // === Mode Tabs ===
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
                    model: [
                        { mode: 0, icon: "chalkboard-teacher", label: "Tableau",   enabled: true },
                        { mode: 1, icon: "folders",            label: "Documents", enabled: true },
                        { mode: 2, icon: "desktop",            label: "Bureau",    enabled: true }
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

        // === Separator ===
        ToolbarSeparator {}

        // === Undo/Redo ===
        ToolbarButton { icon: "arrow-counter-clockwise"; tooltip: "Annuler"; enabled: appController.canUndo; onClicked: appController.undo() }
        ToolbarButton { icon: "arrow-clockwise"; tooltip: "Refaire"; enabled: appController.canRedo; onClicked: appController.redo() }

        // === Separator ===
        ToolbarSeparator {}

        // === Page Navigation ===
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

        // === Separator ===
        ToolbarSeparator {}

        // === Background ===
        ToolbarButton { icon: "sun"; tooltip: "Fond clair"; active: !appController.isDarkBackground; onClicked: appController.setBackgroundLight() }
        ToolbarButton { icon: "moon"; tooltip: "Fond sombre"; active: appController.isDarkBackground; onClicked: appController.setBackgroundDark() }

        // Background type selector (#289/#393): dropdown with rulings + Desktop
        // see-through. The "Bureau" entry (Desktop) is a background choice, not a
        // separate mode (ADR-0007).
        ToolbarButton {
            id: gridButton
            icon: "grid-four"
            tooltip: "Fond d'écran"
            active: appController.gridType !== 0
            onClicked: gridMenu.popup(gridButton, 0, gridButton.height + 4)
        }
        BackgroundTypeMenu {
            id: gridMenu
        }

        // === Spacer ===
        Item { Layout.fillWidth: true }

        // === Right Actions ===
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



    // === Background type dropdown (#289/#393) ===
    // Native QtQuick Menu so the dropdown renders in its own popup window and
    // is not clipped to the 48px TopBar QQuickWidget (#292).
    //
    // Entries are explicit MenuItems (not a Repeater — see #292 follow-up).
    // Each item is a reusable BackgroundMenuItem.
    component BackgroundTypeMenu: Menu {
        id: menuRoot

        // Open in a real top-level popup window (Qt 6.8+), so the menu is not
        // clipped to the 48px TopBar QQuickWidget. Without this the default
        // Popup.Item renders in the host window overlay, capping the menu to the
        // widget height and forcing scroll arrows (#292 follow-up).
        popupType: Popup.Window

        // 0=Plain, 1=Grid, 2=Seyes, 3=SeyesLarge, 4=Double3mm, 5=Desktop,
        // 6=Image — matches UBBackgroundGrid::Type / UBAppController.gridType.
        width: 240

        background: Rectangle {
            color: themeManager.surface
            border.color: themeManager.border
            border.width: 1
            radius: 8
        }

        // --- Rulings ---
        BackgroundMenuItem { bgType: 0; bgIcon: "circle";           bgLabel: "Aucun" }
        BackgroundMenuItem { bgType: 1; bgIcon: "grid-four";        bgLabel: "Quadrillage" }
        BackgroundMenuItem { bgType: 2; bgIcon: "rows-plus-top";    bgLabel: "Séyès" }
        BackgroundMenuItem { bgType: 3; bgIcon: "rows-plus-bottom"; bgLabel: "Séyès agrandi" }
        BackgroundMenuItem { bgType: 4; bgIcon: "list";             bgLabel: "Double lignage 3 mm" }

        MenuSeparator {
            contentItem: Rectangle { implicitHeight: 1; color: themeManager.border }
        }

        // --- Non-ruling backgrounds (ADR-0007 / #393) ---
        BackgroundMenuItem { bgType: 5; bgIcon: "desktop";  bgLabel: "Bureau (annoter le bureau)" }
        // #389: placeholder — image background not yet implemented.
        // BackgroundMenuItem { bgType: 6; bgIcon: "image"; bgLabel: "Image personnalisée" }
    }

    // One row of the background dropdown: leading themed icon, label, trailing
    // check mark when it is the active background type.
    component BackgroundMenuItem: MenuItem {
        id: item
        property int bgType: 0
        property string bgIcon: ""
        property string bgLabel: ""

        text: bgLabel
        implicitHeight: 34

        onTriggered: appController.setGridType(bgType)

        contentItem: Item {
            Image {
                id: rowIcon
                x: 8
                width: 18; height: 18
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/icons/phosphor/" + item.bgIcon + ".svg"
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
                text: item.bgLabel
                font.pixelSize: 13
                color: themeManager.onSurface
            }

            // Trailing check mark for the active background type.
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
                visible: appController.gridType === item.bgType
            }
        }
    }
}
