#ifndef THRESHOLDMANAGER_H
#define THRESHOLDMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

class ThresholdManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // HazardLevel enum
    Q_PROPERTY(int co2Warning READ co2Warning WRITE setCo2Warning NOTIFY co2WarningChanged)
    Q_PROPERTY(int co2Danger READ co2Danger WRITE setCo2Danger NOTIFY co2DangerChanged)

    Q_PROPERTY(float temperatureWarning READ temperatureWarning WRITE setTemperatureWarning NOTIFY temperatureWarningChanged)
    Q_PROPERTY(float temperatureDanger READ temperatureDanger WRITE setTemperatureDanger NOTIFY temperatureDangerChanged)
    Q_PROPERTY(float temperatureLowWarning READ temperatureLowWarning WRITE setTemperatureLowWarning NOTIFY temperatureLowWarningChanged)
    Q_PROPERTY(float temperatureLowDanger READ temperatureLowDanger WRITE setTemperatureLowDanger NOTIFY temperatureLowDangerChanged)

    Q_PROPERTY(float humidityWarning READ humidityWarning WRITE setHumidityWarning NOTIFY humidityWarningChanged)
    Q_PROPERTY(float humidityDanger READ humidityDanger WRITE setHumidityDanger NOTIFY humidityDangerChanged)
    Q_PROPERTY(float humidityLowWarning READ humidityLowWarning WRITE setHumidityLowWarning NOTIFY humidityLowWarningChanged)
    Q_PROPERTY(float humidityLowDanger READ humidityLowDanger WRITE setHumidityLowDanger NOTIFY humidityLowDangerChanged)

    Q_PROPERTY(float partectorMassWarning READ partectorMassWarning WRITE setPartectorMassWarning NOTIFY partectorMassWarningChanged)
    Q_PROPERTY(float partectorMassDanger READ partectorMassDanger WRITE setPartectorMassDanger NOTIFY partectorMassDangerChanged)

    Q_PROPERTY(float grimmValueWarning READ grimmValueWarning WRITE setGrimmValueWarning NOTIFY grimmValueWarningChanged)
    Q_PROPERTY(float grimmValueDanger READ grimmValueDanger WRITE setGrimmValueDanger NOTIFY grimmValueDangerChanged)

    Q_PROPERTY(int partectorNumberWarning READ partectorNumberWarning WRITE setPartectorNumberWarning NOTIFY partectorNumberWarningChanged)
    Q_PROPERTY(int partectorNumberDanger READ partectorNumberDanger WRITE setPartectorNumberDanger NOTIFY partectorNumberDangerChanged)

    Q_PROPERTY(int partectorDiamWarning READ partectorDiamWarning WRITE setPartectorDiamWarning NOTIFY partectorDiamWarningChanged)
    Q_PROPERTY(int partectorDiamDanger READ partectorDiamDanger WRITE setPartectorDiamDanger NOTIFY partectorDiamDangerChanged)

    Q_PROPERTY(float pressureWarning READ pressureWarning WRITE setPressureWarning NOTIFY pressureWarningChanged)
    Q_PROPERTY(float pressureDanger READ pressureDanger WRITE setPressureDanger NOTIFY pressureDangerChanged)

    Q_PROPERTY(float altitudeWarning READ altitudeWarning WRITE setAltitudeWarning NOTIFY altitudeWarningChanged)
    Q_PROPERTY(float altitudeDanger READ altitudeDanger WRITE setAltitudeDanger NOTIFY altitudeDangerChanged)

    // Sensor enable/disable properties (for selective hazard calculation)
    Q_PROPERTY(bool partectorMassEnabled READ partectorMassEnabled WRITE setPartectorMassEnabled NOTIFY partectorMassEnabledChanged)
    Q_PROPERTY(bool partectorNumberEnabled READ partectorNumberEnabled WRITE setPartectorNumberEnabled NOTIFY partectorNumberEnabledChanged)
    Q_PROPERTY(bool partectorDiamEnabled READ partectorDiamEnabled WRITE setPartectorDiamEnabled NOTIFY partectorDiamEnabledChanged)
    Q_PROPERTY(bool grimmValueEnabled READ grimmValueEnabled WRITE setGrimmValueEnabled NOTIFY grimmValueEnabledChanged)
    Q_PROPERTY(bool co2Enabled READ co2Enabled WRITE setCo2Enabled NOTIFY co2EnabledChanged)
    Q_PROPERTY(bool temperatureEnabled READ temperatureEnabled WRITE setTemperatureEnabled NOTIFY temperatureEnabledChanged)
    Q_PROPERTY(bool humidityEnabled READ humidityEnabled WRITE setHumidityEnabled NOTIFY humidityEnabledChanged)
    Q_PROPERTY(bool pressureEnabled READ pressureEnabled WRITE setPressureEnabled NOTIFY pressureEnabledChanged)
    Q_PROPERTY(bool altitudeEnabled READ altitudeEnabled WRITE setAltitudeEnabled NOTIFY altitudeEnabledChanged)

