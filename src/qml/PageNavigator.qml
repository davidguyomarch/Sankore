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
 * PageNavigator — Left sidebar with page thumbnails.
 *
 * Binds to pageController (UBPageController).
 * Displays page numbers with active selection highlight.
 * Add/delete/import buttons at the bottom.
 *
 * Issue #121 Step 4.
 */
Rectangle {
    id: root
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 0

    // Right border only
    Rectangle {
        anchors.right: parent.right
        width: 1
        height: parent.height
        color: themeManager.border
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // === Header ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 6

                Image {
                    id: headerIcon
                    width: 14; height: 14
                    source: "qrc:/icons/phosphor/stack.svg"
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
                    text: "Pages"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                    color: themeManager.onSurface
                    opacity: 0.6
                    Layout.fillWidth: true
                }
            }

            // Bottom border
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: themeManager.border
            }
        }

        // === Page List ===
        ListView {
            id: pageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 8
            spacing: 6
            clip: true
            model: pageController.pageCount
            currentIndex: pageController.currentPage - 1

            // #257: drag-and-drop reordering. `moving` is the source index while
            // a thumbnail is being dragged; the delegate under the cursor shows a
            // drop indicator.
            property int draggedIndex: -1

            delegate: Item {
                id: pageSlot
                width: pageList.width
                height: width * 9 / 16  // 16:9 aspect ratio

                // #257: drop indicator — a thick accent line showing where the
                // dragged page will land (above the hovered slot, or below it for
                // the lower half). Kept on top of the thumbnail (high z) so it is
                // always visible during a drag.
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 2
                    anchors.rightMargin: 2
                    height: 4
                    radius: 2
                    color: themeManager.primary
                    visible: pageList.draggedIndex !== -1
                             && pageList.draggedIndex !== index
                             && dropArea.containsDrag
                    anchors.verticalCenter: dropArea.dropAfter ? parent.bottom : parent.top
                    z: 100
                }

                DropArea {
                    id: dropArea
                    anchors.fill: parent
                    property bool dropAfter: false
                    onPositionChanged: (drag) => { dropAfter = drag.y > height / 2 }
                    onDropped: (drop) => {
                        var from = pageList.draggedIndex
                        if (from < 0) return
                        var to = index + (dropAfter ? 1 : 0)
                        if (to > from) to -= 1   // account for removal of the source
                        pageController.moveSceneToIndex(from, to)
                    }
                }

                Rectangle {
                    id: thumb
                    anchors.fill: parent
                    radius: 4
                    color: "white"
                    border.width: 2
                    border.color: (index === pageList.currentIndex) ? themeManager.primary : (thumbMouse.containsMouse ? themeManager.onSurface : "transparent")
                    opacity: dragActive ? 0.6 : ((index === pageList.currentIndex) ? 1.0 : (thumbMouse.containsMouse ? 0.9 : 0.75))

                    property bool dragActive: pageList.draggedIndex === index

                    // Page number badge
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 4
                        width: pageNumText.contentWidth + 6
                        height: pageNumText.contentHeight + 2
                        radius: 2
                        color: Qt.rgba(1, 1, 1, 0.8)

                        Text {
                            id: pageNumText
                            anchors.centerIn: parent
                            text: (index + 1)
                            font.pixelSize: 10
                            color: "#666"
                        }
                    }

                    // #318: hover action bar at the top of the thumbnail:
                    // move up, move down, duplicate, delete. Shown while the
                    // thumbnail (or the bar itself) is hovered. Its own MouseAreas
                    // sit above the thumbnail's, so clicking an action does not
                    // navigate/drag. Hidden during a drag.
                    Rectangle {
                        id: actionBar
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.topMargin: 3
                        width: actionRow.width + 8
                        height: 22
                        radius: 6
                        z: 50
                        color: Qt.rgba(0.16, 0.16, 0.18, 0.92)
                        visible: (thumbMouse.containsMouse || barHover.hovered)
                                 && pageList.draggedIndex === -1

                        HoverHandler { id: barHover }

                        Row {
                            id: actionRow
                            anchors.centerIn: parent
                            spacing: 2

                            PageAction {
                                icon: "arrow-up"; tip: "Monter"
                                enabled: index > 0
                                onTriggered: pageController.moveSceneToIndex(index, index - 1)
                            }
                            PageAction {
                                icon: "arrow-down"; tip: "Descendre"
                                enabled: index < pageController.pageCount - 1
                                onTriggered: pageController.moveSceneToIndex(index, index + 1)
                            }
                            PageAction {
                                icon: "copy"; tip: "Dupliquer"
                                onTriggered: pageController.duplicatePageAt(index)
                            }
                            PageAction {
                                icon: "trash"; tip: "Supprimer"
                                onTriggered: pageController.deletePageAt(index)
                            }
                        }
                    }

                    // Drag payload: a Drag.active target follows the cursor.
                    Drag.active: thumbMouse.drag.active
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2

                    states: State {
                        when: thumbMouse.drag.active
                        ParentChange { target: thumb; parent: pageList }
                        AnchorChanges { target: thumb; anchors.horizontalCenter: undefined; anchors.verticalCenter: undefined }
                    }

                    MouseArea {
                        id: thumbMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        // #257: open hand on hover, closed (grabbing) hand while
                        // dragging a page — the conventional reorder affordance.
                        cursorShape: drag.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        drag.target: thumb
                        drag.axis: Drag.YAxis

                        onPressed: (mouse) => {
                            if (mouse.button === Qt.LeftButton)
                                pageList.draggedIndex = index
                        }
                        onReleased: {
                            if (thumb.Drag.active)
                                thumb.Drag.drop()
                            pageList.draggedIndex = -1
                            // Snap the visual back into the list layout.
                            thumb.parent = pageSlot
                            thumb.anchors.fill = pageSlot
                        }
                        onClicked: (mouse) => {
                            if (mouse.button === Qt.RightButton)
                                pageMenu.popup()
                            else
                                pageController.goToPage(index)
                        }
                        // Touch: long-press opens the same context menu
                        onPressAndHold: pageMenu.popup()
                    }
                }

                // Right-click / long-press context menu for this page
                Menu {
                    id: pageMenu

                    MenuItem {
                        text: "Monter la page"
                        enabled: index > 0
                        onTriggered: pageController.moveSceneToIndex(index, index - 1)
                    }
                    MenuItem {
                        text: "Descendre la page"
                        enabled: index < pageController.pageCount - 1
                        onTriggered: pageController.moveSceneToIndex(index, index + 1)
                    }
                    MenuSeparator {}
                    MenuItem {
                        text: "Dupliquer la page"
                        onTriggered: pageController.duplicatePageAt(index)
                    }
                    MenuItem {
                        text: "Supprimer la page"
                        onTriggered: pageController.deletePageAt(index)
                    }
                }
            }

            // Auto-scroll to current page
            onCurrentIndexChanged: {
                positionViewAtIndex(currentIndex, ListView.Contain)
            }
        }

        // === Footer Buttons ===
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            color: "transparent"

            // Top border
            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: themeManager.border
            }

            Row {
                anchors.centerIn: parent
                spacing: 4

                SidebarButton { icon: "plus-circle"; tooltip: "Ajouter page"; onClicked: pageController.addPage() }
                SidebarButton { icon: "trash"; tooltip: "Supprimer page"; onClicked: pageController.deletePage() }
                SidebarButton { icon: "upload-simple"; tooltip: "Importer"; onClicked: pageController.importPage() }
            }
        }
    }

    // === Reusable Sidebar Button ===
    component SidebarButton: Rectangle {
        id: sbBtn
        property string icon
        property string tooltip
        signal clicked()

        width: 32; height: 32
        radius: 6
        color: sbMouse.containsMouse ? themeManager.surfaceHover : "transparent"

        Image {
            id: sbIcon
            anchors.centerIn: parent
            width: 18; height: 18
            source: "qrc:/icons/phosphor/" + parent.icon + ".svg"
            sourceSize: Qt.size(18, 18)
            visible: false
        }
        ColorOverlay {
            anchors.fill: sbIcon
            source: sbIcon
            color: themeManager.onSurface
        }
        MouseArea {
            id: sbMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: parent.clicked()
        }
        // #247: mouse-transparent tooltip above the footer button.
        TooltipLabel {
            anchor: sbBtn
            text: sbBtn.tooltip
            show: sbMouse.containsMouse && sbBtn.tooltip !== ""
            placeBelow: false
        }
    }

    // === #318: Reusable thumbnail hover action ===
    // A small icon button used inside the per-thumbnail action bar. White
    // Phosphor icon on the dark bar; dims when disabled. Emits triggered()
    // on click (only when enabled). Tooltip is shown below the bar.
    component PageAction: Rectangle {
        id: paBtn
        property string icon
        property string tip
        property bool enabled: true
        signal triggered()

        width: 20; height: 20
        radius: 4
        color: enabled && paMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.18) : "transparent"

        Image {
            id: paIcon
            anchors.centerIn: parent
            width: 14; height: 14
            source: "qrc:/icons/phosphor/" + parent.icon + ".svg"
            sourceSize: Qt.size(14, 14)
            visible: false
        }
        ColorOverlay {
            anchors.fill: paIcon
            source: paIcon
            color: "white"
            opacity: paBtn.enabled ? 1.0 : 0.35
        }
        MouseArea {
            id: paMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: paBtn.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: { if (paBtn.enabled) paBtn.triggered() }
        }
        TooltipLabel {
            anchor: paBtn
            text: paBtn.tip
            show: paMouse.containsMouse && paBtn.tip !== ""
            placeBelow: true
        }
    }
}
