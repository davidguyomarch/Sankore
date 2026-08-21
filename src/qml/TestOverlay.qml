import QtQuick 2.15
import QtQuick.Controls 2.15

/**
 * Proof-of-concept QML overlay.
 * Demonstrates that QML rendering works inside the board view
 * and that the ThemeManager colors bind correctly.
 *
 * This will be replaced by the real stylus palette in Step 2.
 */
Rectangle {
    id: root
    width: 200
    height: 48
    radius: 12
    color: themeManager.surface
    border.color: themeManager.border
    border.width: 1

    // Anchored bottom-center of the parent
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 80

    Row {
        anchors.centerIn: parent
        spacing: 8

        Text {
            text: "QML OK"
            color: themeManager.onSurface
            font.pixelSize: 14
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            width: 24
            height: 24
            radius: 12
            color: themeManager.primary
            anchors.verticalCenter: parent.verticalCenter

            Text {
                text: "✓"
                color: themeManager.onPrimary
                font.pixelSize: 14
                anchors.centerIn: parent
            }
        }
    }
}
