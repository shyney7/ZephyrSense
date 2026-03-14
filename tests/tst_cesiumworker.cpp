#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "cesiumworker.h"
#include "testhelpers.h"

class tst_CesiumWorker : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testGenerateCzmlEmpty();
    void testGenerateCzmlSingleReading();
    void testGenerateCzmlDocumentPacket();
    void testGenerateCzmlPointPacket();
    void testGenerateCzmlFlightPath();
    void testGenerateCzmlHazardColors();
    void testGenerateCzmlRequestIdPassthrough();

private:
    TestDatabaseHelper *m_dbHelper = nullptr;
};

void tst_CesiumWorker::init()
{
    m_dbHelper = new TestDatabaseHelper(QStringLiteral("tst_cesiumworker"));
    m_dbHelper->initialize();
}

void tst_CesiumWorker::cleanup()
{
    delete m_dbHelper;
    m_dbHelper = nullptr;
}

void tst_CesiumWorker::testGenerateCzmlEmpty()
{
    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);

    QVariantMap thresholds;
    thresholds[QStringLiteral("co2Warning")] = 800;
    thresholds[QStringLiteral("co2Danger")] = 1200;
    thresholds[QStringLiteral("co2Enabled")] = true;

    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now, 1, thresholds);

    QCOMPARE(spy.count(), 1);
    int requestId = spy.at(0).at(1).toInt();
    QCOMPARE(requestId, 1);

    // Empty result should still have a document packet
    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    QVERIFY(czml.size() >= 1);
    QCOMPARE(czml.at(0).toObject()[QStringLiteral("id")].toString(), QStringLiteral("document"));
}

void tst_CesiumWorker::testGenerateCzmlSingleReading()
{
    auto reading = SensorReadingBuilder()
        .withAllSensors()
        .withCoordinates(51.2562f, 7.1508f)
        .build();

    m_dbHelper->insertReading(reading);

    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);

    QVariantMap thresholds;
    thresholds[QStringLiteral("co2Warning")] = 800;
    thresholds[QStringLiteral("co2Danger")] = 1200;
    thresholds[QStringLiteral("co2Enabled")] = true;

    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now + 3600000, 2, thresholds);

    QCOMPARE(spy.count(), 1);
    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();

    // document + 1 point + 1 flight path = 3
    QCOMPARE(czml.size(), 3);
}

void tst_CesiumWorker::testGenerateCzmlDocumentPacket()
{
    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);

    qint64 start = 1709640000000LL; // fixed epoch for test
    qint64 end = start + 3600000;
    worker.generateCzml(start, end, 1, QVariantMap());

    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    auto doc = czml.at(0).toObject();

    QCOMPARE(doc[QStringLiteral("id")].toString(), QStringLiteral("document"));
    QVERIFY(doc.contains(QStringLiteral("clock")));
    auto clock = doc[QStringLiteral("clock")].toObject();
    QVERIFY(clock.contains(QStringLiteral("interval")));
    QCOMPARE(clock[QStringLiteral("multiplier")].toInt(), 1);
}

void tst_CesiumWorker::testGenerateCzmlPointPacket()
{
    auto reading = SensorReadingBuilder()
        .withAllSensors()
        .withCoordinates(51.2562f, 7.1508f)
        .build();
    reading.altitude = 150.0f;
    reading.co2 = 420;

    m_dbHelper->insertReading(reading);

    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);

    QVariantMap thresholds;
    thresholds[QStringLiteral("co2Warning")] = 800;
    thresholds[QStringLiteral("co2Danger")] = 1200;
    thresholds[QStringLiteral("co2Enabled")] = true;

    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now + 3600000, 1, thresholds);

    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    // Find the point packet (not document, not flight-path)
    QJsonObject point;
    for (int i = 0; i < czml.size(); ++i) {
        auto obj = czml.at(i).toObject();
        if (obj[QStringLiteral("id")].toString().startsWith(QStringLiteral("reading-"))) {
            point = obj;
            break;
        }
    }

    QVERIFY(!point.isEmpty());
    QVERIFY(point.contains(QStringLiteral("point")));
    QVERIFY(point.contains(QStringLiteral("position")));
    QVERIFY(point.contains(QStringLiteral("description")));

    // Verify position format: cartographicDegrees [lon, lat, alt]
    auto pos = point[QStringLiteral("position")].toObject();
    auto degrees = pos[QStringLiteral("cartographicDegrees")].toArray();
    QCOMPARE(degrees.size(), 3);
}

