#include "thresholdmanager.h"
#include <QtMath>
#include <QDebug>

// Initialize static member
ThresholdManager* ThresholdManager::s_instance = nullptr;

ThresholdManager::ThresholdManager(QObject *parent)
    : QObject(parent)
    , m_settings("thresholds")
{
    // Set singleton instance
    s_instance = this;

    // Set default values before loading from settings
    m_co2Warning = 1000;
    m_co2Danger = 2000;

    m_temperatureWarning = 30.0f;
    m_temperatureDanger = 35.0f;
    m_temperatureLowWarning = 15.0f;
    m_temperatureLowDanger = 10.0f;

    m_humidityWarning = 60.0f;
    m_humidityDanger = 80.0f;
    m_humidityLowWarning = 30.0f;
    m_humidityLowDanger = 20.0f;

    m_partectorMassWarning = 25.0f;
    m_partectorMassDanger = 50.0f;

    m_grimmValueWarning = 25.0f;
    m_grimmValueDanger = 50.0f;

    m_partectorNumberWarning = 10000;
    m_partectorNumberDanger = 50000;

    m_partectorDiamWarning = 100;
    m_partectorDiamDanger = 200;

    m_pressureWarning = 1030.0f;
    m_pressureDanger = 1050.0f;

    m_altitudeWarning = 3000.0f;
    m_altitudeDanger = 4000.0f;

    // Load persisted settings (overrides defaults)
    loadSettings();

    qDebug() << "ThresholdManager initialized with CO2 warning:" << m_co2Warning << "danger:" << m_co2Danger;
}

ThresholdManager* ThresholdManager::instance()
{
    return s_instance;
}

void ThresholdManager::loadSettings()
{
    m_co2Warning = m_settings.value("co2Warning", 1000).toInt();
    m_co2Danger = m_settings.value("co2Danger", 2000).toInt();

    m_temperatureWarning = m_settings.value("temperatureWarning", 30.0).toFloat();
    m_temperatureDanger = m_settings.value("temperatureDanger", 35.0).toFloat();
    m_temperatureLowWarning = m_settings.value("temperatureLowWarning", 15.0).toFloat();
    m_temperatureLowDanger = m_settings.value("temperatureLowDanger", 10.0).toFloat();

    m_humidityWarning = m_settings.value("humidityWarning", 60.0).toFloat();
    m_humidityDanger = m_settings.value("humidityDanger", 80.0).toFloat();
    m_humidityLowWarning = m_settings.value("humidityLowWarning", 30.0).toFloat();
    m_humidityLowDanger = m_settings.value("humidityLowDanger", 20.0).toFloat();

    m_partectorMassWarning = m_settings.value("partectorMassWarning", 25.0).toFloat();
    m_partectorMassDanger = m_settings.value("partectorMassDanger", 50.0).toFloat();

    m_grimmValueWarning = m_settings.value("grimmValueWarning", 25.0).toFloat();
    m_grimmValueDanger = m_settings.value("grimmValueDanger", 50.0).toFloat();

    m_partectorNumberWarning = m_settings.value("partectorNumberWarning", 10000).toInt();
    m_partectorNumberDanger = m_settings.value("partectorNumberDanger", 50000).toInt();

    m_partectorDiamWarning = m_settings.value("partectorDiamWarning", 100).toInt();
    m_partectorDiamDanger = m_settings.value("partectorDiamDanger", 200).toInt();

    m_pressureWarning = m_settings.value("pressureWarning", 1030.0).toFloat();
    m_pressureDanger = m_settings.value("pressureDanger", 1050.0).toFloat();

    m_altitudeWarning = m_settings.value("altitudeWarning", 3000.0).toFloat();
    m_altitudeDanger = m_settings.value("altitudeDanger", 4000.0).toFloat();
}

