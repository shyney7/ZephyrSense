pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced

import QtQuick

QtObject {
    readonly property var sensorConfig: [
        { sensorKey: "partectorNumber", sensorName: "PNC UFP", unit: "#/cm\u00B3", minValue: 0, maxValue: 500000, precision: 0 },
        { sensorKey: "partectorDiam", sensorName: "\u00D8 UFP", unit: "nm", minValue: 0, maxValue: 500, precision: 0 },
        { sensorKey: "partectorMass", sensorName: "PM0.3", unit: "\u00B5g/m\u00B3", minValue: 0, maxValue: 200, precision: 2 },
        { sensorKey: "grimmValue", sensorName: "PNC PM", unit: "#/cm\u00B3", minValue: 0, maxValue: 200000, precision: 2 },
        { sensorKey: "temperature", sensorName: "Temperature", unit: "\u00B0C", minValue: -20, maxValue: 60, precision: 1 },
        { sensorKey: "humidity", sensorName: "Humidity", unit: "%", minValue: 0, maxValue: 100, precision: 1 },
        { sensorKey: "pressure", sensorName: "Pressure", unit: "hPa", minValue: 900, maxValue: 1150, precision: 1 },
        { sensorKey: "altitude", sensorName: "Altitude", unit: "m", minValue: 0, maxValue: 8000, precision: 1 },
        { sensorKey: "co2", sensorName: "CO\u2082", unit: "ppm", minValue: 0, maxValue: 10000, precision: 0 }
    ]
}
