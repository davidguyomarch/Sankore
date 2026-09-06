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
 * DocumentsTopBar — top toolbar for the Documents view.
 *
 * Same visual style as TopBar.qml (Board mode) but with document-specific
 * actions: new document, new folder, import, export, rename, duplicate,
 * delete, open in board.
 *
 * Binds to docActionController (UBDocumentActionController).
 * Uses Phosphor Icons.
 *
 * Issue #134 Phase 1.
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
                        color: docActionController.activeMode === modelData.mode ? themeManager.primary : "transparent"
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
                                color: docActionController.activeMode === modelData.mode ? themeManager.onPrimary : themeManager.onSurface
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: modelData.label
                                font.pixelSize: 12
                                color: docActionController.activeMode === modelData.mode ? themeManager.onPrimary : themeManager.onSurface
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: if (modelData.enabled) docActionController.activeMode = modelData.mode
                        }
                    }
                }
            }
        }

        // === Separator ===
        ToolbarSeparator {}

        // === Document Actions ===
        ToolbarButton { icon: "file-plus";       tooltip: "Nouveau document"; onClicked: docActionController.newDocument() }
        ToolbarButton { icon: "folder-plus";     tooltip: "Nouveau dossier";  onClicked: docActionController.newFolder() }

        ToolbarSeparator {}

        ToolbarButton { icon: "download-simple"; tooltip: "Importer";         onClicked: docActionController.importFile() }
        ToolbarButton { icon: "arrow-square-out"; tooltip: "Exporter";        onClicked: docActionController.exportDocument() }
        ToolbarButton { icon: "pencil-simple";   tooltip: "Renommer";         onClicked: docActionController.renameItem() }

        ToolbarSeparator {}

        ToolbarButton { icon: "copy";            tooltip: "Dupliquer";        onClicked: docActionController.duplicateItem() }
        ToolbarButton { icon: "trash";           tooltip: "Supprimer";        onClicked: docActionController.deleteItem() }

        ToolbarSeparator {}

        ToolbarButton { icon: "chalkboard-teacher"; tooltip: "Ouvrir au tableau"; onClicked: docActionController.openInBoard() }

        // === Spacer ===
        Item { Layout.fillWidth: true }

        // === Right Actions ===
        ToolbarButton { icon: "sign-out"; tooltip: "Quitter"; onClicked: docActionController.quit() }
    }

    // === Reusable ToolbarButton ===
    component ToolbarButton: Rectangle {
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
        ToolTip {
            enabled: false  // #247: informational only — must not intercept clicks
            visible: btnMa.containsMouse && parent.tooltip !== ""
            delay: 600
            text: parent.tooltip
        }
    }

    // === Reusable Separator ===
    component ToolbarSeparator: Rectangle {
        Layout.preferredWidth: 1
        Layout.preferredHeight: 28
        Layout.alignment: Qt.AlignVCenter
        color: themeManager.border
    }
}
