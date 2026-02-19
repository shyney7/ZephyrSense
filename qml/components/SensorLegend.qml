pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: legendRoot

    // Currently selected sensor (column index 1-9)
    property int selectedSensor: 5  // Default: Temperature

    // Signal emitted when user selects a sensor
    signal sensorSelected(int column)

    // Sensor definitions
    readonly property var sensors: [
        { column: 1, name: "Partector #", color: "#E91E63" },
        { column: 2, name: "Diameter", color: "#9C27B0" },
        { column: 3, name: "Mass", color: "#673AB7" },
        { column: 4, name: "Grimm", color: "#3F51B5" },
        { column: 5, name: "Temp", color: "#FF5722" },
        { column: 6, name: "Humidity", color: "#2196F3" },
        { column: 7, name: "Pressure", color: "#009688" },
        { column: 8, name: "Altitude", color: "#4CAF50" },
        { column: 9, name: "CO2", color: "#795548" }
    ]

    color: "#F5F5F5"
    radius: 4

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        Repeater {
            objectName: "sensorRepeater"
            model: legendRoot.sensors

            Rectangle {
                id: sensorRect

                required property var modelData
                required property int index

                Layout.fillWidth: true
                Layout.fillHeight: true
                color: legendRoot.selectedSensor === sensorRect.modelData.column ? sensorRect.modelData.color : "transparent"
                border.color: sensorRect.modelData.color
                border.width: 2
                radius: 4
                opacity: legendRoot.selectedSensor === sensorRect.modelData.column ? 1.0 : 0.6

                Label {
                    anchors.centerIn: parent
                    text: sensorRect.modelData.name
                    font.pixelSize: 11
                    font.bold: legendRoot.selectedSensor === sensorRect.modelData.column
                    color: legendRoot.selectedSensor === sensorRect.modelData.column ? "white" : sensorRect.modelData.color
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        legendRoot.selectedSensor = sensorRect.modelData.column
                        legendRoot.sensorSelected(sensorRect.modelData.column)
                    }
                }
            }
        }
    }
}
