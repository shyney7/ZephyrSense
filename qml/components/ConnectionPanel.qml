import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Rectangle {
    id: connectionPanel

    implicitWidth: 280
    implicitHeight: contentColumn.implicitHeight + 32
    color: "#f5f5f5"
    radius: 8
    border.color: "#e0e0e0"
    border.width: 1

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Title
        Label {
            text: "Serial Connection"
            font.bold: true
            font.pixelSize: 16
            color: "#333333"
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#e0e0e0"
        }

        // Port selection row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Port:"
                Layout.preferredWidth: 70
                color: "#555555"
            }

            ComboBox {
                id: portComboBox
                Layout.fillWidth: true
                model: SerialHandler.availablePorts
                enabled: !SerialHandler.connected
            }

            Button {
                text: "Refresh"
                enabled: !SerialHandler.connected
                onClicked: SerialHandler.refreshPorts()
            }
        }

        // Baud rate row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Baud Rate:"
                Layout.preferredWidth: 70
                color: "#555555"
            }

            ComboBox {
                id: baudRateComboBox
                Layout.fillWidth: true
                model: [9600, 19200, 38400, 57600, 115200]
                currentIndex: 4  // Default to 115200
                enabled: !SerialHandler.connected
                onCurrentValueChanged: {
                    if (currentValue !== undefined) {
                        SerialHandler.baudRate = currentValue
                    }
                }
            }
        }

        // Status row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Status:"
                Layout.preferredWidth: 70
                color: "#555555"
            }

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: SerialHandler.connected ? "#4CAF50" : "#9E9E9E"
            }

            Label {
                text: SerialHandler.connected
                      ? "Connected to " + SerialHandler.currentPort
                      : "Disconnected"
                color: SerialHandler.connected ? "#4CAF50" : "#757575"
                font.weight: SerialHandler.connected ? Font.Medium : Font.Normal
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        // Error display
        Rectangle {
            Layout.fillWidth: true
            height: errorLabel.implicitHeight + 16
            color: "#FFEBEE"
            radius: 4
            visible: SerialHandler.errorString !== ""
            border.color: "#FFCDD2"
            border.width: 1

            Label {
                id: errorLabel
                anchors.fill: parent
                anchors.margins: 8
                text: SerialHandler.errorString
                color: "#C62828"
                wrapMode: Text.WordWrap
                font.pixelSize: 12
            }
        }

        // Control buttons row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                id: connectButton
                text: "Connect"
                Layout.fillWidth: true
                enabled: !SerialHandler.connected && portComboBox.currentText !== ""
                highlighted: true
                onClicked: {
                    SerialHandler.openPort(portComboBox.currentText)
                }
            }

            Button {
                id: disconnectButton
                text: "Disconnect"
                Layout.fillWidth: true
                enabled: SerialHandler.connected
                onClicked: {
                    SerialHandler.closePort()
                }
            }
        }
    }

    // Handle error signal
    Connections {
        target: SerialHandler
        function onErrorOccurred(message) {
            console.log("Serial error:", message)
        }
    }

    // Initialize port list on component creation
    Component.onCompleted: {
        SerialHandler.refreshPorts()
    }
}
