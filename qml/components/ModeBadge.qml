import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property bool isLive: true
    property string historicalLabel: "HISTORICAL"

    width: label.width + 16
    height: label.height + 8
    radius: 4
    color: isLive ? "#4CAF50" : "#2196F3"

    Label {
        id: label
        anchors.centerIn: parent
        text: root.isLive ? "LIVE" : root.historicalLabel
        color: "white"
        font.bold: true
        font.pixelSize: 10
    }
}
