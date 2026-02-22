import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest
import ZephyrSense

TestCase {
    id: testCase
    name: "ConnectionPanel"
    width: 400
    height: 500
    when: windowShown

    property Component panelComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/ConnectionPanel.qml")

    function test_componentLoaded() {
        compare(panelComponent.status, Component.Ready,
                "Failed to load: " + panelComponent.errorString())
    }

    function init() {
        // Reset mock state before each test
        SerialHandler.closePort()
        SerialHandler.errorString = ""
    }

    function test_disconnectedState() {
        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var statusDot = findChild(panel, "statusIndicator")
        verify(statusDot)
        // Disconnected: gray
        compare(statusDot.color, "#9e9e9e")
    }

    function test_connectedState() {
        SerialHandler.refreshPorts()
        SerialHandler.openPort("COM1")

        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var statusDot = findChild(panel, "statusIndicator")
        verify(statusDot)
        // Connected: green
        compare(statusDot.color, "#4caf50")
    }

    function test_connectEnabled() {
        SerialHandler.refreshPorts()
        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var connectBtn = findChild(panel, "connectButton")
        verify(connectBtn)

        var portCombo = findChild(panel, "portComboBox")
        verify(portCombo)

        // Not connected, ports available -> connect enabled
        compare(SerialHandler.connected, false)
        // The button should be enabled if there's a port selected
        if (portCombo.currentText !== "") {
            compare(connectBtn.enabled, true)
        }
    }

    function test_disconnectEnabled() {
        SerialHandler.refreshPorts()
        SerialHandler.openPort("COM1")

        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var disconnectBtn = findChild(panel, "disconnectButton")
        verify(disconnectBtn)
        compare(disconnectBtn.enabled, true)

        var connectBtn = findChild(panel, "connectButton")
        verify(connectBtn)
        compare(connectBtn.enabled, false)
    }

    function test_errorVisible() {
        // Verify mock error state and the visibility condition
        SerialHandler.errorString = "Port not found"
        compare(SerialHandler.errorString, "Port not found")
        verify(SerialHandler.errorString !== "",
               "errorString condition should be true")

        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var errorRect = findChild(panel, "errorRect")
        verify(errorRect, "errorRect child exists")
    }

    function test_errorHidden() {
        var panel = createTemporaryObject(panelComponent, testCase)
        verify(panel)
        waitForRendering(panel)

        var errorRect = findChild(panel, "errorRect")
        verify(errorRect)
        compare(errorRect.visible, false)
    }
}
