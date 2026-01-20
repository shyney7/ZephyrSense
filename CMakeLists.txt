cmake_minimum_required(VERSION 3.16)

project(ZephyrSense VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick SerialPort)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appZephyrSense
    main.cpp
    src/core/sensorreading.cpp
    src/core/sensorreading.h
    src/serial/serialhandler.cpp
    src/serial/serialhandler.h
)

qt_add_qml_module(appZephyrSense
    URI ZephyrSense
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        src/core/sensorreading.cpp
        src/core/sensorreading.h
        src/serial/serialhandler.cpp
        src/serial/serialhandler.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appZephyrSense PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appZephyrSense
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appZephyrSense
    PRIVATE Qt6::Quick Qt6::SerialPort
)

include(GNUInstallDirs)
install(TARGETS appZephyrSense
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
