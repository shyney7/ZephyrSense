pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtTest
import ZephyrSense

TestCase {
    id: testCase
    name: "TokenSetupPopup"
    width: 600
    height: 500
    when: windowShown

    Component {
        id: popupComponent

        Popup {
            id: tokenPopup
            modal: true
            width: 520
            height: 420
            padding: 24
            closePolicy: Popup.NoAutoClose

            signal tokenSaved()

            property alias tokenInput: tokenInputField
            property alias saveButton: saveBtn
            property alias skipButton: skipBtn

            Column {
                spacing: 16

                TextField {
                    id: tokenInputField
                    width: 400
                    placeholderText: "Paste token"
                    echoMode: TextInput.Password
                    enabled: !CesiumBridge.validatingToken
                }

                Row {
                    spacing: 8

                    Button {
                        id: skipBtn
                        text: "Skip for Now"
                        enabled: !CesiumBridge.validatingToken
                        onClicked: tokenPopup.close()
                    }

                    Button {
                        id: saveBtn
                        text: "Save Token"
                        highlighted: true
                        enabled: tokenInputField.text.length > 0 && !CesiumBridge.validatingToken
                        onClicked: CesiumBridge.validateToken(tokenInputField.text)
                    }
                }
            }

            Connections {
                target: CesiumBridge

                function onTokenValidationSucceeded(): void {
                    CesiumBridge.cesiumToken = tokenPopup.tokenInput.text
                    tokenPopup.tokenSaved()
                    tokenPopup.close()
                }
            }
        }
    }

    function init(): void {
        // Reset mock state before each test (use property assignment, not setter)
        CesiumBridge.cesiumToken = ""
    }

    function test_saveButtonDisabledWhenEmpty(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        // Token input is empty — save button should be disabled
        compare(popup.tokenInput.text, "")
        compare(popup.saveButton.enabled, false)

        popup.close()
    }

    function test_saveButtonEnabledWithText(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        popup.tokenInput.text = "test-token-123"
        compare(popup.saveButton.enabled, true)

        popup.close()
    }

    function test_skipClosesPopup(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        compare(popup.visible, true)
        popup.skipButton.clicked()
        tryCompare(popup, "visible", false)

        // Token should remain empty after skip
        compare(CesiumBridge.cesiumToken, "")
    }

    function test_validationSuccessClosesAndSaves(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var savedSpy = createTemporaryObject(
            Qt.createComponent("QtTest", "SignalSpy"), testCase,
            { target: popup, signalName: "tokenSaved" })

        popup.tokenInput.text = "valid-token-abc"

        // Simulate validation success from the mock
        CesiumBridge.simulateValidationSuccess()

        tryCompare(popup, "visible", false)
        compare(CesiumBridge.cesiumToken, "valid-token-abc")
        compare(savedSpy.count, 1)
    }

    function test_validationFailureKeepsPopupOpen(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        popup.tokenInput.text = "bad-token"

        // Simulate validation failure
        CesiumBridge.simulateValidationFailure("Invalid token")

        // Popup should remain open
        compare(popup.visible, true)
        // Token should NOT have been saved
        compare(CesiumBridge.cesiumToken, "")
    }
}
