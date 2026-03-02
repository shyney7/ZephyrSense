pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtTest
import ZephyrSense

TestCase {
    id: testCase
    name: "SensorDockChart"
    width: 800
    height: 500
    when: windowShown

    property Component chartComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/SensorDockChart.qml")

    property TimeSeriesChartModel chartModel: TimeSeriesChartModel {}

    function init(): void {
        chartModel.resetForTest()
        chartModel.setXRange(1000, 2000)
        chartModel.setBoundsForColumn(5, 10.0, 20.0)
    }

    function test_componentLoaded(): void {
        compare(chartComponent.status, Component.Ready,
                "Failed to load: " + chartComponent.errorString())
    }

    function test_initialBoundsUseCombinedGetter(): void {
        const chart = createTemporaryObject(chartComponent, testCase, {
            "chartModel": chartModel,
            "sensorColumn": 5
        })
        verify(chart)
        waitForRendering(chart)

        compare(chart.localYMin, 10.0)
        compare(chart.localYMax, 20.0)
        compare(chartModel.boundsCallCount, 1)
    }

    function test_boundsChangedRecomputesOnce(): void {
        const chart = createTemporaryObject(chartComponent, testCase, {
            "chartModel": chartModel,
            "sensorColumn": 5
        })
        verify(chart)
        waitForRendering(chart)

        const callsBefore = chartModel.boundsCallCount
        chartModel.setBoundsForColumn(5, 11.5, 23.5)
        chartModel.emitBoundsChanged()

        tryCompare(chart, "localYMin", 11.5, 500)
        tryCompare(chart, "localYMax", 23.5, 500)
        compare(chartModel.boundsCallCount, callsBefore + 1)
    }
}
