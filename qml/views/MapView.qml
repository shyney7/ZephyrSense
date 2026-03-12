pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation as QtLoc
import QtPositioning
import QtWebEngine
import QtWebChannel
import ZephyrSense
import com.kdab.dockwidgets 2.0 as KDDW

Item {
    id: mapViewRoot

    readonly property DatabaseManager dbManager: DatabaseManager

    // Signal to request navigation to dashboard with specific reading
    signal showDashboardForReading(int readingId, sensorReading reading)

    // Mode state
    enum VisualizationMode {
        Live,
        Historical
    }
    property int currentMode: MapView.VisualizationMode.Live
    property int updateIntervalMs: 2000  // Default 2 seconds for live mode
    readonly property int defaultWindowIndex: 2
    // Invariant: selectedWindowMinutes matches the preset minutes at defaultWindowIndex (2 => 60).
    property int selectedWindowMinutes: 60
    property date historicalStart: new Date()
    property date historicalEnd: new Date()
    property list<string> availableDates: []
    property int cesiumRequestId: 0

    // Model instance for map markers
    SensorReadingModel {
        id: sensorModel
    }

    // Live mode prune timer (removes old readings outside time window)
    Timer {
        id: liveUpdateTimer
        interval: mapViewRoot.updateIntervalMs
        running: mapViewRoot.currentMode === MapView.VisualizationMode.Live && mapViewRoot.visible
        repeat: true
        onTriggered: {
            // Prune old readings outside the time window
            var windowMinutes = mapViewRoot.getWindowMinutes();
            sensorModel.pruneOldReadings(windowMinutes);
        }
    }

    WebChannel {
        id: cesiumChannel
        Component.onCompleted: cesiumChannel.registerObject("CesiumBridge", CesiumBridge)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        KDDW.DockingArea {
            id: mapDockArea
            uniqueName: "MapViewDockArea"
            Layout.fillWidth: true
            Layout.fillHeight: true

            KDDW.DockWidget {
                id: dock2DMap
                uniqueName: "dock-2d-map"
                title: "2D Map"

                Item {
                    anchors.fill: parent

                    QtLoc.MapView {
                        id: mapView
                        anchors.fill: parent

                        map.plugin: QtLoc.Plugin {
                            name: "osm"
                            QtLoc.PluginParameter {
                                name: "osm.useragent"
                                value: "ZephyrSense/1.0"
                            }
                        }

                        // Default view: Wuppertal, Germany
                        map.center: QtPositioning.coordinate(51.2562, 7.1508) // qmllint disable compiler
                        map.zoomLevel: 10

                        // Marker layer using MapItemView
                        QtLoc.MapItemView {
                            id: markerView
                            model: sensorModel
                            parent: mapView.map

                            delegate: SensorMarker {
                                // Required properties auto-injected from model roles:
                                // latitude, longitude, tooltipText, readingId
                                required property int index

                                onMarkerClicked: function (id: int): void {
                                    mapViewRoot.showDashboardForReading(id, sensorModel.readingAt(index));
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

                    // Info overlay showing point count
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
                }
            }

            KDDW.DockWidget {
                id: dock3DGlobe
                uniqueName: "dock-3d-globe"
                title: "3D Globe"

                Item {
                    anchors.fill: parent

                    WebEngineView {
                        id: cesiumView
                        anchors.fill: parent
                        profile: AppWebProfile
                        webChannel: cesiumChannel
                        url: CesiumBridge.contentUrl
                        settings.localContentCanAccessRemoteUrls: true
                        settings.javascriptEnabled: true
                        settings.localStorageEnabled: true
                    }

                    // Overlay shown when no Cesium Ion token is configured
                    Rectangle {
                        anchors.fill: parent
                        color: "#E8EAF6"
                        visible: CesiumBridge.cesiumToken === ""
                        z: 10

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 16

                            Label {
                                text: "3D Globe requires a Cesium Ion access token"
                                font.pixelSize: 16
                                font.bold: true
                                Layout.alignment: Qt.AlignHCenter
                                color: "#37474F"
                            }

                            Label {
                                text: "A free token is needed to display terrain and satellite imagery."
                                font.pixelSize: 13
                                Layout.alignment: Qt.AlignHCenter
                                color: "#546E7A"
                            }

                            Button {
                                text: "Set Up Token"
                                highlighted: true
                                Layout.alignment: Qt.AlignHCenter

                                onClicked: tokenSetupPopup.open()
                            }
                        }
                    }
                }
            }

            Component.onCompleted: {
                mapDockArea.addDockWidget(dock2DMap, KDDW.KDDockWidgets.Location_OnBottom)
                dock2DMap.addDockWidgetAsTab(dock3DGlobe)
                dock2DMap.setAsCurrentTab()
            }
        }

        // Control panel at bottom
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: controlLayout.implicitHeight + 24
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
                                    mapViewRoot.switchToLiveMode(true);  // force reload
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
                                label: "10m",
                                minutes: 10
                            },
                            {
                                label: "30m",
                                minutes: 30
                            },
                            {
                                label: "1h",
                                minutes: 60
                            },
                            {
                                label: "6h",
                                minutes: 360
                            },
                            {
                                label: "24h",
                                minutes: 1440
                            }
                        ]

                        Button {
                            id: presetButton
                            required property int index
                            required property string label
                            required property int minutes

                            text: presetButton.label
                            checkable: true
                            checked: presetButton.index === mapViewRoot.defaultWindowIndex
                            ButtonGroup.group: windowGroup
                            Layout.preferredWidth: 50

                            onClicked: {
                                mapViewRoot.selectedWindowMinutes = presetButton.minutes;
                                // Clicking time window stays in current mode but reloads with new window
                                if (mapViewRoot.currentMode === MapView.VisualizationMode.Live) {
                                    mapViewRoot.switchToLiveMode(true);  // force reload with new window
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
                                label: "Last 1h",
                                preset: "1h"
                            },
                            {
                                label: "Last 6h",
                                preset: "6h"
                            },
                            {
                                label: "Last 24h",
                                preset: "24h"
                            },
                            {
                                label: "Last 7d",
                                preset: "7d"
                            },
                            {
                                label: "Last 30d",
                                preset: "30d"
                            }
                        ]

                        Button {
                            required property string label
                            required property string preset

                            text: label
                            Layout.preferredWidth: 80

                            onClicked: mapViewRoot.loadPreset(preset)
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Custom Range..."
                        onClicked: customRangePopup.open()
                    }

                    Button {
                        text: "Clear"
                        onClicked: sensorModel.clear()
                    }
                }
            }
        }
    }

    // Token setup popup (shown on first launch when no Cesium Ion token is configured)
    TokenSetupPopup {
        id: tokenSetupPopup

        onTokenSaved: cesiumView.reload()
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
                    availableDates: mapViewRoot.availableDates

                    onDateTimeChanged: function (dt: date): void {
                        mapViewRoot.historicalStart = dt;
                    }
                }

                DateTimePicker {
                    id: endPicker
                    label: "End Date/Time"
                    Layout.fillWidth: true
                    availableDates: mapViewRoot.availableDates

                    onDateTimeChanged: function (dt: date): void {
                        mapViewRoot.historicalEnd = dt;
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    text: "Cancel"
                    onClicked: customRangePopup.close()
                }

                Button {
                    text: "Load Data"
                    highlighted: true
                    onClicked: {
                        mapViewRoot.switchToHistoricalMode();
                        customRangePopup.close();
                    }
                }
            }
        }
    }

    // Helper functions
    function getWindowMinutes(): int {
        return mapViewRoot.selectedWindowMinutes;
    }

    function switchToLiveMode(forceReload: bool): void {
        var wasLive = (mapViewRoot.currentMode === MapView.VisualizationMode.Live);
        mapViewRoot.currentMode = MapView.VisualizationMode.Live;

        // If already in live mode and not forcing reload, just restart timer
        if (wasLive && !forceReload) {
            liveUpdateTimer.restart();
            return;
        }

        // Load initial data from database for the time window
        var windowMinutes = mapViewRoot.getWindowMinutes();
        var now = new Date();
        var start = new Date(now.getTime() - windowMinutes * 60 * 1000);
        sensorModel.loadFromDatabase(start, now);

        // Start receiving live updates
        sensorModel.startLiveUpdates();
        liveUpdateTimer.restart();

        // Sync CesiumBridge to live mode
        CesiumBridge.windowMinutes = mapViewRoot.selectedWindowMinutes;
        CesiumBridge.liveMode = true;
    }

    function switchToHistoricalMode(): void {
        mapViewRoot.currentMode = MapView.VisualizationMode.Historical;
        liveUpdateTimer.stop();
        sensorModel.stopLiveUpdates();
        sensorModel.loadFromDatabase(mapViewRoot.historicalStart, mapViewRoot.historicalEnd);
        mapViewRoot.centerOnData();

        // Sync CesiumBridge to historical mode
        CesiumBridge.liveMode = false;
        mapViewRoot.cesiumRequestId++;
        CesiumBridge.pendingRequestId = mapViewRoot.cesiumRequestId;
        CesiumBridge.loadRange(
            mapViewRoot.historicalStart.getTime(),
            mapViewRoot.historicalEnd.getTime(),
            mapViewRoot.cesiumRequestId
        );
    }

    function loadLiveData(): void {
        // Initial load when starting live mode
        var windowMinutes = mapViewRoot.getWindowMinutes();
        var now = new Date();
        var start = new Date(now.getTime() - windowMinutes * 60 * 1000);
        sensorModel.loadFromDatabase(start, now);
        sensorModel.startLiveUpdates();
    }

    function loadPreset(preset: string): void {
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
        mapViewRoot.historicalStart = start;
        mapViewRoot.historicalEnd = now;
        mapViewRoot.switchToHistoricalMode();
    }

    function centerOnData(): void {
        if (sensorModel.count > 0) {
            var first = sensorModel.getReading(0);
            mapView.map.center = QtPositioning.coordinate(first.latitude, first.longitude); // qmllint disable compiler
        }
    }

    function refreshAvailableDates(): void {
        mapViewRoot.availableDates = mapViewRoot.dbManager.getAvailableDates();
    }

    Component.onCompleted: {
        mapViewRoot.refreshAvailableDates();
        // Start in live mode
        mapViewRoot.currentMode = MapView.VisualizationMode.Live;
        mapViewRoot.loadLiveData();

        // Show token setup popup if no Cesium Ion token is configured
        if (CesiumBridge.cesiumToken === "")
            tokenSetupPopup.open()
    }
}
