find_package(Qt6 REQUIRED COMPONENTS Test Core Gui Sql Qml)

# Macro to reduce boilerplate for each test executable
macro(add_zephyr_test name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/serial
        ${CMAKE_SOURCE_DIR}/src/data
        ${CMAKE_SOURCE_DIR}/src/models
        ${CMAKE_SOURCE_DIR}/src/threading
    )
    target_link_libraries(${name} PRIVATE Qt6::Test project_warnings ${ARG_LIBRARIES})
    add_test(NAME ${name} COMMAND ${name})
endmacro()

# ThresholdManager tests
add_zephyr_test(tst_thresholdmanager
    SOURCES
        tst_thresholdmanager.cpp
        ${CMAKE_SOURCE_DIR}/src/core/thresholdmanager.cpp
    LIBRARIES
        Qt6::Core Qt6::Gui Qt6::Qml
)

# SensorReading tests
add_zephyr_test(tst_sensorreading
    SOURCES
        tst_sensorreading.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::QmlIntegration
)

# Coordinate validation tests (standalone, no source dependency)
add_zephyr_test(tst_coordinatevalidation
    SOURCES
        tst_coordinatevalidation.cpp
    LIBRARIES
        Qt6::Core
)

# IOWorker tests
add_zephyr_test(tst_ioworker
    SOURCES
        tst_ioworker.cpp
        ${CMAKE_SOURCE_DIR}/src/threading/ioworker.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::Sql Qt6::QmlIntegration
)
