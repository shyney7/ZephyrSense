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
                text: "Hazard Threshold Configuration"
                font.pixelSize: 18
                font.bold: true
            }

            // Core Sensors Section
            GroupBox {
                title: "Core Sensors (Air Quality)"
                Layout.fillWidth: true
                Layout.maximumWidth: 700

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Header row
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Enabled"; Layout.preferredWidth: 60; font.bold: true }
                        Label { text: "Sensor"; Layout.preferredWidth: 150; font.bold: true }
                        Label { text: "Warning (Yellow)"; Layout.preferredWidth: 140; font.bold: true }
                        Label { text: "Danger (Red)"; Layout.preferredWidth: 140; font.bold: true }
                        Label { text: "Info"; Layout.preferredWidth: 30; font.bold: true }
                    }

                    // PM Mass
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.partectorMassEnabled
                            onToggled: ThresholdManager.partectorMassEnabled = checked
                        }
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

                    // Particle Count
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.partectorNumberEnabled
                            onToggled: ThresholdManager.partectorNumberEnabled = checked
                        }
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

                    // Particle Diameter (inverted)
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.partectorDiamEnabled
                            onToggled: ThresholdManager.partectorDiamEnabled = checked
                        }
                        Label { text: "Diameter (nm)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 50; to: 300
                            value: ThresholdManager.partectorDiamWarning
                            onValueModified: ThresholdManager.partectorDiamWarning = value
                        }
                        SpinBox {
                            from: 10; to: 500
                            value: ThresholdManager.partectorDiamDanger
                            onValueModified: ThresholdManager.partectorDiamDanger = value
                        }
                        Label {
                            text: "⚠"
                            font.pixelSize: 16
                            ToolTip.visible: diameterInfo.containsMouse
                            ToolTip.text: "Inverted threshold: Lower values are more concerning (smaller particles penetrate deeper)"
                            MouseArea {
                                id: diameterInfo
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }

                    // GRIMM Value
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.grimmValueEnabled
                            onToggled: ThresholdManager.grimmValueEnabled = checked
                        }
                        Label { text: "GRIMM (p/cm3)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 10; to: 100000; stepSize: 1000
                            value: Math.round(ThresholdManager.grimmValueWarning)
                            onValueModified: ThresholdManager.grimmValueWarning = value
                        }
                        SpinBox {
                            from: 20; to: 200000; stepSize: 1000
                            value: Math.round(ThresholdManager.grimmValueDanger)
                            onValueModified: ThresholdManager.grimmValueDanger = value
                        }
                    }

                    // CO2
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.co2Enabled
                            onToggled: ThresholdManager.co2Enabled = checked
                        }
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
                }
            }

            // Comfort Sensors Section
            GroupBox {
                title: "Comfort Sensors (Environmental)"
                Layout.fillWidth: true
                Layout.maximumWidth: 700

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    // Header row
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "Enabled"; Layout.preferredWidth: 60; font.bold: true }
                        Label { text: "Sensor"; Layout.preferredWidth: 150; font.bold: true }
                        Label { text: "Warning (Yellow)"; Layout.preferredWidth: 140; font.bold: true }
                        Label { text: "Danger (Red)"; Layout.preferredWidth: 140; font.bold: true }
                        Label { text: "Info"; Layout.preferredWidth: 30; font.bold: true }
                    }

                    // Temperature High
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.temperatureEnabled
                            onToggled: ThresholdManager.temperatureEnabled = checked
                        }
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

                    // Temperature Low (inverted)
                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.preferredWidth: 60 } // Spacer (shares checkbox with high)
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
                        Label {
                            text: "⚠"
                            font.pixelSize: 16
                            ToolTip.visible: tempInfo.containsMouse
                            ToolTip.text: "Inverted threshold: Lower values are more concerning (extreme cold)"
                            MouseArea {
                                id: tempInfo
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }

                    // Humidity High
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.humidityEnabled
                            onToggled: ThresholdManager.humidityEnabled = checked
                        }
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

                    // Humidity Low (inverted)
                    RowLayout {
                        Layout.fillWidth: true
                        Item { Layout.preferredWidth: 60 } // Spacer (shares checkbox with high)
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
                        Label {
                            text: "⚠"
                            font.pixelSize: 16
                            ToolTip.visible: humidityInfo.containsMouse
                            ToolTip.text: "Inverted threshold: Lower values are more concerning (extreme dryness)"
                            MouseArea {
                                id: humidityInfo
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }

                    // Pressure (inverted)
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.pressureEnabled
                            onToggled: ThresholdManager.pressureEnabled = checked
                        }
                        Label { text: "Pressure (hPa)"; Layout.preferredWidth: 150 }
                        SpinBox {
                            from: 900; to: 1100
                            value: Math.round(ThresholdManager.pressureWarning)
                            onValueModified: ThresholdManager.pressureWarning = value
                        }
                        SpinBox {
                            from: 850; to: 1150
                            value: Math.round(ThresholdManager.pressureDanger)
                            onValueModified: ThresholdManager.pressureDanger = value
                        }
                        Label {
                            text: "⚠"
                            font.pixelSize: 16
                            ToolTip.visible: pressureInfo.containsMouse
                            ToolTip.text: "Inverted threshold: Lower values are more concerning (low pressure)"
                            MouseArea {
                                id: pressureInfo
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }

                    // Altitude
                    RowLayout {
                        Layout.fillWidth: true
                        CheckBox {
                            checked: ThresholdManager.altitudeEnabled
                            onToggled: ThresholdManager.altitudeEnabled = checked
                        }
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

            // Reset button
            Button {
                text: "Reset All to Defaults"
                Layout.alignment: Qt.AlignRight
                highlighted: true
                onClicked: {
                    ThresholdManager.resetToDefaults();
                }
            }

            Label {
                text: "Note: Disabled sensors show neutral blue color in gauges and do not contribute to hazard level calculation"
                font.italic: true
                color: palette.mid
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.maximumWidth: 700
            }

            Item { Layout.fillHeight: true }
        }
    }
}
