#ifndef SENSORREADING_H
#define SENSORREADING_H

#include <QDateTime>
#include <QObject>
#include <cstdint>

// Raw binary struct matching embedded device protocol (46 bytes packed)
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

static_assert(sizeof(SensorDataRaw) == 42, "Struct packing mismatch!");

// Application-level sensor reading with QML integration
class SensorReading
{
    Q_GADGET
    Q_PROPERTY(int partectorNumber MEMBER partectorNumber)
    Q_PROPERTY(int partectorDiam MEMBER partectorDiam)
    Q_PROPERTY(float partectorMass MEMBER partectorMass)
    Q_PROPERTY(float grimmValue MEMBER grimmValue)
    Q_PROPERTY(float temperature MEMBER temperature)
    Q_PROPERTY(float humidity MEMBER humidity)
    Q_PROPERTY(float pressure MEMBER pressure)
    Q_PROPERTY(float altitude MEMBER altitude)
    Q_PROPERTY(float latitude MEMBER latitude)
    Q_PROPERTY(float longitude MEMBER longitude)
    Q_PROPERTY(int co2 MEMBER co2)
    Q_PROPERTY(QDateTime timestamp MEMBER timestamp)

public:
    SensorReading();
    explicit SensorReading(const SensorDataRaw &raw);

    // Sensor fields
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
    QDateTime timestamp;
};

Q_DECLARE_METATYPE(SensorReading)

#endif // SENSORREADING_H
