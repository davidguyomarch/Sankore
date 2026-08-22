import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

/**
 * QML Stylus Palette — Issue #110 Step 2
 *
 * Replaces the old UBStylusPalette C++ widget with a modern QML equivalent.
 * Adapts orientation (vertical/horizontal) from the stylusController.
 * Theme colors come from themeManager (context property).
 * Icons are tinted via ColorOverlay to match the active theme.
 */
Rectangle {
    id: root

    // Layout
    property bool isVertical: stylusController.vertical
    property int buttonSize: 44
    property int buttonSpacing: 2
    property int padding: 6
    property int radius_: 12

    // Computed dimensions
    property int toolCount: stylusController.tools.length
    property int contentLength: toolCount * buttonSize + (toolCount - 1) * buttonSpacing + padding * 2
    property int contentThickness: buttonSize + padding * 2

    width: isVertical ? contentThickness : contentLength
    height: isVertical ? contentLength : contentThickness
    radius: radius_
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Tool buttons grid/flow
    GridLayout {
        id: grid
        anchors.centerIn: parent
        columns: isVertical ? 1 : toolCount
        rows: isVertical ? toolCount : 1
        columnSpacing: buttonSpacing
        rowSpacing: buttonSpacing

        Repeater {
            model: stylusController.tools

            delegate: Rectangle {
                id: btn

                property bool isActive: !modelData.isToggle && (index === stylusController.activeToolIndex)
                property bool isHovered: btnMouse.containsMouse

                width: root.buttonSize
                height: root.buttonSize
                radius: 8
                color: isActive ? themeManager.primary
                     : isHovered ? themeManager.surfaceHover
                     : "transparent"

                // Tool icon (hidden, used as source for ColorOverlay)
                Image {
                    id: iconImage
                    anchors.centerIn: parent
                    width: 28
                    height: 28
                    source: modelData.iconSource
                    sourceSize: Qt.size(28, 28)
                    smooth: true
                    mipmap: true
                    visible: false
                }

                // Tinted icon overlay — colors the icon to match theme
                ColorOverlay {
                    anchors.fill: iconImage
                    source: iconImage
                    color: btn.isActive ? themeManager.onPrimary : themeManager.onSurface
                    opacity: btn.isActive ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
                }

                // Indicator for active tool (small bar below/right)
                Rectangle {
                    visible: btn.isActive
                    color: themeManager.onPrimary
                    radius: 1
                    width: root.isVertical ? 3 : parent.width * 0.5
                    height: root.isVertical ? parent.height * 0.5 : 3
                    anchors {
                        horizontalCenter: root.isVertical ? undefined : parent.horizontalCenter
                        verticalCenter: root.isVertical ? parent.verticalCenter : undefined
                        right: root.isVertical ? parent.right : undefined
                        rightMargin: root.isVertical ? 2 : 0
                        bottom: root.isVertical ? undefined : parent.bottom
                        bottomMargin: root.isVertical ? 0 : 2
                    }
                }

                // Toggle state indicator (for AutoOCR)
                Rectangle {
                    visible: modelData.isToggle && btn.isHovered
                    anchors.fill: parent
                    radius: 8
                    color: themeManager.surfaceHover
                    z: -1
                }

                MouseArea {
                    id: btnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: stylusController.selectTool(index)
                    onDoubleClicked: {
                        // Double-click opens properties (pen width, etc.)
                        stylusController.selectTool(index)
                    }
                }

                // Tooltip
                ToolTip {
                    visible: btnMouse.containsMouse
                    delay: 800
                    text: modelData.name
                    font.pixelSize: 11
                }
            }
        }
    }
}
