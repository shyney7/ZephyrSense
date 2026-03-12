#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QLoggingCategory>

#include "src/core/sensorreading.h"
#include "src/data/databasemanager.h"
#include "src/data/csvexporter.h"
#include "src/serial/serialhandler.h"
#include "src/threading/iothread.h"
#include "src/threading/ioworker.h"
#include "src/bridge/cesiumbridge.h"
#include "src/core/thresholdmanager.h"

#include <QtWebEngineQuick/qtwebenginequickglobal.h>
#include <QQuickWebEngineProfile>
#include <QWebEngineUrlScheme>
#include "src/bridge/zephyrschemehandler.h"
#include "src/bridge/appwebprofile.h"
#include <kddockwidgets/Config.h>
#include <kddockwidgets/qtquick/Platform.h>
#include <kddockwidgets/core/DockWidget.h>
#include <kddockwidgets/core/Group.h>

int main(int argc, char *argv[])
{
#ifdef QT_NO_DEBUG_OUTPUT
    // Suppress QML console.log()/console.debug() in Release builds
    // console.log maps to QtDebugMsg in "qml" and "js" logging categories
    QLoggingCategory::setFilterRules(QStringLiteral("qml.debug=false\njs.debug=false"));
#endif

    // Set organization info before QGuiApplication (required for QML Settings)
    QCoreApplication::setOrganizationName(QStringLiteral("ZephyrSense"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("zephyrsense.local"));
    QCoreApplication::setApplicationName(QStringLiteral("ZephyrSense"));

    ZephyrSchemeHandler::registerScheme();
    QtWebEngineQuick::initialize();

#ifdef Q_OS_WIN
    qputenv("QSG_RHI_BACKEND", "d3d11");
#endif

#ifdef QT_DEBUG
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9222");
#endif

    QApplication app(argc, argv);

    // Add icon to app
    app.setWindowIcon(QIcon(":/installer/appico.ico"));

    // Apply Fusion style before loading QML
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    // Initialize KDDockWidgets with QtQuick frontend
    KDDockWidgets::initFrontend(KDDockWidgets::FrontendType::QtQuick);

    auto flags = KDDockWidgets::Config::self().flags();
    flags |= KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;
    flags |= KDDockWidgets::Config::Flag_AlwaysShowTabs;
    KDDockWidgets::Config::self().setFlags(flags);

    // Make dragged windows semi-transparent so drop indicator arrows are visible
    KDDockWidgets::Config::self().setDraggedWindowOpacity(0.6);

    // Use transparent floating windows for better drop indicator interaction on QtQuick
    auto internalFlags = KDDockWidgets::Config::self().internalFlags();
    internalFlags |= KDDockWidgets::Config::InternalFlag_UseTransparentFloatingWindow;
    KDDockWidgets::Config::self().setInternalFlags(internalFlags);

    // Enforce tab ordering for MapView docks when re-docking via double-click
    // Clamp to group->dockWidgetCount() because KDDW passes the returned index
    // directly to QList::insert without bounds checking (Layout.cpp:168)
    KDDockWidgets::Config::self().setDockWidgetTabIndexOverrideFunc(
        [](KDDockWidgets::Core::DockWidget *dw,
           KDDockWidgets::Core::Group *group, int tabIndex) -> int {
            const auto name = dw->uniqueName();
            int desired = tabIndex;
            if (name == QStringLiteral("dock-2d-map"))
                desired = 0;
            else if (name == QStringLiteral("dock-3d-globe"))
                desired = 1;
            return std::min(desired, group->dockWidgetCount());
        });

    // Register SensorReading for use in signal/slot and QML
    qRegisterMetaType<SensorReading>("SensorReading");

    // Set up I/O worker thread for non-blocking database and CSV operations
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/zephyrsense.db";

    IOThread ioThread(dbPath);
    ioThread.start();

    // -- Persistent WebEngine profile for CesiumJS tile caching --
    // CRITICAL: This profile MUST be fully configured (cache + scheme handler + singleton
    // registration) BEFORE engine.loadFromModule(), because StackLayout instantiates all
    // views eagerly and WebEngineView begins loading its URL binding immediately.
    // The default profile is off-the-record in Qt 6 (no disk cache). A named profile
    // with a storageName enables disk-based HTTP cache and persistent storage.
    auto *webProfile = new QQuickWebEngineProfile(QStringLiteral("ZephyrSense"), &app);
    webProfile->setHttpCacheType(QQuickWebEngineProfile::DiskHttpCache);
    webProfile->setHttpCacheMaximumSize(512 * 1024 * 1024); // 512 MB for 3D tiles
    webProfile->setPersistentCookiesPolicy(QQuickWebEngineProfile::AllowPersistentCookies);

    const QString webCachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                 + QStringLiteral("/WebEngine");
    const QString webStoragePath = dataPath + QStringLiteral("/WebEngine");
    QDir().mkpath(webCachePath);
    QDir().mkpath(webStoragePath);
    webProfile->setCachePath(webCachePath);
    webProfile->setPersistentStoragePath(webStoragePath);

    // CRITICAL: The zephyr:// scheme handler MUST be on the same profile that the
    // WebEngineView uses. Handlers are per-profile — installing on defaultProfile()
    // while the view uses a named profile would break zephyr:// URL resolution.
#ifndef WEBDEV_MODE
    auto *schemeHandler = new ZephyrSchemeHandler(&app);
    webProfile->installUrlSchemeHandler(QByteArrayLiteral("zephyr"), schemeHandler);
#endif

    AppWebProfileForeign::setInstance(webProfile);

    QQmlApplicationEngine engine;

    // Set QML engine for KDDockWidgets
    KDDockWidgets::QtQuick::Platform::instance()->setQmlEngine(&engine);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Connect signals after QML objects are created (singletons are now instantiated)
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [&engine, &ioThread](QObject *obj, const QUrl &/*url*/) {
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

            // Wire CesiumBridge for 3D map view
            auto *bridge = engine.singletonInstance<CesiumBridge*>("ZephyrSense", "CesiumBridge");
            auto *thresholds = engine.singletonInstance<ThresholdManager*>("ZephyrSense", "ThresholdManager");

            if (bridge && serialHandler) {
                QObject::connect(serialHandler, &SerialHandler::newReading,
                                 bridge, &CesiumBridge::onNewReading,
                                 Qt::QueuedConnection);
                qDebug() << "Connected SerialHandler::newReading -> CesiumBridge::onNewReading";
            }
            if (bridge && thresholds) {
                bridge->setThresholdManager(thresholds);
                QObject::connect(thresholds, &ThresholdManager::thresholdsChanged,
                                 bridge, &CesiumBridge::onThresholdsChanged,
                                 Qt::QueuedConnection);
                qDebug() << "Connected ThresholdManager::thresholdsChanged -> CesiumBridge";
            }
        }, Qt::QueuedConnection);

    engine.loadFromModule("ZephyrSense", "Main");

    int result = app.exec();

    // Clean shutdown: stop I/O thread and flush pending writes
    ioThread.stop();

    return result;
}