void ThresholdManager::saveSettings()
{
    m_settings.setValue("co2Warning", m_co2Warning);
    m_settings.setValue("co2Danger", m_co2Danger);

    m_settings.setValue("temperatureWarning", m_temperatureWarning);
    m_settings.setValue("temperatureDanger", m_temperatureDanger);
    m_settings.setValue("temperatureLowWarning", m_temperatureLowWarning);
    m_settings.setValue("temperatureLowDanger", m_temperatureLowDanger);

    m_settings.setValue("humidityWarning", m_humidityWarning);
    m_settings.setValue("humidityDanger", m_humidityDanger);
    m_settings.setValue("humidityLowWarning", m_humidityLowWarning);
    m_settings.setValue("humidityLowDanger", m_humidityLowDanger);

    m_settings.setValue("partectorMassWarning", m_partectorMassWarning);
    m_settings.setValue("partectorMassDanger", m_partectorMassDanger);

    m_settings.setValue("grimmValueWarning", m_grimmValueWarning);
    m_settings.setValue("grimmValueDanger", m_grimmValueDanger);

    m_settings.setValue("partectorNumberWarning", m_partectorNumberWarning);
    m_settings.setValue("partectorNumberDanger", m_partectorNumberDanger);

    m_settings.setValue("partectorDiamWarning", m_partectorDiamWarning);
    m_settings.setValue("partectorDiamDanger", m_partectorDiamDanger);

    m_settings.setValue("pressureWarning", m_pressureWarning);
    m_settings.setValue("pressureDanger", m_pressureDanger);

    m_settings.setValue("altitudeWarning", m_altitudeWarning);
    m_settings.setValue("altitudeDanger", m_altitudeDanger);

    m_settings.sync();
}

