pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property bool collapsed: false
    property alias currentIndex: navList.currentIndex

    signal navigationRequested(int index, string viewPath)

    // Programmatic item selection (called from Main.qml when navigating via map marker click)
    function selectItem(index: int): void {
        navList.currentIndex = index;
    }

    width: collapsed ? 60 : 220
    color: "#f5f5f5"
    border.color: "#e0e0e0"
    border.width: 1

    Behavior on width {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Navigation items
        ListView {
            id: navList
            objectName: "navList"
            Layout.fillWidth: true
            Layout.fillHeight: true
            interactive: false
            currentIndex: 0

            model: ListModel {
                ListElement {
                    title: "Map"
                    iconText: "M"
                    viewPath: "qml/views/MapView.qml"
                }
                ListElement {
                    title: "Dashboard"
                    iconText: "D"
                    viewPath: "qml/views/DashboardView.qml"
                }
                ListElement {
                    title: "Graphs"
                    iconText: "G"
                    viewPath: "qml/views/GraphsView.qml"
                }
                ListElement {
                    title: "Settings"
                    iconText: "S"
                    viewPath: "qml/views/SettingsView.qml"
                }
            }

            delegate: ItemDelegate {
                id: navItem

                required property string iconText
                required property string title
                required property string viewPath
                required property int index

                width: ListView.view.width
                height: 48
                highlighted: ListView.isCurrentItem

                contentItem: RowLayout {
                    spacing: 12

                    // Icon/symbol
                    Rectangle {
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        Layout.leftMargin: 8
                        color: navItem.highlighted ? "#2196F3" : "#757575"
                        radius: 4

                        Label {
                            anchors.centerIn: parent
                            text: navItem.iconText
                            font.pixelSize: 16
                            font.bold: true
                            color: "white"
                        }
                    }

                    // Title text (hidden when collapsed)
                    Label {
                        text: navItem.title
                        font.pixelSize: 14
                        color: navItem.highlighted ? "#000000" : "#424242"
                        visible: !root.collapsed
                        Layout.fillWidth: true
                    }
                }

                background: Rectangle {
                    color: navItem.highlighted ? "#e0e0e0" : (navItem.hovered ? "#eeeeee" : "transparent")
                }

                onClicked: {
                    navList.currentIndex = navItem.index;
                    root.navigationRequested(navItem.index, navItem.viewPath);
                }

                // Tooltip when collapsed
                ToolTip.visible: root.collapsed && navItem.hovered
                ToolTip.text: navItem.title
                ToolTip.delay: 500
            }
        }

        // Collapse/expand toggle button
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#e0e0e0"
        }

        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            text: root.collapsed ? ">>" : "<<"
            flat: true
            background: Rectangle {
                color: '#4a82fc'
            }

            onClicked: {
                root.collapsed = !root.collapsed;
            }

            ToolTip.visible: hovered
            ToolTip.text: root.collapsed ? "Expand sidebar" : "Collapse sidebar"
            ToolTip.delay: 500
        }
    }
}
