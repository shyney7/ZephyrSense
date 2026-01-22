import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning
import ZephyrSense

Item {
    id: mapViewRoot

    // Signal to request navigation to dashboard with specific reading
    signal showDashboardForReading(int readingId)

    // Model instance for map markers
    SensorReadingModel {
        id: sensorModel
    }

    // Main map container
    MapView {
        id: mapView
        anchors.fill: parent

        map.plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.useragent"
                value: "ZephyrSense/1.0"
            }
        }

        // Default view: Oslo, Norway (reasonable starting point)
        map.center: QtPositioning.coordinate(59.91, 10.75)
        map.zoomLevel: 10

        // Marker layer using MapItemView
        MapItemView {
            id: markerView
            model: sensorModel
            parent: mapView.map

            delegate: SensorMarker {
                // Required properties auto-injected from model roles:
                // latitude, longitude, tooltipText, readingId

                onMarkerClicked: function(id) {
                    mapViewRoot.showDashboardForReading(id)
                }
            }
        }
    }

    // Info overlay showing point count
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 16
        width: infoLabel.width + 24
        height: infoLabel.height + 12
        color: "#E0E0E0"
        radius: 4
        opacity: 0.9

        Label {
            id: infoLabel
            anchors.centerIn: parent
            text: sensorModel.count + " points"
            font.pixelSize: 12
        }
    }

    // Control buttons
    ColumnLayout {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        spacing: 8

        Button {
            text: "Load Last Hour"
            onClicked: {
                let now = new Date()
                let oneHourAgo = new Date(now.getTime() - 60 * 60 * 1000)
                sensorModel.loadFromDatabase(oneHourAgo, now)

                // Center map on first point if data exists
                if (sensorModel.count > 0) {
                    let first = sensorModel.getReading(0)
                    mapView.map.center = QtPositioning.coordinate(first.latitude, first.longitude)
                }
            }
        }

        Button {
            text: "Load Last 24h"
            onClicked: {
                let now = new Date()
                let dayAgo = new Date(now.getTime() - 24 * 60 * 60 * 1000)
                sensorModel.loadFromDatabase(dayAgo, now)

                if (sensorModel.count > 0) {
                    let first = sensorModel.getReading(0)
                    mapView.map.center = QtPositioning.coordinate(first.latitude, first.longitude)
                }
            }
        }

        Button {
            text: "Clear"
            onClicked: sensorModel.clear()
        }
    }

    // NOTE: Real-time updates come through database polling, not direct signal connection
    // SerialHandler.newReading -> DatabaseManager.insertReading is wired in main.cpp
    // Future enhancement: Add auto-refresh timer to periodically call loadFromDatabase()
}
