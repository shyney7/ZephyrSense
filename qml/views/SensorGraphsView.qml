pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense
import com.kdab.dockwidgets 2.0 as KDDW
import "../components"


Item {
    id: graphsViewRoot

    readonly property DockStateTracker dockTracker: DockStateTracker
    readonly property DatabaseManager dbManager: DatabaseManager

    // Mode state
    enum VisualizationMode { Live, Historical }
    property int currentMode: SensorGraphsView.VisualizationMode.Live
    property int updateIntervalMs: 2000
    property date historicalStart: new Date()
    property date historicalEnd: new Date()
    property list<string> availableDates: []
    property alias rangeGroup: rangeGroupInst

    // Chart data model (shared by all 9 dock charts)
    TimeSeriesChartModel {
        id: chartModel
    }

    // Live update timer — keeps running when hidden if any dock is outside
    // the main window, so detached charts stay updated in other views
    Timer {
        id: liveUpdateTimer
        interval: graphsViewRoot.updateIntervalMs
        running: graphsViewRoot.currentMode === SensorGraphsView.VisualizationMode.Live
                 && (graphsViewRoot.visible || graphsViewRoot.dockTracker.hasDocksOutsideMainWindow)
        repeat: true
        onTriggered: graphsViewRoot.loadLiveData()
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
                text: "Sensor Graphs"
                font.pixelSize: 20
                font.bold: true
            }

            ModeBadge {
                isLive: graphsViewRoot.currentMode === SensorGraphsView.VisualizationMode.Live
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
                        graphsViewRoot.switchToLiveMode()
                    }
                }
            }

            // Time range selector (presets)
            Label {
                text: "Preset:"
                verticalAlignment: Text.AlignVCenter
            }

            ButtonGroup { id: rangeGroupInst }

            Repeater {
                model: [
                    { label: "10m", minutes: 10 },
                    { label: "30m", minutes: 30 },
                    { label: "1h", minutes: 60 },
                    { label: "5h", minutes: 300 }
                ]

                Button {
                    required property int index
                    required property string label
                    required property int minutes
                    text: label
                    checkable: true
                    checked: index === 2  // Default: 1h
                    ButtonGroup.group: graphsViewRoot.rangeGroup

                    onClicked: {
                        graphsViewRoot.loadPresetFromNow(minutes)
                    }
                }
            }

            Button {
                text: "Custom Range..."
                onClicked: customRangePopup.open()
            }
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
                          "Range: " + graphsViewRoot.formatTime(chartModel.xMin) + " - " + graphsViewRoot.formatTime(chartModel.xMax) :
                          "No data loaded"
                    font.pixelSize: 12
                    color: "#757575"
                }
            }
        }

        // Dockable charts area
        KDDW.DockingArea {
            id: dockingArea
            uniqueName: "SensorGraphsDockArea"
            Layout.fillWidth: true
            Layout.fillHeight: true
            readonly property int locationOnBottom: KDDW.KDDockWidgets.Location_OnBottom

            // Dock widgets for each sensor
            KDDW.DockWidget {
                id: dockPartectorNum
                uniqueName: "dock-partector-number"
                title: "PNC UFP"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 1
                }
            }

            KDDW.DockWidget {
                id: dockPartectorDiam
                uniqueName: "dock-partector-diameter"
                title: "\u00D8 UFP"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 2
                }
            }

            KDDW.DockWidget {
                id: dockPartectorMass
                uniqueName: "dock-partector-mass"
                title: "PM0.3"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 3
                }
            }

            KDDW.DockWidget {
                id: dockGrimmValue
                uniqueName: "dock-grimm-value"
                title: "PNC PM"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 4
                }
            }

            KDDW.DockWidget {
                id: dockTemperature
                uniqueName: "dock-temperature"
                title: "Temperature"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 5
                }
            }

            KDDW.DockWidget {
                id: dockHumidity
                uniqueName: "dock-humidity"
                title: "Humidity"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 6
                }
            }

            KDDW.DockWidget {
                id: dockPressure
                uniqueName: "dock-pressure"
                title: "Pressure"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 7
                }
            }

            KDDW.DockWidget {
                id: dockAltitude
                uniqueName: "dock-altitude"
                title: "Altitude"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 8
                }
            }

            KDDW.DockWidget {
                id: dockCo2
                uniqueName: "dock-co2"
                title: "CO\u2082"

                SensorDockChart {
                    chartModel: chartModel
                    sensorColumn: 9
                }
            }

            // Initial layout: all 9 as tabs in a single group
            Component.onCompleted: {
                // Register docks for outside-main-window tracking
                graphsViewRoot.dockTracker.trackDockWidget(dockPartectorNum)
                graphsViewRoot.dockTracker.trackDockWidget(dockPartectorDiam)
                graphsViewRoot.dockTracker.trackDockWidget(dockPartectorMass)
                graphsViewRoot.dockTracker.trackDockWidget(dockGrimmValue)
                graphsViewRoot.dockTracker.trackDockWidget(dockTemperature)
                graphsViewRoot.dockTracker.trackDockWidget(dockHumidity)
                graphsViewRoot.dockTracker.trackDockWidget(dockPressure)
                graphsViewRoot.dockTracker.trackDockWidget(dockAltitude)
                graphsViewRoot.dockTracker.trackDockWidget(dockCo2)

                dockingArea.addDockWidget(dockPartectorNum, dockingArea.locationOnBottom)
                dockPartectorNum.addDockWidgetAsTab(dockPartectorDiam)
                dockPartectorNum.addDockWidgetAsTab(dockPartectorMass)
                dockPartectorNum.addDockWidgetAsTab(dockGrimmValue)
                dockPartectorNum.addDockWidgetAsTab(dockTemperature)
                dockPartectorNum.addDockWidgetAsTab(dockHumidity)
                dockPartectorNum.addDockWidgetAsTab(dockPressure)
                dockPartectorNum.addDockWidgetAsTab(dockAltitude)
                dockPartectorNum.addDockWidgetAsTab(dockCo2)
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

                    onDateTimeChanged: function(dt: date): void {
                        graphsViewRoot.historicalStart = dt
                    }
                }

                DateTimePicker {
                    id: endPicker
                    label: "End Date/Time"
                    Layout.fillWidth: true
                    availableDates: graphsViewRoot.availableDates

                    onDateTimeChanged: function(dt: date): void {
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
                        graphsViewRoot.switchToHistoricalMode()
                        customRangePopup.close()
                    }
                }
            }
        }
    }

    // Helper functions
    function switchToLiveMode(): void {
        graphsViewRoot.currentMode = SensorGraphsView.VisualizationMode.Live
        liveUpdateTimer.restart()
        graphsViewRoot.loadLiveData()
    }

    function switchToHistoricalMode(): void {
        graphsViewRoot.currentMode = SensorGraphsView.VisualizationMode.Historical
        liveUpdateTimer.stop()
        chartModel.loadData(graphsViewRoot.historicalStart, graphsViewRoot.historicalEnd)
    }

    function loadLiveData(): void {
        // Use selected time range from preset buttons
        var minutes = 60  // Default
        for (var i = 0; i < graphsViewRoot.rangeGroup.buttons.length; i++) {
            if (graphsViewRoot.rangeGroup.buttons[i].checked) {
                minutes = [10, 30, 60, 300][i]
                break
            }
        }
        graphsViewRoot.loadDataForRange(minutes)
    }

    function loadDataForRange(minutes: int): void {
        var now = new Date()
        var start = new Date(now.getTime() - minutes * 60 * 1000)
        chartModel.loadData(start, now)
    }

    function loadPresetFromNow(minutes: int): void {
        var now = new Date()
        var start = new Date(now.getTime() - minutes * 60 * 1000)
        graphsViewRoot.switchToHistoricalMode()
        graphsViewRoot.historicalStart = start
        graphsViewRoot.historicalEnd = now
        chartModel.loadData(start, now)
    }

    function refreshAvailableDates(): void {
        graphsViewRoot.availableDates = graphsViewRoot.dbManager.getAvailableDates()
    }

    function formatTime(msecs: real): string {
        var dt = new Date(msecs)
        return dt.toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    // Load default data on component completion
    Component.onCompleted: {
        graphsViewRoot.refreshAvailableDates()
        // Small delay to ensure model is ready
        Qt.callLater(function() {
            graphsViewRoot.loadLiveData()
        })
    }
}
