pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

ScrollView {
    id: root

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

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
                    color: "#666666"
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
                    Label {
                        text: "Qt Style:"
                    }
                    Label {
                        text: "Fusion"
                        font.bold: true
                    }
                }

                RowLayout {
                    Label {
                        text: "Map Provider:"
                    }
                    Label {
                        text: "OpenStreetMap"
                        font.bold: true
                    }
                }

                RowLayout {
                    Label {
                        text: "Database Location:"
                    }
                    Label {
                        text: DatabaseManager.databasePath
                        font.bold: true
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // 3D Globe settings
        GroupBox {
            title: "3D Globe (CesiumJS)"
            Layout.fillWidth: true

            ColumnLayout {
                spacing: 8
                Layout.fillWidth: true

                Label {
                    text: "A Cesium Ion access token is required for 3D terrain and imagery tiles."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: palette.text
                }

                RowLayout {
                    spacing: 8
                    Layout.fillWidth: true

                    Label {
                        text: "Access Token:"
                    }

                    TextField {
                        id: tokenField
                        Layout.fillWidth: true
                        placeholderText: "Paste your Cesium Ion access token here"
                        text: CesiumBridge.cesiumToken
                        echoMode: TextInput.Password
                        enabled: !CesiumBridge.validatingToken
                    }

                    Button {
                        text: tokenField.echoMode === TextInput.Password ? "Show" : "Hide"
                        Layout.preferredWidth: 60

                        onClicked: {
                            tokenField.echoMode = tokenField.echoMode === TextInput.Password
                                ? TextInput.Normal : TextInput.Password
                        }
                    }

                    Button {
                        text: CesiumBridge.validatingToken ? "Validating..." : "Save Token"
                        highlighted: true
                        enabled: tokenField.text.length > 0 && !CesiumBridge.validatingToken
                        Layout.preferredWidth: 130

                        // [compiler] validateToken is Q_INVOKABLE on singleton — QML compiler
                        // cannot AOT-compile this call site. Low severity: one-time user
                        // action on button click, interpreter fallback has no UX impact.
                        onClicked: CesiumBridge.validateToken(tokenField.text)
                    }
                }

                // Validation feedback
                RowLayout {
                    spacing: 8
                    Layout.fillWidth: true

                    BusyIndicator {
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: 20
                        running: CesiumBridge.validatingToken
                        visible: CesiumBridge.validatingToken
                    }

                    Label {
                        text: CesiumBridge.tokenError
                        color: "#C62828"
                        font.pixelSize: 12
                        visible: CesiumBridge.tokenError !== "" && !CesiumBridge.validatingToken
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        id: successLabel
                        text: "Token validated and saved successfully."
                        color: "#2E7D32"
                        font.pixelSize: 12
                        visible: false
                        Layout.fillWidth: true
                    }
                }

                Text {
                    id: tokenLinkText
                    text: "<a href='https://ion.cesium.com/tokens'>Get a free token at cesium.com/ion</a>"
                    textFormat: Text.RichText
                    font.pixelSize: 11
                    color: palette.mid

                    onLinkActivated: function (link: string): void {
                        Qt.openUrlExternally(link)
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.NoButton
                        cursorShape: tokenLinkText.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }

            Connections {
                target: CesiumBridge

                function onTokenValidationSucceeded(): void {
                    CesiumBridge.cesiumToken = tokenField.text
                    successLabel.visible = true
                }

                function onTokenValidationFailed(error: string): void {
                    successLabel.visible = false
                }

                function onCesiumTokenChanged(): void {
                    tokenField.text = CesiumBridge.cesiumToken
                }

                function onValidatingTokenChanged(): void {
                    if (CesiumBridge.validatingToken) {
                        successLabel.visible = false
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