void tst_CesiumWorker::testGenerateCzmlFlightPath()
{
    // Insert two readings to get a polyline
    auto r1 = SensorReadingBuilder().withAllSensors().withCoordinates(51.25f, 7.15f).build();
    r1.altitude = 100.0f;
    r1.timestamp = QDateTime::currentDateTime().addSecs(-60);

    auto r2 = SensorReadingBuilder().withAllSensors().withCoordinates(51.26f, 7.16f).build();
    r2.altitude = 110.0f;

    m_dbHelper->insertReading(r1);
    m_dbHelper->insertReading(r2);

    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);
    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now + 3600000, 1, QVariantMap());

    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    QJsonObject flightPath;
    for (int i = 0; i < czml.size(); ++i) {
        auto obj = czml.at(i).toObject();
        if (obj[QStringLiteral("id")].toString() == QStringLiteral("flight-path")) {
            flightPath = obj;
            break;
        }
    }

    QVERIFY(!flightPath.isEmpty());
    auto polyline = flightPath[QStringLiteral("polyline")].toObject();
    auto positions = polyline[QStringLiteral("positions")].toObject();
    auto coords = positions[QStringLiteral("cartographicDegrees")].toArray();
    // Two readings * 3 values (lon, lat, alt) = 6
    QCOMPARE(coords.size(), 6);

    // Verify arcType is NONE (prevents geodesic cut-through artifacts)
    QCOMPARE(polyline[QStringLiteral("arcType")].toString(), QStringLiteral("NONE"));
}

void tst_CesiumWorker::testGenerateCzmlHazardColors()
{
    auto reading = SensorReadingBuilder().withAllSensors().withCoordinates(51.25f, 7.15f).build();
    reading.co2 = 1500; // Above danger threshold
    m_dbHelper->insertReading(reading);

    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);

    QVariantMap thresholds;
    thresholds[QStringLiteral("co2Warning")] = 800;
    thresholds[QStringLiteral("co2Danger")] = 1200;
    thresholds[QStringLiteral("co2Enabled")] = true;

    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now + 3600000, 1, thresholds);

    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    QJsonObject point;
    for (int i = 0; i < czml.size(); ++i) {
        auto obj = czml.at(i).toObject();
        if (obj[QStringLiteral("id")].toString().startsWith(QStringLiteral("reading-"))) {
            point = obj;
            break;
        }
    }

    QVERIFY(!point.isEmpty());

    auto color = point[QStringLiteral("point")].toObject()
                      [QStringLiteral("color")].toObject()
                      [QStringLiteral("rgba")].toArray();
    // Danger = red = [244, 67, 54, 255]
    QCOMPARE(color.at(0).toInt(), 244);
    QCOMPARE(color.at(1).toInt(), 67);
    QCOMPARE(color.at(2).toInt(), 54);
}

void tst_CesiumWorker::testGenerateCzmlRequestIdPassthrough()
{
    CesiumWorker worker;
    worker.initialize(m_dbHelper->dbPath());

    QSignalSpy spy(&worker, &CesiumWorker::czmlGenerated);
    auto now = QDateTime::currentMSecsSinceEpoch();
    worker.generateCzml(now - 3600000, now, 42, QVariantMap());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).toInt(), 42);
}

QTEST_GUILESS_MAIN(tst_CesiumWorker)
#include "tst_cesiumworker.moc"
