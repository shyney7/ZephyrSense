#include <QTest>
#include "sensorreading.h"

class TestSensorReading : public QObject
{
    Q_OBJECT

private slots:
    void defaultConstructor()
    {
        SensorReading r;
        QCOMPARE(r.partectorNumber, 0);
        QCOMPARE(r.partectorDiam, 0);
        QCOMPARE(r.partectorMass, 0.0f);
        QCOMPARE(r.grimmValue, 0.0f);
        QCOMPARE(r.temperature, 0.0f);
        QCOMPARE(r.humidity, 0.0f);
        QCOMPARE(r.pressure, 0.0f);
        QCOMPARE(r.altitude, 0.0f);
        QCOMPARE(r.latitude, 0.0f);
        QCOMPARE(r.longitude, 0.0f);
        QCOMPARE(r.co2, 0);
        QVERIFY(r.timestamp.isValid());
    }

    void rawConstructor()
    {
        SensorDataRaw raw{};
        raw.partectorNumber = 12345;
        raw.partectorDiam = 67;
        raw.partectorMass = 1.5f;
        raw.grimmValue = 2.3f;
        raw.temperature = 22.5f;
        raw.humidity = 45.0f;
        raw.pressure = 1013.25f;
        raw.altitude = 500.0f;
        raw.latitude = 48.1234f;
        raw.longitude = 11.5678f;
        raw.co2 = 800;

        SensorReading r(raw);
        QCOMPARE(r.partectorNumber, 12345);
        QCOMPARE(r.partectorDiam, 67);
        QCOMPARE(r.partectorMass, 1.5f);
        QCOMPARE(r.grimmValue, 2.3f);
        QCOMPARE(r.temperature, 22.5f);
        QCOMPARE(r.humidity, 45.0f);
        QCOMPARE(r.pressure, 1013.25f);
        QCOMPARE(r.altitude, 500.0f);
        QCOMPARE(r.latitude, 48.1234f);
        QCOMPARE(r.longitude, 11.5678f);
        QCOMPARE(r.co2, 800);
    }

    void rawConstructor_negativeCoordinates()
    {
        SensorDataRaw raw{};
        raw.latitude = -33.8688f;
        raw.longitude = -151.2093f;

        SensorReading r(raw);
        QCOMPARE(r.latitude, -33.8688f);
        QCOMPARE(r.longitude, -151.2093f);
    }

    void rawConstructor_co2Max()
    {
        SensorDataRaw raw{};
        raw.co2 = 65535;  // uint16_t max

        SensorReading r(raw);
        QCOMPARE(r.co2, 65535);
    }

    void timestampIsRecent()
    {
        QDateTime before = QDateTime::currentDateTime();
        SensorReading r;
        QDateTime after = QDateTime::currentDateTime();
        // Timestamp should be set during construction, between before and after
        QVERIFY(r.timestamp >= before && r.timestamp <= after);
    }
};

QTEST_GUILESS_MAIN(TestSensorReading)
#include "tst_sensorreading.moc"
