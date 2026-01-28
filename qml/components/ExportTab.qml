import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import ZephyrSense

Item {
    ScrollView {
        anchors.fill: parent
        anchors.margins: 16

        ColumnLayout {
            width: parent.width
            spacing: 16

            Label {
                text: "CSV Export Configuration"
                font.pixelSize: 18
                font.bold: true
            }

            GroupBox {
                title: "Export Settings"
                Layout.fillWidth: true
                Layout.maximumWidth: 600

                ColumnLayout {
                    width: parent.width
                    spacing: 12

                    // Enable/disable toggle
                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "Enable CSV Export:"
                            Layout.preferredWidth: 150
                        }
                        Switch {
                            id: exportSwitch
                            checked: CsvExporter.enabled
                            onToggled: {
                                CsvExporter.enabled = checked;
                            }
                        }
                        Label {
                            text: exportSwitch.checked ? "Enabled" : "Disabled"
                            color: exportSwitch.checked ? "green" : "red"
                            font.bold: true
                        }
                    }

                    // File path display and browse
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: "Export File Path:"
                            font.bold: true
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            color: palette.base
                            border.color: palette.mid
                            border.width: 1
                            radius: 4

                            Label {
                                anchors.fill: parent
                                anchors.margins: 8
                                text: CsvExporter.filePath || "No file selected"
                                elide: Text.ElideMiddle
                                verticalAlignment: Text.AlignVCenter
                                color: CsvExporter.filePath ? palette.text : palette.mid
                            }
                        }

                        Button {
                            text: "Browse..."
                            Layout.alignment: Qt.AlignLeft
                            onClicked: fileDialog.open()
                        }
                    }

                    // Info text
                    Label {
                        text: "When enabled, all incoming sensor readings will be appended to the selected CSV file in real-time."
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        font.italic: true
                        color: '#d9e6f1'
                    }

                    // Reset button
                    Button {
                        text: "Reset (Disable and Clear Path)"
                        Layout.alignment: Qt.AlignRight
                        onClicked: {
                            CsvExporter.enabled = false;
                            CsvExporter.filePath = "";
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    // File dialog for selecting export path
    FileDialog {
        id: fileDialog
        fileMode: FileDialog.SaveFile
        nameFilters: ["CSV files (*.csv)", "All files (*)"]
        defaultSuffix: "csv"
        onAccepted: {
            CsvExporter.filePath = selectedFile.toString().replace("file:///", "");
        }
    }
}
