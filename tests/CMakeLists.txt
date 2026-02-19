find_package(Qt6 REQUIRED COMPONENTS Test Core Gui Sql Qml SerialPort QuickTest Quick QuickControls2 QuickDialogs2)

option(ENABLE_COVERAGE "Enable Clang source-based code coverage instrumentation" OFF)

# Macro to reduce boilerplate for each test executable
macro(add_zephyr_test name)
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs SOURCES LIBRARIES DEFINES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/serial
        ${CMAKE_SOURCE_DIR}/src/data
        ${CMAKE_SOURCE_DIR}/src/models
        ${CMAKE_SOURCE_DIR}/src/threading
        ${CMAKE_SOURCE_DIR}/tests
    )
    target_link_libraries(${name} PRIVATE Qt6::Test project_warnings ${ARG_LIBRARIES})

    if(ARG_DEFINES)
        target_compile_definitions(${name} PRIVATE ${ARG_DEFINES})
    endif()

    if(ENABLE_COVERAGE)
        target_compile_options(${name} PRIVATE
            -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc)
        target_link_options(${name} PRIVATE -fprofile-instr-generate)
    endif()

    add_test(NAME ${name} COMMAND ${name} -o ${CMAKE_BINARY_DIR}/test-results/${name}.xml,junitxml)

    # Also track test targets for coverage scripts
    set_property(GLOBAL APPEND PROPERTY ZEPHYR_TEST_TARGETS ${name})
endmacro()

# Ensure test-results directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test-results)

# ── Existing test suites ─────────────────────────────────────────────────────

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

# ── New test suites ──────────────────────────────────────────────────────────

# CsvExporter tests
add_zephyr_test(tst_csvexporter
    SOURCES
        tst_csvexporter.cpp
        ${CMAKE_SOURCE_DIR}/src/data/csvexporter.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::Qml Qt6::QmlIntegration
)

# SerialHandler tests
add_zephyr_test(tst_serialhandler
    SOURCES
        tst_serialhandler.cpp
        ${CMAKE_SOURCE_DIR}/src/serial/serialhandler.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::Sql Qt6::SerialPort Qt6::Qml Qt6::QmlIntegration
    DEFINES
        ZEPHYR_TESTING
)

# DatabaseManager tests
add_zephyr_test(tst_databasemanager
    SOURCES
        tst_databasemanager.cpp
        ${CMAKE_SOURCE_DIR}/src/data/databasemanager.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::Sql Qt6::Qml Qt6::QmlIntegration
)

# SensorReadingModel tests
add_zephyr_test(tst_sensorreadingmodel
    SOURCES
        tst_sensorreadingmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/models/sensorreadingmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
        ${CMAKE_SOURCE_DIR}/src/core/thresholdmanager.cpp
        ${CMAKE_SOURCE_DIR}/src/data/databasemanager.cpp
        ${CMAKE_SOURCE_DIR}/src/serial/serialhandler.cpp
    LIBRARIES
        Qt6::Core Qt6::Gui Qt6::Sql Qt6::Qml Qt6::QmlIntegration Qt6::SerialPort
    DEFINES
        ZEPHYR_TESTING
)

# TimeSeriesChartModel tests
add_zephyr_test(tst_timeserieschartmodel
    SOURCES
        tst_timeserieschartmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/models/timeserieschartmodel.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
        ${CMAKE_SOURCE_DIR}/src/data/databasemanager.cpp
    LIBRARIES
        Qt6::Core Qt6::Gui Qt6::Sql Qt6::Qml Qt6::QmlIntegration
    DEFINES
        ZEPHYR_TESTING
)

# IOThread tests
add_zephyr_test(tst_iothread
    SOURCES
        tst_iothread.cpp
        ${CMAKE_SOURCE_DIR}/src/threading/iothread.cpp
        ${CMAKE_SOURCE_DIR}/src/threading/ioworker.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
    LIBRARIES
        Qt6::Core Qt6::Sql Qt6::QmlIntegration
)

# ── QML Test Suite ──────────────────────────────────────────────────────────
qt_add_executable(tst_qml qml/setup/tst_qml_setup.cpp)

qt_add_qml_module(tst_qml
    URI ZephyrSense
    VERSION 1.0
    OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports/ZephyrSense"
    NO_PLUGIN
    SOURCES
        qml/setup/mocks.h
        qml/setup/mocks.cpp
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.h
        ${CMAKE_SOURCE_DIR}/src/core/sensorreading.cpp
)

target_compile_definitions(tst_qml PRIVATE
    QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/qml"
    QML_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)
target_include_directories(tst_qml PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/setup
)
target_link_libraries(tst_qml PRIVATE
    Qt6::QuickTest Qt6::Quick Qt6::QuickControls2 Qt6::Qml
    Qt6::QmlIntegration Qt6::QuickDialogs2
    project_warnings
)

if(ENABLE_COVERAGE)
    target_compile_options(tst_qml PRIVATE
        -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc)
    target_link_options(tst_qml PRIVATE -fprofile-instr-generate)
endif()
add_test(NAME tst_qml COMMAND tst_qml
    -o ${CMAKE_BINARY_DIR}/test-results/tst_qml.xml,junitxml)
set_property(GLOBAL APPEND PROPERTY ZEPHYR_TEST_TARGETS tst_qml)
