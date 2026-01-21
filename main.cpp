#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>

#include "src/core/sensorreading.h"
#include "src/data/databasemanager.h"
#include "src/data/csvexporter.h"
#include "src/serial/serialhandler.h"

int main(int argc, char *argv[])
{
    // Set organization info before QGuiApplication (required for QML Settings)
    QCoreApplication::setOrganizationName("ZephyrSense");
    QCoreApplication::setOrganizationDomain("zephyrsense.local");
    QCoreApplication::setApplicationName("ZephyrSense");

    QGuiApplication app(argc, argv);

    // Apply Fusion style before loading QML
    QQuickStyle::setStyle("Fusion");

    // Register SensorReading for use in signal/slot and QML
    qRegisterMetaType<SensorReading>("SensorReading");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Connect signals after QML objects are created (singletons are now instantiated)
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [&engine](QObject *obj, const QUrl &url) {
            if (!obj) return;  // Object creation failed

            // Get singleton instances - now they should be instantiated
            auto *serialHandler = engine.singletonInstance<SerialHandler*>("ZephyrSense", "SerialHandler");
            auto *dbManager = engine.singletonInstance<DatabaseManager*>("ZephyrSense", "DatabaseManager");
            auto *csvExporter = engine.singletonInstance<CsvExporter*>("ZephyrSense", "CsvExporter");

            qDebug() << "Singletons - SerialHandler:" << serialHandler
                     << "DatabaseManager:" << dbManager
                     << "CsvExporter:" << csvExporter;

            // Connect SerialHandler::newReading to DatabaseManager::insertReading
            if (serialHandler && dbManager) {
                QObject::connect(serialHandler, &SerialHandler::newReading,
                                 dbManager, &DatabaseManager::insertReading);
                qDebug() << "Connected SerialHandler::newReading -> DatabaseManager::insertReading";
            }

            // Connect SerialHandler::newReading to CsvExporter::appendReading
            if (serialHandler && csvExporter) {
                QObject::connect(serialHandler, &SerialHandler::newReading,
                                 csvExporter, &CsvExporter::appendReading);
                qDebug() << "Connected SerialHandler::newReading -> CsvExporter::appendReading";
            }
        }, Qt::QueuedConnection);

    engine.loadFromModule("ZephyrSense", "Main");

    return app.exec();
}
