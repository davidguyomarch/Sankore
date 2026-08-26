import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

/**
 * DrawingPropsBar — floating bar showing color/width for pen/marker/eraser.
 *
 * Binds to toolController (UBToolController).
 * Appears contextually when Pen, Marker, Line, or Eraser is active.
 *
 * Issue #121 Step 5.
 */
Rectangle {
    id: root
    visible: toolController.showDrawingProps
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 150 } }

    implicitWidth: propsRow.implicitWidth + 24
    implicitHeight: 44
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        radius: 12
        samples: 25
        color: "#40000000"
        verticalOffset: 4
    }

    // Is this eraser mode? (show eraser widths instead of colors)
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
                model: toolController.penColors

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: modelData
                    border.width: 2
                    border.color: (index === toolController.penColorIndex) ? themeManager.onPrimary : "transparent"

                    Rectangle {
                        visible: index === toolController.penColorIndex
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
                        onClicked: toolController.penColorIndex = index
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

            property int currentWidth: root.isEraser ? toolController.eraserWidthIndex : toolController.penWidthIndex

            Repeater {
                model: 3

                Rectangle {
                    width: 28; height: 28
                    radius: 6
                    color: (index === parent.parent.currentWidth) ? themeManager.primary
                         : widthMouse.containsMouse ? themeManager.surfaceHover
                         : "transparent"

                    Rectangle {
                        anchors.centerIn: parent
                        width: 4 + index * 4
                        height: 4 + index * 4
                        radius: width / 2
                        color: (index === parent.parent.parent.currentWidth) ? themeManager.onPrimary : themeManager.onSurface
                    }

                    MouseArea {
                        id: widthMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (root.isEraser)
                                toolController.eraserWidthIndex = index
                            else
                                toolController.penWidthIndex = index
                        }
                    }
                }
            }
        }
    }
}
