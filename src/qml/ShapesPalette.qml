import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

/**
 * QML Shapes Palette — Issue #110 Step 5
 *
 * Replaces UBDrawingPalette (shapes/drawing tools).
 * Shows shape creation tools, stroke/fill properties, and alignment.
 * Toggled by the Drawing button in the Stylus palette.
 */
Rectangle {
    id: root

    visible: shapesController.visible
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 150 } }

    property int btnSize: 40
    property int padding: 8
    property int sectionSpacing: 8

    // 3 columns × 4 rows of shape buttons + property buttons
    width: 3 * btnSize + 2 * 4 + padding * 2
    height: contentCol.implicitHeight + padding * 2
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    Column {
        id: contentCol
        anchors.centerIn: parent
        spacing: root.sectionSpacing

        // === Basic Shapes ===
        Grid {
            columns: 3
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Ellipse"); onActivated: shapesController.createEllipse() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Circle"); onActivated: shapesController.createCircle() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Rectangle"); onActivated: shapesController.createRectangle() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Square"); onActivated: shapesController.createSquare() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Triangle"); onActivated: shapesController.createRegularPolygon(3) }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Pentagon"); onActivated: shapesController.createRegularPolygon(5) }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Hexagon"); onActivated: shapesController.createRegularPolygon(6) }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Line"); onActivated: shapesController.createLine() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Polygon"); onActivated: shapesController.createPolygon() }
        }

        // === Separator ===
        Rectangle { width: parent.width - 8; height: 1; color: themeManager.border; anchors.horizontalCenter: parent.horizontalCenter }

        // === Stroke & Fill properties ===
        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            // Stroke thickness buttons
            Repeater {
                model: 3
                delegate: Rectangle {
                    width: root.btnSize; height: root.btnSize; radius: 6
                    color: index === shapesController.strokeThickness ? themeManager.primary : "transparent"
                    border.color: themeManager.border; border.width: 1
                    Rectangle {
                        anchors.centerIn: parent
                        width: index === 0 ? 4 : (index === 1 ? 7 : 12)
                        height: width; radius: width / 2
                        color: index === shapesController.strokeThickness ? themeManager.onPrimary : themeManager.onSurface
                    }
                    MouseArea { anchors.fill: parent; onClicked: shapesController.setStrokeThickness(index); cursorShape: Qt.PointingHandCursor }
                }
            }
        }

        Row {
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            // Stroke color swatch
            Rectangle {
                width: root.btnSize; height: root.btnSize; radius: 6
                color: shapesController.strokeColor
                border.color: themeManager.border; border.width: 1
                Text { anchors.centerIn: parent; text: "S"; color: isLightColor(shapesController.strokeColor) ? "black" : "white"; font.bold: true; font.pixelSize: 12 }
                MouseArea { anchors.fill: parent; onClicked: shapesController.pickStrokeColor(); cursorShape: Qt.PointingHandCursor }
                ToolTip { visible: parent.children[1].containsMouse; delay: 600; text: qsTr("Stroke color") }
            }

            // Fill color swatch
            Rectangle {
                width: root.btnSize; height: root.btnSize; radius: 6
                color: shapesController.fillColor
                border.color: themeManager.border; border.width: 1
                Text { anchors.centerIn: parent; text: "F"; color: isLightColor(shapesController.fillColor) ? "black" : "white"; font.bold: true; font.pixelSize: 12 }
                MouseArea { anchors.fill: parent; onClicked: shapesController.pickFillColor(); cursorShape: Qt.PointingHandCursor }
                ToolTip { visible: parent.children[1].containsMouse; delay: 600; text: qsTr("Fill color") }
            }

            // Paint bucket
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Paint bucket"); onActivated: shapesController.changeFill() }
        }

        // === Separator ===
        Rectangle { width: parent.width - 8; height: 1; color: themeManager.border; anchors.horizontalCenter: parent.horizontalCenter }

        // === Alignment ===
        Grid {
            columns: 3
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter

            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align left"); onActivated: shapesController.alignLeft() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align H center"); onActivated: shapesController.alignHCenter() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align right"); onActivated: shapesController.alignRight() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align top"); onActivated: shapesController.alignTop() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align V center"); onActivated: shapesController.alignVCenter() }
            ShapeButton { icon: "qrc:/images/stylusPalette/svg/drawing.svg"; tooltip: qsTr("Align bottom"); onActivated: shapesController.alignBottom() }
        }
    }

    function isLightColor(color) {
        var c = Qt.darker(color, 1.0)
        return (c.r * 0.299 + c.g * 0.587 + c.b * 0.114) > 0.6
    }

    // Reusable shape button component
    component ShapeButton: Rectangle {
        property string icon: ""
        property string tooltip: ""
        signal activated()

        width: root.btnSize; height: root.btnSize; radius: 6
        color: btnArea.containsMouse ? themeManager.surfaceHover : "transparent"

        Image {
            id: btnIcon
            anchors.centerIn: parent; width: 22; height: 22
            source: parent.icon; sourceSize: Qt.size(22, 22)
            visible: false
        }
        ColorOverlay { anchors.fill: btnIcon; source: btnIcon; color: themeManager.onSurface }

        MouseArea { id: btnArea; anchors.fill: parent; hoverEnabled: true; onClicked: parent.activated(); cursorShape: Qt.PointingHandCursor }
        ToolTip { visible: btnArea.containsMouse; delay: 600; text: parent.tooltip; font.pixelSize: 11 }
    }
}
