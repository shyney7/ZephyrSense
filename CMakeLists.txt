cmake_minimum_required(VERSION 3.25)

project(ZephyrSense VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTORCC ON)
if(NOT DEFINED CMAKE_PREFIX_PATH AND NOT DEFINED ENV{CMAKE_PREFIX_PATH})
    set(CMAKE_PREFIX_PATH "C:/Qt/6.10.1/msvc2022_64")
endif()
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/installer/appicon.rc")

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 SerialPort Sql Location Positioning Charts Widgets)

qt_standard_project_setup(REQUIRES 6.8)

# Project-wide compiler warnings (applied only to our own targets)
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wall;-Wextra;-Woverloaded-virtual;-Wweak-vtables>
    $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Woverloaded-virtual>
)

# KDDockWidgets — QtQuick frontend only
set(KDDockWidgets_FRONTENDS "qtquick" CACHE STRING "" FORCE)
set(KDDockWidgets_QT6 ON CACHE BOOL "" FORCE)
set(KDDockWidgets_EXAMPLES OFF CACHE BOOL "" FORCE)
set(KDDockWidgets_TESTS OFF CACHE BOOL "" FORCE)
set(KDDockWidgets_DOCS OFF CACHE BOOL "" FORCE)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/KDDockWidgets-2.4.0/CMakeLists.txt")
    add_subdirectory(libs/KDDockWidgets-2.4.0 SYSTEM)
else()
    message(STATUS "KDDockWidgets not found locally, fetching from GitHub...")
    include(FetchContent)
    FetchContent_Declare(KDDockWidgets
        GIT_REPOSITORY https://github.com/KDAB/KDDockWidgets.git
        GIT_TAG        v2.4.0
        GIT_SHALLOW    TRUE
        SYSTEM
    )
    FetchContent_MakeAvailable(KDDockWidgets)
endif()

# Place KDDW DLL next to the app executable
set_target_properties(kddockwidgets PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

qt_add_executable(appZephyrSense
    main.cpp
    ${app_icon_resource_windows}
    installer/resources.qrc
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
        qml/components/SensorDockChart.qml
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
        src/threading/ioworker.cpp
        src/threading/ioworker.h
        src/threading/iothread.cpp
        src/threading/iothread.h
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
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_BINARY_DIR}/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release
)

target_include_directories(appZephyrSense PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serial
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/threading
)

target_link_libraries(appZephyrSense
    PRIVATE project_warnings Qt6::Quick Qt6::QuickControls2 Qt6::SerialPort Qt6::Sql Qt6::Location Qt6::Positioning Qt6::Charts Qt6::Widgets KDAB::kddockwidgets
)

include(GNUInstallDirs)
install(TARGETS appZephyrSense
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
