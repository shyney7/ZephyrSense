#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "src/core/sensorreading.h"
#include "src/data/databasemanager.h"
#include "src/data/csvexporter.h"
#include "src/serial/serialhandler.h"
#include "src/threading/iothread.h"
#include "src/threading/ioworker.h"

int main(int argc, char *argv[])
{
    // Set organization info before QGuiApplication (required for QML Settings)
    QCoreApplication::setOrganizationName("ZephyrSense");
    QCoreApplication::setOrganizationDomain("zephyrsense.local");
    QCoreApplication::setApplicationName("ZephyrSense");

    QApplication app(argc, argv);

    // Add icon to app
    app.setWindowIcon(QIcon(":/installer/appico.ico"));

    // Apply Fusion style before loading QML
    QQuickStyle::setStyle("Fusion");

    // Register SensorReading for use in signal/slot and QML
    qRegisterMetaType<SensorReading>("SensorReading");

    // Set up I/O worker thread for non-blocking database and CSV operations
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/zephyrsense.db";

    IOThread ioThread(dbPath);
    ioThread.start();

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Connect signals after QML objects are created (singletons are now instantiated)
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [&engine, &ioThread](QObject *obj, const QUrl &url) {
            if (!obj) return;  // Object creation failed

            // Get singleton instances - now they should be instantiated
            auto *serialHandler = engine.singletonInstance<SerialHandler*>("ZephyrSense", "SerialHandler");
            auto *dbManager = engine.singletonInstance<DatabaseManager*>("ZephyrSense", "DatabaseManager");
            auto *csvExporter = engine.singletonInstance<CsvExporter*>("ZephyrSense", "CsvExporter");

            qDebug() << "Singletons - SerialHandler:" << serialHandler
                     << "DatabaseManager:" << dbManager
                     << "CsvExporter:" << csvExporter;

            IOWorker *worker = ioThread.worker();

            // Connect SerialHandler::newReading to IOWorker::processReading (Qt::QueuedConnection)
            // This moves database and CSV I/O to a dedicated thread, keeping UI responsive
            if (serialHandler && worker) {
                QObject::connect(serialHandler, &SerialHandler::newReading,
                                 worker, &IOWorker::processReading,
                                 Qt::QueuedConnection);
                qDebug() << "Connected SerialHandler::newReading -> IOWorker::processReading (QueuedConnection)";
            }

            // Connect IOWorker error signals to DatabaseManager/CsvExporter for UI error reporting
            if (worker && dbManager) {
                QObject::connect(worker, &IOWorker::databaseError,
                                 dbManager, &DatabaseManager::databaseError,
                                 Qt::QueuedConnection);
            }
            if (worker && csvExporter) {
                QObject::connect(worker, &IOWorker::csvError,
                                 csvExporter, &CsvExporter::exportError,
                                 Qt::QueuedConnection);
            }

            // Relay CSV configuration changes from CsvExporter (QML) to IOWorker (worker thread)
            if (csvExporter && worker) {
                // Initial state sync
                QMetaObject::invokeMethod(worker, "setCsvEnabled",
                                          Qt::QueuedConnection,
                                          Q_ARG(bool, csvExporter->isEnabled()));
                QMetaObject::invokeMethod(worker, "setCsvFilePath",
                                          Qt::QueuedConnection,
                                          Q_ARG(QString, csvExporter->filePath()));

                // Relay changes
                QObject::connect(csvExporter, &CsvExporter::enabledChanged, worker,
                    [worker, csvExporter]() {
                        QMetaObject::invokeMethod(worker, "setCsvEnabled",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(bool, csvExporter->isEnabled()));
                    });
                QObject::connect(csvExporter, &CsvExporter::filePathChanged, worker,
                    [worker, csvExporter]() {
                        QMetaObject::invokeMethod(worker, "setCsvFilePath",
                                                  Qt::QueuedConnection,
                                                  Q_ARG(QString, csvExporter->filePath()));
                    });
                qDebug() << "Connected CsvExporter config changes -> IOWorker";
            }
        }, Qt::QueuedConnection);

    engine.loadFromModule("ZephyrSense", "Main");

    int result = app.exec();

    // Clean shutdown: stop I/O thread and flush pending writes
    ioThread.stop();

    return result;
}
