pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: legendRoot

    // Currently selected sensor (column index 1-9)
    property int selectedSensor: 5  // Default: Temperature

    // Signal emitted when user selects a sensor
    signal sensorSelected(int column)

    // Sensor definitions (use "sensorColumn" to avoid QModelIndex::column() shadowing)
    readonly property var sensors: [
        { sensorColumn: 1, name: "PNC UFP", sensorColor: "#E91E63" },
        { sensorColumn: 2, name: "\u00D8 UFP", sensorColor: "#9C27B0" },
        { sensorColumn: 3, name: "PM0.3", sensorColor: "#673AB7" },
        { sensorColumn: 4, name: "PNC PM", sensorColor: "#3F51B5" },
        { sensorColumn: 5, name: "Temperature", sensorColor: "#FF5722" },
        { sensorColumn: 6, name: "Humidity", sensorColor: "#2196F3" },
        { sensorColumn: 7, name: "Pressure", sensorColor: "#009688" },
        { sensorColumn: 8, name: "Altitude", sensorColor: "#4CAF50" },
        { sensorColumn: 9, name: "CO\u2082", sensorColor: "#795548" }
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

                required property int index
                required property int sensorColumn
                required property string name
                required property color sensorColor

                Layout.fillWidth: true
                Layout.fillHeight: true
                color: legendRoot.selectedSensor === sensorRect.sensorColumn ? sensorRect.sensorColor : "transparent"
                border.color: sensorRect.sensorColor
                border.width: 2
                radius: 4
                opacity: legendRoot.selectedSensor === sensorRect.sensorColumn ? 1.0 : 0.6

                Label {
                    anchors.centerIn: parent
                    text: sensorRect.name
                    font.pixelSize: 11
                    font.bold: legendRoot.selectedSensor === sensorRect.sensorColumn
                    color: legendRoot.selectedSensor === sensorRect.sensorColumn ? "white" : sensorRect.sensorColor
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        legendRoot.selectedSensor = sensorRect.sensorColumn
                        legendRoot.sensorSelected(sensorRect.sensorColumn)
                    }
                }
            }
        }
    }
}