// CO2 setters
void ThresholdManager::setCo2Warning(int value)
{
    if (m_co2Warning != value) {
        m_co2Warning = value;
        saveSettings();
        emit co2WarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setCo2Danger(int value)
{
    if (m_co2Danger != value) {
        m_co2Danger = value;
        saveSettings();
        emit co2DangerChanged();
        emit thresholdsChanged();
    }
}

// Temperature setters
void ThresholdManager::setTemperatureWarning(float value)
{
    if (!qFuzzyCompare(m_temperatureWarning, value)) {
        m_temperatureWarning = value;
        saveSettings();
        emit temperatureWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setTemperatureDanger(float value)
{
    if (!qFuzzyCompare(m_temperatureDanger, value)) {
        m_temperatureDanger = value;
        saveSettings();
        emit temperatureDangerChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setTemperatureLowWarning(float value)
{
    if (!qFuzzyCompare(m_temperatureLowWarning, value)) {
        m_temperatureLowWarning = value;
        saveSettings();
        emit temperatureLowWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setTemperatureLowDanger(float value)
{
    if (!qFuzzyCompare(m_temperatureLowDanger, value)) {
        m_temperatureLowDanger = value;
        saveSettings();
        emit temperatureLowDangerChanged();
        emit thresholdsChanged();
    }
}

// Humidity setters
void ThresholdManager::setHumidityWarning(float value)
{
    if (!qFuzzyCompare(m_humidityWarning, value)) {
        m_humidityWarning = value;
        saveSettings();
        emit humidityWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setHumidityDanger(float value)
{
    if (!qFuzzyCompare(m_humidityDanger, value)) {
        m_humidityDanger = value;
        saveSettings();
        emit humidityDangerChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setHumidityLowWarning(float value)
{
    if (!qFuzzyCompare(m_humidityLowWarning, value)) {
        m_humidityLowWarning = value;
        saveSettings();
        emit humidityLowWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setHumidityLowDanger(float value)
{
    if (!qFuzzyCompare(m_humidityLowDanger, value)) {
        m_humidityLowDanger = value;
        saveSettings();
        emit humidityLowDangerChanged();
        emit thresholdsChanged();
    }
}

// PartectorMass setters
void ThresholdManager::setPartectorMassWarning(float value)
{
    if (!qFuzzyCompare(m_partectorMassWarning, value)) {
        m_partectorMassWarning = value;
        saveSettings();
        emit partectorMassWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setPartectorMassDanger(float value)
{
    if (!qFuzzyCompare(m_partectorMassDanger, value)) {
        m_partectorMassDanger = value;
        saveSettings();
        emit partectorMassDangerChanged();
        emit thresholdsChanged();
    }
}

// GrimmValue setters
void ThresholdManager::setGrimmValueWarning(float value)
{
    if (!qFuzzyCompare(m_grimmValueWarning, value)) {
        m_grimmValueWarning = value;
        saveSettings();
        emit grimmValueWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setGrimmValueDanger(float value)
{
    if (!qFuzzyCompare(m_grimmValueDanger, value)) {
        m_grimmValueDanger = value;
        saveSettings();
        emit grimmValueDangerChanged();
        emit thresholdsChanged();
    }
}

// PartectorNumber setters
void ThresholdManager::setPartectorNumberWarning(int value)
{
    if (m_partectorNumberWarning != value) {
        m_partectorNumberWarning = value;
        saveSettings();
        emit partectorNumberWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setPartectorNumberDanger(int value)
{
    if (m_partectorNumberDanger != value) {
        m_partectorNumberDanger = value;
        saveSettings();
        emit partectorNumberDangerChanged();
        emit thresholdsChanged();
    }
}

// PartectorDiam setters
void ThresholdManager::setPartectorDiamWarning(int value)
{
    if (m_partectorDiamWarning != value) {
        m_partectorDiamWarning = value;
        saveSettings();
        emit partectorDiamWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setPartectorDiamDanger(int value)
{
    if (m_partectorDiamDanger != value) {
        m_partectorDiamDanger = value;
        saveSettings();
        emit partectorDiamDangerChanged();
        emit thresholdsChanged();
    }
}

// Pressure setters
void ThresholdManager::setPressureWarning(float value)
{
    if (!qFuzzyCompare(m_pressureWarning, value)) {
        m_pressureWarning = value;
        saveSettings();
        emit pressureWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setPressureDanger(float value)
{
    if (!qFuzzyCompare(m_pressureDanger, value)) {
        m_pressureDanger = value;
        saveSettings();
        emit pressureDangerChanged();
        emit thresholdsChanged();
    }
}

// Altitude setters
void ThresholdManager::setAltitudeWarning(float value)
{
    if (!qFuzzyCompare(m_altitudeWarning, value)) {
        m_altitudeWarning = value;
        saveSettings();
        emit altitudeWarningChanged();
        emit thresholdsChanged();
    }
}

void ThresholdManager::setAltitudeDanger(float value)
{
    if (!qFuzzyCompare(m_altitudeDanger, value)) {
        m_altitudeDanger = value;
        saveSettings();
        emit altitudeDangerChanged();
        emit thresholdsChanged();
    }
}

// Compute hazard level based on all sensor values
int ThresholdManager::computeHazardLevel(int partectorNumber, int partectorDiam,
                                          float partectorMass, float grimmValue,
                                          float temperature, float humidity,
                                          float pressure, float altitude, int co2)
{
    int maxLevel = Green;

    // Check CO2
    if (co2 >= m_co2Danger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (co2 >= m_co2Warning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check Temperature (both high and low)
    if (temperature >= m_temperatureDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (temperature >= m_temperatureWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }
    // Temperature LOW thresholds (inverted - lower is worse)
    if (temperature <= m_temperatureLowDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (temperature <= m_temperatureLowWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check Humidity (both high and low)
    if (humidity >= m_humidityDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (humidity >= m_humidityWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }
    // Humidity LOW thresholds (inverted - lower is worse)
    if (humidity <= m_humidityLowDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (humidity <= m_humidityLowWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check PartectorMass
    if (partectorMass >= m_partectorMassDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (partectorMass >= m_partectorMassWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check GrimmValue
    if (grimmValue >= m_grimmValueDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (grimmValue >= m_grimmValueWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check PartectorNumber
    if (partectorNumber >= m_partectorNumberDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (partectorNumber >= m_partectorNumberWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check PartectorDiam
    if (partectorDiam >= m_partectorDiamDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (partectorDiam >= m_partectorDiamWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check Pressure
    if (pressure >= m_pressureDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (pressure >= m_pressureWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    // Check Altitude
    if (altitude >= m_altitudeDanger) {
        maxLevel = qMax(maxLevel, static_cast<int>(Red));
    } else if (altitude >= m_altitudeWarning) {
        maxLevel = qMax(maxLevel, static_cast<int>(Yellow));
    }

    return maxLevel;
}
