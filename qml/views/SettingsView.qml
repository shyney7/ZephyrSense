pragma ComponentBehavior: Bound
pragma FunctionSignatureBehavior: Enforced
pragma NativeMethodBehavior: AcceptThisObject
pragma ValueTypeBehavior: Addressable

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import ZephyrSense
import "../components"

Item {
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 0

        // Header
        Label {
            text: "Settings"
            font.pixelSize: 24
            font.bold: true
            Layout.bottomMargin: 16
        }

        // Tab bar
        TabBar {
            id: settingsTabBar
            Layout.fillWidth: true

            TabButton { text: "Connection" }
            TabButton { text: "Export" }
            TabButton { text: "Thresholds" }
            TabButton { text: "Display" }
        }

        // Tab content - use Loaders for lazy instantiation
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: settingsTabBar.currentIndex

            Loader {
                active: settingsTabBar.currentIndex === 0
                sourceComponent: ConnectionTab { }
            }
            Loader {
                active: settingsTabBar.currentIndex === 1
                sourceComponent: ExportTab { }
            }
            Loader {
                active: settingsTabBar.currentIndex === 2
                sourceComponent: ThresholdsTab { }
            }
            Loader {
                active: settingsTabBar.currentIndex === 3
                sourceComponent: DisplayTab { }
            }
        }
    }
}
