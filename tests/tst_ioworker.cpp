#include <QTest>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include "ioworker.h"
#include "sensorreading.h"

class TestIOWorker : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_tmpDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_tmpDir->isValid());
    }

    void cleanup()
    {
        // IOWorker destructor closes and removes the DB connection,
        // but if the test created one and it's still lingering, remove it.
        if (QSqlDatabase::contains("ZephyrSenseIOWorker")) {
            QSqlDatabase::removeDatabase("ZephyrSenseIOWorker");
        }
        m_tmpDir.reset();
    }

    void initialize_createsDatabase()
    {
        IOWorker worker;
        QSignalSpy spy(&worker, &IOWorker::initialized);

        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        QCOMPARE(spy.count(), 1);
        QList<QVariant> args = spy.takeFirst();
        QVERIFY(args.at(0).toBool());  // success = true

        // Verify database file exists
        QVERIFY(QFile::exists(dbPath));
    }

    void processReading_insertsToDb()
    {
        IOWorker worker;
        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        SensorReading reading;
        reading.partectorNumber = 42;
        reading.co2 = 800;
        reading.temperature = 22.5f;
        reading.timestamp = QDateTime::currentDateTime();

        worker.processReading(reading);

        // Verify via raw SQL
        {
            QSqlDatabase db = QSqlDatabase::database("ZephyrSenseIOWorker");
            QVERIFY(db.isOpen());

            QSqlQuery query(db);
            QVERIFY(query.exec("SELECT partectorNumber, co2, temperature FROM readings"));
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toInt(), 42);
            QCOMPARE(query.value(1).toInt(), 800);
            QVERIFY(qAbs(query.value(2).toFloat() - 22.5f) < 0.01f);
        }
    }

    void processReading_multipleReadings()
    {
        IOWorker worker;
        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        for (int i = 0; i < 3; ++i) {
            SensorReading reading;
            reading.partectorNumber = i;
            reading.timestamp = QDateTime::currentDateTime();
            worker.processReading(reading);
        }

        {
            QSqlDatabase db = QSqlDatabase::database("ZephyrSenseIOWorker");
            QSqlQuery query(db);
            QVERIFY(query.exec("SELECT COUNT(*) FROM readings"));
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toInt(), 3);
        }
    }

    void csvWrite_headerAndData()
    {
        IOWorker worker;
        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        QString csvPath = m_tmpDir->filePath("export.csv");
        worker.setCsvEnabled(true);
        worker.setCsvFilePath(csvPath);

        SensorReading reading;
        reading.partectorNumber = 100;
        reading.co2 = 500;
        reading.timestamp = QDateTime::currentDateTime();
        worker.processReading(reading);

        // Flush to ensure data is written
        worker.flushAll();

        // Read CSV and verify
        QFile file(csvPath);
        QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream stream(&file);

        QString headerLine = stream.readLine();
        QVERIFY(headerLine.startsWith("timestamp,"));
        QVERIFY(headerLine.contains("co2"));

        QString dataLine = stream.readLine();
        QVERIFY(!dataLine.isEmpty());
        QVERIFY(dataLine.contains("100"));  // partectorNumber
        QVERIFY(dataLine.contains("500"));  // co2
    }

    void csvDisabled_noFileCreated()
    {
        IOWorker worker;
        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        QString csvPath = m_tmpDir->filePath("should_not_exist.csv");
        worker.setCsvFilePath(csvPath);
        // CSV is disabled by default

        SensorReading reading;
        reading.timestamp = QDateTime::currentDateTime();
        worker.processReading(reading);

        QVERIFY(!QFile::exists(csvPath));
    }

    void csvPathChange_closesOldOpensNew()
    {
        IOWorker worker;
        QString dbPath = m_tmpDir->filePath("test.db");
        worker.initialize(dbPath);

        QString csv1 = m_tmpDir->filePath("first.csv");
        QString csv2 = m_tmpDir->filePath("second.csv");

        worker.setCsvEnabled(true);
        worker.setCsvFilePath(csv1);

        SensorReading r1;
        r1.partectorNumber = 111;
        r1.timestamp = QDateTime::currentDateTime();
        worker.processReading(r1);
        worker.flushAll();

        // Switch to second file
        worker.setCsvFilePath(csv2);

        SensorReading r2;
        r2.partectorNumber = 222;
        r2.timestamp = QDateTime::currentDateTime();
        worker.processReading(r2);
        worker.flushAll();

        // Verify second file exists and has data
        QFile file2(csv2);
        QVERIFY(file2.open(QIODevice::ReadOnly | QIODevice::Text));
        QTextStream stream2(&file2);
        QString header2 = stream2.readLine();
        QVERIFY(header2.startsWith("timestamp,"));
        QString data2 = stream2.readLine();
        QVERIFY(data2.contains("222"));
    }

private:
    std::unique_ptr<QTemporaryDir> m_tmpDir;
};

QTEST_GUILESS_MAIN(TestIOWorker)
#include "tst_ioworker.moc"
