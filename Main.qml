import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Window {
    id: mainWindow
    width: 800
    height: 600
    visible: true
    title: "ZephyrSense"

    color: "#ECEFF1"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Left sidebar with connection panel
        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: 280
            spacing: 16

            ConnectionPanel {
                id: connectionPanel
                Layout.fillWidth: true
            }

            // Placeholder for future sidebar content
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "transparent"
            }
        }

        // Main content area placeholder
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#FFFFFF"
            radius: 8
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 8

                Label {
                    text: "Data will display here"
                    font.pixelSize: 18
                    color: "#9E9E9E"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Connect to a serial port to begin receiving sensor data"
                    font.pixelSize: 12
                    color: "#BDBDBD"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    // Debug output for received readings
    Connections {
        target: SerialHandler
        function onNewReading(reading) {
            console.log("Received reading - Temp:", reading.temperature,
                        "Humidity:", reading.humidity,
                        "Lat:", reading.latitude, "Lon:", reading.longitude)
        }
    }
}
