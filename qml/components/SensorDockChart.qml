import QtQuick
import QtCharts

ChartView {
    id: chartView

    // Model reference (shared across all dock charts)
    required property var chartModel
    // Sensor column index (1-9, fixed at creation)
    required property int sensorColumn

    // Sensor metadata
    readonly property var sensorNames: [
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

    readonly property var sensorColors: [
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

    readonly property var sensorUnits: [
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

    title: sensorNames[sensorColumn] + " [" + sensorUnits[sensorColumn] + "]"
    titleFont.bold: true
    titleFont.pixelSize: 15
    antialiasing: true
    animationOptions: ChartView.NoAnimation
    legend.visible: false

    DateTimeAxis {
        id: timeAxis
        format: "hh:mm"
        tickCount: 6
        labelsFont.bold: true
        labelsFont.pixelSize: 12
        min: chartView.chartModel ? new Date(chartView.chartModel.xMin) : new Date()
        max: chartView.chartModel ? new Date(chartView.chartModel.xMax) : new Date()
    }

    ValueAxis {
        id: valueAxis
        labelFormat: "%.1f"
        labelsFont.bold: true
        labelsFont.pixelSize: 12
        min: chartView.localYMin
        max: chartView.localYMax
    }

    LineSeries {
        id: dataSeries
        name: chartView.sensorNames[chartView.sensorColumn]
        color: chartView.sensorColors[chartView.sensorColumn]
        width: 3
        axisX: timeAxis
        axisY: valueAxis

        VXYModelMapper {
            model: chartView.chartModel
            xColumn: 0
            yColumn: chartView.sensorColumn
        }
    }

    function updateLocalBounds() {
        if (chartModel) {
            var b = chartModel.getYBoundsForColumn(sensorColumn)
            localYMin = b.yMin
            localYMax = b.yMax
        }
    }

    Connections {
        target: chartView.chartModel
        function onBoundsChanged() { chartView.updateLocalBounds() }
    }

    Component.onCompleted: updateLocalBounds()
}
