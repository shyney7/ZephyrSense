#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "databasemanager.h"
#include "sensorreading.h"
#include "testhelpers.h"

class TestDatabaseManager : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        QStandardPaths::setTestModeEnabled(true);
        m_tmpDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_tmpDir->isValid());
    }

    void cleanup()
    {
        // Remove named connection if it exists
        if (QSqlDatabase::contains(DatabaseManager::CONNECTION_NAME)) {
            QSqlDatabase::database(DatabaseManager::CONNECTION_NAME).close();
            QSqlDatabase::removeDatabase(DatabaseManager::CONNECTION_NAME);
        }
        // Delete the test database file so the next test starts clean
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QFile::remove(dataPath + QStringLiteral("/zephyrsense.db"));
        QFile::remove(dataPath + QStringLiteral("/zephyrsense.db.backup"));
        // Also clean up any import source connections
        if (QSqlDatabase::contains(QStringLiteral("importSrc"))) {
            QSqlDatabase::removeDatabase(QStringLiteral("importSrc"));
        }
        m_tmpDir.reset();
        QStandardPaths::setTestModeEnabled(false);
    }

    void initialize_createsDatabase()
    {
        DatabaseManager dbm;
        QVERIFY(dbm.initialize());
        QVERIFY(QFile::exists(dbm.databasePath()));
        QVERIFY(QSqlDatabase::contains(DatabaseManager::CONNECTION_NAME));
    }

    void initialize_idempotent()
    {
        DatabaseManager dbm;
        QVERIFY(dbm.initialize());
        QVERIFY(dbm.initialize());  // Second call should also succeed
    }

    void getReadingsInRange_empty()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QDateTime start = QDateTime::currentDateTime().addSecs(-3600);
        QDateTime end = QDateTime::currentDateTime();
        QVariantList results = dbm.getReadingsInRange(start, end);
        QCOMPARE(results.count(), 0);
    }

    void getReadingsInRange_findsInWindow()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QDateTime now = QDateTime::currentDateTime();
        seedReadings(dbm, {
            now.addSecs(-120),  // 2 min ago (in window)
            now.addSecs(-60),   // 1 min ago (in window)
            now.addSecs(-300),  // 5 min ago (outside window)
        });

        QVariantList results = dbm.getReadingsInRange(now.addSecs(-180), now);
        QCOMPARE(results.count(), 2);
    }

    void getReadingsInRange_excludesOutside()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QDateTime now = QDateTime::currentDateTime();
        seedReadings(dbm, {now.addSecs(-3600)});

        QVariantList results = dbm.getReadingsInRange(now.addSecs(-60), now);
        QCOMPARE(results.count(), 0);
    }

    void getReadingsInRange_dbNotOpen_emitsError()
    {
        DatabaseManager dbm;
        // Don't initialize - no DB connection
        QSignalSpy spy(&dbm, &DatabaseManager::databaseError);
        QVariantList results = dbm.getReadingsInRange(
            QDateTime::currentDateTime().addSecs(-60),
            QDateTime::currentDateTime()
        );
        QCOMPARE(results.count(), 0);
        QCOMPARE(spy.count(), 1);
    }

    void getReadingById_found()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QDateTime now = QDateTime::currentDateTime();
        SensorReading reading = SensorReadingBuilder()
            .withAllSensors()
            .withTimestamp(now)
            .build();
        insertDirectly(reading);

        QVariantMap result = dbm.getReadingById(1);
        QVERIFY(!result.isEmpty());
        QCOMPARE(result[QStringLiteral("id")].toInt(), 1);
        QCOMPARE(result[QStringLiteral("partectorNumber")].toInt(), reading.partectorNumber);
        QCOMPARE(result[QStringLiteral("co2")].toInt(), reading.co2);
        QVERIFY(qAbs(result[QStringLiteral("temperature")].toDouble() - reading.temperature) < 0.01);
    }

    void getReadingById_notFound()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QVariantMap result = dbm.getReadingById(999);
        QVERIFY(result.isEmpty());
    }

    void getReadingById_dbNotOpen()
    {
        DatabaseManager dbm;
        QVariantMap result = dbm.getReadingById(1);
        QVERIFY(result.isEmpty());
    }

    void exportDatabase_success()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QDateTime now = QDateTime::currentDateTime();
        insertDirectly(SensorReadingBuilder().withAllSensors().withTimestamp(now).build());

        QString exportPath = m_tmpDir->filePath(QStringLiteral("exported.db"));
        QSignalSpy spy(&dbm, &DatabaseManager::exportCompleted);
        bool ok = dbm.exportDatabase(QUrl::fromLocalFile(exportPath));
        QVERIFY(ok);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.constFirst().at(0).toBool());
        QVERIFY(QFile::exists(exportPath));
    }

    void exportDatabase_invalidUrl()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QSignalSpy errorSpy(&dbm, &DatabaseManager::databaseError);
        QSignalSpy completedSpy(&dbm, &DatabaseManager::exportCompleted);
        bool ok = dbm.exportDatabase(QUrl());
        QVERIFY(!ok);
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(completedSpy.count(), 1);
        QVERIFY(!completedSpy.constFirst().at(0).toBool());
    }

    void exportDatabase_reopensDb()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QString exportPath = m_tmpDir->filePath(QStringLiteral("exported.db"));
        dbm.exportDatabase(QUrl::fromLocalFile(exportPath));

        // DB should be usable after export
        QVariantList dates = dbm.getAvailableDates();
        // No crash = pass; dates will be empty which is fine
        Q_UNUSED(dates)
    }

    void importDatabase_success()
    {
        // Create a source DB with data
        QString srcPath = m_tmpDir->filePath(QStringLiteral("source.db"));
        {
            QSqlDatabase srcDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("importSrc"));
            srcDb.setDatabaseName(srcPath);
            QVERIFY(srcDb.open());
            QSqlQuery q(srcDb);
            q.exec(QStringLiteral(R"(
                CREATE TABLE readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    partectorNumber INTEGER, partectorDiam INTEGER,
                    partectorMass REAL, grimmValue REAL,
                    temperature REAL, humidity REAL, pressure REAL,
                    altitude REAL, latitude REAL, longitude REAL, co2 INTEGER
                )
            )"));
            q.exec(QStringLiteral("INSERT INTO readings (timestamp, partectorNumber) VALUES (1000, 42)"));
            srcDb.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("importSrc"));

        DatabaseManager dbm;
        dbm.initialize();

        QSignalSpy spy(&dbm, &DatabaseManager::importCompleted);
        bool ok = dbm.importDatabase(QUrl::fromLocalFile(srcPath));
        QVERIFY(ok);
        QCOMPARE(spy.count(), 1);
        QVERIFY(spy.constFirst().at(0).toBool());
    }

    void importDatabase_fileNotFound()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QSignalSpy errorSpy(&dbm, &DatabaseManager::databaseError);
        bool ok = dbm.importDatabase(QUrl::fromLocalFile(QStringLiteral("/nonexistent/file.db")));
        QVERIFY(!ok);
        QCOMPARE(errorSpy.count(), 1);
    }

    void importDatabase_invalidUrl()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QSignalSpy errorSpy(&dbm, &DatabaseManager::databaseError);
        bool ok = dbm.importDatabase(QUrl());
        QVERIFY(!ok);
        QCOMPARE(errorSpy.count(), 1);
    }

    void getAvailableDates_empty()
    {
        DatabaseManager dbm;
        dbm.initialize();

        QVariantList dates = dbm.getAvailableDates();
        QCOMPARE(dates.count(), 0);
    }

    void getAvailableDates_returnsDates()
    {
        DatabaseManager dbm;
        dbm.initialize();

        // Insert readings on two different days
        QDateTime day1(QDate(2026, 1, 15), QTime(10, 0));
        QDateTime day2(QDate(2026, 1, 16), QTime(14, 0));
        QDateTime day2b(QDate(2026, 1, 16), QTime(16, 0));
        insertDirectly(SensorReadingBuilder().withAllSensors().withTimestamp(day1).build());
        insertDirectly(SensorReadingBuilder().withAllSensors().withTimestamp(day2).build());
        insertDirectly(SensorReadingBuilder().withAllSensors().withTimestamp(day2b).build());

        QVariantList dates = dbm.getAvailableDates();
        QCOMPARE(dates.count(), 2);
    }

    void insertReading_deprecated_noop()
    {
        DatabaseManager dbm;
        dbm.initialize();

        SensorReading reading;
        reading.partectorNumber = 42;
        // Should log critical but not actually insert
        dbm.insertReading(reading);

        // Verify no row was inserted
        QSqlDatabase db = QSqlDatabase::database(DatabaseManager::CONNECTION_NAME);
        QSqlQuery query(db);
        QVERIFY(query.exec(QStringLiteral("SELECT COUNT(*) FROM readings")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 0);
    }

    // ── New error branch tests ──

    void getAvailableDates_dbNotOpen()
    {
        DatabaseManager dbm;
        // Don't initialize — no DB connection
        QVariantList dates = dbm.getAvailableDates();
        QCOMPARE(dates.count(), 0);
    }

    void exportDatabase_copyFails()
    {
        DatabaseManager dbm;
        dbm.initialize();

        // Export to a path that can't be written (nonexistent directory)
        QSignalSpy errorSpy(&dbm, &DatabaseManager::databaseError);
        QSignalSpy completedSpy(&dbm, &DatabaseManager::exportCompleted);
        bool ok = dbm.exportDatabase(QUrl::fromLocalFile(QStringLiteral("Z:/nonexistent/dir/exported.db")));
        QVERIFY(!ok);
        QCOMPARE(errorSpy.count(), 1);
        QCOMPARE(completedSpy.count(), 1);
        QVERIFY(!completedSpy.constFirst().at(0).toBool());
    }

    void getReadingsInRange_dbClosedMidway()
    {
        DatabaseManager dbm;
        dbm.initialize();

        // Remove the named connection entirely so isOpen() returns false
        QSqlDatabase::removeDatabase(DatabaseManager::CONNECTION_NAME);

        QSignalSpy spy(&dbm, &DatabaseManager::databaseError);
        QVariantList results = dbm.getReadingsInRange(
            QDateTime::currentDateTime().addSecs(-60),
            QDateTime::currentDateTime()
        );
        QCOMPARE(results.count(), 0);
        QCOMPARE(spy.count(), 1);
    }

    void getReadingById_dbClosedMidway()
    {
        DatabaseManager dbm;
        dbm.initialize();

        // Remove the named connection entirely so isOpen() returns false
        QSqlDatabase::removeDatabase(DatabaseManager::CONNECTION_NAME);

        QVariantMap result = dbm.getReadingById(1);
        QVERIFY(result.isEmpty());
    }

    void importDatabase_copyFails_restoresBackup()
    {
        DatabaseManager dbm;
        dbm.initialize();

        // Insert a reading so the DB file has content
        insertDirectly(SensorReadingBuilder().withAllSensors().build());

        // Create a valid source DB
        QString srcPath = m_tmpDir->filePath(QStringLiteral("source.db"));
        {
            QSqlDatabase srcDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("importSrc"));
            srcDb.setDatabaseName(srcPath);
            QVERIFY(srcDb.open());
            QSqlQuery q(srcDb);
            q.exec(QStringLiteral(R"(
                CREATE TABLE readings (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp INTEGER NOT NULL,
                    partectorNumber INTEGER, partectorDiam INTEGER,
                    partectorMass REAL, grimmValue REAL,
                    temperature REAL, humidity REAL, pressure REAL,
                    altitude REAL, latitude REAL, longitude REAL, co2 INTEGER
                )
            )"));
            q.exec(QStringLiteral("INSERT INTO readings (timestamp, partectorNumber) VALUES (1000, 99)"));
            srcDb.close();
        }
        QSqlDatabase::removeDatabase(QStringLiteral("importSrc"));

        // Import should succeed
        QSignalSpy spy(&dbm, &DatabaseManager::importCompleted);
        bool ok = dbm.importDatabase(QUrl::fromLocalFile(srcPath));
        QVERIFY(ok);
        QCOMPARE(spy.count(), 1);
    }

private:
    std::unique_ptr<QTemporaryDir> m_tmpDir;

    // Insert a reading directly via SQL on the DatabaseManager's connection
    void insertDirectly(const SensorReading &reading)
    {
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
        if (!query.exec()) {
            qWarning() << "insertDirectly failed:" << query.lastError().text();
        }
    }

    void seedReadings(DatabaseManager &dbm, const QList<QDateTime> &timestamps)
    {
        Q_UNUSED(dbm)
        for (const QDateTime &ts : timestamps) {
            insertDirectly(SensorReadingBuilder().withAllSensors().withTimestamp(ts).build());
        }
    }
};

QTEST_GUILESS_MAIN(TestDatabaseManager)
#include "tst_databasemanager.moc"
