import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * QML Drawing Properties Panel — Issue #110 Step 3
 *
 * Shows pen/marker color palette (4 colors) and width selector (3 sizes)
 * or eraser size selector when eraser tool is active.
 * Appears automatically when a drawing tool is selected.
 */
Rectangle {
    id: root

    visible: drawingProps.visible
    opacity: visible ? 1.0 : 0.0

    Behavior on opacity { NumberAnimation { duration: 150 } }

    property int btnSize: 36
    property int padding: 8
    property int spacing_: 6

    // Dimensions: horizontal layout — [colors | separator | widths]
    property bool isEraser: drawingProps.activeTool === "eraser"
    property int colorCount: isEraser ? 0 : drawingProps.colors.length
    property int widthCount: 3
    property int totalButtons: colorCount + widthCount
    property int separatorWidth: isEraser ? 0 : 1

    width: totalButtons * btnSize + (totalButtons - 1) * spacing_
           + (separatorWidth > 0 ? spacing_ + separatorWidth + spacing_ : 0)
           + padding * 2
    height: btnSize + padding * 2
    radius: 10
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    Row {
        anchors.centerIn: parent
        spacing: root.spacing_

        // Color buttons (pen/marker only)
        Repeater {
            model: isEraser ? 0 : drawingProps.colors

            delegate: Rectangle {
                id: colorBtn
                width: root.btnSize
                height: root.btnSize
                radius: 6
                color: modelData
                border.color: index === drawingProps.colorIndex
                              ? themeManager.onSurface
                              : themeManager.border
                border.width: index === drawingProps.colorIndex ? 2.5 : 1

                // Selection checkmark
                Text {
                    visible: index === drawingProps.colorIndex
                    anchors.centerIn: parent
                    text: "\u2713"
                    color: isLightColor(modelData) ? "#000000" : "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: drawingProps.setColorIndex(index)
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }

        // Separator between colors and widths
        Rectangle {
            visible: !isEraser && colorCount > 0
            width: 1
            height: root.btnSize
            color: themeManager.border
        }

        // Width buttons (3 sizes: Fine, Medium, Strong)
        Repeater {
            model: root.widthCount

            delegate: Rectangle {
                id: widthBtn

                property bool isActiveWidth: isEraser
                    ? (index === drawingProps.eraserWidthIndex)
                    : (index === drawingProps.widthIndex)
                property int dotSize: index === 0 ? 6 : (index === 1 ? 10 : 16)

                width: root.btnSize
                height: root.btnSize
                radius: 6
                color: isActiveWidth ? themeManager.primary : "transparent"
                border.color: isActiveWidth ? "transparent" : themeManager.border
                border.width: 1

                // Size indicator dot
                Rectangle {
                    anchors.centerIn: parent
                    width: parent.dotSize
                    height: parent.dotSize
                    radius: parent.dotSize / 2
                    color: isActiveWidth ? themeManager.onPrimary : themeManager.onSurface
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (isEraser)
                            drawingProps.setEraserWidthIndex(index)
                        else
                            drawingProps.setWidthIndex(index)
                    }
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }

    // Helper function to determine if a color is light
    function isLightColor(color) {
        var c = Qt.darker(color, 1.0) // force QColor conversion
        return (c.r * 0.299 + c.g * 0.587 + c.b * 0.114) > 0.6
    }
}
