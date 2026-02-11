#include "sensorreading.h"

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
