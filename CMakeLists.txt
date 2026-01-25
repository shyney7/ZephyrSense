cmake_minimum_required(VERSION 3.16)

project(ZephyrSense VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 SerialPort Sql Location Positioning Charts Widgets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appZephyrSense
    main.cpp
    src/core/sensorreading.cpp
    src/core/sensorreading.h
    src/core/thresholdmanager.cpp
    src/core/thresholdmanager.h
    src/serial/serialhandler.cpp
    src/serial/serialhandler.h
    src/data/databasemanager.cpp
    src/data/databasemanager.h
    src/data/csvexporter.cpp
    src/data/csvexporter.h
    src/models/sensorreadingmodel.cpp
    src/models/sensorreadingmodel.h
    src/models/timeserieschartmodel.cpp
    src/models/timeserieschartmodel.h
)

qt_add_qml_module(appZephyrSense
    URI ZephyrSense
    VERSION 1.0
    QML_FILES
        Main.qml
        qml/components/ConnectionPanel.qml
        qml/components/NavigationDrawer.qml
        qml/components/RadialBarGauge.qml
        qml/components/SensorMarker.qml
        qml/components/TimeSeriesChart.qml
        qml/components/SensorLegend.qml
        qml/components/ConnectionTab.qml
        qml/components/ExportTab.qml
        qml/components/ThresholdsTab.qml
        qml/components/DisplayTab.qml
        qml/components/DateTimePicker.qml
        qml/components/ModeBadge.qml
        qml/views/MapView.qml
        qml/views/DashboardView.qml
        qml/views/GraphsView.qml
        qml/views/SettingsView.qml
    SOURCES
        src/core/sensorreading.cpp
        src/core/sensorreading.h
        src/core/thresholdmanager.cpp
        src/core/thresholdmanager.h
        src/serial/serialhandler.cpp
        src/serial/serialhandler.h
        src/data/databasemanager.cpp
        src/data/databasemanager.h
        src/data/csvexporter.cpp
        src/data/csvexporter.h
        src/models/sensorreadingmodel.cpp
        src/models/sensorreadingmodel.h
        src/models/timeserieschartmodel.cpp
        src/models/timeserieschartmodel.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appZephyrSense PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appZephyrSense
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_include_directories(appZephyrSense PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serial
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
)

target_link_libraries(appZephyrSense
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::SerialPort Qt6::Sql Qt6::Location Qt6::Positioning Qt6::Charts Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS appZephyrSense
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
