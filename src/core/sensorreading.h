#pragma once

#include <QDateTime>
#include <QObject>
#include <QtQmlIntegration>
#include <cstdint>

// Raw binary struct matching embedded device protocol (42 bytes packed)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-pack"
#pragma pack(push, 1)
struct SensorDataRaw {
    int32_t partectorNumber;  // 4 bytes - particle count (parts/cm3)
    int32_t partectorDiam;    // 4 bytes - diameter (nm)
    float partectorMass;      // 4 bytes - mass concentration (ug/m3)
    float grimmValue;         // 4 bytes - particles/cm3
    float temperature;        // 4 bytes - Celsius
    float humidity;           // 4 bytes - percent
    float pressure;           // 4 bytes - hPa
    float altitude;           // 4 bytes - meters
    float latitude;           // 4 bytes - degrees
    float longitude;          // 4 bytes - degrees
    uint16_t co2;             // 2 bytes - ppm
};                            // Total: 42 bytes
#pragma pack(pop)
#pragma clang diagnostic pop

static_assert(sizeof(SensorDataRaw) == 42, "Struct packing mismatch!");

// Application-level sensor reading with QML integration
class SensorReading
{
    Q_GADGET
    QML_VALUE_TYPE(sensorReading)
    Q_PROPERTY(QDateTime timestamp MEMBER timestamp FINAL)
    Q_PROPERTY(int partectorNumber MEMBER partectorNumber FINAL)
    Q_PROPERTY(int partectorDiam MEMBER partectorDiam FINAL)
    Q_PROPERTY(float partectorMass MEMBER partectorMass FINAL)
    Q_PROPERTY(float grimmValue MEMBER grimmValue FINAL)
    Q_PROPERTY(float temperature MEMBER temperature FINAL)
    Q_PROPERTY(float humidity MEMBER humidity FINAL)
    Q_PROPERTY(float pressure MEMBER pressure FINAL)
    Q_PROPERTY(float altitude MEMBER altitude FINAL)
    Q_PROPERTY(float latitude MEMBER latitude FINAL)
    Q_PROPERTY(float longitude MEMBER longitude FINAL)
    Q_PROPERTY(int co2 MEMBER co2 FINAL)

public:
    SensorReading() = default;
    explicit SensorReading(const SensorDataRaw &raw);

    // Sensor fields — ordered largest-alignment first to avoid internal padding
    QDateTime timestamp = QDateTime::currentDateTime();
    int partectorNumber = 0;
    int partectorDiam = 0;
    float partectorMass = 0.0f;
    float grimmValue = 0.0f;
    float temperature = 0.0f;
    float humidity = 0.0f;
    float pressure = 0.0f;
    float altitude = 0.0f;
    float latitude = 0.0f;
    float longitude = 0.0f;
    int co2 = 0;
};

Q_DECLARE_METATYPE(SensorReading)

