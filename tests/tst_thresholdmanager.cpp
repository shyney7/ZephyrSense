#include <QTest>
#include <QSignalSpy>
#include <QSettings>
#include "thresholdmanager.h"

class TestThresholdManager : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        // QSettings("thresholds") always uses NativeFormat (Windows registry),
        // ignoring setDefaultFormat(). Clear persisted settings so the
        // constructor sees clean defaults. These are trivial threshold values
        // that the app recreates on next launch.
        QSettings settings("thresholds");
        settings.clear();
        settings.sync();

        m_mgr = new ThresholdManager(this);
    }

    void cleanup()
    {
        delete m_mgr;
        m_mgr = nullptr;
    }

    void defaultValues()
    {
        QCOMPARE(m_mgr->co2Warning(), 1000);
        QCOMPARE(m_mgr->co2Danger(), 2000);
        QCOMPARE(m_mgr->temperatureWarning(), 30.0f);
        QCOMPARE(m_mgr->temperatureDanger(), 35.0f);
        QCOMPARE(m_mgr->temperatureLowWarning(), 15.0f);
        QCOMPARE(m_mgr->temperatureLowDanger(), 10.0f);
        QCOMPARE(m_mgr->humidityWarning(), 60.0f);
        QCOMPARE(m_mgr->humidityDanger(), 80.0f);
        QCOMPARE(m_mgr->humidityLowWarning(), 30.0f);
        QCOMPARE(m_mgr->humidityLowDanger(), 20.0f);
        QCOMPARE(m_mgr->partectorMassWarning(), 25.0f);
        QCOMPARE(m_mgr->partectorMassDanger(), 50.0f);
        QCOMPARE(m_mgr->grimmValueWarning(), 25.0f);
        QCOMPARE(m_mgr->grimmValueDanger(), 50.0f);
        QCOMPARE(m_mgr->partectorNumberWarning(), 10000);
        QCOMPARE(m_mgr->partectorNumberDanger(), 50000);
        QCOMPARE(m_mgr->partectorDiamWarning(), 100);
        QCOMPARE(m_mgr->partectorDiamDanger(), 200);
        QCOMPARE(m_mgr->pressureWarning(), 1030.0f);
        QCOMPARE(m_mgr->pressureDanger(), 1050.0f);
        QCOMPARE(m_mgr->altitudeWarning(), 3000.0f);
        QCOMPARE(m_mgr->altitudeDanger(), 4000.0f);
    }

    void defaultEnabledStates()
    {
        // Core sensors enabled
        QVERIFY(m_mgr->partectorMassEnabled());
        QVERIFY(m_mgr->partectorNumberEnabled());
        QVERIFY(m_mgr->partectorDiamEnabled());
        QVERIFY(m_mgr->grimmValueEnabled());
        QVERIFY(m_mgr->co2Enabled());
        // Comfort sensors disabled
        QVERIFY(!m_mgr->temperatureEnabled());
        QVERIFY(!m_mgr->humidityEnabled());
        QVERIFY(!m_mgr->pressureEnabled());
        QVERIFY(!m_mgr->altitudeEnabled());
    }

    void computeHazardLevel_allGreen()
    {
        // All values safely below thresholds
        int level = m_mgr->computeHazardLevel(
            /*partectorNumber=*/0, /*partectorDiam=*/0,
            /*partectorMass=*/0.0f, /*grimmValue=*/0.0f,
            /*temperature=*/22.0f, /*humidity=*/45.0f,
            /*pressure=*/1013.0f, /*altitude=*/100.0f, /*co2=*/400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Green));
    }

    void computeHazardLevel_co2Warning()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, /*co2=*/1000);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_co2Danger()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, /*co2=*/2500);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_maxAcrossSensors()
    {
        // CO2 at danger (Red), everything else green — result should be Red
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, /*co2=*/3000);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_disabledSensorIgnored()
    {
        // CO2 is enabled by default — disable it, then set CO2 extremely high
        m_mgr->setCo2Enabled(false);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, /*co2=*/99999);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Green));
    }

    void computeHazardLevel_temperatureHighDanger()
    {
        // Temperature is disabled by default — enable it
        m_mgr->setTemperatureEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, /*temperature=*/36.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_temperatureHighWarning()
    {
        m_mgr->setTemperatureEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, /*temperature=*/32.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_temperatureLowDanger()
    {
        m_mgr->setTemperatureEnabled(true);
        // temperatureLowDanger default = 10.0, so temp=5 should be Red
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, /*temperature=*/5.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_temperatureLowWarning()
    {
        m_mgr->setTemperatureEnabled(true);
        // temperatureLowWarning default = 15.0, so temp=12 should be Yellow
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, /*temperature=*/12.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_humidityBidirectional()
    {
        m_mgr->setHumidityEnabled(true);
        // humidityLowDanger default = 20.0, so humidity=15 should be Red
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, /*humidity=*/15.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_humidityHighDanger()
    {
        m_mgr->setHumidityEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, /*humidity=*/85.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_humidityHighWarning()
    {
        m_mgr->setHumidityEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, /*humidity=*/65.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_humidityLowWarning()
    {
        m_mgr->setHumidityEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, /*humidity=*/25.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_partectorMassWarning()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, /*partectorMass=*/30.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_partectorMassDanger()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, /*partectorMass=*/55.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_grimmValueWarning()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, /*grimmValue=*/30.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_grimmValueDanger()
    {
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, /*grimmValue=*/55.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_partectorNumberWarning()
    {
        int level = m_mgr->computeHazardLevel(
            /*partectorNumber=*/15000, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_partectorNumberDanger()
    {
        int level = m_mgr->computeHazardLevel(
            /*partectorNumber=*/55000, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_partectorDiamWarning()
    {
        int level = m_mgr->computeHazardLevel(
            0, /*partectorDiam=*/150, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_partectorDiamDanger()
    {
        int level = m_mgr->computeHazardLevel(
            0, /*partectorDiam=*/250, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_pressureWarning()
    {
        m_mgr->setPressureEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, /*pressure=*/1035.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_pressureDanger()
    {
        m_mgr->setPressureEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, /*pressure=*/1055.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_altitudeWarning()
    {
        m_mgr->setAltitudeEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, /*altitude=*/3500.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Yellow));
    }

    void computeHazardLevel_altitudeDanger()
    {
        m_mgr->setAltitudeEnabled(true);
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, 45.0f, 1013.0f, /*altitude=*/4500.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    // ── getColorForSensor tests ──

    void getColorForSensor_green()
    {
        // CO2 = 400, well below warning (1000)
        QString color = m_mgr->getColorForSensor("co2", 400);
        QCOMPARE(color, QStringLiteral("#4CAF50"));
    }

    void getColorForSensor_yellow()
    {
        // CO2 = 1500, between warning (1000) and danger (2000)
        QString color = m_mgr->getColorForSensor("co2", 1500);
        QCOMPARE(color, QStringLiteral("#FF9800"));
    }

    void getColorForSensor_red()
    {
        // CO2 = 2500, above danger (2000)
        QString color = m_mgr->getColorForSensor("co2", 2500);
        QCOMPARE(color, QStringLiteral("#F44336"));
    }

    void getColorForSensor_disabledReturnsBlue()
    {
        m_mgr->setCo2Enabled(false);
        QString color = m_mgr->getColorForSensor("co2", 99999);
        QCOMPARE(color, QStringLiteral("#2196F3"));
    }

    void getColorForSensor_temperature_highDanger()
    {
        m_mgr->setTemperatureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("temperature", 36.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_temperature_highWarning()
    {
        m_mgr->setTemperatureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("temperature", 32.0), QStringLiteral("#FF9800"));
    }

    void getColorForSensor_temperature_lowDanger()
    {
        m_mgr->setTemperatureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("temperature", 5.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_temperature_lowWarning()
    {
        m_mgr->setTemperatureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("temperature", 12.0), QStringLiteral("#FF9800"));
    }

    void getColorForSensor_temperature_normal()
    {
        m_mgr->setTemperatureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("temperature", 22.0), QStringLiteral("#4CAF50"));
    }

    void getColorForSensor_humidity_highDanger()
    {
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("humidity", 85.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_humidity_highWarning()
    {
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("humidity", 65.0), QStringLiteral("#FF9800"));
    }

    void getColorForSensor_humidity_lowDanger()
    {
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("humidity", 15.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_humidity_lowWarning()
    {
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("humidity", 25.0), QStringLiteral("#FF9800"));
    }

    void getColorForSensor_humidity_normal()
    {
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("humidity", 45.0), QStringLiteral("#4CAF50"));
    }

    void getColorForSensor_partectorNumber()
    {
        QCOMPARE(m_mgr->getColorForSensor("partectorNumber", 5000), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("partectorNumber", 15000), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("partectorNumber", 55000), QStringLiteral("#F44336"));
    }

    void getColorForSensor_partectorDiam()
    {
        QCOMPARE(m_mgr->getColorForSensor("partectorDiam", 50), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("partectorDiam", 150), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("partectorDiam", 250), QStringLiteral("#F44336"));
    }

    void getColorForSensor_partectorMass()
    {
        QCOMPARE(m_mgr->getColorForSensor("partectorMass", 10.0), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("partectorMass", 30.0), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("partectorMass", 55.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_grimmValue()
    {
        QCOMPARE(m_mgr->getColorForSensor("grimmValue", 10.0), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("grimmValue", 30.0), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("grimmValue", 55.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_pressure()
    {
        m_mgr->setPressureEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("pressure", 1013.0), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("pressure", 1035.0), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("pressure", 1055.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_altitude()
    {
        m_mgr->setAltitudeEnabled(true);
        QCOMPARE(m_mgr->getColorForSensor("altitude", 100.0), QStringLiteral("#4CAF50"));
        QCOMPARE(m_mgr->getColorForSensor("altitude", 3500.0), QStringLiteral("#FF9800"));
        QCOMPARE(m_mgr->getColorForSensor("altitude", 4500.0), QStringLiteral("#F44336"));
    }

    void getColorForSensor_unknownKey()
    {
        QCOMPARE(m_mgr->getColorForSensor("unknownSensor", 999.0), QStringLiteral("#4CAF50"));
    }

    // ── isSensorEnabledForKey tests ──

    void isSensorEnabledForKey_defaults()
    {
        // Core sensors enabled
        QVERIFY(m_mgr->isSensorEnabledForKey("partectorNumber"));
        QVERIFY(m_mgr->isSensorEnabledForKey("partectorDiam"));
        QVERIFY(m_mgr->isSensorEnabledForKey("partectorMass"));
        QVERIFY(m_mgr->isSensorEnabledForKey("grimmValue"));
        QVERIFY(m_mgr->isSensorEnabledForKey("co2"));

        // Comfort sensors disabled
        QVERIFY(!m_mgr->isSensorEnabledForKey("temperature"));
        QVERIFY(!m_mgr->isSensorEnabledForKey("humidity"));
        QVERIFY(!m_mgr->isSensorEnabledForKey("pressure"));
        QVERIFY(!m_mgr->isSensorEnabledForKey("altitude"));
    }

    void isSensorEnabledForKey_unknownReturnsTrue()
    {
        QVERIFY(m_mgr->isSensorEnabledForKey("nonexistent"));
    }

    // ── resetToDefaults ──

    void resetToDefaults()
    {
        // Modify a value, then reset
        m_mgr->setCo2Warning(9999);
        QCOMPARE(m_mgr->co2Warning(), 9999);

        m_mgr->resetToDefaults();

        // resetToDefaults uses different values than constructor defaults for some fields
        QCOMPARE(m_mgr->co2Warning(), 1000);
        QCOMPARE(m_mgr->co2Danger(), 2000);
        QCOMPARE(m_mgr->partectorMassWarning(), 15.0f);  // reset default differs from constructor
        QCOMPARE(m_mgr->partectorMassDanger(), 35.5f);    // reset default differs from constructor

        // Enabled states should be restored to defaults
        QVERIFY(m_mgr->co2Enabled());
        QVERIFY(!m_mgr->temperatureEnabled());
    }

    // ── Setter signal tests ──

    void setterEmitsSignals()
    {
        QSignalSpy warningSpy(m_mgr, &ThresholdManager::co2WarningChanged);
        QSignalSpy thresholdsSpy(m_mgr, &ThresholdManager::thresholdsChanged);

        m_mgr->setCo2Warning(1500);

        QCOMPARE(warningSpy.count(), 1);
        QCOMPARE(thresholdsSpy.count(), 1);

        // Setting to same value should NOT emit
        m_mgr->setCo2Warning(1500);
        QCOMPARE(warningSpy.count(), 1);
        QCOMPARE(thresholdsSpy.count(), 1);
    }

    void setCo2Danger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::co2DangerChanged);
        QSignalSpy thresholdsSpy(m_mgr, &ThresholdManager::thresholdsChanged);

        m_mgr->setCo2Danger(3000);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(thresholdsSpy.count(), 1);
        QCOMPARE(m_mgr->co2Danger(), 3000);

        // Same value — no signal
        m_mgr->setCo2Danger(3000);
        QCOMPARE(spy.count(), 1);
    }

    void setTemperatureWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::temperatureWarningChanged);
        QSignalSpy thresholdsSpy(m_mgr, &ThresholdManager::thresholdsChanged);

        m_mgr->setTemperatureWarning(28.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(thresholdsSpy.count(), 1);
        QCOMPARE(m_mgr->temperatureWarning(), 28.0f);

        m_mgr->setTemperatureWarning(28.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setTemperatureDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::temperatureDangerChanged);
        m_mgr->setTemperatureDanger(40.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->temperatureDanger(), 40.0f);
        m_mgr->setTemperatureDanger(40.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setTemperatureLowWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::temperatureLowWarningChanged);
        m_mgr->setTemperatureLowWarning(12.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->temperatureLowWarning(), 12.0f);
        m_mgr->setTemperatureLowWarning(12.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setTemperatureLowDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::temperatureLowDangerChanged);
        m_mgr->setTemperatureLowDanger(5.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->temperatureLowDanger(), 5.0f);
        m_mgr->setTemperatureLowDanger(5.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setHumidityWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::humidityWarningChanged);
        m_mgr->setHumidityWarning(55.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->humidityWarning(), 55.0f);
        m_mgr->setHumidityWarning(55.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setHumidityDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::humidityDangerChanged);
        m_mgr->setHumidityDanger(90.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->humidityDanger(), 90.0f);
        m_mgr->setHumidityDanger(90.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setHumidityLowWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::humidityLowWarningChanged);
        m_mgr->setHumidityLowWarning(25.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->humidityLowWarning(), 25.0f);
        m_mgr->setHumidityLowWarning(25.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setHumidityLowDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::humidityLowDangerChanged);
        m_mgr->setHumidityLowDanger(15.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->humidityLowDanger(), 15.0f);
        m_mgr->setHumidityLowDanger(15.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorMassWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorMassWarningChanged);
        m_mgr->setPartectorMassWarning(20.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorMassWarning(), 20.0f);
        m_mgr->setPartectorMassWarning(20.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorMassDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorMassDangerChanged);
        m_mgr->setPartectorMassDanger(60.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorMassDanger(), 60.0f);
        m_mgr->setPartectorMassDanger(60.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setGrimmValueWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::grimmValueWarningChanged);
        m_mgr->setGrimmValueWarning(20.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->grimmValueWarning(), 20.0f);
        m_mgr->setGrimmValueWarning(20.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setGrimmValueDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::grimmValueDangerChanged);
        m_mgr->setGrimmValueDanger(60.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->grimmValueDanger(), 60.0f);
        m_mgr->setGrimmValueDanger(60.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorNumberWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorNumberWarningChanged);
        m_mgr->setPartectorNumberWarning(8000);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorNumberWarning(), 8000);
        m_mgr->setPartectorNumberWarning(8000);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorNumberDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorNumberDangerChanged);
        m_mgr->setPartectorNumberDanger(60000);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorNumberDanger(), 60000);
        m_mgr->setPartectorNumberDanger(60000);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorDiamWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorDiamWarningChanged);
        m_mgr->setPartectorDiamWarning(120);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorDiamWarning(), 120);
        m_mgr->setPartectorDiamWarning(120);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorDiamDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorDiamDangerChanged);
        m_mgr->setPartectorDiamDanger(250);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->partectorDiamDanger(), 250);
        m_mgr->setPartectorDiamDanger(250);
        QCOMPARE(spy.count(), 1);
    }

    void setPressureWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::pressureWarningChanged);
        m_mgr->setPressureWarning(1025.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->pressureWarning(), 1025.0f);
        m_mgr->setPressureWarning(1025.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setPressureDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::pressureDangerChanged);
        m_mgr->setPressureDanger(1060.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->pressureDanger(), 1060.0f);
        m_mgr->setPressureDanger(1060.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setAltitudeWarning_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::altitudeWarningChanged);
        m_mgr->setAltitudeWarning(2500.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->altitudeWarning(), 2500.0f);
        m_mgr->setAltitudeWarning(2500.0f);
        QCOMPARE(spy.count(), 1);
    }

    void setAltitudeDanger_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::altitudeDangerChanged);
        m_mgr->setAltitudeDanger(5000.0f);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(m_mgr->altitudeDanger(), 5000.0f);
        m_mgr->setAltitudeDanger(5000.0f);
        QCOMPARE(spy.count(), 1);
    }

    // ── Enabled state setter tests ──

    void setPartectorMassEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorMassEnabledChanged);
        QSignalSpy thresholdsSpy(m_mgr, &ThresholdManager::thresholdsChanged);
        m_mgr->setPartectorMassEnabled(false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(thresholdsSpy.count(), 1);
        QVERIFY(!m_mgr->partectorMassEnabled());
        m_mgr->setPartectorMassEnabled(false);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorNumberEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorNumberEnabledChanged);
        m_mgr->setPartectorNumberEnabled(false);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!m_mgr->partectorNumberEnabled());
        m_mgr->setPartectorNumberEnabled(false);
        QCOMPARE(spy.count(), 1);
    }

    void setPartectorDiamEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::partectorDiamEnabledChanged);
        m_mgr->setPartectorDiamEnabled(false);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!m_mgr->partectorDiamEnabled());
        m_mgr->setPartectorDiamEnabled(false);
        QCOMPARE(spy.count(), 1);
    }

    void setGrimmValueEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::grimmValueEnabledChanged);
        m_mgr->setGrimmValueEnabled(false);
        QCOMPARE(spy.count(), 1);
        QVERIFY(!m_mgr->grimmValueEnabled());
        m_mgr->setGrimmValueEnabled(false);
        QCOMPARE(spy.count(), 1);
    }

    void setPressureEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::pressureEnabledChanged);
        m_mgr->setPressureEnabled(true);
        QCOMPARE(spy.count(), 1);
        QVERIFY(m_mgr->pressureEnabled());
        m_mgr->setPressureEnabled(true);
        QCOMPARE(spy.count(), 1);
    }

    void setAltitudeEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::altitudeEnabledChanged);
        m_mgr->setAltitudeEnabled(true);
        QCOMPARE(spy.count(), 1);
        QVERIFY(m_mgr->altitudeEnabled());
        m_mgr->setAltitudeEnabled(true);
        QCOMPARE(spy.count(), 1);
    }

    void setHumidityEnabled_emitsSignals()
    {
        QSignalSpy spy(m_mgr, &ThresholdManager::humidityEnabledChanged);
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(spy.count(), 1);
        QVERIFY(m_mgr->humidityEnabled());
        m_mgr->setHumidityEnabled(true);
        QCOMPARE(spy.count(), 1);
    }

private:
    ThresholdManager *m_mgr = nullptr;
};

QTEST_GUILESS_MAIN(TestThresholdManager)
#include "tst_thresholdmanager.moc"
