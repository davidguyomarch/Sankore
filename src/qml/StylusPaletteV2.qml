import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

/**
 * StylusPaletteV2 — Bottom toolbar for drawing tools.
 *
 * Binds directly to UBToolController (no QAction, no QActionGroup).
 * Uses Phosphor Icons SVGs from qrc:/icons/phosphor/.
 *
 * Issue #121 Step 2.
 */
Rectangle {
    id: root

    property bool isVertical: false
    property int buttonSize: 40
    property int padding: 6
    property int spacing: 2

    // Computed dimensions
    property int contentLength: toolRow.implicitWidth + padding * 2
    property int thickness: buttonSize + padding * 2

    width: isVertical ? thickness : contentLength
    height: isVertical ? contentLength : thickness
    radius: 0  // No radius on root — avoids white corners on Windows QQuickWidget
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Tool definitions — IDs match UBStylusTool::Enum
    // Pen=0 Eraser=1 Marker=2 Selector=3 Play=4 Hand=5 ZoomIn=6 ZoomOut=7
    // Pointer=8 Line=9 Text=10 Capture=11 RichText=12 ChangeFill=13 Drawing=14 Ocr=15
    readonly property var tools: [
        { id: -1,  icon: "shapes",                  tooltip: "Formes",          isToggle: true },
        { id: 0,   icon: "pen",                     tooltip: "Stylo" },
        { id: 1,   icon: "eraser",                  tooltip: "Gomme" },
        { id: 2,   icon: "highlighter-circle",      tooltip: "Marqueur" },
        { id: 3,   icon: "cursor",                  tooltip: "Sélection" },
        { id: -2,  icon: "",                        tooltip: "",                isSeparator: true },
        { id: 4,   icon: "play",                    tooltip: "Lecture" },
        { id: 5,   icon: "hand",                    tooltip: "Main" },
        { id: 6,   icon: "magnifying-glass-plus",   tooltip: "Zoom +" },
        { id: 7,   icon: "magnifying-glass-minus",  tooltip: "Zoom -" },
        { id: 8,   icon: "crosshair",              tooltip: "Pointeur" },
        { id: -3,  icon: "",                        tooltip: "",                isSeparator: true },
        { id: 9,   icon: "line-segment",            tooltip: "Ligne" },
        { id: 10,  icon: "text-t",                  tooltip: "Texte" },
        { id: 11,  icon: "selection",               tooltip: "Capture" },
        { id: 15,  icon: "text-aa",                 tooltip: "OCR" }
    ]

    Row {
        id: toolRow
        anchors.centerIn: parent
        spacing: root.spacing

        Repeater {
            model: root.tools

            Loader {
                sourceComponent: modelData.isSeparator ? separatorComp : toolButtonComp
                property var toolData: modelData
            }
        }
    }

    // --- Tool Button Component ---
    Component {
        id: toolButtonComp

        Rectangle {
            id: btn
            width: root.buttonSize
            height: root.buttonSize
            radius: 8

            property bool isActive: {
                if (toolData.isToggle)
                    return toolController.shapesVisible
                return toolController.activeTool === toolData.id
            }
            property bool isHovered: btnMouse.containsMouse

            color: isActive ? themeManager.primary
                 : isHovered ? themeManager.surfaceHover
                 : "transparent"

            // Icon (hidden source for ColorOverlay)
            Image {
                id: iconImg
                anchors.centerIn: parent
                width: 24
                height: 24
                source: "qrc:/icons/phosphor/" + toolData.icon + ".svg"
                sourceSize: Qt.size(24, 24)
                smooth: true
                mipmap: true
                visible: false
            }

            // Colored icon overlay
            ColorOverlay {
                anchors.fill: iconImg
                source: iconImg
                color: btn.isActive ? themeManager.onPrimary : themeManager.onSurface
                opacity: btn.isActive ? 1.0 : (btn.isHovered ? 1.0 : 0.85)
            }

            // Active indicator bar
            Rectangle {
                visible: btn.isActive && !toolData.isToggle
                color: themeManager.onPrimary
                radius: 1.5
                width: root.isVertical ? 3 : parent.width * 0.45
                height: root.isVertical ? parent.height * 0.45 : 3
                anchors {
                    horizontalCenter: root.isVertical ? undefined : parent.horizontalCenter
                    verticalCenter: root.isVertical ? parent.verticalCenter : undefined
                    right: root.isVertical ? parent.right : undefined
                    rightMargin: root.isVertical ? 2 : 0
                    bottom: root.isVertical ? undefined : parent.bottom
                    bottomMargin: root.isVertical ? 0 : 2
                }
            }

            MouseArea {
                id: btnMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    if (toolData.isToggle) {
                        toolController.toggleShapes()
                    } else {
                        toolController.activeTool = toolData.id
                    }
                }
            }

            ToolTip {
                visible: btnMouse.containsMouse
                delay: 600
                text: toolData.tooltip
            }
        }
    }

    // --- Separator Component ---
    Component {
        id: separatorComp

        Rectangle {
            width: root.isVertical ? root.buttonSize * 0.6 : 1
            height: root.isVertical ? 1 : root.buttonSize * 0.6
            color: themeManager.border
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
        }
    }
}
