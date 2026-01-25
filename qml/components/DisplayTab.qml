import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        Label {
            text: "Display Settings"
            font.pixelSize: 18
            font.bold: true
        }

        GroupBox {
            title: "Coming Soon"
            Layout.fillWidth: true
            Layout.maximumWidth: 400

            ColumnLayout {
                width: parent.width
                spacing: 12

                Label {
                    text: "Display settings coming soon"
                    font.italic: true
                    color: palette.mid
                }

                Label {
                    text: "Future options may include:"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Label {
                    text: "• Theme selection (Light/Dark)\n• Font size preferences\n• Map tile source\n• Gauge style customization"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    leftPadding: 16
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