public:
    enum HazardLevel {
        Green = 0,
        Yellow = 1,
        Red = 2
    };
    Q_ENUM(HazardLevel)

    explicit ThresholdManager(QObject *parent = nullptr);

    // Static instance access for C++
    static ThresholdManager* instance();

    // CO2 getters/setters
    int co2Warning() const { return m_co2Warning; }
    void setCo2Warning(int value);
    int co2Danger() const { return m_co2Danger; }
    void setCo2Danger(int value);

    // Temperature getters/setters
    float temperatureWarning() const { return m_temperatureWarning; }
    void setTemperatureWarning(float value);
    float temperatureDanger() const { return m_temperatureDanger; }
    void setTemperatureDanger(float value);
    float temperatureLowWarning() const { return m_temperatureLowWarning; }
    void setTemperatureLowWarning(float value);
    float temperatureLowDanger() const { return m_temperatureLowDanger; }
    void setTemperatureLowDanger(float value);

    // Humidity getters/setters
    float humidityWarning() const { return m_humidityWarning; }
    void setHumidityWarning(float value);
    float humidityDanger() const { return m_humidityDanger; }
    void setHumidityDanger(float value);
    float humidityLowWarning() const { return m_humidityLowWarning; }
    void setHumidityLowWarning(float value);
    float humidityLowDanger() const { return m_humidityLowDanger; }
    void setHumidityLowDanger(float value);

    // PartectorMass getters/setters
    float partectorMassWarning() const { return m_partectorMassWarning; }
    void setPartectorMassWarning(float value);
    float partectorMassDanger() const { return m_partectorMassDanger; }
    void setPartectorMassDanger(float value);

    // GrimmValue getters/setters
    float grimmValueWarning() const { return m_grimmValueWarning; }
    void setGrimmValueWarning(float value);
    float grimmValueDanger() const { return m_grimmValueDanger; }
    void setGrimmValueDanger(float value);

    // PartectorNumber getters/setters
    int partectorNumberWarning() const { return m_partectorNumberWarning; }
    void setPartectorNumberWarning(int value);
    int partectorNumberDanger() const { return m_partectorNumberDanger; }
    void setPartectorNumberDanger(int value);

    // PartectorDiam getters/setters
    int partectorDiamWarning() const { return m_partectorDiamWarning; }
    void setPartectorDiamWarning(int value);
    int partectorDiamDanger() const { return m_partectorDiamDanger; }
    void setPartectorDiamDanger(int value);

    // Pressure getters/setters
    float pressureWarning() const { return m_pressureWarning; }
    void setPressureWarning(float value);
    float pressureDanger() const { return m_pressureDanger; }
    void setPressureDanger(float value);

    // Altitude getters/setters
    float altitudeWarning() const { return m_altitudeWarning; }
    void setAltitudeWarning(float value);
    float altitudeDanger() const { return m_altitudeDanger; }
    void setAltitudeDanger(float value);

    // Sensor enabled getters/setters
    bool partectorMassEnabled() const { return m_partectorMassEnabled; }
    void setPartectorMassEnabled(bool value);
    bool partectorNumberEnabled() const { return m_partectorNumberEnabled; }
    void setPartectorNumberEnabled(bool value);
    bool partectorDiamEnabled() const { return m_partectorDiamEnabled; }
    void setPartectorDiamEnabled(bool value);
    bool grimmValueEnabled() const { return m_grimmValueEnabled; }
    void setGrimmValueEnabled(bool value);
    bool co2Enabled() const { return m_co2Enabled; }
    void setCo2Enabled(bool value);
    bool temperatureEnabled() const { return m_temperatureEnabled; }
    void setTemperatureEnabled(bool value);
    bool humidityEnabled() const { return m_humidityEnabled; }
    void setHumidityEnabled(bool value);
    bool pressureEnabled() const { return m_pressureEnabled; }
    void setPressureEnabled(bool value);
    bool altitudeEnabled() const { return m_altitudeEnabled; }
    void setAltitudeEnabled(bool value);

    // Compute hazard level for a complete sensor reading
    Q_INVOKABLE int computeHazardLevel(int partectorNumber, int partectorDiam,
                                       float partectorMass, float grimmValue,
                                       float temperature, float humidity,
                                       float pressure, float altitude, int co2);

    // Reset all thresholds and enabled states to defaults
    Q_INVOKABLE void resetToDefaults();

