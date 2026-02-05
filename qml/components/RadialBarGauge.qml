import QtQuick
import QtQuick.Shapes
import ZephyrSense

Item {
    id: root

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }

    // Public properties
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property string sensorKey: ""
    property string sensorName: ""
    property string unit: ""
    property int precision: 1

    implicitWidth: 140
    implicitHeight: 140

    // Square gauge size derived from the smaller layout dimension
    readonly property real gaugeSize: Math.max(Math.min(width, height), 1)
    readonly property real dialWidth: 14
    readonly property real normalizedValue: Math.min(Math.max(value, minValue), maxValue)
    readonly property real sweepAngle: ((normalizedValue - minValue) / (maxValue - minValue)) * 360

    // Color computed in C++ for performance - responds to thresholdsChanged signal automatically
    readonly property string progressColor: ThresholdManager.getColorForSensor(sensorKey, normalizedValue)

    // Sensor name label at top
    Text {
        id: sensorLabel
        text: root.sensorName
        font.pixelSize: 14
        font.bold: true
        color: palette.text
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
    }

    // Background dial (full circle)
    Shape {
        id: backgroundShape
        anchors.centerIn: parent
        width: root.gaugeSize
        height: root.gaugeSize

        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeWidth: root.dialWidth
            strokeColor: "#E0E0E0"
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: backgroundShape.width / 2
                centerY: backgroundShape.height / 2
                radiusX: (backgroundShape.width - root.dialWidth) / 2
                radiusY: (backgroundShape.height - root.dialWidth) / 2
                startAngle: -90
                sweepAngle: 360
            }
        }
    }

    // Progress arc (colored based on threshold)
    Shape {
        id: progressShape
        anchors.centerIn: parent
        width: root.gaugeSize
        height: root.gaugeSize

        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeWidth: root.dialWidth
            strokeColor: root.progressColor
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: progressShape.width / 2
                centerY: progressShape.height / 2
                radiusX: (progressShape.width - root.dialWidth) / 2
                radiusY: (progressShape.height - root.dialWidth) / 2
                startAngle: -90
                sweepAngle: root.sweepAngle

                Behavior on sweepAngle {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
    }

    // Center content - value and unit
    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            id: valueText
            text: root.normalizedValue.toFixed(root.precision)
            font.pixelSize: 20
            font.bold: true
            color: palette.text
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Text {
            id: unitText
            text: root.unit
            font.pixelSize: 11
            color: palette.windowText
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
