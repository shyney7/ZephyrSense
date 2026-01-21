import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8

        Label {
            text: "Graphs View"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "Time-series charts will be implemented in Phase 6"
            color: "#757575"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
