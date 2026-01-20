#include "sensorreading.h"

SensorReading::SensorReading()
    : partectorNumber(0)
    , partectorDiam(0)
    , partectorMass(0.0f)
    , grimmValue(0.0f)
    , temperature(0.0f)
    , humidity(0.0f)
    , pressure(0.0f)
    , altitude(0.0f)
    , latitude(0.0f)
    , longitude(0.0f)
    , co2(0)
    , timestamp(QDateTime::currentDateTime())
{
}

SensorReading::SensorReading(const SensorDataRaw &raw)
    : partectorNumber(raw.partectorNumber)
    , partectorDiam(raw.partectorDiam)
    , partectorMass(raw.partectorMass)
    , grimmValue(raw.grimmValue)
    , temperature(raw.temperature)
    , humidity(raw.humidity)
    , pressure(raw.pressure)
    , altitude(raw.altitude)
    , latitude(raw.latitude)
    , longitude(raw.longitude)
    , co2(raw.co2)
    , timestamp(QDateTime::currentDateTime())
{
}

// Register metatype for signal/slot usage
static const int sensorReadingMetaTypeId = qRegisterMetaType<SensorReading>("SensorReading");
