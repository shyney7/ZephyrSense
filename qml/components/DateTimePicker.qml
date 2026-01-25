import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense

Item {
    id: root

    property alias selectedDate: internal.selectedDate
    property alias selectedHour: internal.selectedHour
    property var availableDates: []  // List of "yyyy-MM-dd" strings
    property string label: "Date/Time"

    signal dateTimeChanged(date dateTime)

    implicitWidth: 200
    implicitHeight: contentCol.implicitHeight

    QtObject {
        id: internal
        property date selectedDate: new Date()
        property int selectedHour: 12
    }

    ColumnLayout {
        id: contentCol
        anchors.fill: parent
        spacing: 4

        Label {
            text: root.label
            font.pixelSize: 12
            color: "#666666"
        }

        RowLayout {
            spacing: 8

            // Date button that opens popup
            Button {
                id: dateButton
                text: Qt.formatDate(internal.selectedDate, "MMM d, yyyy")
                Layout.fillWidth: true
                onClicked: calendarPopup.open()
            }

            // Hour selector
            ComboBox {
                id: hourCombo
                Layout.preferredWidth: 80
                model: {
                    var hours = []
                    for (var i = 0; i < 24; i++) {
                        hours.push({
                            text: i.toString().padStart(2, '0') + ":00",
                            value: i
                        })
                    }
                    return hours
                }
                textRole: "text"
                valueRole: "value"
                currentIndex: internal.selectedHour

                onCurrentValueChanged: {
                    if (currentValue !== undefined) {
                        internal.selectedHour = currentValue
                        emitDateTime()
                    }
                }
            }
        }
    }

    // Calendar popup
    Popup {
        id: calendarPopup
        modal: true
        x: (parent.width - width) / 2
        y: dateButton.y + dateButton.height + 4
        width: 280
        height: 320
        padding: 8

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            // Month/Year navigation
            RowLayout {
                Layout.fillWidth: true

                Button {
                    text: "<"
                    flat: true
                    onClicked: {
                        var d = new Date(monthGrid.year, monthGrid.month - 1, 1)
                        monthGrid.month = d.getMonth()
                        monthGrid.year = d.getFullYear()
                    }
                }

                Label {
                    text: monthGrid.title
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }

                Button {
                    text: ">"
                    flat: true
                    onClicked: {
                        var d = new Date(monthGrid.year, monthGrid.month + 1, 1)
                        monthGrid.month = d.getMonth()
                        monthGrid.year = d.getFullYear()
                    }
                }
            }

            // Day of week header
            DayOfWeekRow {
                Layout.fillWidth: true
                locale: monthGrid.locale
            }

            // Calendar grid
            MonthGrid {
                id: monthGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                month: internal.selectedDate.getMonth()
                year: internal.selectedDate.getFullYear()

                delegate: Rectangle {
                    required property var model
                    required property date date

                    width: monthGrid.width / 7
                    height: 36
                    radius: 4

                    property bool isSelected: date.toDateString() === internal.selectedDate.toDateString()
                    property bool hasData: {
                        var dateStr = Qt.formatDate(date, "yyyy-MM-dd")
                        return root.availableDates.indexOf(dateStr) !== -1
                    }
                    property bool isCurrentMonth: model.month === monthGrid.month

                    color: isSelected ? "#2196F3" : (hasData ? "#E3F2FD" : "transparent")

                    Text {
                        anchors.centerIn: parent
                        text: model.day
                        font.bold: parent.hasData
                        color: {
                            if (parent.isSelected) return "white"
                            if (!parent.isCurrentMonth) return "#BDBDBD"
                            if (model.today) return "#2196F3"
                            return "#333333"
                        }
                    }

                    // Data indicator dot
                    Rectangle {
                        visible: parent.hasData && !parent.isSelected
                        width: 4
                        height: 4
                        radius: 2
                        color: "#4CAF50"
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (parent.isCurrentMonth) {
                                internal.selectedDate = date
                                emitDateTime()
                                calendarPopup.close()
                            }
                        }
                    }
                }
            }
        }
    }

    function emitDateTime() {
        var dt = new Date(
            internal.selectedDate.getFullYear(),
            internal.selectedDate.getMonth(),
            internal.selectedDate.getDate(),
            internal.selectedHour, 0, 0
        )
        root.dateTimeChanged(dt)
    }

    Component.onCompleted: {
        hourCombo.currentIndex = internal.selectedHour
    }
}
