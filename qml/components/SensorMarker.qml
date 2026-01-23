import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

MapQuickItem {
    id: marker

    // Required properties from model
    required property real latitude
    required property real longitude
    required property string tooltipText
    required property int readingId
    required property int hazardLevel

    // Signal for click handling
    signal markerClicked(int id)

    coordinate: QtPositioning.coordinate(latitude, longitude)
    anchorPoint.x: markerCircle.width / 2
    anchorPoint.y: markerCircle.height / 2

    sourceItem: Item {
        id: markerItem
        width: 24
        height: 24

        Rectangle {
            id: markerCircle
            anchors.fill: parent
            radius: width / 2
            color: marker.hazardLevel === 2 ? "#F44336" :  // Red (danger)
                   marker.hazardLevel === 1 ? "#FFC107" :  // Yellow/Amber (warning)
                                              "#4CAF50"    // Green (normal)
            border.color: "white"
            border.width: 2

            // Hover effect
            scale: hoverHandler.hovered ? 1.3 : 1.0
            Behavior on scale {
                NumberAnimation { duration: 100 }
            }
        }

        HoverHandler {
            id: hoverHandler
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: marker.markerClicked(marker.readingId)
        }

        ToolTip {
            visible: hoverHandler.hovered
            delay: 300
            timeout: 10000
            text: marker.tooltipText

            // Style the tooltip for readability
            contentItem: Text {
                text: marker.tooltipText
                font.family: "monospace"
                font.pixelSize: 11
                color: "#333333"
            }

            background: Rectangle {
                color: "#FFFDE7"  // Light yellow
                border.color: "#FFC107"
                border.width: 1
                radius: 4
            }
        }
    }
}
