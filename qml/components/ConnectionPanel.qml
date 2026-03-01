pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Rectangle {
    id: connectionPanel

    readonly property SerialHandler serialHandler: SerialHandler

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
            Layout.preferredHeight: 1
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
                objectName: "portComboBox"
                Layout.fillWidth: true
                model: connectionPanel.serialHandler.availablePorts
                enabled: !connectionPanel.serialHandler.connected
            }

            Button {
                objectName: "refreshButton"
                text: "Refresh"
                enabled: !connectionPanel.serialHandler.connected
                onClicked: connectionPanel.serialHandler.refreshPorts()
                palette.buttonText: "#333333"
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
                enabled: !connectionPanel.serialHandler.connected
                onCurrentValueChanged: {
                    if (currentValue !== undefined) {
                        connectionPanel.serialHandler.baudRate = currentValue;
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
                objectName: "statusIndicator"
                Layout.preferredWidth: 12
                Layout.preferredHeight: 12
                radius: 6
                color: connectionPanel.serialHandler.connected ? "#4CAF50" : "#9E9E9E"
            }

            Label {
                text: connectionPanel.serialHandler.connected ? "Connected to " + connectionPanel.serialHandler.currentPort : "Disconnected"
                color: connectionPanel.serialHandler.connected ? "#4CAF50" : "#757575"
                font.weight: connectionPanel.serialHandler.connected ? Font.Medium : Font.Normal
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        // Error display
        Rectangle {
            objectName: "errorRect"
            Layout.fillWidth: true
            Layout.preferredHeight: errorLabel.implicitHeight + 16
            color: "#FFEBEE"
            radius: 4
            visible: connectionPanel.serialHandler.errorString !== ""
            border.color: "#FFCDD2"
            border.width: 1

            Label {
                id: errorLabel
                anchors.fill: parent
                anchors.margins: 8
                text: connectionPanel.serialHandler.errorString
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
                objectName: "connectButton"
                text: "Connect"
                Layout.fillWidth: true
                enabled: !connectionPanel.serialHandler.connected && portComboBox.currentText !== ""
                highlighted: true
                palette.buttonText: highlighted ? "#ffffff" : "#333333"
                palette.highlightedText: "#ffffff"
                onClicked: {
                    connectionPanel.serialHandler.openPort(portComboBox.currentText);
                }
            }

            Button {
                id: disconnectButton
                objectName: "disconnectButton"
                text: "Disconnect"
                Layout.fillWidth: true
                enabled: connectionPanel.serialHandler.connected
                palette.buttonText: "#333333"
                onClicked: {
                    connectionPanel.serialHandler.closePort();
                }
            }
        }
    }

    // Handle error signal
    Connections {
        target: connectionPanel.serialHandler
        function onErrorOccurred(message: string): void {
            console.log("Serial error:", message);
        }
    }

    // Initialize port list on component creation
    Component.onCompleted: {
        connectionPanel.serialHandler.refreshPorts();
    }
}
