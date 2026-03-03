pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Item {
    id: connectionTabRoot

    readonly property SerialHandler serialHandler: SerialHandler

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
                            model: connectionTabRoot.serialHandler.availablePorts
                            currentIndex: {
                                var idx = connectionTabRoot.serialHandler.availablePorts.indexOf(connectionTabRoot.serialHandler.currentPort);
                                return idx >= 0 ? idx : 0;
                            }
                        }
                        Button {
                            text: "Refresh"
                            onClicked: connectionTabRoot.serialHandler.refreshPorts()
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
                                var idx = model.indexOf(connectionTabRoot.serialHandler.baudRate);
                                return idx >= 0 ? idx : 4; // Default to 115200
                            }
                            onActivated: {
                                connectionTabRoot.serialHandler.baudRate = model[currentIndex];
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
                                text: "Status: " + (connectionTabRoot.serialHandler.connected ? "Connected" : "Disconnected")
                                font.bold: true
                                color: connectionTabRoot.serialHandler.connected ? "green" : "red"
                            }
                            Label {
                                text: "Current Port: " + (connectionTabRoot.serialHandler.currentPort || "None")
                            }
                        }
                    }

                    // Connect/Disconnect buttons
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Button {
                            text: "Connect"
                            enabled: !connectionTabRoot.serialHandler.connected && portComboBox.currentText !== ""
                            highlighted: true
                            onClicked: connectionTabRoot.serialHandler.openPort(portComboBox.currentText)
                        }

                        Button {
                            text: "Disconnect"
                            enabled: connectionTabRoot.serialHandler.connected
                            onClicked: connectionTabRoot.serialHandler.closePort()
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "Reset to Default (115200)"
                            onClicked: {
                                connectionTabRoot.serialHandler.baudRate = 115200;
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
