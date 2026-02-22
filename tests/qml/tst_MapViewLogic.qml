import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "MapViewLogic"

    // Replicate MapView's loadPreset logic
    QtObject {
        id: logic

        property int currentMode: 0  // 0 = Live, 1 = Historical
        property date historicalStart: new Date()
        property date historicalEnd: new Date()

        function loadPreset(preset) {
            var now = new Date()
            var start
            switch (preset) {
            case "1h":
                start = new Date(now.getTime() - 3600000)
                break
            case "6h":
                start = new Date(now.getTime() - 6 * 3600000)
                break
            case "24h":
                start = new Date(now.getTime() - 24 * 3600000)
                break
            case "7d":
                start = new Date(now.getTime() - 7 * 24 * 3600000)
                break
            case "30d":
                start = new Date(now.getTime() - 30 * 24 * 3600000)
                break
            }
            logic.historicalStart = start
            logic.historicalEnd = now
            logic.currentMode = 1  // Historical
        }
    }

    function test_loadPreset_1h() {
        var before = new Date()
        logic.loadPreset("1h")
        var after = new Date()
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        // Should be approximately 1 hour (3600000ms)
        verify(Math.abs(diff - 3600000) < 100, "1h preset: diff=" + diff)
    }

    function test_loadPreset_6h() {
        logic.loadPreset("6h")
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        verify(Math.abs(diff - 6 * 3600000) < 100, "6h preset: diff=" + diff)
    }

    function test_loadPreset_24h() {
        logic.loadPreset("24h")
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        verify(Math.abs(diff - 24 * 3600000) < 100, "24h preset: diff=" + diff)
    }

    function test_loadPreset_7d() {
        logic.loadPreset("7d")
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        verify(Math.abs(diff - 7 * 24 * 3600000) < 100, "7d preset: diff=" + diff)
    }

    function test_loadPreset_30d() {
        logic.loadPreset("30d")
        var diff = logic.historicalEnd.getTime() - logic.historicalStart.getTime()
        verify(Math.abs(diff - 30 * 24 * 3600000) < 100, "30d preset: diff=" + diff)
    }

    function test_switchToHistoricalMode() {
        logic.currentMode = 0  // Start in Live
        logic.loadPreset("1h")
        compare(logic.currentMode, 1)  // Should be Historical
    }
}
