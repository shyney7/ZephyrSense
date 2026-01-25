import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense
import "../components"

Item {
    id: graphsViewRoot

    // Mode state
    enum VisualizationMode { Live, Historical }
    property int currentMode: GraphsView.VisualizationMode.Live
    property int updateIntervalMs: 2000
    property date historicalStart: new Date()
    property date historicalEnd: new Date()
    property var availableDates: []

    // Chart data model
    TimeSeriesChartModel {
        id: chartModel
    }

    // Live update timer
    Timer {
        id: liveUpdateTimer
        interval: graphsViewRoot.updateIntervalMs
        running: graphsViewRoot.currentMode === GraphsView.VisualizationMode.Live
        repeat: true
        onTriggered: loadLiveData()
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

            ModeBadge {
                isLive: graphsViewRoot.currentMode === GraphsView.VisualizationMode.Live
            }

            Item { Layout.fillWidth: true }

            // Update interval selector (for live mode)
            Label {
                text: "Update Interval:"
                verticalAlignment: Text.AlignVCenter
            }

            ComboBox {
                id: intervalCombo
                model: [
                    { text: "1s", ms: 1000 },
                    { text: "2s", ms: 2000 },
                    { text: "5s", ms: 5000 },
                    { text: "10s", ms: 10000 },
                    { text: "30s", ms: 30000 }
                ]
                textRole: "text"
                currentIndex: 1  // Default 2s
                Layout.preferredWidth: 80

                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        graphsViewRoot.updateIntervalMs = model[currentIndex].ms
                        switchToLiveMode()
                    }
                }
            }

            // Time range selector (presets)
            Label {
                text: "Preset:"
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
                        loadPresetFromNow(modelData.minutes)
                    }
                }
            }

            Button {
                text: "Custom Range..."
                onClicked: customRangePopup.open()
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

    // Custom date range popup
    Popup {
        id: customRangePopup
        modal: true
        width: 500
        height: 280
        anchors.centerIn: Overlay.overlay
        padding: 16

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Label {
                text: "Select Custom Date Range"
                font.pixelSize: 16
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                DateTimePicker {
                    id: startPicker
                    label: "Start Date/Time"
                    Layout.fillWidth: true
                    availableDates: graphsViewRoot.availableDates

                    onDateTimeChanged: function(dt) {
                        graphsViewRoot.historicalStart = dt
                    }
                }

                DateTimePicker {
                    id: endPicker
                    label: "End Date/Time"
                    Layout.fillWidth: true
                    availableDates: graphsViewRoot.availableDates

                    onDateTimeChanged: function(dt) {
                        graphsViewRoot.historicalEnd = dt
                    }
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Item { Layout.fillWidth: true }

                Button {
                    text: "Cancel"
                    onClicked: customRangePopup.close()
                }

                Button {
                    text: "Load Data"
                    highlighted: true
                    onClicked: {
                        switchToHistoricalMode()
                        customRangePopup.close()
                    }
                }
            }
        }
    }

    // Helper functions
    function switchToLiveMode() {
        currentMode = GraphsView.VisualizationMode.Live
        liveUpdateTimer.restart()
        loadLiveData()
    }

    function switchToHistoricalMode() {
        currentMode = GraphsView.VisualizationMode.Historical
        liveUpdateTimer.stop()
        chartModel.loadData(historicalStart, historicalEnd)
    }

    function loadLiveData() {
        // Use selected time range from preset buttons
        var minutes = 60  // Default
        for (var i = 0; i < rangeGroup.buttons.length; i++) {
            if (rangeGroup.buttons[i].checked) {
                minutes = [10, 30, 60, 300][i]
                break
            }
        }
        loadDataForRange(minutes)
    }

    function loadDataForRange(minutes) {
        let now = new Date()
        let start = new Date(now.getTime() - minutes * 60 * 1000)
        chartModel.loadData(start, now)
    }

    function loadPresetFromNow(minutes) {
        var now = new Date()
        var start = new Date(now.getTime() - minutes * 60 * 1000)
        switchToHistoricalMode()
        graphsViewRoot.historicalStart = start
        graphsViewRoot.historicalEnd = now
        chartModel.loadData(start, now)
    }

    function refreshAvailableDates() {
        availableDates = DatabaseManager.getAvailableDates()
    }

    function formatTime(msecs) {
        let date = new Date(msecs)
        return date.toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    // Load default data on component completion
    Component.onCompleted: {
        refreshAvailableDates()
        // Small delay to ensure model is ready
        Qt.callLater(function() {
            loadLiveData()
        })
    }
}
