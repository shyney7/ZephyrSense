import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Item {
    id: graphsViewRoot

    // Chart data model
    TimeSeriesChartModel {
        id: chartModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Header with title and time range selector
        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Label {
                text: "Time-Series Graph"
                font.pixelSize: 20
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            // Time range selector
            Label {
                text: "Time Range:"
                verticalAlignment: Text.AlignVCenter
            }

            ButtonGroup { id: rangeGroup }

            Repeater {
                model: [
                    { label: "10m", minutes: 10 },
                    { label: "30m", minutes: 30 },
                    { label: "1h", minutes: 60 },
                    { label: "5h", minutes: 300 }
                ]

                Button {
                    text: modelData.label
                    checkable: true
                    checked: index === 2  // Default: 1h
                    ButtonGroup.group: rangeGroup

                    onClicked: {
                        loadDataForRange(modelData.minutes)
                    }
                }
            }

            Button {
                text: "Refresh"
                onClicked: {
                    // Reload with current selection (default 1h if none)
                    let selectedMinutes = 60
                    for (let i = 0; i < rangeGroup.buttons.length; i++) {
                        if (rangeGroup.buttons[i].checked) {
                            selectedMinutes = [10, 30, 60, 300][i]
                            break
                        }
                    }
                    loadDataForRange(selectedMinutes)
                }
            }
        }

        // Sensor legend
        SensorLegend {
            id: legend
            Layout.fillWidth: true
            Layout.preferredHeight: 40

            onSensorSelected: function(column) {
                chart.activeColumn = column
            }
        }

        // Main chart area
        TimeSeriesChart {
            id: chart
            Layout.fillWidth: true
            Layout.fillHeight: true

            chartModel: chartModel
            activeColumn: legend.selectedSensor
        }

        // Status bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "#EEEEEE"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8

                Label {
                    text: chartModel.dataCount + " data points"
                    font.pixelSize: 12
                    color: "#757575"
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: chartModel.dataCount > 0 ?
                          "Range: " + formatTime(chartModel.xMin) + " - " + formatTime(chartModel.xMax) :
                          "No data loaded"
                    font.pixelSize: 12
                    color: "#757575"
                }
            }
        }
    }

    // Helper functions
    function loadDataForRange(minutes) {
        let now = new Date()
        let start = new Date(now.getTime() - minutes * 60 * 1000)
        chartModel.loadData(start, now)
    }

    function formatTime(msecs) {
        let date = new Date(msecs)
        return date.toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    // Load default data on component completion
    Component.onCompleted: {
        // Small delay to ensure model is ready
        Qt.callLater(function() {
            loadDataForRange(60)  // Default: last hour
        })
    }
}
