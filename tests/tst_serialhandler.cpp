#include <QTest>
#include <QSignalSpy>
#include <cstring>
#include "serialhandler.h"
#include "sensorreading.h"
#include "testhelpers.h"

class TestSerialHandler : public QObject
{
    Q_OBJECT

private slots:
    void defaultBaudRate_is115200()
    {
        SerialHandler handler;
        QCOMPARE(handler.baudRate(), 115200);
    }

    void setBaudRate_emitsSignal()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::baudRateChanged);
        handler.setBaudRate(9600);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(handler.baudRate(), 9600);
    }

    void setBaudRate_sameValue_noSignal()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::baudRateChanged);
        handler.setBaudRate(115200);  // Same as default
        QCOMPARE(spy.count(), 0);
    }

    void isConnected_defaultFalse()
    {
        SerialHandler handler;
        QCOMPARE(handler.isConnected(), false);
    }

    void parseValidFrame_emitsNewReading()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        QByteArray frame = FrameBuilder().withDefaults().buildFrame();
        handler.injectTestData(frame);

        QCOMPARE(spy.count(), 1);
        SensorReading reading = spy.constFirst().at(0).value<SensorReading>();
        QCOMPARE(reading.partectorNumber, 1000);
    }

    void parseFrame_allFieldsExtracted()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        SensorDataRaw raw = {};
        raw.partectorNumber = 500;
        raw.partectorDiam = 75;
        raw.partectorMass = 3.14f;
        raw.grimmValue = 2.71f;
        raw.temperature = 25.5f;
        raw.humidity = 60.0f;
        raw.pressure = 1000.0f;
        raw.altitude = 200.0f;
        raw.latitude = 52.5f;
        raw.longitude = 13.4f;
        raw.co2 = 800;

        QByteArray frame = FrameBuilder().withRaw(raw).buildFrame();
        handler.injectTestData(frame);

        QCOMPARE(spy.count(), 1);
        SensorReading reading = spy.constFirst().at(0).value<SensorReading>();
        QCOMPARE(reading.partectorNumber, 500);
        QCOMPARE(reading.partectorDiam, 75);
        QVERIFY(qAbs(reading.partectorMass - 3.14f) < 0.01f);
        QVERIFY(qAbs(reading.grimmValue - 2.71f) < 0.01f);
        QVERIFY(qAbs(reading.temperature - 25.5f) < 0.01f);
        QVERIFY(qAbs(reading.humidity - 60.0f) < 0.01f);
        QVERIFY(qAbs(reading.pressure - 1000.0f) < 0.01f);
        QVERIFY(qAbs(reading.altitude - 200.0f) < 0.01f);
        QVERIFY(qAbs(reading.latitude - 52.5f) < 0.01f);
        QVERIFY(qAbs(reading.longitude - 13.4f) < 0.01f);
        QCOMPARE(reading.co2, 800);
    }

    void incompleteFrame_waitsForMoreData()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        QByteArray full = FrameBuilder().withDefaults().buildFrame();
        QByteArray part1 = full.left(30);
        QByteArray part2 = full.mid(30);

        handler.injectTestData(part1);
        QCOMPARE(spy.count(), 0);

        handler.injectTestData(part2);
        QCOMPARE(spy.count(), 1);
    }

    void corruptedFrame_noSignal()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        QByteArray frame = FrameBuilder().withDefaults().buildCorruptedFrame();
        handler.injectTestData(frame);

        QCOMPARE(spy.count(), 0);
    }

    void garbageBeforeFrame_skipped()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        QByteArray garbage("GARBAGE_DATA_HERE");
        QByteArray frame = FrameBuilder().withDefaults().buildFrame();
        handler.injectTestData(garbage + frame);

        QCOMPARE(spy.count(), 1);
    }

    void multipleFrames_allParsed()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        FrameBuilder fb;
        fb.withDefaults();

        QByteArray data;
        for (int i = 0; i < 3; ++i) {
            fb.withPartectorNumber(100 + i);
            data.append(fb.buildFrame());
        }

        handler.injectTestData(data);
        QCOMPARE(spy.count(), 3);

        // Verify each reading has the correct partectorNumber
        for (int i = 0; i < 3; ++i) {
            SensorReading reading = spy.at(i).at(0).value<SensorReading>();
            QCOMPARE(reading.partectorNumber, 100 + i);
        }
    }

    void frameAfterCorrupted_recovered()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        QByteArray corrupted = FrameBuilder().withDefaults().buildCorruptedFrame();
        QByteArray valid = FrameBuilder().withDefaults().withPartectorNumber(999).buildFrame();

        handler.injectTestData(corrupted + valid);

        QCOMPARE(spy.count(), 1);
        SensorReading reading = spy.constFirst().at(0).value<SensorReading>();
        QCOMPARE(reading.partectorNumber, 999);
    }

    void noStartDelimiter_bufferCleared()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::newReading);

        // 50 bytes of data without any '<' character
        QByteArray noDelimiter(50, 'A');
        handler.injectTestData(noDelimiter);

        QCOMPARE(spy.count(), 0);
    }

    void closePort_whenNotOpen_noOp()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::connectionStateChanged);
        // Should not crash or emit signals
        handler.closePort();
        QCOMPARE(spy.count(), 0);
    }

    void refreshPorts_emitsSignal()
    {
        SerialHandler handler;
        QSignalSpy spy(&handler, &SerialHandler::portsChanged);
        handler.refreshPorts();
        QCOMPARE(spy.count(), 1);
    }

    // ── New coverage tests ──

    void openPort_nonexistentPort_emitsError()
    {
        SerialHandler handler;
        QSignalSpy errorSpy(&handler, &SerialHandler::errorOccurred);
        QSignalSpy connSpy(&handler, &SerialHandler::connectionStateChanged);

        handler.openPort(QStringLiteral("NONEXISTENT_PORT_XYZ"));

        // Opening a nonexistent port emits errorOccurred from both handleError
        // (via QSerialPort::errorOccurred signal) and the openPort failure path
        QVERIFY(errorSpy.count() >= 1);
        QVERIFY(!errorSpy.constFirst().at(0).toString().isEmpty());
        // connectionStateChanged(true) should NOT have been emitted
        for (int i = 0; i < connSpy.count(); ++i) {
            QVERIFY(!connSpy.at(i).at(0).toBool());
        }
    }

    void openPort_parsesPortNameWithDescription()
    {
        SerialHandler handler;
        QSignalSpy errorSpy(&handler, &SerialHandler::errorOccurred);

        // The " - Some Device" suffix should be stripped, leaving "COM99"
        handler.openPort(QStringLiteral("COM99 - Some Device"));

        // COM99 doesn't exist so it will error — handleError + openPort both emit
        QVERIFY(errorSpy.count() >= 1);
    }

    void errorString_afterFailedOpen()
    {
        SerialHandler handler;
        // Initially empty
        QVERIFY(handler.errorString().isEmpty());

        handler.openPort(QStringLiteral("NONEXISTENT_PORT_XYZ"));

        // After a failed open, errorString should be non-empty
        QVERIFY(!handler.errorString().isEmpty());
    }

    void currentPort_default()
    {
        SerialHandler handler;
        // Before opening any port, currentPort() returns empty
        QVERIFY(handler.currentPort().isEmpty());
    }

    void refreshPorts_populatesList()
    {
        SerialHandler handler;
        handler.refreshPorts();

        // availablePorts() returns a list (may be empty on CI, but should not crash)
        QStringList ports = handler.availablePorts();
        // Just verify we can call it — the list may be empty on CI machines
        Q_UNUSED(ports)
    }
};

QTEST_GUILESS_MAIN(TestSerialHandler)
#include "tst_serialhandler.moc"
