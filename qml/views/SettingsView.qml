import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Item {
    ScrollView {
        anchors.fill: parent
        anchors.margins: 16

        ColumnLayout {
            width: parent.width
            spacing: 16

            Label {
                text: "Settings"
                font.pixelSize: 24
                font.bold: true
            }

            // Connection settings (existing component)
            ConnectionPanel {
                Layout.fillWidth: true
                Layout.maximumWidth: 400
            }

            // Threshold configuration section
            GroupBox {
                title: "Hazard Thresholds"
                Layout.fillWidth: true
                Layout.maximumWidth: 600

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Header row
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Sensor"; Layout.preferredWidth: 150; font.bold: true }
                        Label { text: "Warning (Yellow)"; Layout.preferredWidth: 120; font.bold: true }
                        Label { text: "Danger (Red)"; Layout.preferredWidth: 120; font.bold: true }
                    }

                    // CO2 (ppm)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "CO2 (ppm)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 400; to: 5000; stepSize: 100
                            value: ThresholdManager.co2Warning
                            onValueModified: ThresholdManager.co2Warning = value
                        }
                        SpinBox {
                            from: 400; to: 10000; stepSize: 100
                            value: ThresholdManager.co2Danger
                            onValueModified: ThresholdManager.co2Danger = value
                        }
                    }

                    // Temperature High (C)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Temperature High (C)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 20; to: 50
                            value: Math.round(ThresholdManager.temperatureWarning)
                            onValueModified: ThresholdManager.temperatureWarning = value
                        }
                        SpinBox {
                            from: 25; to: 60
                            value: Math.round(ThresholdManager.temperatureDanger)
                            onValueModified: ThresholdManager.temperatureDanger = value
                        }
                    }

                    // Temperature Low (C)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Temperature Low (C)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: -10; to: 20
                            value: Math.round(ThresholdManager.temperatureLowWarning)
                            onValueModified: ThresholdManager.temperatureLowWarning = value
                        }
                        SpinBox {
                            from: -20; to: 15
                            value: Math.round(ThresholdManager.temperatureLowDanger)
                            onValueModified: ThresholdManager.temperatureLowDanger = value
                        }
                    }

                    // Humidity High (%)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Humidity High (%)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 40; to: 90
                            value: Math.round(ThresholdManager.humidityWarning)
                            onValueModified: ThresholdManager.humidityWarning = value
                        }
                        SpinBox {
                            from: 50; to: 100
                            value: Math.round(ThresholdManager.humidityDanger)
                            onValueModified: ThresholdManager.humidityDanger = value
                        }
                    }

                    // Humidity Low (%)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Humidity Low (%)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 10; to: 40
                            value: Math.round(ThresholdManager.humidityLowWarning)
                            onValueModified: ThresholdManager.humidityLowWarning = value
                        }
                        SpinBox {
                            from: 5; to: 30
                            value: Math.round(ThresholdManager.humidityLowDanger)
                            onValueModified: ThresholdManager.humidityLowDanger = value
                        }
                    }

                    // Particulate Mass (ug/m3)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "PM Mass (ug/m3)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 10; to: 100
                            value: Math.round(ThresholdManager.partectorMassWarning)
                            onValueModified: ThresholdManager.partectorMassWarning = value
                        }
                        SpinBox {
                            from: 20; to: 200
                            value: Math.round(ThresholdManager.partectorMassDanger)
                            onValueModified: ThresholdManager.partectorMassDanger = value
                        }
                    }

                    // GRIMM Value (particles/cm3)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "GRIMM (p/cm3)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 10; to: 100
                            value: Math.round(ThresholdManager.grimmValueWarning)
                            onValueModified: ThresholdManager.grimmValueWarning = value
                        }
                        SpinBox {
                            from: 20; to: 200
                            value: Math.round(ThresholdManager.grimmValueDanger)
                            onValueModified: ThresholdManager.grimmValueDanger = value
                        }
                    }

                    // Particle Count (particles/cm3) - use thousands
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Particles (k/cm3)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 1; to: 100
                            value: ThresholdManager.partectorNumberWarning / 1000
                            onValueModified: ThresholdManager.partectorNumberWarning = value * 1000
                        }
                        SpinBox {
                            from: 10; to: 500
                            value: ThresholdManager.partectorNumberDanger / 1000
                            onValueModified: ThresholdManager.partectorNumberDanger = value * 1000
                        }
                    }

                    // Particle Diameter (nm)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Diameter (nm)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 50; to: 300
                            value: ThresholdManager.partectorDiamWarning
                            onValueModified: ThresholdManager.partectorDiamWarning = value
                        }
                        SpinBox {
                            from: 100; to: 500
                            value: ThresholdManager.partectorDiamDanger
                            onValueModified: ThresholdManager.partectorDiamDanger = value
                        }
                    }

                    // Pressure (hPa)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Pressure (hPa)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 900; to: 1100
                            value: Math.round(ThresholdManager.pressureWarning)
                            onValueModified: ThresholdManager.pressureWarning = value
                        }
                        SpinBox {
                            from: 950; to: 1150
                            value: Math.round(ThresholdManager.pressureDanger)
                            onValueModified: ThresholdManager.pressureDanger = value
                        }
                    }

                    // Altitude (m)
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Altitude (m)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 1000; to: 5000; stepSize: 100
                            value: Math.round(ThresholdManager.altitudeWarning)
                            onValueModified: ThresholdManager.altitudeWarning = value
                        }
                        SpinBox {
                            from: 2000; to: 8000; stepSize: 100
                            value: Math.round(ThresholdManager.altitudeDanger)
                            onValueModified: ThresholdManager.altitudeDanger = value
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
