#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include "cesiumbridge.h"
#include "thresholdmanager.h"

class tst_CesiumBridge : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void testDefaultLiveMode();
    void testContentUrlProduction();
    void testObjectName();
    void testSetLiveModeToFalse();
    void testSetLiveModeIdempotent();
    void testSetLiveModePreventsDoubleDispatch();
    void testPendingRequestIdNotifiesOnChange();
    void testWindowMinutesDefault();
    void testCesiumTokenFromSettings();
    void testOnNewReadingNoOpWhenNotLive();
    void testOnNewReadingEmitsCzmlPacket();
    void testOnThresholdsChangedEmitsSignal();
    void testTokenValidPropertyDefault();
    void testValidateTokenEmptyString();
    void testValidateTokenSetsValidating();
    void testValidateTokenPreventsDoubleDispatch();
    void testValidateTokenResetsTokenValid();

private:
    ThresholdManager *m_thresholdManager = nullptr;
};

void tst_CesiumBridge::init()
{
    QStandardPaths::setTestModeEnabled(true);
    QSettings settings(QStringLiteral("ZephyrSense"), QStringLiteral("ZephyrSense"));
    settings.clear();
    m_thresholdManager = new ThresholdManager();
}

void tst_CesiumBridge::cleanup()
{
    delete m_thresholdManager;
    m_thresholdManager = nullptr;
    QStandardPaths::setTestModeEnabled(false);
}

void tst_CesiumBridge::testDefaultLiveMode()
{
    CesiumBridge bridge;
    QCOMPARE(bridge.liveMode(), true);
}

void tst_CesiumBridge::testContentUrlProduction()
{
    CesiumBridge bridge;
    // Without WEBDEV_MODE, should use the custom zephyr:// scheme
#ifndef WEBDEV_MODE
    QCOMPARE(bridge.contentUrl().scheme(), QStringLiteral("zephyr"));
    QVERIFY(bridge.contentUrl().toString().endsWith(QStringLiteral("index.html")));
#endif
}

void tst_CesiumBridge::testObjectName()
{
    CesiumBridge bridge;
    QCOMPARE(bridge.objectName(), QStringLiteral("CesiumBridge"));
}

void tst_CesiumBridge::testSetLiveModeToFalse()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);

    QSignalSpy spy(&bridge, &CesiumBridge::liveModeChanged);
    bridge.setLiveMode(false);

    QCOMPARE(bridge.liveMode(), false);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);
}

void tst_CesiumBridge::testSetLiveModeIdempotent()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);

    bridge.setLiveMode(false);
    QSignalSpy spy(&bridge, &CesiumBridge::liveModeChanged);
    bridge.setLiveMode(false); // no-op

    QCOMPARE(spy.count(), 0);
}

void tst_CesiumBridge::testSetLiveModePreventsDoubleDispatch()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);
    bridge.setLiveMode(false);

    // First call: triggers pending live switch
    bridge.setLiveMode(true);

    // Second call should be no-op (m_pendingLiveSwitch is true)
    int requestBefore = bridge.pendingRequestId();
    bridge.setLiveMode(true);
    QCOMPARE(bridge.pendingRequestId(), requestBefore);
}

void tst_CesiumBridge::testPendingRequestIdNotifiesOnChange()
{
    CesiumBridge bridge;
    QSignalSpy spy(&bridge, &CesiumBridge::pendingRequestIdChanged);

    bridge.setPendingRequestId(5);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toInt(), 5);
    QCOMPARE(bridge.pendingRequestId(), 5);
}

void tst_CesiumBridge::testWindowMinutesDefault()
{
    CesiumBridge bridge;
    QCOMPARE(bridge.windowMinutes(), 60);
}

void tst_CesiumBridge::testCesiumTokenFromSettings()
{
    QSettings settings(QStringLiteral("ZephyrSense"), QStringLiteral("ZephyrSense"));
    settings.setValue(QStringLiteral("cesiumIonToken"), QStringLiteral("test-token-123"));

    CesiumBridge bridge;
    QCOMPARE(bridge.cesiumToken(), QStringLiteral("test-token-123"));
}

