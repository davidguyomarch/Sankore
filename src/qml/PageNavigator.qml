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

            delegate: Rectangle {
                width: pageList.width
                height: width * 9 / 16  // 16:9 aspect ratio
                radius: 4
                color: "white"
                border.width: 2
                border.color: (index === pageList.currentIndex) ? themeManager.primary : (thumbMouse.containsMouse ? themeManager.onSurface : "transparent")
                opacity: (index === pageList.currentIndex) ? 1.0 : (thumbMouse.containsMouse ? 0.9 : 0.75)

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

                MouseArea {
                    id: thumbMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: pageController.goToPage(index)
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
                SidebarButton { icon: "upload-simple"; tooltip: "Importer"; onClicked: { /* TODO */ } }
            }
        }
    }

    // === Reusable Sidebar Button ===
    component SidebarButton: Rectangle {
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
        ToolTip {
            visible: sbMouse.containsMouse
            delay: 600
            text: parent.tooltip
        }
    }
}
