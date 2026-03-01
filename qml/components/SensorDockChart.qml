pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtGraphs
import ZephyrSense

Item {
    id: chartView

    // Model reference (shared across all dock charts)
    required property TimeSeriesChartModel chartModel
    // Sensor column index (1-9, fixed at creation)
    required property int sensorColumn

    // Sensor metadata
    readonly property list<string> sensorNames: [
        "",
        "PNC UFP",
        "\u00D8 UFP",
        "PM0.3",
        "PNC PM",
        "Temperature",
        "Humidity",
        "Pressure",
        "Altitude",
        "CO\u2082"
    ]

    readonly property list<color> sensorColors: [
        "transparent",
        "#E91E63",
        "#9C27B0",
        "#673AB7",
        "#3F51B5",
        "#FF5722",
        "#2196F3",
        "#009688",
        "#4CAF50",
        "#795548"
    ]

    readonly property list<string> sensorUnits: [
        "",
        "#/cm\u00B3",
        "nm",
        "\u00B5g/m\u00B3",
        "#/cm\u00B3",
        "\u00B0C",
        "%",
        "hPa",
        "m",
        "ppm"
    ]

    // Per-chart Y-bounds (local, not shared)
    property real localYMin: 0
    property real localYMax: 100

    XYModelMapper {
        model: chartView.chartModel
        series: dataSeries
        orientation: Qt.Vertical
        xSection: 0
        ySection: chartView.sensorColumn
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: chartView.sensorNames[chartView.sensorColumn]
                  + " [" + chartView.sensorUnits[chartView.sensorColumn] + "]"
            font.bold: true
            font.pixelSize: 15
        }

        GraphsView {
            id: graphsView
            Layout.fillWidth: true
            Layout.fillHeight: true

            axisX: DateTimeAxis {
                id: timeAxis
                labelFormat: "hh:mm"
                tickInterval: 0
                min: chartView.chartModel ? new Date(chartView.chartModel.xMin) : new Date()
                max: chartView.chartModel ? new Date(chartView.chartModel.xMax) : new Date()
            }

            axisY: ValueAxis {
                id: valueAxis
                labelDecimals: 1
                min: chartView.localYMin
                max: chartView.localYMax
            }

            theme: GraphsTheme {
                axisXLabelFont.bold: true
                axisXLabelFont.pixelSize: 12
                axisYLabelFont.bold: true
                axisYLabelFont.pixelSize: 12
            }

            LineSeries {
                id: dataSeries
                name: chartView.sensorNames[chartView.sensorColumn]
                color: chartView.sensorColors[chartView.sensorColumn]
                width: 3
            }
        }
    }

    function updateLocalBounds(): void {
        if (chartView.chartModel) {
            chartView.localYMin = chartView.chartModel.getYMinForColumn(chartView.sensorColumn)
            chartView.localYMax = chartView.chartModel.getYMaxForColumn(chartView.sensorColumn)
        }
    }

    Connections {
        target: chartView.chartModel
        function onBoundsChanged(): void { chartView.updateLocalBounds() }
    }

    Component.onCompleted: chartView.updateLocalBounds()
}
