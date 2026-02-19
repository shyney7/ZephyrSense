import QtQuick
import QtQuick.Controls
import QtTest

TestCase {
    id: testCase
    name: "ModeBadge"
    width: 200
    height: 100

    property Component badgeComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/ModeBadge.qml")

    function test_componentLoaded() {
        compare(badgeComponent.status, Component.Ready,
                "Failed to load: " + badgeComponent.errorString())
    }

    function test_defaultState() {
        var badge = createTemporaryObject(badgeComponent, testCase)
        verify(badge)
        compare(badge.isLive, true)
        // QML normalizes hex colors to lowercase
        compare(badge.color, "#4caf50")
    }

    function test_liveMode() {
        var badge = createTemporaryObject(badgeComponent, testCase, { isLive: true })
        verify(badge)
        compare(badge.color, "#4caf50")
        var label = badge.children[0]
        compare(label.text, "LIVE")
    }

    function test_historicalMode() {
        var badge = createTemporaryObject(badgeComponent, testCase, { isLive: false })
        verify(badge)
        compare(badge.color, "#2196f3")
        var label = badge.children[0]
        compare(label.text, "HISTORICAL")
    }

    function test_customLabel() {
        var badge = createTemporaryObject(badgeComponent, testCase,
                                          { isLive: false, historicalLabel: "FROZEN" })
        verify(badge)
        var label = badge.children[0]
        compare(label.text, "FROZEN")
    }
}
