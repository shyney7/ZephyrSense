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
            model: sensors

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: selectedSensor === modelData.column ? modelData.color : "transparent"
                border.color: modelData.color
                border.width: 2
                radius: 4
                opacity: selectedSensor === modelData.column ? 1.0 : 0.6

                Label {
                    anchors.centerIn: parent
                    text: modelData.name
                    font.pixelSize: 11
                    font.bold: selectedSensor === modelData.column
                    color: selectedSensor === modelData.column ? "white" : modelData.color
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        selectedSensor = modelData.column
                        sensorSelected(modelData.column)
                    }
                }
            }
        }
    }
}
