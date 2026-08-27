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
                        { mode: 1, icon: "folders",            label: "Documents", enabled: false },
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
        ToolbarButton { icon: "grid-four"; tooltip: "Quadrillage"; active: appController.isCrossedBackground; onClicked: appController.toggleGrid() }

        // === Spacer ===
        Item { Layout.fillWidth: true }

        // === Right Actions ===
        ToolbarButton { icon: "gear"; tooltip: "Préférences"; onClicked: appController.openPreferences() }
        ToolbarButton { icon: "sign-out"; tooltip: "Quitter"; onClicked: appController.quit() }
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
