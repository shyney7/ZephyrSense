import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 8

        Label {
            text: "Dashboard View"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: "RadialBar gauges will be implemented in Phase 7"
            color: "#757575"
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
