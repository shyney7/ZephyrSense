import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

ApplicationWindow {
    id: mainWindow
    width: 1024
    height: 768
    visible: true
    title: "ZephyrSense"

    // Selected reading for dashboard view (set when clicking map marker)
    property int selectedReadingId: -1
    property var lastMarkerClickTime: null  // Debounce for overlapping markers

    // Header toolbar
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            Label {
                text: "ZephyrSense"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
        }
    }

    // Navigation drawer (persistent, not modal)
    NavigationDrawer {
        id: navDrawer
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        z: 1

        onNavigationRequested: function (index, viewPath) {
            // Clear selected reading when manually navigating
            mainWindow.selectedReadingId = -1;
            stackView.replace(viewPath);
        }
    }

    // Main content area with StackView
    StackView {
        id: stackView
        anchors.left: navDrawer.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        initialItem: "qml/views/MapView.qml"

        // Smooth transitions between views
        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }

        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
    }

    // Handle map marker click -> dashboard navigation
    Connections {
        target: stackView.currentItem
        function onShowDashboardForReading(readingId) {
            // Debounce: ignore clicks within 300ms (handles overlapping markers)
            var now = new Date();
            if (mainWindow.lastMarkerClickTime) {
                var elapsed = now - mainWindow.lastMarkerClickTime;
                if (elapsed < 300) {
                    // Ignore rapid successive clicks from overlapping markers
                    return;
                }
            }
            mainWindow.lastMarkerClickTime = now;

            mainWindow.selectedReadingId = readingId;
            navDrawer.selectItem(1);  // Dashboard is index 1
            stackView.replace("qml/views/DashboardView.qml");
        }
        ignoreUnknownSignals: true
    }

    // Debug output for received readings
    Connections {
        target: SerialHandler
        function onNewReading(reading) {
            console.log("Received reading - Temp:", reading.temperature, "Humidity:", reading.humidity, "Lat:", reading.latitude, "Lon:", reading.longitude);
        }
    }

    // Initialize data layer
    Component.onCompleted: {
        // Initialize database (creates tables if needed)
        if (DatabaseManager.initialize()) {
            console.log("Database initialized at:", DatabaseManager.databasePath);
        }
        console.log("CSV export enabled:", CsvExporter.enabled);
    }
}
