pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Popup {
    id: tokenPopup

    modal: true
    width: 520
    height: 420
    anchors.centerIn: Overlay.overlay
    padding: 24
    closePolicy: Popup.NoAutoClose

    signal tokenSaved()

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Label {
            text: "3D Globe Setup"
            font.pixelSize: 20
            font.bold: true
        }

        Label {
            text: "The 3D Globe needs a free access token from Cesium Ion "
                  + "to display terrain and satellite imagery.\n\n"
                  + "1. Visit ion.cesium.com and create a free account\n"
                  + "2. Go to Access Tokens in your account dashboard\n"
                  + "3. Click 'Create token' and paste it below"
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            font.pixelSize: 13
            lineHeight: 1.4
        }

        Text {
            id: linkText
            text: "<a href='https://ion.cesium.com/tokens'>Open Cesium Ion Token Page</a>"
            textFormat: Text.RichText
            font.pixelSize: 12
            color: palette.text

            onLinkActivated: function (link: string): void {
                Qt.openUrlExternally(link)
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.NoButton
                cursorShape: linkText.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: tokenInput
                objectName: "tokenInput"
                Layout.fillWidth: true
                placeholderText: "Paste your Cesium Ion access token here"
                echoMode: TextInput.Password
                enabled: !CesiumBridge.validatingToken
            }

            Button {
                objectName: "showHideButton"
                text: tokenInput.echoMode === TextInput.Password ? "Show" : "Hide"
                Layout.preferredWidth: 60

                onClicked: {
                    tokenInput.echoMode = tokenInput.echoMode === TextInput.Password
                        ? TextInput.Normal : TextInput.Password
                }
            }
        }

        // Error display
        Rectangle {
            objectName: "errorRect"
            Layout.fillWidth: true
            Layout.preferredHeight: errorLabel.implicitHeight + 16
            color: "#FFEBEE"
            radius: 4
            visible: CesiumBridge.tokenError !== "" && !CesiumBridge.validatingToken
            border.color: "#FFCDD2"
            border.width: 1

            Label {
                id: errorLabel
                anchors.fill: parent
                anchors.margins: 8
                text: CesiumBridge.tokenError
                color: "#C62828"
                wrapMode: Text.WordWrap
                font.pixelSize: 12
            }
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            BusyIndicator {
                objectName: "busyIndicator"
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                running: CesiumBridge.validatingToken
                visible: CesiumBridge.validatingToken
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                objectName: "skipButton"
                text: "Skip for Now"
                enabled: !CesiumBridge.validatingToken

                onClicked: tokenPopup.close()
            }

            Button {
                objectName: "saveButton"
                text: CesiumBridge.validatingToken ? "Validating..." : "Save Token"
                highlighted: true
                enabled: tokenInput.text.length > 0 && !CesiumBridge.validatingToken

                // [compiler] validateToken is Q_INVOKABLE on singleton — QML compiler
                // cannot AOT-compile this call site. Low severity: one-time user
                // action on button click, interpreter fallback has no UX impact.
                onClicked: CesiumBridge.validateToken(tokenInput.text)
            }
        }
    }

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    Connections {
        target: CesiumBridge

        function onTokenValidationSucceeded(): void {
            CesiumBridge.cesiumToken = tokenInput.text
            tokenPopup.tokenSaved()
            tokenPopup.close()
        }
    }
}
