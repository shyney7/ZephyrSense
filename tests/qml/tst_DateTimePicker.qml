import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest

pragma ComponentBehavior: Bound

TestCase {
    id: testCase
    name: "DateTimePicker"
    width: 400
    height: 400
    when: windowShown

    property Component pickerComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/DateTimePicker.qml")

    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    function test_componentLoaded() {
        compare(pickerComponent.status, Component.Ready,
                "Failed to load: " + pickerComponent.errorString())
    }

    function test_defaultHour() {
        var picker = createTemporaryObject(pickerComponent, testCase)
        verify(picker)
        compare(picker.selectedHour, 12)
    }

    function test_hourComboEntries() {
        var picker = createTemporaryObject(pickerComponent, testCase)
        verify(picker)
        waitForRendering(picker)

        var combo = findChild(picker, "hourCombo")
        verify(combo)
        compare(combo.count, 24)

        // Check first and last entries
        compare(combo.model[0].text, "00:00")
        compare(combo.model[23].text, "23:00")
    }

    function test_labelProperty() {
        var picker = createTemporaryObject(pickerComponent, testCase,
                                           { label: "Start Date/Time" })
        verify(picker)
        waitForRendering(picker)

        var labelItem = findChild(picker, "labelText")
        verify(labelItem)
        compare(labelItem.text, "Start Date/Time")
    }

    function test_defaultSelectedDate() {
        var picker = createTemporaryObject(pickerComponent, testCase)
        verify(picker)
        // selectedDate defaults to today
        var today = new Date()
        compare(picker.selectedDate.getFullYear(), today.getFullYear())
        compare(picker.selectedDate.getMonth(), today.getMonth())
        compare(picker.selectedDate.getDate(), today.getDate())
    }

    function test_emitDateTime() {
        var picker = createTemporaryObject(pickerComponent, testCase)
        verify(picker)
        waitForRendering(picker)

        var spy = createTemporaryObject(signalSpyComponent, testCase,
                                        { target: picker, signalName: "dateTimeChanged" })
        verify(spy)
        verify(spy.valid)

        // Set a known date and hour, then emit
        var testDate = new Date(2025, 0, 15)  // Jan 15, 2025
        picker.selectedDate = testDate
        picker.selectedHour = 8
        picker.emitDateTime()

        compare(spy.count, 1)
        var emitted = spy.signalArguments[0][0]
        compare(emitted.getFullYear(), 2025)
        compare(emitted.getMonth(), 0)
        compare(emitted.getDate(), 15)
        compare(emitted.getHours(), 8)
    }

    function test_dateTimeChangedOnHourChange() {
        var picker = createTemporaryObject(pickerComponent, testCase)
        verify(picker)
        waitForRendering(picker)

        var spy = createTemporaryObject(signalSpyComponent, testCase,
                                        { target: picker, signalName: "dateTimeChanged" })
        verify(spy)

        // Directly call emitDateTime to simulate hour change
        picker.emitDateTime()
        compare(spy.count, 1)
    }

    function test_availableDatesProperty() {
        var picker = createTemporaryObject(pickerComponent, testCase,
                                           { availableDates: ["2025-01-15", "2025-01-16"] })
        verify(picker)
        compare(picker.availableDates.length, 2)
        compare(picker.availableDates[0], "2025-01-15")
    }
}
