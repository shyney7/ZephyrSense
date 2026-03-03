pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Shapes
import ZephyrSense

Item {
    id: root

    readonly property ThresholdManager thresholdMgr: ThresholdManager

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
    QtObject {
        id: internalState
        property int thresholdsEpoch: 0
    }

    // Public properties
    property real value: 0
    property real minValue: 0
    property real maxValue: 100
    property string sensorKey: ""
    property string sensorName: ""
    property string unit: ""
    property int precision: 1
    readonly property int thresholdsEpoch: internalState.thresholdsEpoch

    implicitWidth: 140
    implicitHeight: 140

    // Square gauge size derived from the smaller layout dimension
    readonly property real gaugeSize: Math.max(Math.min(width, height), 1)
    readonly property real dialWidth: 14
    readonly property real normalizedValue: Math.min(Math.max(value, minValue), maxValue)
    readonly property real sweepAngle: root.computeAdaptiveSweep(root.thresholdsEpoch)

    // Color switching logic remains in ThresholdManager.getColorForSensor().
    // thresholdsEpoch ensures this binding re-evaluates on thresholdsChanged.
    readonly property color progressColor: root.computeProgressColor(root.thresholdsEpoch)

    function computeAdaptiveSweep(_thresholdsEpoch: int): real {
        // _thresholdsEpoch is a reactivity dependency; value is intentionally unused.
        return root.thresholdMgr.getSweepAngleForSensor(root.sensorKey, root.value, root.minValue, root.maxValue);
    }

    function computeProgressColor(_thresholdsEpoch: int): color {
        // _thresholdsEpoch is a reactivity dependency; value is intentionally unused.
        return root.thresholdMgr.getColorForSensor(root.sensorKey, root.normalizedValue);
    }

    Connections {
        target: root.thresholdMgr
        function onThresholdsChanged(): void {
            internalState.thresholdsEpoch += 1;
        }
    }

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
