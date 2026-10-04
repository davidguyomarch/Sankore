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
 * LeftSidebar — the single left panel that toggles between two views (#258):
 *   - Pages (page thumbnails, PageNavigator)
 *   - Bibliothèque / Médias (media library, LibraryPanel)
 *
 * A small segmented toggle in the header switches which one is shown; only one
 * is visible at a time, so they share the full sidebar height. Both inner views
 * keep binding to their own context properties (pageController / libraryController
 * / themeManager) injected on the hosting QQuickWidget.
 *
 * Navigation design note, Option A + left-toggle decision: the Library is a
 * left-side panel that shares the slot with Pages (not a right panel, not a
 * full-screen view — that may change later).
 */
Rectangle {
    id: root
    color: themeManager.surface
    border.width: 0

    // 0 = Pages, 1 = Médias (Library).
    property int activePanel: 0

    // Right border (the sidebar sits on the left edge).
    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: themeManager.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // === Segmented toggle: Pages | Médias ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: "transparent"

            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 16
                height: 28
                radius: 6
                color: Qt.darker(themeManager.surface, 1.3)

                Row {
                    anchors.fill: parent
                    anchors.margins: 2

                    Repeater {
                        model: [
                            { id: 0, icon: "stack",  label: "Pages"  },
                            { id: 1, icon: "folder", label: "Médias" }
                        ]

                        Rectangle {
                            width: (parent.width - 0) / 2
                            height: parent.height
                            radius: 4
                            color: root.activePanel === modelData.id ? themeManager.primary : "transparent"

                            Row {
                                anchors.centerIn: parent
                                spacing: 5

                                Image {
                                    id: segIcon
                                    width: 14; height: 14
                                    source: "qrc:/icons/phosphor/" + modelData.icon + ".svg"
                                    sourceSize: Qt.size(14, 14)
                                    visible: false
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                ColorOverlay {
                                    width: 14; height: 14
                                    source: segIcon
                                    color: root.activePanel === modelData.id ? themeManager.onPrimary : themeManager.onSurface
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: modelData.label
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                    color: root.activePanel === modelData.id ? themeManager.onPrimary : themeManager.onSurface
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.activePanel = modelData.id
                            }
                        }
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: themeManager.border
            }
        }

        // === The two stacked views — only the active one is visible ===
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            PageNavigator {
                anchors.fill: parent
                visible: root.activePanel === 0
            }

            LibraryPanel {
                anchors.fill: parent
                visible: root.activePanel === 1
            }
        }
    }
}
