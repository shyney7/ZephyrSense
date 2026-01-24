import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense
import "../components"

Item {
    id: dashboardRoot

    // Mode state management
    property int updateIntervalMs: 1000  // Default 1 second, -1 means frozen
    property int frozenReadingId: mainWindow.selectedReadingId
    property var frozenTimestamp: null
    property var lastUpdateTime: null

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
        { key: "partectorNumber", name: "Particles", unit: "/cm3", min: 0, max: 50000, precision: 0 },
        { key: "partectorDiam", name: "Diameter", unit: "nm", min: 0, max: 500, precision: 0 },
        { key: "partectorMass", name: "Mass", unit: "ug/m3", min: 0, max: 100, precision: 2 },
        { key: "grimmValue", name: "GRIMM", unit: "/cm3", min: 0, max: 100, precision: 2 },
        { key: "temperature", name: "Temperature", unit: "C", min: -20, max: 60, precision: 1 },
        { key: "humidity", name: "Humidity", unit: "%", min: 0, max: 100, precision: 1 },
        { key: "pressure", name: "Pressure", unit: "hPa", min: 900, max: 1100, precision: 1 },
        { key: "altitude", name: "Altitude", unit: "m", min: 0, max: 3000, precision: 1 },
        { key: "co2", name: "CO2", unit: "ppm", min: 0, max: 5000, precision: 0 }
    ]

    // Helper model for frozen mode
    SensorReadingModel {
        id: readingModel
    }

    // Timer for live updates
    Timer {
        id: updateTimer
        interval: dashboardRoot.updateIntervalMs
        running: dashboardRoot.isLiveMode && dashboardRoot.updateIntervalMs > 0
        repeat: true
        onTriggered: fetchLatestReading()
    }

    // Live data connection
    Connections {
        target: SerialHandler
        enabled: dashboardRoot.isLiveMode
        function onNewReading(reading) {
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
            }
            dashboardRoot.lastUpdateTime = new Date()
        }
    }

    // Timestamp formatting helper
    function formatTimestamp(date) {
        if (!date) return ""
        return Qt.formatDateTime(date, "yyyy-MM-dd hh:mm:ss")
    }

    // Fetch latest reading from database (fallback when no serial data)
    function fetchLatestReading() {
        var endTime = new Date()
        var startTime = new Date(endTime.getTime() - 60000) // Last minute
        readingModel.loadFromDatabase(startTime, endTime)

        if (readingModel.count > 0) {
            var reading = readingModel.getReading(readingModel.count - 1)
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
            }
            dashboardRoot.lastUpdateTime = reading.timestamp || new Date()
        }
    }

    // Load frozen reading from database
    function loadFrozenReading(readingId) {
        if (readingId < 0) return

        // Load last 24 hours of data
        var endTime = new Date()
        var startTime = new Date(endTime.getTime() - 86400000) // 24 hours ago
        readingModel.loadFromDatabase(startTime, endTime)

        // Find the reading with matching ID
        for (var i = 0; i < readingModel.count; i++) {
            var reading = readingModel.getReading(i)
            if (reading.id === readingId) {
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
                }
                dashboardRoot.frozenTimestamp = reading.timestamp
                console.log("Loaded frozen reading ID:", readingId)
                return
            }
        }

        console.warn("Frozen reading ID not found:", readingId)
    }

    // Switch back to live mode
    function switchToLive(intervalMs) {
        mainWindow.selectedReadingId = -1
        dashboardRoot.frozenReadingId = -1
        dashboardRoot.updateIntervalMs = intervalMs || 1000
        updateTimer.stop()
        updateTimer.start()
        fetchLatestReading()
    }

    // Monitor frozen reading ID changes
    onFrozenReadingIdChanged: {
        if (frozenReadingId >= 0) {
            // Switch to frozen mode
            dashboardRoot.updateIntervalMs = -1
            updateTimer.stop()
            loadFrozenReading(frozenReadingId)
        }
    }

    // Initialize on load
    Component.onCompleted: {
        if (frozenReadingId >= 0) {
            // Start in frozen mode
            updateIntervalMs = -1
            loadFrozenReading(frozenReadingId)
        } else {
            // Start in live mode
            fetchLatestReading()
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
            color: "#F5F5F5"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 12

                // Mode indicator circle
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: dashboardRoot.isLiveMode ? "#4CAF50" : "#2196F3"
                }

                // Mode text
                Text {
                    text: dashboardRoot.isFrozenMode
                          ? "Showing data from " + formatTimestamp(dashboardRoot.frozenTimestamp)
                          : "Live"
                    font.pixelSize: 13
                    color: "#424242"
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
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 140
                    Layout.minimumHeight: 140

                    value: dashboardRoot.currentReading[modelData.key] || 0
                    minValue: modelData.min
                    maxValue: modelData.max
                    sensorKey: modelData.key
                    sensorName: modelData.name
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
                color: "#424242"
            }

            ComboBox {
                id: intervalSelector
                Layout.preferredWidth: 100
                enabled: !dashboardRoot.isFrozenMode

                model: [
                    { text: "1s", value: 1000 },
                    { text: "2s", value: 2000 },
                    { text: "5s", value: 5000 },
                    { text: "10s", value: 10000 }
                ]

                textRole: "text"
                currentIndex: 0

                onActivated: function(index) {
                    if (!dashboardRoot.isFrozenMode) {
                        var newInterval = model[index].value
                        dashboardRoot.updateIntervalMs = newInterval
                        updateTimer.interval = newInterval
                        updateTimer.restart()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "Return to Live"
                visible: dashboardRoot.isFrozenMode
                onClicked: switchToLive(1000)
            }
        }
    }
}
