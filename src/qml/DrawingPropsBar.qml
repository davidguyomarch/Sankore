import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

/**
 * DrawingPropsBar — floating bar showing color/width for pen/marker/eraser.
 *
 * Binds to toolController (UBToolController).
 * Appears contextually when Pen, Marker, Line, or Eraser is active.
 * Uses tool-aware currentColors / currentColorIndex / currentWidthIndex
 * so the bar automatically adapts to Pen vs Marker vs Eraser.
 *
 * Issue #121 Step 5 / Issue #122 Bug 2.
 */
Rectangle {
    id: root
    visible: toolController.showDrawingProps
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 150 } }

    implicitWidth: propsRow.implicitWidth + 24
    implicitHeight: 44
    radius: 0  // No radius — QQuickWidget on Windows needs opaque background for mouse events
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Is this eraser mode? (show only widths, no colors)
    property bool isEraser: toolController.activeTool === 1

    Row {
        id: propsRow
        anchors.centerIn: parent
        spacing: 8

        // === Colors (hidden for eraser) ===
        Row {
            visible: !root.isEraser
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: toolController.currentColors

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: modelData
                    border.width: 2
                    border.color: (index === toolController.currentColorIndex) ? themeManager.onPrimary : "transparent"

                    Rectangle {
                        visible: index === toolController.currentColorIndex
                        anchors.fill: parent
                        anchors.margins: -3
                        radius: 8
                        color: "transparent"
                        border.width: 2
                        border.color: themeManager.primary
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: toolController.currentColorIndex = index
                    }
                }
            }
        }

        // === Separator ===
        Rectangle {
            width: 1; height: 24
            color: themeManager.border
            anchors.verticalCenter: parent.verticalCenter
        }

        // === Widths ===
        Row {
            spacing: 4
            anchors.verticalCenter: parent.verticalCenter

            Repeater {
                model: 3

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: (index === toolController.currentWidthIndex) ? themeManager.primary
                         : widthMouse.containsMouse ? themeManager.surfaceHover
                         : "transparent"

                    Rectangle {
                        anchors.centerIn: parent
                        width: 4 + index * 4
                        height: 4 + index * 4
                        radius: width / 2
                        color: (index === toolController.currentWidthIndex) ? themeManager.onPrimary : themeManager.onSurface
                    }

                    MouseArea {
                        id: widthMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: toolController.currentWidthIndex = index
                    }
                }
            }
        }
    }
}
