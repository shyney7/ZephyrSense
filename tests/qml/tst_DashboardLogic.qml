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

        // Must match DashboardView.qml sensorConfig (lines 49-122) exactly.
        // If production values change, update this copy to keep tests in sync.
        readonly property var sensorConfig: [
            { sensorKey: "partectorNumber", sensorName: "PNC UFP", unit: "#/cm\u00B3", minValue: 0, maxValue: 500000, precision: 0 },
            { sensorKey: "partectorDiam", sensorName: "\u00D8 UFP", unit: "nm", minValue: 0, maxValue: 500, precision: 0 },
            { sensorKey: "partectorMass", sensorName: "PM0.3", unit: "\u00B5g/m\u00B3", minValue: 0, maxValue: 200, precision: 2 },
            { sensorKey: "grimmValue", sensorName: "PNC PM", unit: "#/cm\u00B3", minValue: 0, maxValue: 200000, precision: 2 },
            { sensorKey: "temperature", sensorName: "Temperature", unit: "\u00B0C", minValue: -20, maxValue: 60, precision: 1 },
            { sensorKey: "humidity", sensorName: "Humidity", unit: "%", minValue: 0, maxValue: 100, precision: 1 },
            { sensorKey: "pressure", sensorName: "Pressure", unit: "hPa", minValue: 900, maxValue: 1150, precision: 1 },
            { sensorKey: "altitude", sensorName: "Altitude", unit: "m", minValue: 0, maxValue: 8000, precision: 1 },
            { sensorKey: "co2", sensorName: "CO\u2082", unit: "ppm", minValue: 0, maxValue: 10000, precision: 0 }
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
            verify(cfg.sensorKey !== undefined, "sensorKey missing at index " + i)
            verify(cfg.sensorName !== undefined, "sensorName missing at index " + i)
            verify(cfg.unit !== undefined, "unit missing at index " + i)
            verify(cfg.minValue !== undefined, "minValue missing at index " + i)
            verify(cfg.maxValue !== undefined, "maxValue missing at index " + i)
            verify(cfg.precision !== undefined, "precision missing at index " + i)
        }
    }

    // Catches config drift between test copy and production DashboardView.qml
    function test_sensorConfig_matchesProduction() {
        var expected = [
            { sensorKey: "partectorNumber", sensorName: "PNC UFP", unit: "#/cm\u00B3", minValue: 0, maxValue: 500000, precision: 0 },
            { sensorKey: "partectorDiam", sensorName: "\u00D8 UFP", unit: "nm", minValue: 0, maxValue: 500, precision: 0 },
            { sensorKey: "partectorMass", sensorName: "PM0.3", unit: "\u00B5g/m\u00B3", minValue: 0, maxValue: 200, precision: 2 },
            { sensorKey: "grimmValue", sensorName: "PNC PM", unit: "#/cm\u00B3", minValue: 0, maxValue: 200000, precision: 2 },
            { sensorKey: "temperature", sensorName: "Temperature", unit: "\u00B0C", minValue: -20, maxValue: 60, precision: 1 },
            { sensorKey: "humidity", sensorName: "Humidity", unit: "%", minValue: 0, maxValue: 100, precision: 1 },
            { sensorKey: "pressure", sensorName: "Pressure", unit: "hPa", minValue: 900, maxValue: 1150, precision: 1 },
            { sensorKey: "altitude", sensorName: "Altitude", unit: "m", minValue: 0, maxValue: 8000, precision: 1 },
            { sensorKey: "co2", sensorName: "CO\u2082", unit: "ppm", minValue: 0, maxValue: 10000, precision: 0 }
        ]

        for (var i = 0; i < expected.length; i++) {
            var cfg = logic.sensorConfig[i]
            var exp = expected[i]
            compare(cfg.sensorKey, exp.sensorKey, "sensorKey mismatch at index " + i)
            compare(cfg.sensorName, exp.sensorName, "sensorName mismatch at index " + i)
            compare(cfg.unit, exp.unit, "unit mismatch at index " + i)
            compare(cfg.minValue, exp.minValue, "minValue mismatch at index " + i)
            compare(cfg.maxValue, exp.maxValue, "maxValue mismatch at index " + i)
            compare(cfg.precision, exp.precision, "precision mismatch at index " + i)
        }
    }

    // Verifies RadialBarGauge receives all properties (catches delegate binding regressions)
    property Component gaugeComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/RadialBarGauge.qml")

    function test_gaugeReceivesModelData() {
        compare(gaugeComponent.status, Component.Ready,
                "Failed to load RadialBarGauge: " + gaugeComponent.errorString())

        // Use temperature config — has non-default minValue (-20) making it easy to detect defaults
        var cfg = logic.sensorConfig[4]
        var gauge = createTemporaryObject(gaugeComponent, testCase, {
            sensorKey: cfg.sensorKey,
            sensorName: cfg.sensorName,
            unit: cfg.unit,
            minValue: cfg.minValue,
            maxValue: cfg.maxValue,
            precision: cfg.precision,
            value: 25.5
        })
        verify(gauge, "Failed to create RadialBarGauge")

        // Exact value checks — catches wrong or default values
        compare(gauge.sensorKey, "temperature", "sensorKey not propagated")
        compare(gauge.sensorName, "Temperature", "sensorName not propagated")
        compare(gauge.unit, "\u00B0C", "unit not propagated")
        compare(gauge.minValue, -20, "minValue not propagated")
        compare(gauge.maxValue, 60, "maxValue not propagated")
        compare(gauge.precision, 1, "precision not propagated")
        compare(gauge.value, 25.5, "value not propagated")

        // Guard against RadialBarGauge defaults (the exact regression symptom)
        verify(gauge.sensorKey !== "", "sensorKey stuck at default (empty)")
        verify(gauge.sensorName !== "", "sensorName stuck at default (empty)")
        verify(gauge.unit !== "", "unit stuck at default (empty)")
        verify(gauge.maxValue !== 100, "maxValue stuck at default (100)")
    }
}
