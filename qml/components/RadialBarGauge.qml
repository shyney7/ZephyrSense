import QtQuick
import QtQuick.Shapes
import ZephyrSense

Item {
    id: root
    width: height  // Maintain circular aspect ratio

    SystemPalette { id: palette; colorGroup: SystemPalette.Active }

    // Listen for threshold changes to update colors
    Connections {
        target: ThresholdManager
        function onThresholdsChanged() {
            // Force re-evaluation of color bindings
            root.value = root.value
        }
    }

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

    readonly property real dialWidth: 14
    readonly property real normalizedValue: Math.min(Math.max(value, minValue), maxValue)
    readonly property real sweepAngle: ((normalizedValue - minValue) / (maxValue - minValue)) * 360

    // Check if sensor is enabled for hazard calculation
    function isSensorEnabled() {
        switch (sensorKey) {
            case "partectorNumber": return ThresholdManager.partectorNumberEnabled
            case "partectorDiam": return ThresholdManager.partectorDiamEnabled
            case "partectorMass": return ThresholdManager.partectorMassEnabled
            case "grimmValue": return ThresholdManager.grimmValueEnabled
            case "temperature": return ThresholdManager.temperatureEnabled
            case "humidity": return ThresholdManager.humidityEnabled
            case "pressure": return ThresholdManager.pressureEnabled
            case "altitude": return ThresholdManager.altitudeEnabled
            case "co2": return ThresholdManager.co2Enabled
            default: return true
        }
    }

    // Threshold-based color computation
    function getProgressColor(val) {
        // If sensor is disabled, show neutral blue
        if (!isSensorEnabled()) {
            return "#2196F3"  // Neutral blue for disabled sensors
        }
        if (sensorKey === "temperature") {
            // Bidirectional - check both high and low
            if (val >= ThresholdManager.temperatureDanger || val <= ThresholdManager.temperatureLowDanger) {
                return "#F44336" // red
            }
            if (val >= ThresholdManager.temperatureWarning || val <= ThresholdManager.temperatureLowWarning) {
                return "#FF9800" // orange/yellow
            }
            return "#4CAF50" // green
        } else if (sensorKey === "humidity") {
            // Bidirectional - check both high and low
            if (val >= ThresholdManager.humidityDanger || val <= ThresholdManager.humidityLowDanger) {
                return "#F44336" // red
            }
            if (val >= ThresholdManager.humidityWarning || val <= ThresholdManager.humidityLowWarning) {
                return "#FF9800" // orange/yellow
            }
            return "#4CAF50" // green
        } else if (sensorKey === "co2") {
            // Unidirectional - high is dangerous
            if (val >= ThresholdManager.co2Danger) {
                return "#F44336" // red
            }
            if (val >= ThresholdManager.co2Warning) {
                return "#FF9800" // orange/yellow
            }
            return "#4CAF50" // green
        } else if (sensorKey === "partectorNumber") {
            if (val >= ThresholdManager.partectorNumberDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.partectorNumberWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        } else if (sensorKey === "partectorDiam") {
            if (val >= ThresholdManager.partectorDiamDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.partectorDiamWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        } else if (sensorKey === "partectorMass") {
            if (val >= ThresholdManager.partectorMassDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.partectorMassWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        } else if (sensorKey === "grimmValue") {
            if (val >= ThresholdManager.grimmValueDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.grimmValueWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        } else if (sensorKey === "pressure") {
            if (val >= ThresholdManager.pressureDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.pressureWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        } else if (sensorKey === "altitude") {
            if (val >= ThresholdManager.altitudeDanger) {
                return "#F44336"
            }
            if (val >= ThresholdManager.altitudeWarning) {
                return "#FF9800"
            }
            return "#4CAF50"
        }

        // Default green if sensor not recognized
        return "#4CAF50"
    }

    // Sensor name label at top
    Text {
        id: sensorLabel
        text: root.sensorName
        font.pixelSize: 11
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
        width: root.width
        height: root.height

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
        width: root.width
        height: root.height

        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeWidth: root.dialWidth
            strokeColor: getProgressColor(root.normalizedValue)
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
