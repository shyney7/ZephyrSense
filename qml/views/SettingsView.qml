import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Item {
    ScrollView {
        anchors.fill: parent
        anchors.margins: 16

        ColumnLayout {
            width: parent.width
            spacing: 16

            Label {
                text: "Settings"
                font.pixelSize: 24
                font.bold: true
            }

            // Connection settings (existing component)
            ConnectionPanel {
                Layout.fillWidth: true
                Layout.maximumWidth: 400
            }

            // Placeholder for future settings
            Rectangle {
                Layout.fillWidth: true
                Layout.maximumWidth: 400
                height: 100
                color: "#f5f5f5"
                radius: 8
                border.color: "#e0e0e0"

                Label {
                    anchors.centerIn: parent
                    text: "Additional settings (CSV, thresholds) coming in Phase 8"
                    color: "#757575"
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
