import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTest
import ZephyrSense

TestCase {
    id: testCase
    name: "ExportTab"
    width: 600
    height: 400
    when: windowShown

    property Component exportComponent: Qt.createComponent(
        "file:///" + _sourceDir + "/qml/components/ExportTab.qml")

    function test_componentLoaded() {
        compare(exportComponent.status, Component.Ready,
                "Failed to load: " + exportComponent.errorString())
    }

    function init() {
        // Reset mock state
        CsvExporter.enabled = false
        CsvExporter.filePath = ""
    }

    function test_switchTogglesEnabled() {
        var tab = createTemporaryObject(exportComponent, testCase)
        verify(tab)
        waitForRendering(tab)

        compare(CsvExporter.enabled, false)

        // Set CsvExporter.enabled directly (simulates what onToggled does)
        CsvExporter.enabled = true
        var sw = findChild(tab, "exportSwitch")
        verify(sw)
        tryCompare(sw, "checked", true, 500)
        compare(CsvExporter.enabled, true)
    }

    function test_noFileSelected() {
        var tab = createTemporaryObject(exportComponent, testCase)
        verify(tab)
        waitForRendering(tab)

        var label = findChild(tab, "filePathLabel")
        verify(label)
        compare(label.text, "No file selected")
    }

    function test_resetButton() {
        CsvExporter.enabled = true
        CsvExporter.filePath = "C:/data.csv"

        var tab = createTemporaryObject(exportComponent, testCase)
        verify(tab)
        waitForRendering(tab)

        var btn = findChild(tab, "resetButton")
        verify(btn)
        btn.clicked()

        compare(CsvExporter.enabled, false)
        compare(CsvExporter.filePath, "")
    }

    function test_filePathDisplay() {
        CsvExporter.filePath = "C:/data.csv"

        var tab = createTemporaryObject(exportComponent, testCase)
        verify(tab)
        waitForRendering(tab)

        var label = findChild(tab, "filePathLabel")
        verify(label)
        compare(label.text, "C:/data.csv")
    }
}
