import QtQuick
import QtQuick.Controls
import QtCharts
import ZephyrSense

ChartView {
    id: chartView

    // Model reference (set from parent)
    property var chartModel: null

    // Currently displayed sensor column (1-9, matching TimeSeriesChartModel.Columns)
    property int activeColumn: 5  // Default: Temperature

    // Sensor names for display
    readonly property var sensorNames: [
        "", // 0 = Timestamp (not displayed)
        "Partector Number",
        "Partector Diameter",
        "Partector Mass",
        "Grimm Value",
        "Temperature",
        "Humidity",
        "Pressure",
        "Altitude",
        "CO2"
    ]

    // Sensor colors
    readonly property var sensorColors: [
        "transparent",
        "#E91E63",  // Partector Number - Pink
        "#9C27B0",  // Partector Diameter - Purple
        "#673AB7",  // Partector Mass - Deep Purple
        "#3F51B5",  // Grimm Value - Indigo
        "#FF5722",  // Temperature - Deep Orange
        "#2196F3",  // Humidity - Blue
        "#009688",  // Pressure - Teal
        "#4CAF50",  // Altitude - Green
        "#795548"   // CO2 - Brown
    ]

    title: sensorNames[activeColumn] || "Sensor Data"
    antialiasing: true
    animationOptions: ChartView.NoAnimation
    legend.visible: false  // Using custom legend

    DateTimeAxis {
        id: timeAxis
        format: "hh:mm"
        tickCount: 6
        min: chartModel ? new Date(chartModel.xMin) : new Date()
        max: chartModel ? new Date(chartModel.xMax) : new Date()
    }

    ValueAxis {
        id: valueAxis
        labelFormat: "%.1f"
        min: chartModel ? chartModel.yMin : 0
        max: chartModel ? chartModel.yMax : 100
    }

    LineSeries {
        id: dataSeries
        name: sensorNames[activeColumn]
        color: sensorColors[activeColumn]
        width: 2
        axisX: timeAxis
        axisY: valueAxis

        VXYModelMapper {
            model: chartModel
            xColumn: 0  // Timestamp column
            yColumn: activeColumn
        }
    }

    // Update Y bounds when active column changes
    onActiveColumnChanged: {
        if (chartModel) {
            chartModel.updateYBoundsForColumn(activeColumn)
        }
    }
}
