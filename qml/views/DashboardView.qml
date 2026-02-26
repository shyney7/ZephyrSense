pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense
import "../components"


Item {
    id: dashboardRoot

    // Properties bound from Main.qml (decoupled from mainWindow id)
    property int selectedReadingId: -1
    property sensorReading selectedReadingData
    signal liveModeSwitched()

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Active
    }

    // Mode state management
    property int updateIntervalMs: 1000  // Default 1 second, -1 means frozen
    property int frozenReadingId: dashboardRoot.selectedReadingId
    property int lastProcessedFrozenId: -1  // Guard against duplicate processing
    property date frozenTimestamp
    property date lastUpdateTime
    property date lastSerialUpdateTime

    readonly property bool isLiveMode: updateIntervalMs > 0
    readonly property bool isFrozenMode: frozenReadingId >= 0 && updateIntervalMs < 0

    // Current sensor values
    property var currentReading: ({
            partectorNumber: 0,
            partectorDiam: 0,
            partectorMass: 0,
            grimmValue: 0,
            temperature: 0,
            humidity: 0,
            pressure: 0,
            altitude: 0,
            co2: 0
        })

    // Sensor configuration for the 9 gauges
    readonly property var sensorConfig: [
        {
            sensorKey: "partectorNumber",
            sensorName: "PNC UFP",
            unit: "#/cm\u00B3",
            minValue: 0,
            maxValue: 500000,
            precision: 0
        },
        {
            sensorKey: "partectorDiam",
            sensorName: "\u00D8 UFP",
            unit: "nm",
            minValue: 0,
            maxValue: 500,
            precision: 0
        },
        {
            sensorKey: "partectorMass",
            sensorName: "PM0.3",
            unit: "\u00B5g/m\u00B3",
            minValue: 0,
            maxValue: 200,
            precision: 2
        },
        {
            sensorKey: "grimmValue",
            sensorName: "PNC PM",
            unit: "#/cm\u00B3",
            minValue: 0,
            maxValue: 200000,
            precision: 2
        },
        {
            sensorKey: "temperature",
            sensorName: "Temperature",
            unit: "\u00B0C",
            minValue: -20,
            maxValue: 60,
            precision: 1
        },
        {
            sensorKey: "humidity",
            sensorName: "Humidity",
            unit: "%",
            minValue: 0,
            maxValue: 100,
            precision: 1
        },
        {
            sensorKey: "pressure",
            sensorName: "Pressure",
            unit: "hPa",
            minValue: 900,
            maxValue: 1150,
            precision: 1
        },
        {
            sensorKey: "altitude",
            sensorName: "Altitude",
            unit: "m",
            minValue: 0,
            maxValue: 8000,
            precision: 1
        },
        {
            sensorKey: "co2",
            sensorName: "CO\u2082",
            unit: "ppm",
            minValue: 0,
            maxValue: 10000,
            precision: 0
        }
    ]

    // Helper model for frozen mode
    SensorReadingModel {
        id: readingModel
    }

    // Timer for live updates
    Timer {
        id: updateTimer
        interval: dashboardRoot.updateIntervalMs
        running: dashboardRoot.isLiveMode && dashboardRoot.updateIntervalMs > 0 && dashboardRoot.visible
        repeat: true
        onTriggered: dashboardRoot.fetchLatestReading()
    }

    // Live data connection
    Connections {
        target: SerialHandler
        enabled: dashboardRoot.isLiveMode
        function onNewReading(reading: sensorReading): void {
            // Throttle updates based on selected interval
            var now = new Date();
            if (dashboardRoot.lastSerialUpdateTime.getTime() > 0) {
                var elapsed = now - dashboardRoot.lastSerialUpdateTime;
                if (elapsed < dashboardRoot.updateIntervalMs) {
                    // Skip update, too soon
                    return;
                }
            }

            dashboardRoot.currentReading = {
                partectorNumber: reading.partectorNumber,
                partectorDiam: reading.partectorDiam,
                partectorMass: reading.partectorMass,
                grimmValue: reading.grimmValue,
                temperature: reading.temperature,
                humidity: reading.humidity,
                pressure: reading.pressure,
                altitude: reading.altitude,
                co2: reading.co2
            };
            dashboardRoot.lastUpdateTime = now;
            dashboardRoot.lastSerialUpdateTime = now;
        }
    }

    // Timestamp formatting helper
    function formatTimestamp(dt: date): string {
        if (dt.getTime() === 0)
            return "";
        return Qt.formatDateTime(dt, "yyyy-MM-dd hh:mm:ss");
    }

    // Fetch latest reading from database (fallback when no serial data)
    function fetchLatestReading(): void {
        // Load recent data from database
        var endTime = new Date();
        var startTime = new Date(endTime.getTime() - 3600000); // Last hour for better chance of data
        readingModel.loadFromDatabase(startTime, endTime);

        if (readingModel.count > 0) {
            var reading = readingModel.getReading(readingModel.count - 1);
            dashboardRoot.currentReading = {
                partectorNumber: reading.partectorNumber || 0,
                partectorDiam: reading.partectorDiam || 0,
                partectorMass: reading.partectorMass || 0,
                grimmValue: reading.grimmValue || 0,
                temperature: reading.temperature || 0,
                humidity: reading.humidity || 0,
                pressure: reading.pressure || 0,
                altitude: reading.altitude || 0,
                co2: reading.co2 || 0
            };
            dashboardRoot.lastUpdateTime = reading.timestamp || new Date();
        } else {
            console.log("No data available in database");
        }
    }

    // Load frozen reading using data passed from map view, with database fallback
    function loadFrozenReading(readingId: int): void {
        if (readingId < 0)
            return;

        // selectedReadingData is set before selectedReadingId in Main.qml,
        // so the typed reading data is always available when this fires
        let reading = dashboardRoot.selectedReadingData;
        dashboardRoot.currentReading = {
            partectorNumber: reading.partectorNumber,
            partectorDiam: reading.partectorDiam,
            partectorMass: reading.partectorMass,
            grimmValue: reading.grimmValue,
            temperature: reading.temperature,
            humidity: reading.humidity,
            pressure: reading.pressure,
            altitude: reading.altitude,
            co2: reading.co2
        };
        dashboardRoot.frozenTimestamp = reading.timestamp;
        console.log("Loaded frozen reading ID:", readingId);
    }

    // Switch back to live mode
    function switchToLive(intervalMs: int): void {
        dashboardRoot.liveModeSwitched()
        dashboardRoot.lastProcessedFrozenId = -1;  // Reset guard for future clicks
        dashboardRoot.updateIntervalMs = intervalMs || 1000;
        updateTimer.stop();
        updateTimer.start();
        fetchLatestReading();
    }

    // Monitor frozen reading ID changes
    onFrozenReadingIdChanged: {
        // Guard: only process if ID is valid and different from last processed
        if (frozenReadingId >= 0 && frozenReadingId !== lastProcessedFrozenId) {
            lastProcessedFrozenId = frozenReadingId;
            // Switch to frozen mode
            dashboardRoot.updateIntervalMs = -1;
            updateTimer.stop();
            loadFrozenReading(frozenReadingId);
        }
    }

    // Initialize on load
    Component.onCompleted: {
        // Only fetch for live mode - frozen mode is handled by onFrozenReadingIdChanged
        // which fires when the binding initializes
        if (frozenReadingId < 0) {
            fetchLatestReading();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Mode banner
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: '#27f5f5f5'
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 12

                // Mode indicator circle
                Rectangle {
                    Layout.preferredWidth: 12
                    Layout.preferredHeight: 12
                    radius: 6
                    color: dashboardRoot.isLiveMode ? "#4CAF50" : "#2196F3"
                }

                // Mode text
                Text {
                    text: {
                        if (dashboardRoot.isFrozenMode) {
                            return "Showing data from " + dashboardRoot.formatTimestamp(dashboardRoot.frozenTimestamp);
                        } else if (dashboardRoot.lastUpdateTime.getTime() > 0) {
                            return "Live - Last update: " + dashboardRoot.formatTimestamp(dashboardRoot.lastUpdateTime);
                        } else {
                            return "Live - No data yet";
                        }
                    }
                    font.pixelSize: 13
                    color: palette.text
                    Layout.fillWidth: true
                }
            }
        }

        // Gauge grid (3x3)
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 3
            rowSpacing: 16
            columnSpacing: 16

            Repeater {
                model: dashboardRoot.sensorConfig

                RadialBarGauge {
                    required property var modelData

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 140
                    Layout.minimumHeight: 140

                    value: dashboardRoot.currentReading[modelData.sensorKey] ?? 0
                    minValue: modelData.minValue
                    maxValue: modelData.maxValue
                    sensorKey: modelData.sensorKey
                    sensorName: modelData.sensorName
                    unit: modelData.unit
                    precision: modelData.precision
                }
            }
        }

        // Control bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: "Update:"
                font.pixelSize: 13
                color: palette.text
            }

            ComboBox {
                id: intervalSelector
                Layout.preferredWidth: 100
                enabled: true  // Always enabled (was: !dashboardRoot.isFrozenMode)

                model: [
                    {
                        text: "1s",
                        value: 1000
                    },
                    {
                        text: "2s",
                        value: 2000
                    },
                    {
                        text: "5s",
                        value: 5000
                    },
                    {
                        text: "10s",
                        value: 10000
                    }
                ]

                textRole: "text"
                currentIndex: 0

                onActivated: function (index) {
                    var newInterval = model[index].value;
                    if (dashboardRoot.isFrozenMode) {
                        // Switch back to live mode
                        dashboardRoot.switchToLive(newInterval);
                    } else {
                        // Update interval in live mode
                        dashboardRoot.updateIntervalMs = newInterval;
                        updateTimer.interval = newInterval;
                        updateTimer.restart();
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Return to Live"
                visible: dashboardRoot.isFrozenMode
                onClicked: dashboardRoot.switchToLive(1000)
            }
        }
    }
}
