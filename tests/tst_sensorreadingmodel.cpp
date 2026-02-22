#include <QTest>
#include <QSignalSpy>
#include <QSettings>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "sensorreadingmodel.h"
#include "thresholdmanager.h"
#include "databasemanager.h"
#include "serialhandler.h"
#include "sensorreading.h"
#include "testhelpers.h"

class TestSensorReadingModel : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        QStandardPaths::setTestModeEnabled(true);

        // Clear QSettings before constructing ThresholdManager to avoid host registry bleed
        QSettings settings(QStringLiteral("thresholds"));
        settings.clear();
        settings.sync();

        m_thresholdManager = std::make_unique<ThresholdManager>();
    }

    void cleanup()
    {
        m_thresholdManager.reset();

        if (QSqlDatabase::contains(DatabaseManager::CONNECTION_NAME)) {
            QSqlDatabase::database(DatabaseManager::CONNECTION_NAME).close();
            QSqlDatabase::removeDatabase(DatabaseManager::CONNECTION_NAME);
        }
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QFile::remove(dataPath + QStringLiteral("/zephyrsense.db"));

        QStandardPaths::setTestModeEnabled(false);
    }

    void initialState_empty()
    {
        SensorReadingModel model;
        QCOMPARE(model.rowCount(), 0);
        QCOMPARE(model.count(), 0);
    }

    void addReading_validCoords()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .build();
        model.addReading(reading);
        QCOMPARE(model.rowCount(), 1);
    }

    void addReading_nullIsland_rejected()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(0.0f, 0.0f)
            .build();
        model.addReading(reading);
        QCOMPARE(model.rowCount(), 0);
    }

    void addReading_outOfRange_rejected()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(91.0f, 0.0f)
            .build();
        model.addReading(reading);
        QCOMPARE(model.rowCount(), 0);
    }

    void addReading_emitsCountChanged()
    {
        SensorReadingModel model;
        QSignalSpy spy(&model, &SensorReadingModel::countChanged);
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .build();
        model.addReading(reading);
        QCOMPARE(spy.count(), 1);
    }

    void addReading_invalidCoords_noSignal()
    {
        SensorReadingModel model;
        QSignalSpy spy(&model, &SensorReadingModel::countChanged);
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(0.0f, 0.0f)
            .build();
        model.addReading(reading);
        QCOMPARE(spy.count(), 0);
    }

    void data_allRoles()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withAllSensors()
            .build();
        model.addReading(reading);

        QModelIndex idx = model.index(0);

        QCOMPARE(model.data(idx, SensorReadingModel::LatitudeRole).toFloat(), reading.latitude);
        QCOMPARE(model.data(idx, SensorReadingModel::LongitudeRole).toFloat(), reading.longitude);
        QCOMPARE(model.data(idx, SensorReadingModel::PartectorNumberRole).toInt(), reading.partectorNumber);
        QCOMPARE(model.data(idx, SensorReadingModel::PartectorDiamRole).toInt(), reading.partectorDiam);
        QCOMPARE(model.data(idx, SensorReadingModel::PartectorMassRole).toFloat(), reading.partectorMass);
        QCOMPARE(model.data(idx, SensorReadingModel::GrimmValueRole).toFloat(), reading.grimmValue);
        QCOMPARE(model.data(idx, SensorReadingModel::TemperatureRole).toFloat(), reading.temperature);
        QCOMPARE(model.data(idx, SensorReadingModel::HumidityRole).toFloat(), reading.humidity);
        QCOMPARE(model.data(idx, SensorReadingModel::PressureRole).toFloat(), reading.pressure);
        QCOMPARE(model.data(idx, SensorReadingModel::AltitudeRole).toFloat(), reading.altitude);
        QCOMPARE(model.data(idx, SensorReadingModel::Co2Role).toInt(), reading.co2);
        QVERIFY(model.data(idx, SensorReadingModel::TimestampRole).toDateTime().isValid());
        QVERIFY(!model.data(idx, SensorReadingModel::TooltipTextRole).toString().isEmpty());
        QVERIFY(model.data(idx, SensorReadingModel::IdRole).toLongLong() > 0);
        // HazardLevel should be a valid int
        QVERIFY(model.data(idx, SensorReadingModel::HazardLevelRole).toInt() >= 0);
    }

    void data_invalidIndex()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder().withCoordinates(48.1f, 11.5f).build();
        model.addReading(reading);

        // Row -1
        QVERIFY(!model.data(model.index(-1), SensorReadingModel::LatitudeRole).isValid());
        // Row >= count
        QVERIFY(!model.data(model.index(1), SensorReadingModel::LatitudeRole).isValid());
    }

    void data_parentValid_returnsZero()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder().withCoordinates(48.1f, 11.5f).build();
        model.addReading(reading);

        // With a valid parent, rowCount should return 0 (flat list)
        QModelIndex parent = model.index(0);
        QCOMPARE(model.rowCount(parent), 0);
    }

    void data_unknownRole_returnsInvalid()
    {
        SensorReadingModel model;
        model.addReading(SensorReadingBuilder().withCoordinates(48.1f, 11.5f).build());

        // Use an arbitrary role number that doesn't match any defined role
        QVariant result = model.data(model.index(0), Qt::UserRole + 999);
        QVERIFY(!result.isValid());
    }

    void roleNames_allPresent()
    {
        SensorReadingModel model;
        QHash<int, QByteArray> roles = model.roleNames();
        QVERIFY(roles.contains(SensorReadingModel::IdRole));
        QVERIFY(roles.contains(SensorReadingModel::LatitudeRole));
        QVERIFY(roles.contains(SensorReadingModel::LongitudeRole));
        QVERIFY(roles.contains(SensorReadingModel::PartectorNumberRole));
        QVERIFY(roles.contains(SensorReadingModel::PartectorDiamRole));
        QVERIFY(roles.contains(SensorReadingModel::PartectorMassRole));
        QVERIFY(roles.contains(SensorReadingModel::GrimmValueRole));
        QVERIFY(roles.contains(SensorReadingModel::TemperatureRole));
        QVERIFY(roles.contains(SensorReadingModel::HumidityRole));
        QVERIFY(roles.contains(SensorReadingModel::PressureRole));
        QVERIFY(roles.contains(SensorReadingModel::AltitudeRole));
        QVERIFY(roles.contains(SensorReadingModel::Co2Role));
        QVERIFY(roles.contains(SensorReadingModel::TimestampRole));
        QVERIFY(roles.contains(SensorReadingModel::TooltipTextRole));
        QVERIFY(roles.contains(SensorReadingModel::HazardLevelRole));
        QCOMPARE(roles.size(), 15);
    }

    void clear_removesAll()
    {
        SensorReadingModel model;
        for (int i = 0; i < 3; ++i) {
            model.addReading(SensorReadingBuilder().withCoordinates(48.0f + static_cast<float>(i), 11.0f).build());
        }
        QCOMPARE(model.count(), 3);
        model.clear();
        QCOMPARE(model.count(), 0);
    }

    void clear_emitsCountChanged()
    {
        SensorReadingModel model;
        model.addReading(SensorReadingBuilder().withCoordinates(48.1f, 11.5f).build());
        QSignalSpy spy(&model, &SensorReadingModel::countChanged);
        model.clear();
        QCOMPARE(spy.count(), 1);
    }

    void readingAt_valid()
    {
        SensorReadingModel model;
        SensorReading original = SensorReadingBuilder()
            .withAllSensors()
            .build();
        model.addReading(original);

        SensorReading retrieved = model.readingAt(0);
        QCOMPARE(retrieved.partectorNumber, original.partectorNumber);
        QCOMPARE(retrieved.co2, original.co2);
    }

    void readingAt_outOfBounds()
    {
        SensorReadingModel model;
        SensorReading reading = model.readingAt(-1);
        QCOMPARE(reading.partectorNumber, 0);  // Default SensorReading
    }

    void getReading_valid()
    {
        SensorReadingModel model;
        model.addReading(SensorReadingBuilder().withAllSensors().build());

        QVariantMap map = model.getReading(0);
        QVERIFY(!map.isEmpty());
        QVERIFY(map.contains(QStringLiteral("readingId")));
        QVERIFY(map.contains(QStringLiteral("latitude")));
        QVERIFY(map.contains(QStringLiteral("temperature")));
        QVERIFY(map.contains(QStringLiteral("co2")));
    }

    void getReading_outOfBounds()
    {
        SensorReadingModel model;
        QVariantMap map = model.getReading(0);
        QVERIFY(map.isEmpty());
    }

    void pruneOldReadings_removesOld()
    {
        SensorReadingModel model;
        QDateTime now = QDateTime::currentDateTime();

        // 1 old reading (10 min ago) + 2 new readings (within last minute)
        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .withTimestamp(now.addSecs(-600))
            .build());
        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.2f, 11.5f)
            .withTimestamp(now.addSecs(-30))
            .build());
        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.3f, 11.5f)
            .withTimestamp(now.addSecs(-10))
            .build());

        QCOMPARE(model.count(), 3);
        model.pruneOldReadings(5);  // Keep last 5 minutes
        QCOMPARE(model.count(), 2);
    }

    void pruneOldReadings_emptyModel()
    {
        SensorReadingModel model;
        // Should not crash
        model.pruneOldReadings(5);
        QCOMPARE(model.count(), 0);
    }

    void pruneOldReadings_allOld()
    {
        SensorReadingModel model;
        QDateTime now = QDateTime::currentDateTime();

        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .withTimestamp(now.addSecs(-600))
            .build());
        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.2f, 11.5f)
            .withTimestamp(now.addSecs(-500))
            .build());

        model.pruneOldReadings(1);  // Keep last 1 minute
        QCOMPARE(model.count(), 0);
    }

    void pruneOldReadings_noneOld()
    {
        SensorReadingModel model;
        QDateTime now = QDateTime::currentDateTime();

        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .withTimestamp(now.addSecs(-10))
            .build());
        model.addReading(SensorReadingBuilder()
            .withCoordinates(48.2f, 11.5f)
            .withTimestamp(now.addSecs(-5))
            .build());

        model.pruneOldReadings(5);  // Keep last 5 minutes
        QCOMPARE(model.count(), 2);
    }

    void hazardLevel_withThresholdManager()
    {
        SensorReadingModel model;

        // Add reading with high CO2 that should trigger warning/danger
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .withCo2(2000)  // High CO2
            .build();
        model.addReading(reading);

        QModelIndex idx = model.index(0);
        int hazard = model.data(idx, SensorReadingModel::HazardLevelRole).toInt();
        // With ThresholdManager active and high CO2, should be > 0
        QVERIFY(hazard > 0);
    }

    void data_hazardLevel_normalReading()
    {
        // With ThresholdManager active and normal sensor values, hazard should be 0
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withCoordinates(48.1f, 11.5f)
            .withCo2(400)  // Normal CO2
            .withTemperature(22.0f)
            .build();
        model.addReading(reading);

        QModelIndex idx = model.index(0);
        int hazard = model.data(idx, SensorReadingModel::HazardLevelRole).toInt();
        QCOMPARE(hazard, 0);  // Normal readings = Green
    }

    void onThresholdsChanged_emitsDataChanged()
    {
        SensorReadingModel model;
        model.addReading(SensorReadingBuilder().withCoordinates(48.1f, 11.5f).build());

        // Manually invoke the private slot via QMetaObject
        QSignalSpy spy(&model, &SensorReadingModel::dataChanged);
        QMetaObject::invokeMethod(&model, "onThresholdsChanged");
        QCOMPARE(spy.count(), 1);
    }

    void onThresholdsChanged_emptyModel_noSignal()
    {
        SensorReadingModel model;
        QSignalSpy spy(&model, &SensorReadingModel::dataChanged);
        QMetaObject::invokeMethod(&model, "onThresholdsChanged");
        QCOMPARE(spy.count(), 0);
    }

    void idAutoIncrement()
    {
        SensorReadingModel model;
        for (int i = 0; i < 3; ++i) {
            model.addReading(SensorReadingBuilder()
                .withCoordinates(48.0f + static_cast<float>(i), 11.0f)
                .build());
        }

        QCOMPARE(model.data(model.index(0), SensorReadingModel::IdRole).toLongLong(), 1);
        QCOMPARE(model.data(model.index(1), SensorReadingModel::IdRole).toLongLong(), 2);
        QCOMPARE(model.data(model.index(2), SensorReadingModel::IdRole).toLongLong(), 3);
    }

    void formatTooltip_containsFields()
    {
        SensorReadingModel model;
        SensorReading reading = SensorReadingBuilder()
            .withAllSensors()
            .withCo2(500)
            .withTemperature(22.5f)
            .build();
        model.addReading(reading);

        QString tooltip = model.data(model.index(0), SensorReadingModel::TooltipTextRole).toString();
        QVERIFY(tooltip.contains(QStringLiteral("22.5")));
        QVERIFY(tooltip.contains(QStringLiteral("500")));
        QVERIFY(tooltip.contains(QStringLiteral("48.0999")));  // latitude from withAllSensors (48.1f → "48.099998")
    }

    // ── QQmlEngine integration tests ──

    void loadFromDatabase_loadsAndFilters()
    {
        DatabaseManager dbm;
        QVERIFY(dbm.initialize());

        QDateTime now = QDateTime::currentDateTime();

        // Insert readings: 2 with valid coords, 1 with null island (0,0)
        auto insertReading = [](const SensorReading &reading) {
            QSqlDatabase db = QSqlDatabase::database(DatabaseManager::CONNECTION_NAME);
            QSqlQuery query(db);
            query.prepare(QStringLiteral(R"(
                INSERT INTO readings (timestamp, partectorNumber, partectorDiam, partectorMass,
                                      grimmValue, temperature, humidity, pressure,
                                      altitude, latitude, longitude, co2)
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            )"));
            query.addBindValue(reading.timestamp.toMSecsSinceEpoch());
            query.addBindValue(reading.partectorNumber);
            query.addBindValue(reading.partectorDiam);
            query.addBindValue(static_cast<double>(reading.partectorMass));
            query.addBindValue(static_cast<double>(reading.grimmValue));
            query.addBindValue(static_cast<double>(reading.temperature));
            query.addBindValue(static_cast<double>(reading.humidity));
            query.addBindValue(static_cast<double>(reading.pressure));
            query.addBindValue(static_cast<double>(reading.altitude));
            query.addBindValue(static_cast<double>(reading.latitude));
            query.addBindValue(static_cast<double>(reading.longitude));
            query.addBindValue(reading.co2);
            return query.exec();
        };

        QVERIFY(insertReading(SensorReadingBuilder().withAllSensors().withTimestamp(now.addSecs(-60)).build()));
        QVERIFY(insertReading(SensorReadingBuilder().withAllSensors().withTimestamp(now.addSecs(-30)).build()));
        QVERIFY(insertReading(SensorReadingBuilder().withCoordinates(0.0f, 0.0f).withTimestamp(now.addSecs(-10)).build()));

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        model.loadFromDatabase(now.addSecs(-120), now);

        // Should only have 2 readings (null island filtered out)
        QCOMPARE(model.count(), 2);
    }

    void startLiveUpdates_connectsSerial()
    {
        DatabaseManager dbm;
        dbm.initialize();

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        model.startLiveUpdates();

        // Inject a reading via SerialHandler — model should receive it
        QByteArray frame = FrameBuilder().withDefaults().buildFrame();
        serialHandler.injectTestData(frame);

        QCOMPARE(model.count(), 1);
    }

    void startLiveUpdates_idempotent()
    {
        DatabaseManager dbm;
        dbm.initialize();

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        model.startLiveUpdates();
        model.startLiveUpdates();  // Second call — should be idempotent

        // Inject one frame — should only get 1 reading (not 2 from duplicate connections)
        QByteArray frame = FrameBuilder().withDefaults().buildFrame();
        serialHandler.injectTestData(frame);

        QCOMPARE(model.count(), 1);
    }

    void stopLiveUpdates_disconnects()
    {
        DatabaseManager dbm;
        dbm.initialize();

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        model.startLiveUpdates();
        model.stopLiveUpdates();

        // Inject a reading — model should NOT receive it
        QByteArray frame = FrameBuilder().withDefaults().buildFrame();
        serialHandler.injectTestData(frame);

        QCOMPARE(model.count(), 0);
    }

    void stopLiveUpdates_whenNotStarted_noop()
    {
        DatabaseManager dbm;
        dbm.initialize();

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        // Should not crash — early return when m_liveUpdatesConnected is false
        model.stopLiveUpdates();
    }

    void loadFromDatabase_noDbManagerSingleton()
    {
        QQmlEngine engine;
        // Register engine context but do NOT register DatabaseManager singleton
        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        QDateTime now = QDateTime::currentDateTime();
        model.loadFromDatabase(now.addSecs(-60), now);
        // Should warn but not crash, model stays empty
        QCOMPARE(model.count(), 0);
    }

    void loadFromDatabase_twiceTriggers_connectToThresholdManager_idempotent()
    {
        DatabaseManager dbm;
        QVERIFY(dbm.initialize());

        SerialHandler serialHandler;
        QQmlEngine engine;
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "DatabaseManager", &dbm);
        qmlRegisterSingletonInstance("ZephyrSense", 1, 0, "SerialHandler", &serialHandler);

        SensorReadingModel model;
        engine.setObjectOwnership(&model, QQmlEngine::CppOwnership);
        QQmlEngine::setContextForObject(&model, engine.rootContext());

        QDateTime now = QDateTime::currentDateTime();
        model.loadFromDatabase(now.addSecs(-60), now);
        // Second call hits m_thresholdManagerConnected early return
        model.loadFromDatabase(now.addSecs(-60), now);
        QCOMPARE(model.count(), 0);
    }

private:
    std::unique_ptr<ThresholdManager> m_thresholdManager;
};

QTEST_GUILESS_MAIN(TestSensorReadingModel)
#include "tst_sensorreadingmodel.moc"
