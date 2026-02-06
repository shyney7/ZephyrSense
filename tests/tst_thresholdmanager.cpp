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
        // Clear any persisted settings so the constructor gets clean defaults
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

    void computeHazardLevel_temperatureLowDanger()
    {
        m_mgr->setTemperatureEnabled(true);
        // temperatureLowDanger default = 10.0, so temp=5 should be Red
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, /*temperature=*/5.0f, 45.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

    void computeHazardLevel_humidityBidirectional()
    {
        m_mgr->setHumidityEnabled(true);
        // humidityLowDanger default = 20.0, so humidity=15 should be Red
        int level = m_mgr->computeHazardLevel(
            0, 0, 0.0f, 0.0f, 22.0f, /*humidity=*/15.0f, 1013.0f, 100.0f, 400);
        QCOMPARE(level, static_cast<int>(ThresholdManager::Red));
    }

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

private:
    ThresholdManager *m_mgr = nullptr;
};

QTEST_GUILESS_MAIN(TestThresholdManager)
#include "tst_thresholdmanager.moc"
