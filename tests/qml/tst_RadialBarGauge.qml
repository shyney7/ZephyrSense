import QtQuick
import QtQuick.Shapes
import QtTest
import ZephyrSense

TestCase {
    id: testCase
    name: "RadialBarGauge"
    width: 200
    height: 200

    property Component gaugeComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/RadialBarGauge.qml")

    function test_componentLoaded() {
        compare(gaugeComponent.status, Component.Ready,
                "Failed to load: " + gaugeComponent.errorString())
    }

    function test_normalizedValue_clamp_below() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { value: -10, minValue: 0, maxValue: 100 })
        verify(gauge)
        compare(gauge.normalizedValue, 0)
    }

    function test_normalizedValue_clamp_above() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { value: 150, minValue: 0, maxValue: 100 })
        verify(gauge)
        compare(gauge.normalizedValue, 100)
    }

    function test_sweepAngle_zero() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { value: 0, minValue: 0, maxValue: 100 })
        verify(gauge)
        compare(gauge.sweepAngle, 0)
    }

    function test_sweepAngle_full() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { value: 100, minValue: 0, maxValue: 100 })
        verify(gauge)
        compare(gauge.sweepAngle, 360)
    }

    function test_sweepAngle_half() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { value: 50, minValue: 0, maxValue: 100 })
        verify(gauge)
        compare(gauge.sweepAngle, 180)
    }

    function test_sweepAngle_adaptive_co2_warning() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "co2", value: 1000, minValue: 0, maxValue: 5000 })
        verify(gauge)
        compare(gauge.sweepAngle, 180)
    }

    function test_sweepAngle_adaptive_co2_buffered_max() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "co2", value: 2500, minValue: 0, maxValue: 5000 })
        verify(gauge)
        compare(gauge.sweepAngle, 360)
    }

    function test_sweepAngle_adaptive_co2_danger() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "co2", value: 2000, minValue: 0, maxValue: 5000 })
        verify(gauge)
        compare(gauge.sweepAngle, 270)
    }

    function test_sweepAngle_updates_on_threshold_change() {
        ThresholdManager.setCo2Thresholds(1000, 2000)
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "co2", value: 1500, minValue: 0, maxValue: 5000 })
        verify(gauge)

        var beforeSweep = gauge.sweepAngle
        var beforeEpoch = gauge.thresholdsEpoch
        ThresholdManager.setCo2Thresholds(1200, 2200)
        wait(0)

        compare(gauge.thresholdsEpoch, beforeEpoch + 1)
        verify(gauge.sweepAngle !== beforeSweep)
        ThresholdManager.setCo2Thresholds(1000, 2000)
    }

    function test_progressColor_updates_on_threshold_change() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "co2", value: 1500, minValue: 0, maxValue: 5000 })
        verify(gauge)

        var beforeEpoch = gauge.thresholdsEpoch
        ThresholdManager.emitThresholdsChanged()
        wait(0)

        compare(gauge.thresholdsEpoch, beforeEpoch + 1)
    }

    function test_gaugeSize() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { width: 200, height: 150 })
        verify(gauge)
        // gaugeSize = Math.max(Math.min(200, 150), 1) = 150
        compare(gauge.gaugeSize, 150)
    }

    function test_gaugeSize_minimum() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { width: 0, height: 0 })
        verify(gauge)
        // gaugeSize minimum is 1
        compare(gauge.gaugeSize, 1)
    }

    function test_progressColor_from_mock() {
        var gauge = createTemporaryObject(gaugeComponent, testCase,
                                          { sensorKey: "temperature", value: 25 })
        verify(gauge)
        // Mock always returns "#4CAF50"
        compare(gauge.progressColor, "#4caf50")
    }
}
