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
    property date lastMarkerClickTime  // Debounce for overlapping markers

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

        onNavigationRequested: function (index: int, viewPath: string): void {
            // Clear selected reading when manually navigating
            mainWindow.selectedReadingId = -1;
            viewStack.currentIndex = index;
        }
    }

    // Main content area with StackLayout (keeps all views alive for dock state persistence)
    StackLayout {
        id: viewStack
        anchors.left: navDrawer.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        currentIndex: 0  // Default: MapView

        MapView { id: mapViewItem }
        DashboardView { id: dashboardViewItem }
        GraphsView { id: graphsViewItem }
        SettingsView { id: settingsViewItem }
    }

    // Handle map marker click -> dashboard navigation
    Connections {
        target: mapViewItem
        function onShowDashboardForReading(readingId: int): void {
            // Debounce: ignore clicks within 300ms (handles overlapping markers)
            var now = new Date();
            if (mainWindow.lastMarkerClickTime.getTime() > 0) {
                var elapsed = now - mainWindow.lastMarkerClickTime;
                if (elapsed < 300) {
                    return;
                }
            }
            mainWindow.lastMarkerClickTime = now;

            mainWindow.selectedReadingId = readingId;
            navDrawer.selectItem(1);  // Dashboard is index 1
            viewStack.currentIndex = 1;
        }
        ignoreUnknownSignals: true
    }

    // Debug output for received readings
    Connections {
        target: SerialHandler
        function onNewReading(reading: sensorReading): void {
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
