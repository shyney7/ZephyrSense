import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

ScrollView {
    id: root
    clip: true

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }

    ColumnLayout {
        width: root.width - 32
        spacing: 16

        // Section header
        Label {
            text: "Display Settings"
            font.pixelSize: 18
            font.bold: true
        }

        // Info text
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: infoColumn.implicitHeight + 24
            color: "#E3F2FD"
            radius: 4

            ColumnLayout {
                id: infoColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Label {
                    text: "Display customization options"
                    font.bold: true
                }

                Label {
                    text: "Future versions will include:"
                    color: "#666666"
                }

                Label {
                    text: "- Theme selection (Light/Dark)"
                    color: "#666666"
                    leftPadding: 16
                }

                Label {
                    text: "- Map style options"
                    color: "#666666"
                    leftPadding: 16
                }

                Label {
                    text: "- Gauge appearance customization"
                    color: "#666666"
                    leftPadding: 16
                }

                Label {
                    text: "- Font size adjustments"
                    color: "#666666"
                    leftPadding: 16
                }
            }
        }

        // Current mode information
        GroupBox {
            title: "Current Configuration"
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 8

                RowLayout {
                    Label { text: "Qt Style:" }
                    Label { text: "Fusion"; font.bold: true }
                }

                RowLayout {
                    Label { text: "Map Provider:" }
                    Label { text: "OpenStreetMap"; font.bold: true }
                }

                RowLayout {
                    Label { text: "Database Location:" }
                    Label {
                        text: DatabaseManager.databasePath
                        font.bold: true
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
