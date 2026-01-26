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
                text: "Serial Port Configuration"
                font.pixelSize: 18
                font.bold: true
            }

            GroupBox {
                title: "Port Settings"
                Layout.fillWidth: true
                Layout.maximumWidth: 400

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    // Port selection
                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Port:"
                            Layout.preferredWidth: 80
                        }
                        ComboBox {
                            id: portComboBox
                            Layout.fillWidth: true
                            model: SerialHandler.availablePorts
                            currentIndex: {
                                var idx = SerialHandler.availablePorts.indexOf(SerialHandler.portName);
                                return idx >= 0 ? idx : 0;
                            }
                            onActivated: {
                                SerialHandler.portName = currentText;
                            }
                        }
                        Button {
                            text: "Refresh"
                            onClicked: SerialHandler.refreshPorts()
                        }
                    }

                    // Baud rate selection
                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Baud Rate:"
                            Layout.preferredWidth: 80
                        }
                        ComboBox {
                            id: baudComboBox
                            Layout.fillWidth: true
                            model: [9600, 19200, 38400, 57600, 115200]
                            currentIndex: {
                                var idx = model.indexOf(SerialHandler.baudRate);
                                return idx >= 0 ? idx : 4; // Default to 115200
                            }
                            onActivated: {
                                SerialHandler.baudRate = model[currentIndex];
                            }
                        }
                    }

                    // Status display
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 60
                        color: palette.base
                        border.color: palette.mid
                        border.width: 1
                        radius: 4

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4

                            Label {
                                text: "Status: " + (SerialHandler.connected ? "Connected" : "Disconnected")
                                font.bold: true
                                color: SerialHandler.connected ? "green" : "red"
                            }
                            Label {
                                text: "Current Port: " + (SerialHandler.portName || "None")
                            }
                        }
                    }

                    // Connect/Disconnect buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            text: "Connect"
                            enabled: !SerialHandler.connected && portComboBox.currentText !== ""
                            highlighted: true
                            onClicked: SerialHandler.openPort(portComboBox.currentText)
                        }

                        Button {
                            text: "Disconnect"
                            enabled: SerialHandler.connected
                            onClicked: SerialHandler.closePort()
                        }

                        Item { Layout.fillWidth: true }

                        Button {
                            text: "Reset to Default (115200)"
                            onClicked: {
                                SerialHandler.baudRate = 115200;
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
