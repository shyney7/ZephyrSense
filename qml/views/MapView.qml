import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning
import ZephyrSense

Item {
    id: mapViewRoot

    // Signal to request navigation to dashboard with specific reading
    signal showDashboardForReading(int readingId)

    // Mode state
    enum VisualizationMode {
        Live,
        Historical
    }
    property int currentMode: MapView.VisualizationMode.Live
    property int updateIntervalMs: 2000  // Default 2 seconds for live mode
    property date historicalStart: new Date()
    property date historicalEnd: new Date()
    property var availableDates: []

    // Model instance for map markers
    SensorReadingModel {
        id: sensorModel
    }

    // Main map container
    MapView {
        id: mapView
        anchors.fill: parent

        map.plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.useragent"
                value: "ZephyrSense/1.0"
            }
        }

        // Default view: Wuppertal, Germany
        map.center: QtPositioning.coordinate(51.2562, 7.1508)
        map.zoomLevel: 10

        // Marker layer using MapItemView
        MapItemView {
            id: markerView
            model: sensorModel
            parent: mapView.map

            delegate: SensorMarker {
                // Required properties auto-injected from model roles:
                // latitude, longitude, tooltipText, readingId

                onMarkerClicked: function (id) {
                    mapViewRoot.showDashboardForReading(id);
                }
            }
        }
    }

    // Mode badge overlay
    ModeBadge {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        isLive: mapViewRoot.currentMode === MapView.VisualizationMode.Live
        z: 2
    }

    // Info overlay showing point count (moved to left to avoid badge overlap)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 16
        width: infoLabel.width + 24
        height: infoLabel.height + 12
        color: '#3f589e'
        radius: 4
        opacity: 0.95

        Label {
            id: infoLabel
            anchors.centerIn: parent
            text: sensorModel.count + " points"
            font.pixelSize: 12
        }
    }

    // Live mode prune timer (removes old readings outside time window)
    Timer {
        id: liveUpdateTimer
        interval: mapViewRoot.updateIntervalMs
        running: mapViewRoot.currentMode === MapView.VisualizationMode.Live
        repeat: true
        onTriggered: {
            // Prune old readings outside the time window
            var windowMinutes = getWindowMinutes();
            sensorModel.pruneOldReadings(windowMinutes);
        }
    }

    // Control panel at bottom
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: controlLayout.implicitHeight + 24
        color: '#3e3f41'
        opacity: 0.93
        border.color: "#CCCCCC"
        border.width: 1

        ColumnLayout {
            id: controlLayout
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // Update interval selector (for live mode)
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: "Update Interval:"
                    font.pixelSize: 12
                }

                ComboBox {
                    id: intervalCombo
                    Layout.preferredWidth: 120
                    model: [
                        {
                            text: "1 second",
                            value: 1000
                        },
                        {
                            text: "2 seconds",
                            value: 2000
                        },
                        {
                            text: "5 seconds",
                            value: 5000
                        },
                        {
                            text: "10 seconds",
                            value: 10000
                        },
                        {
                            text: "30 seconds",
                            value: 30000
                        }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    currentIndex: 1  // Default to 2 seconds

                    onCurrentValueChanged: {
                        if (currentValue !== undefined) {
                            mapViewRoot.updateIntervalMs = currentValue;
                            // If already in live mode, don't reload - just update interval
                            // If in historical mode, switch to live mode with full reload
                            if (mapViewRoot.currentMode === MapView.VisualizationMode.Historical) {
                                switchToLiveMode(true);  // force reload
                            } else {
                                liveUpdateTimer.restart();
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Label {
                    text: "Time Window:"
                    font.pixelSize: 12
                }

                ButtonGroup {
                    id: windowGroup
                }

                Repeater {
                    model: [
                        {
                            text: "10m",
                            minutes: 10
                        },
                        {
                            text: "30m",
                            minutes: 30
                        },
                        {
                            text: "1h",
                            minutes: 60
                        },
                        {
                            text: "6h",
                            minutes: 360
                        },
                        {
                            text: "24h",
                            minutes: 1440
                        }
                    ]

                    Button {
                        required property var modelData
                        required property int index
                        property int minutes: modelData.minutes

                        text: modelData.text
                        checkable: true
                        checked: index === 2  // Default to 1h
                        ButtonGroup.group: windowGroup
                        Layout.preferredWidth: 50

                        onClicked: {
                            // Clicking time window stays in current mode but reloads with new window
                            if (mapViewRoot.currentMode === MapView.VisualizationMode.Live) {
                                switchToLiveMode(true);  // force reload with new window
                            }
                        }
                    }
                }
            }

            // Preset buttons for quick historical ranges
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: "Quick Range:"
                    font.pixelSize: 12
                }

                Repeater {
                    model: [
                        {
                            text: "Last 1h",
                            preset: "1h"
                        },
                        {
                            text: "Last 6h",
                            preset: "6h"
                        },
                        {
                            text: "Last 24h",
                            preset: "24h"
                        },
                        {
                            text: "Last 7d",
                            preset: "7d"
                        },
                        {
                            text: "Last 30d",
                            preset: "30d"
                        }
                    ]

                    Button {
                        required property var modelData

                        text: modelData.text
                        Layout.preferredWidth: 80

                        onClicked: loadPreset(modelData.preset)
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Clear"
                    onClicked: sensorModel.clear()
                }
            }

            // Custom date range selector
            GroupBox {
                title: "Select Range"
                Layout.fillWidth: true

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    DateTimePicker {
                        id: startPicker
                        label: "Start"
                        Layout.preferredWidth: 220
                        availableDates: mapViewRoot.availableDates

                        onDateTimeChanged: function (dt) {
                            mapViewRoot.historicalStart = dt;
                        }
                    }

                    DateTimePicker {
                        id: endPicker
                        label: "End"
                        Layout.preferredWidth: 220
                        availableDates: mapViewRoot.availableDates

                        onDateTimeChanged: function (dt) {
                            mapViewRoot.historicalEnd = dt;
                        }
                    }

                    Button {
                        text: "Load"
                        Layout.preferredWidth: 100
                        onClicked: switchToHistoricalMode()
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    // Helper functions
    function getWindowMinutes() {
        var windowMinutes = 60;  // Default
        for (var i = 0; i < windowGroup.buttons.length; i++) {
            if (windowGroup.buttons[i].checked) {
                windowMinutes = windowGroup.buttons[i].minutes;
                break;
            }
        }
        return windowMinutes;
    }

    function switchToLiveMode(forceReload) {
        var wasLive = (currentMode === MapView.VisualizationMode.Live);
        currentMode = MapView.VisualizationMode.Live;

        // If already in live mode and not forcing reload, just restart timer
        if (wasLive && !forceReload) {
            liveUpdateTimer.restart();
            return;
        }

        // Load initial data from database for the time window
        var windowMinutes = getWindowMinutes();
        var now = new Date();
        var start = new Date(now.getTime() - windowMinutes * 60 * 1000);
        sensorModel.loadFromDatabase(start, now);

        // Start receiving live updates
        sensorModel.startLiveUpdates();
        liveUpdateTimer.restart();
    }

    function switchToHistoricalMode() {
        if (currentMode === MapView.VisualizationMode.Historical)
            return;
        currentMode = MapView.VisualizationMode.Historical;
        liveUpdateTimer.stop();
        sensorModel.stopLiveUpdates();
    }

    function loadLiveData() {
        // Initial load when starting live mode
        var windowMinutes = getWindowMinutes();
        var now = new Date();
        var start = new Date(now.getTime() - windowMinutes * 60 * 1000);
        sensorModel.loadFromDatabase(start, now);
        sensorModel.startLiveUpdates();
    }

    function loadPreset(preset) {
        var now = new Date();
        var start;
        switch (preset) {
        case "1h":
            start = new Date(now.getTime() - 3600000);
            break;
        case "6h":
            start = new Date(now.getTime() - 6 * 3600000);
            break;
        case "24h":
            start = new Date(now.getTime() - 24 * 3600000);
            break;
        case "7d":
            start = new Date(now.getTime() - 7 * 24 * 3600000);
            break;
        case "30d":
            start = new Date(now.getTime() - 30 * 24 * 3600000);
            break;
        }
        // Selecting a preset triggers historical mode
        switchToHistoricalMode();
        sensorModel.loadFromDatabase(start, now);
        centerOnData();
    }

    function centerOnData() {
        if (sensorModel.count > 0) {
            var first = sensorModel.getReading(0);
            mapView.map.center = QtPositioning.coordinate(first.latitude, first.longitude);
        }
    }

    function refreshAvailableDates() {
        availableDates = DatabaseManager.getAvailableDates();
    }

    Component.onCompleted: {
        refreshAvailableDates();
        // Start in live mode
        currentMode = MapView.VisualizationMode.Live;
        loadLiveData();
    }
}