void tst_CesiumBridge::testOnNewReadingNoOpWhenNotLive()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);
    bridge.setLiveMode(false);

    QSignalSpy spy(&bridge, &CesiumBridge::czmlPacket);

    SensorReading reading;
    reading.latitude = 51.25f;
    reading.longitude = 7.15f;
    reading.altitude = 100.0f;
    bridge.onNewReading(reading);

    QCOMPARE(spy.count(), 0);
}

void tst_CesiumBridge::testOnNewReadingEmitsCzmlPacket()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);

    QSignalSpy spy(&bridge, &CesiumBridge::czmlPacket);

    SensorReading reading;
    reading.latitude = 51.25f;
    reading.longitude = 7.15f;
    reading.altitude = 100.0f;
    reading.co2 = 400;
    bridge.onNewReading(reading);

    QCOMPARE(spy.count(), 1);
    auto czml = QJsonDocument::fromJson(spy.at(0).at(0).toString().toUtf8()).array();
    QVERIFY(czml.size() >= 1); // At least the point packet
}

void tst_CesiumBridge::testOnThresholdsChangedEmitsSignal()
{
    CesiumBridge bridge;
    bridge.setThresholdManager(m_thresholdManager);

    QSignalSpy spy(&bridge, &CesiumBridge::thresholdsChanged);
    bridge.onThresholdsChanged();

    QCOMPARE(spy.count(), 1);
    auto config = spy.at(0).at(0).toMap();
    QVERIFY(!config.isEmpty());
}

void tst_CesiumBridge::testTokenValidPropertyDefault()
{
    CesiumBridge bridge;
    QCOMPARE(bridge.tokenValid(), false);
    QCOMPARE(bridge.tokenError(), QString());
    QCOMPARE(bridge.validatingToken(), false);
}

void tst_CesiumBridge::testValidateTokenEmptyString()
{
    CesiumBridge bridge;

    QSignalSpy failSpy(&bridge, &CesiumBridge::tokenValidationFailed);
    QSignalSpy validatingSpy(&bridge, &CesiumBridge::validatingTokenChanged);

    bridge.validateToken(QStringLiteral(""));

    // Should fail immediately without starting a network request
    QCOMPARE(failSpy.count(), 1);
    QVERIFY(!failSpy.at(0).at(0).toString().isEmpty());
    QCOMPARE(bridge.validatingToken(), false);
    // validatingTokenChanged should NOT have been emitted (no network request)
    QCOMPARE(validatingSpy.count(), 0);
}

void tst_CesiumBridge::testValidateTokenSetsValidating()
{
    CesiumBridge bridge;

    QSignalSpy validatingSpy(&bridge, &CesiumBridge::validatingTokenChanged);

    bridge.validateToken(QStringLiteral("some-test-token"));

    // Should immediately enter validating state
    QCOMPARE(bridge.validatingToken(), true);
    QCOMPARE(validatingSpy.count(), 1);
}

void tst_CesiumBridge::testValidateTokenPreventsDoubleDispatch()
{
    CesiumBridge bridge;

    bridge.validateToken(QStringLiteral("first-token"));
    QCOMPARE(bridge.validatingToken(), true);

    QSignalSpy validatingSpy(&bridge, &CesiumBridge::validatingTokenChanged);

    // Second call while first is in-flight should be a no-op
    bridge.validateToken(QStringLiteral("second-token"));
    QCOMPARE(validatingSpy.count(), 0);
}

void tst_CesiumBridge::testValidateTokenResetsTokenValid()
{
    CesiumBridge bridge;

    QCOMPARE(bridge.tokenValid(), false);

    QSignalSpy tokenValidSpy(&bridge, &CesiumBridge::tokenValidChanged);

    bridge.validateToken(QStringLiteral("some-new-token"));

    QCOMPARE(bridge.tokenValid(), false);
    QCOMPARE(tokenValidSpy.count(), 1);
}

QTEST_GUILESS_MAIN(tst_CesiumBridge)
#include "tst_cesiumbridge.moc"
