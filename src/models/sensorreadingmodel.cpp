#include "sensorreadingmodel.h"
#include "databasemanager.h"
#include "thresholdmanager.h"
#include <QDateTime>

SensorReadingModel::SensorReadingModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SensorReadingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_readings.count();
}

QVariant SensorReadingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_readings.count())
        return QVariant();

    const ReadingEntry &entry = m_readings.at(index.row());
    const SensorReading &reading = entry.reading;

    switch (role) {
    case IdRole:
        return entry.id;
    case LatitudeRole:
        return reading.latitude;
    case LongitudeRole:
        return reading.longitude;
    case PartectorNumberRole:
        return reading.partectorNumber;
    case PartectorDiamRole:
        return reading.partectorDiam;
    case PartectorMassRole:
        return reading.partectorMass;
    case GrimmValueRole:
        return reading.grimmValue;
    case TemperatureRole:
        return reading.temperature;
    case HumidityRole:
        return reading.humidity;
    case PressureRole:
        return reading.pressure;
    case AltitudeRole:
        return reading.altitude;
    case Co2Role:
        return reading.co2;
    case TimestampRole:
        return reading.timestamp;
    case TooltipTextRole:
        return formatTooltip(reading);
    case HazardLevelRole: {
        ThresholdManager *tm = ThresholdManager::instance();
        if (tm) {
            return tm->computeHazardLevel(
                reading.partectorNumber, reading.partectorDiam,
                reading.partectorMass, reading.grimmValue,
                reading.temperature, reading.humidity,
                reading.pressure, reading.altitude, reading.co2
            );
        }
        return 0;  // Green default if manager not yet available
    }
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> SensorReadingModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "readingId";
    roles[LatitudeRole] = "latitude";
    roles[LongitudeRole] = "longitude";
    roles[PartectorNumberRole] = "partectorNumber";
    roles[PartectorDiamRole] = "partectorDiam";
    roles[PartectorMassRole] = "partectorMass";
    roles[GrimmValueRole] = "grimmValue";
    roles[TemperatureRole] = "temperature";
    roles[HumidityRole] = "humidity";
    roles[PressureRole] = "pressure";
    roles[AltitudeRole] = "altitude";
    roles[Co2Role] = "co2";
    roles[TimestampRole] = "timestamp";
    roles[TooltipTextRole] = "tooltipText";
    roles[HazardLevelRole] = "hazardLevel";
    return roles;
}

void SensorReadingModel::loadFromDatabase(const QDateTime &start, const QDateTime &end)
{
    // Get singleton instance - created by QML engine
    DatabaseManager *dbManager = qobject_cast<DatabaseManager*>(
        qmlEngine(this)->singletonInstance<DatabaseManager*>("ZephyrSense", "DatabaseManager")
    );

    if (!dbManager) {
        qWarning() << "SensorReadingModel: Could not access DatabaseManager singleton";
        return;
    }

    beginResetModel();
    m_readings.clear();

    QVariantList results = dbManager->getReadingsInRange(start, end);
    for (const QVariant &var : results) {
        QVariantMap map = var.toMap();
        SensorReading reading;
        reading.partectorNumber = map["partectorNumber"].toInt();
        reading.partectorDiam = map["partectorDiam"].toInt();
        reading.partectorMass = map["partectorMass"].toFloat();
        reading.grimmValue = map["grimmValue"].toFloat();
        reading.temperature = map["temperature"].toFloat();
        reading.humidity = map["humidity"].toFloat();
        reading.pressure = map["pressure"].toFloat();
        reading.altitude = map["altitude"].toFloat();
        reading.latitude = map["latitude"].toFloat();
        reading.longitude = map["longitude"].toFloat();
        reading.co2 = map["co2"].toInt();
        reading.timestamp = map["timestamp"].toDateTime();

        // Only add readings with valid GPS coordinates
        if (isValidCoordinate(reading.latitude, reading.longitude)) {
            ReadingEntry entry;
            entry.id = m_nextId++;
            entry.reading = reading;
            m_readings.append(entry);
        }
    }

    // Connect to ThresholdManager for live updates (instance available after QML loads)
    connectToThresholdManager();

    endResetModel();
    emit countChanged();
}

