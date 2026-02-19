import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "DashboardLogic"

    // Replicate DashboardView JS logic in a minimal QtObject
    QtObject {
        id: logic

        property int updateIntervalMs: 1000
        property int frozenReadingId: -1
        readonly property bool isLiveMode: updateIntervalMs > 0
        readonly property bool isFrozenMode: frozenReadingId >= 0 && updateIntervalMs < 0

        function formatTimestamp(dt) {
            if (dt.getTime() === 0)
                return ""
            return Qt.formatDateTime(dt, "yyyy-MM-dd hh:mm:ss")
        }

        readonly property var sensorConfig: [
            { key: "partectorNumber", name: "PNC UFP", unit: "#/cm\u00B3", min: 0, max: 50000, precision: 0 },
            { key: "partectorDiam", name: "\u00D8 UFP", unit: "nm", min: 0, max: 500, precision: 0 },
            { key: "partectorMass", name: "PM0.3", unit: "\u00B5g/m\u00B3", min: 0, max: 100, precision: 2 },
            { key: "grimmValue", name: "PNC PM", unit: "#/cm\u00B3", min: 0, max: 100, precision: 2 },
            { key: "temperature", name: "Temperature", unit: "\u00B0C", min: -20, max: 60, precision: 1 },
            { key: "humidity", name: "Humidity", unit: "%", min: 0, max: 100, precision: 1 },
            { key: "pressure", name: "Pressure", unit: "hPa", min: 900, max: 1100, precision: 1 },
            { key: "altitude", name: "Altitude", unit: "m", min: 0, max: 3000, precision: 1 },
            { key: "co2", name: "CO\u2082", unit: "ppm", min: 0, max: 5000, precision: 0 }
        ]
    }

    function test_formatTimestamp_epoch0() {
        var result = logic.formatTimestamp(new Date(0))
        compare(result, "")
    }

    function test_formatTimestamp_valid() {
        // Use a known date: 2025-06-15 14:30:00
        var dt = new Date(2025, 5, 15, 14, 30, 0)
        var result = logic.formatTimestamp(dt)
        compare(result, "2025-06-15 14:30:00")
    }

    function test_isLiveMode_positive() {
        logic.updateIntervalMs = 1000
        compare(logic.isLiveMode, true)
    }

    function test_isLiveMode_negative() {
        logic.updateIntervalMs = -1
        compare(logic.isLiveMode, false)
        // Reset
        logic.updateIntervalMs = 1000
    }

    function test_isFrozenMode_true() {
        logic.frozenReadingId = 42
        logic.updateIntervalMs = -1
        compare(logic.isFrozenMode, true)
        // Reset
        logic.frozenReadingId = -1
        logic.updateIntervalMs = 1000
    }

    function test_isFrozenMode_noReading() {
        logic.frozenReadingId = -1
        logic.updateIntervalMs = -1
        compare(logic.isFrozenMode, false)
        // Reset
        logic.updateIntervalMs = 1000
    }

    function test_isFrozenMode_liveMode() {
        logic.frozenReadingId = 42
        logic.updateIntervalMs = 1000
        compare(logic.isFrozenMode, false)
        // Reset
        logic.frozenReadingId = -1
    }

    function test_sensorConfig_count() {
        compare(logic.sensorConfig.length, 9)

        // Verify each entry has required keys
        for (var i = 0; i < logic.sensorConfig.length; i++) {
            var cfg = logic.sensorConfig[i]
            verify(cfg.key !== undefined, "key missing at index " + i)
            verify(cfg.name !== undefined, "name missing at index " + i)
            verify(cfg.unit !== undefined, "unit missing at index " + i)
            verify(cfg.min !== undefined, "min missing at index " + i)
            verify(cfg.max !== undefined, "max missing at index " + i)
            verify(cfg.precision !== undefined, "precision missing at index " + i)
        }
    }
}
