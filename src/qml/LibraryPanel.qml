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
 * LibraryPanel — right sidebar media Library (#258), QML V2 replacement for the
 * old UBFeaturesWidget QWidget dock.
 *
 * Binds to libraryController (UBLibraryController): a flat list of the current
 * virtual folder's children (libraryController.itemModel, a UBLibraryItemModel)
 * with a breadcrumb path and up-navigation. Each item exposes the roles
 * name / thumbnailUrl / iconName / isFolder. Clicking a folder enters it;
 * clicking the back button goes up one level.
 *
 * Display + navigation only for this first cut — drag-to-board insertion is a
 * separate step (see #258 / #251).
 */
Rectangle {
    id: root
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 0

    // Left border only (the panel sits on the right edge of the board).
    Rectangle {
        anchors.left: parent.left
        width: 1
        height: parent.height
        color: themeManager.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // === Header: back button + breadcrumb ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 6

                // Up / back one folder (disabled at /root).
                Rectangle {
                    Layout.preferredWidth: 22
                    Layout.preferredHeight: 22
                    radius: 4
                    visible: libraryController.canGoUp
                    color: upMouse.containsMouse ? themeManager.surfaceHover : "transparent"

                    Image {
                        id: upIcon
                        anchors.centerIn: parent
                        width: 14; height: 14
                        source: "qrc:/icons/phosphor/arrow-left.svg"
                        sourceSize: Qt.size(14, 14)
                        visible: false
                    }
                    ColorOverlay {
                        anchors.fill: upIcon
                        source: upIcon
                        color: themeManager.onSurface
                    }
                    MouseArea {
                        id: upMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: libraryController.goUp()
                    }
                }

                Image {
                    id: headerIcon
                    width: 14; height: 14
                    source: "qrc:/icons/phosphor/folder.svg"
                    sourceSize: Qt.size(14, 14)
                    visible: false
                }
                ColorOverlay {
                    width: 14; height: 14
                    source: headerIcon
                    color: themeManager.onSurface
                    opacity: 0.6
                }
                Text {
                    text: "Bibliothèque"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                    color: themeManager.onSurface
                    opacity: 0.6
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: themeManager.border
            }
        }

        // === Item grid ===
        GridView {
            id: itemGrid
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8
            clip: true
            cellWidth: 84
            cellHeight: 92
            model: libraryController.itemModel

            delegate: Item {
                width: itemGrid.cellWidth
                height: itemGrid.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 4
                    radius: 6
                    color: itemMouse.containsMouse ? themeManager.surfaceHover : "transparent"
                    border.width: 1
                    border.color: itemMouse.containsMouse ? themeManager.border : "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 4
                        width: parent.width - 8

                        // Image items: real file:// thumbnail. Non-image items
                        // (folders, audio, apps…): a Phosphor icon fallback from
                        // the model's iconName role.
                        Item {
                            width: 48; height: 48
                            anchors.horizontalCenter: parent.horizontalCenter

                            Image {
                                anchors.fill: parent
                                source: model.thumbnailUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                cache: false
                                smooth: true
                                visible: model.thumbnailUrl !== "" && status === Image.Ready
                            }

                            Image {
                                id: fallbackIcon
                                anchors.centerIn: parent
                                width: 32; height: 32
                                source: "qrc:/icons/phosphor/" + (model.iconName === "" ? "file" : model.iconName) + ".svg"
                                sourceSize: Qt.size(32, 32)
                                visible: false
                            }
                            ColorOverlay {
                                anchors.fill: fallbackIcon
                                source: fallbackIcon
                                color: themeManager.onSurface
                                visible: model.thumbnailUrl === ""
                            }
                        }

                        Text {
                            width: parent.width
                            text: model.name
                            font.pixelSize: 10
                            color: themeManager.onSurface
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            maximumLineCount: 2
                            wrapMode: Text.Wrap
                        }
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        // Only folders navigate for now; leaf items are display-
                        // only until drag-to-board (#251) is wired. enterFolder is
                        // a no-op on a non-folder row (guarded controller-side).
                        onClicked: libraryController.enterFolder(index)
                    }
                }
            }
        }
    }

    // Empty-state hint when the current folder has no children.
    Text {
        anchors.centerIn: parent
        visible: itemGrid.count === 0
        text: "Vide"
        font.pixelSize: 12
        color: themeManager.onSurface
        opacity: 0.4
    }
}