void SensorReadingModel::clear()
{
    beginResetModel();
    m_readings.clear();
    endResetModel();
    emit countChanged();
}

void SensorReadingModel::addReading(const SensorReading &reading)
{
    // Only add readings with valid GPS coordinates
    if (!isValidCoordinate(reading.latitude, reading.longitude)) {
        return;
    }

    beginInsertRows(QModelIndex(), m_readings.count(), m_readings.count());
    ReadingEntry entry;
    entry.id = m_nextId++;
    entry.reading = reading;
    m_readings.append(entry);
    endInsertRows();
    emit countChanged();
}

QVariantMap SensorReadingModel::getReading(int index) const
{
    QVariantMap result;
    if (index < 0 || index >= m_readings.count())
        return result;

    const SensorReading &reading = m_readings.at(index).reading;
    result["readingId"] = m_readings.at(index).id;
    result["latitude"] = reading.latitude;
    result["longitude"] = reading.longitude;
    result["partectorNumber"] = reading.partectorNumber;
    result["partectorDiam"] = reading.partectorDiam;
    result["partectorMass"] = reading.partectorMass;
    result["grimmValue"] = reading.grimmValue;
    result["temperature"] = reading.temperature;
    result["humidity"] = reading.humidity;
    result["pressure"] = reading.pressure;
    result["altitude"] = reading.altitude;
    result["co2"] = reading.co2;
    result["timestamp"] = reading.timestamp;
    return result;
}

QString SensorReadingModel::formatTooltip(const SensorReading &reading) const
{
    return QString(
        "Time: %1\n"
        "Position: %2, %3\n"
        "Altitude: %4 m\n"
        "\n"
        "Particles: %5 /cm3\n"
        "Diameter: %6 nm\n"
        "Mass: %7 ug/m3\n"
        "GRIMM: %8 /cm3\n"
        "\n"
        "Temperature: %9 C\n"
        "Humidity: %10 %\n"
        "Pressure: %11 hPa\n"
        "CO2: %12 ppm"
    ).arg(reading.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(reading.latitude, 0, 'f', 6)
     .arg(reading.longitude, 0, 'f', 6)
     .arg(reading.altitude, 0, 'f', 1)
     .arg(reading.partectorNumber)
     .arg(reading.partectorDiam)
     .arg(reading.partectorMass, 0, 'f', 2)
     .arg(reading.grimmValue, 0, 'f', 2)
     .arg(reading.temperature, 0, 'f', 1)
     .arg(reading.humidity, 0, 'f', 1)
     .arg(reading.pressure, 0, 'f', 1)
     .arg(reading.co2);
}

bool SensorReadingModel::isValidCoordinate(float lat, float lon) const
{
    // Valid latitude: [-90, 90], longitude: [-180, 180]
    // Also reject 0,0 as likely invalid default
    if (lat < -90.0f || lat > 90.0f)
        return false;
    if (lon < -180.0f || lon > 180.0f)
        return false;
    if (lat == 0.0f && lon == 0.0f)
        return false;
    return true;
}

void SensorReadingModel::connectToThresholdManager()
{
    if (m_thresholdManagerConnected)
        return;

    ThresholdManager *tm = ThresholdManager::instance();
    if (tm) {
        connect(tm, &ThresholdManager::thresholdsChanged,
                this, &SensorReadingModel::onThresholdsChanged);
        m_thresholdManagerConnected = true;
    }
}

void SensorReadingModel::onThresholdsChanged()
{
    if (!m_readings.isEmpty()) {
        emit dataChanged(index(0), index(m_readings.count() - 1), {HazardLevelRole});
    }
}
