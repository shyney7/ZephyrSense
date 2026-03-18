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

    property Component popupComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/TokenSetupPopup.qml")

    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    function init(): void {
        // Reset all mock state: cesiumToken is writable; tokenError and validatingToken
        // are read-only so reset them via simulateValidationSuccess (no popup exists
        // between tests, so the emitted signal has no handler to trigger)
        CesiumBridge.cesiumToken = ""
        CesiumBridge.simulateValidationSuccess()
    }

    function test_componentLoaded(): void {
        compare(popupComponent.status, Component.Ready,
                "Failed to load: " + popupComponent.errorString())
    }

    function test_saveButtonDisabledWhenEmpty(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var tokenInput = findChild(popup, "tokenInput")
        verify(tokenInput)
        var saveButton = findChild(popup, "saveButton")
        verify(saveButton)

        compare(tokenInput.text, "")
        compare(saveButton.enabled, false)

        popup.close()
    }

    function test_saveButtonEnabledWithText(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var tokenInput = findChild(popup, "tokenInput")
        verify(tokenInput)
        var saveButton = findChild(popup, "saveButton")
        verify(saveButton)

        tokenInput.text = "test-token-123"
        compare(saveButton.enabled, true)

        popup.close()
    }

    function test_skipClosesPopup(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        compare(popup.visible, true)

        var skipButton = findChild(popup, "skipButton")
        verify(skipButton)
        skipButton.clicked()
        tryCompare(popup, "visible", false)

        compare(CesiumBridge.cesiumToken, "")
    }

    function test_validationSuccessClosesAndSaves(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var savedSpy = createTemporaryObject(signalSpyComponent, testCase,
            { target: popup, signalName: "tokenSaved" })

        var tokenInput = findChild(popup, "tokenInput")
        verify(tokenInput)
        tokenInput.text = "valid-token-abc"

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

        var tokenInput = findChild(popup, "tokenInput")
        verify(tokenInput)
        tokenInput.text = "bad-token"

        CesiumBridge.simulateValidationFailure("Invalid token")

        compare(popup.visible, true)
        compare(CesiumBridge.cesiumToken, "")
    }

    function test_errorDisplayOnFailure(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var errorRect = findChild(popup, "errorRect")
        verify(errorRect)

        // Initially hidden (no error)
        compare(errorRect.visible, false)

        CesiumBridge.simulateValidationFailure("Invalid token format")

        // Error rect becomes visible with the error message
        tryCompare(errorRect, "visible", true)
    }

    function test_busyIndicatorDuringValidation(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var busyIndicator = findChild(popup, "busyIndicator")
        verify(busyIndicator)
        var saveButton = findChild(popup, "saveButton")
        verify(saveButton)

        // Initially not validating
        compare(busyIndicator.visible, false)

        // validateToken() sets validatingToken=true (read-only from QML)
        CesiumBridge.validateToken("test")

        tryCompare(busyIndicator, "visible", true)
        tryCompare(busyIndicator, "running", true)
        compare(saveButton.text, "Validating...")

        popup.close()
    }

    function test_showHideToggle(): void {
        var popup = createTemporaryObject(popupComponent, testCase)
        verify(popup)
        popup.open()
        waitForRendering(testCase)

        var tokenInput = findChild(popup, "tokenInput")
        verify(tokenInput)
        var showHideButton = findChild(popup, "showHideButton")
        verify(showHideButton)

        // Initially password mode
        compare(tokenInput.echoMode, TextInput.Password)
        compare(showHideButton.text, "Show")

        // Toggle to show
        showHideButton.clicked()
        compare(tokenInput.echoMode, TextInput.Normal)
        compare(showHideButton.text, "Hide")

        // Toggle back to hide
        showHideButton.clicked()
        compare(tokenInput.echoMode, TextInput.Password)
        compare(showHideButton.text, "Show")

        popup.close()
    }
}
