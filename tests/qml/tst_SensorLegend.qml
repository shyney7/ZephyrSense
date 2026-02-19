import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest

pragma ComponentBehavior: Bound

TestCase {
    id: testCase
    name: "SensorLegend"
    width: 800
    height: 60
    when: windowShown

    property Component legendComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/SensorLegend.qml")

    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    function test_componentLoaded() {
        compare(legendComponent.status, Component.Ready,
                "Failed to load: " + legendComponent.errorString())
    }

    function test_defaultSelectedSensor() {
        var legend = createTemporaryObject(legendComponent, testCase)
        verify(legend)
        compare(legend.selectedSensor, 5)
    }

    function test_sensorSelectedSignal() {
        var legend = createTemporaryObject(legendComponent, testCase)
        verify(legend)
        var spy = createTemporaryObject(signalSpyComponent, testCase,
                                        { target: legend, signalName: "sensorSelected" })
        verify(spy)
        verify(spy.valid)

        // Change selection programmatically and emit signal
        legend.selectedSensor = 3
        legend.sensorSelected(3)
        compare(spy.count, 1)
        compare(spy.signalArguments[0][0], 3)
    }

    function test_selectedHighlight() {
        var legend = createTemporaryObject(legendComponent, testCase, { selectedSensor: 1 })
        verify(legend)
        waitForRendering(legend)

        // The selected sensor (column 1, index 0) should have opacity 1.0
        var repeater = findChild(legend, "sensorRepeater")
        verify(repeater)
        var selectedItem = repeater.itemAt(0)
        verify(selectedItem)
        compare(selectedItem.opacity, 1.0)
    }

    function test_unselectedTransparent() {
        var legend = createTemporaryObject(legendComponent, testCase, { selectedSensor: 1 })
        verify(legend)
        waitForRendering(legend)

        // Non-selected sensor (index 1, column 2) should have opacity 0.6
        var repeater = findChild(legend, "sensorRepeater")
        verify(repeater)
        var unselectedItem = repeater.itemAt(1)
        verify(unselectedItem)
        compare(unselectedItem.opacity, 0.6)
        compare(unselectedItem.color, "#00000000")  // transparent
    }

    function test_nineSensors() {
        var legend = createTemporaryObject(legendComponent, testCase)
        verify(legend)
        waitForRendering(legend)

        var repeater = findChild(legend, "sensorRepeater")
        verify(repeater)
        compare(repeater.count, 9)
    }
}