signals:
    void co2WarningChanged();
    void co2DangerChanged();
    void temperatureWarningChanged();
    void temperatureDangerChanged();
    void temperatureLowWarningChanged();
    void temperatureLowDangerChanged();
    void humidityWarningChanged();
    void humidityDangerChanged();
    void humidityLowWarningChanged();
    void humidityLowDangerChanged();
    void partectorMassWarningChanged();
    void partectorMassDangerChanged();
    void grimmValueWarningChanged();
    void grimmValueDangerChanged();
    void partectorNumberWarningChanged();
    void partectorNumberDangerChanged();
    void partectorDiamWarningChanged();
    void partectorDiamDangerChanged();
    void pressureWarningChanged();
    void pressureDangerChanged();
    void altitudeWarningChanged();
    void altitudeDangerChanged();

    void partectorMassEnabledChanged();
    void partectorNumberEnabledChanged();
    void partectorDiamEnabledChanged();
    void grimmValueEnabledChanged();
    void co2EnabledChanged();
    void temperatureEnabledChanged();
    void humidityEnabledChanged();
    void pressureEnabledChanged();
    void altitudeEnabledChanged();

    void thresholdsChanged();

private:
    void loadSettings();
    void saveSettings();

    static ThresholdManager* s_instance;

    QSettings m_settings;

    // Member variables for all thresholds
    int m_co2Warning;
    int m_co2Danger;

    float m_temperatureWarning;
    float m_temperatureDanger;
    float m_temperatureLowWarning;
    float m_temperatureLowDanger;

    float m_humidityWarning;
    float m_humidityDanger;
    float m_humidityLowWarning;
    float m_humidityLowDanger;

    float m_partectorMassWarning;
    float m_partectorMassDanger;

    float m_grimmValueWarning;
    float m_grimmValueDanger;

    int m_partectorNumberWarning;
    int m_partectorNumberDanger;

    int m_partectorDiamWarning;
    int m_partectorDiamDanger;

    float m_pressureWarning;
    float m_pressureDanger;

    float m_altitudeWarning;
    float m_altitudeDanger;

    // Sensor enabled flags (default: core sensors true, comfort sensors false)
    bool m_partectorMassEnabled;
    bool m_partectorNumberEnabled;
    bool m_partectorDiamEnabled;
    bool m_grimmValueEnabled;
    bool m_co2Enabled;
    bool m_temperatureEnabled;
    bool m_humidityEnabled;
    bool m_pressureEnabled;
    bool m_altitudeEnabled;
};

#endif // THRESHOLDMANAGER_H
