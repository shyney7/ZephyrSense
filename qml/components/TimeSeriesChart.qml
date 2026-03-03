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

    // Model reference (set from parent)
    property TimeSeriesChartModel chartModel: null

    // Currently displayed sensor column (1-9, matching TimeSeriesChartModel.Columns)
    property int activeColumn: 5  // Default: Temperature

    // Sensor names for display
    readonly property list<string> sensorNames: [
        "", // 0 = Timestamp (not displayed)
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

    // Sensor colors
    readonly property list<color> sensorColors: [
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

    // NOTE: ySection binding triggers QTBUG-142437 console warnings on Qt <6.10.2
    XYModelMapper {
        model: chartView.chartModel
        series: dataSeries
        orientation: Qt.Vertical
        xSection: 0  // Timestamp column
        ySection: chartView.activeColumn
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: chartView.sensorNames[chartView.activeColumn] || "Sensor Data"
            font.bold: true
            font.pixelSize: 14
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
                min: chartView.chartModel ? chartView.chartModel.yMin : 0
                max: chartView.chartModel ? chartView.chartModel.yMax : 100
            }

            LineSeries {
                id: dataSeries
                name: chartView.sensorNames[chartView.activeColumn]
                color: chartView.sensorColors[chartView.activeColumn]
                width: 2
            }
        }
    }

    // Update Y bounds when active column changes
    onActiveColumnChanged: {
        if (chartView.chartModel) {
            chartView.chartModel.activeColumn = chartView.activeColumn
        }
    }

    // Sync active column when chartModel is first assigned
    onChartModelChanged: {
        if (chartView.chartModel) {
            chartView.chartModel.activeColumn = chartView.activeColumn
        }
    }
}
