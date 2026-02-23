import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "SensorGraphsViewLogic"

    // Replicate SensorGraphsView JS logic
    QtObject {
        id: logic

        property int currentMode: 0  // 0 = Live, 1 = Historical
        property date historicalStart: new Date()
        property date historicalEnd: new Date()

        function formatTime(msecs) {
            var dt = new Date(msecs)
            return dt.toLocaleTimeString(Qt.locale(), "hh:mm")
        }

        function switchToLiveMode() {
            currentMode = 0
        }

        function switchToHistoricalMode() {
            currentMode = 1
        }

        function loadPresetFromNow(minutes) {
            var now = new Date()
            var start = new Date(now.getTime() - minutes * 60 * 1000)
            logic.historicalStart = start
            logic.historicalEnd = now
            switchToHistoricalMode()
        }

        function loadDataForRange(minutes) {
            var now = new Date()
            var start = new Date(now.getTime() - minutes * 60 * 1000)
            // Return start/end for testing
            return { start: start, end: now }
        }
    }

    function test_formatTime() {
        // Create a known timestamp: 2025-06-15 14:30:00
        var dt = new Date(2025, 5, 15, 14, 30, 0)
        var result = logic.formatTime(dt.getTime())
        compare(result, "14:30")
    }

    function test_loadDataForRange_dateArithmetic() {
        var result = logic.loadDataForRange(60)
        var diff = result.end.getTime() - result.start.getTime()
        // 60 minutes = 3600000ms
        verify(Math.abs(diff - 3600000) < 100,
               "60 min range: diff=" + diff)
    }

    function test_switchToLiveMode() {
        logic.currentMode = 1  // Start in Historical
        logic.switchToLiveMode()
        compare(logic.currentMode, 0)
    }

    function test_switchToHistoricalMode() {
        logic.currentMode = 0  // Start in Live
        logic.switchToHistoricalMode()
        compare(logic.currentMode, 1)
    }

    function test_loadPresetFromNow() {
        logic.currentMode = 0
        logic.loadPresetFromNow(60)

        // Should switch to historical
        compare(logic.currentMode, 1)

        // Time range should be ~60 minutes
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        verify(Math.abs(diff - 3600000) < 100,
               "Preset 60min: diff=" + diff)
    }
}
