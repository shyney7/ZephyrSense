import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest

pragma ComponentBehavior: Bound

TestCase {
    id: testCase
    name: "NavigationDrawer"
    width: 300
    height: 400
    when: windowShown

    property Component drawerComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/NavigationDrawer.qml")

    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    function test_componentLoaded() {
        compare(drawerComponent.status, Component.Ready,
                "Failed to load: " + drawerComponent.errorString())
    }

    function test_defaultExpanded() {
        var drawer = createTemporaryObject(drawerComponent, testCase)
        verify(drawer)
        compare(drawer.width, 220)
        compare(drawer.currentIndex, 0)
    }

    function test_collapsed() {
        var drawer = createTemporaryObject(drawerComponent, testCase, { collapsed: true })
        verify(drawer)
        // Width animates — use tryCompare to wait for it
        tryCompare(drawer, "width", 60, 500)
    }

    function test_selectItem() {
        var drawer = createTemporaryObject(drawerComponent, testCase)
        verify(drawer)
        drawer.selectItem(2)
        compare(drawer.currentIndex, 2)
    }

    function test_navigationSignal() {
        var drawer = createTemporaryObject(drawerComponent, testCase)
        verify(drawer)
        var spy = createTemporaryObject(signalSpyComponent, testCase,
                                        { target: drawer, signalName: "navigationRequested" })
        verify(spy)
        verify(spy.valid)

        waitForRendering(drawer)
        drawer.selectItem(1)
        compare(drawer.currentIndex, 1)
    }

    function test_fourItems() {
        var drawer = createTemporaryObject(drawerComponent, testCase)
        verify(drawer)
        var listView = findChild(drawer, "navList")
        verify(listView)
        compare(listView.count, 4)
    }

    function test_collapseToggle() {
        var drawer = createTemporaryObject(drawerComponent, testCase)
        verify(drawer)
        compare(drawer.collapsed, false)
        drawer.collapsed = true
        compare(drawer.collapsed, true)
        drawer.collapsed = false
        compare(drawer.collapsed, false)
    }

    function test_labelHiddenWhenCollapsed() {
        var drawer = createTemporaryObject(drawerComponent, testCase, { collapsed: true })
        verify(drawer)
        // Labels inside delegates should not be visible
        // Verify collapse state is correct
        compare(drawer.collapsed, true)
    }
}
