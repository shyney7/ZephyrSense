#include "sensorreadingmodel.h"
#include "databasemanager.h"
#include "thresholdmanager.h"
#include "serialhandler.h"
#include <QDateTime>

SensorReadingModel::SensorReadingModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SensorReadingModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_readings.count());
}

QVariant SensorReadingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_readings.count()))
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
        reading.partectorNumber = map[QStringLiteral("partectorNumber")].toInt();
        reading.partectorDiam = map[QStringLiteral("partectorDiam")].toInt();
        reading.partectorMass = map[QStringLiteral("partectorMass")].toFloat();
        reading.grimmValue = map[QStringLiteral("grimmValue")].toFloat();
        reading.temperature = map[QStringLiteral("temperature")].toFloat();
        reading.humidity = map[QStringLiteral("humidity")].toFloat();
        reading.pressure = map[QStringLiteral("pressure")].toFloat();
        reading.altitude = map[QStringLiteral("altitude")].toFloat();
        reading.latitude = map[QStringLiteral("latitude")].toFloat();
        reading.longitude = map[QStringLiteral("longitude")].toFloat();
        reading.co2 = map[QStringLiteral("co2")].toInt();
        reading.timestamp = map[QStringLiteral("timestamp")].toDateTime();

        // Only add readings with valid GPS coordinates
        if (isValidCoordinate(reading.latitude, reading.longitude)) {
            ReadingEntry entry;
            entry.id = map[QStringLiteral("id")].toInt();
            entry.reading = reading;
            m_readings.append(entry);
        }
    }

    // Sync m_nextId to avoid collisions between live reading IDs and database IDs
    qint64 maxId = 0;
    for (const auto &entry : m_readings) {
        maxId = qMax(maxId, entry.id);
    }
    if (maxId >= m_nextId) {
        m_nextId = maxId + 1;
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

    const int newRow = static_cast<int>(m_readings.count());
    beginInsertRows(QModelIndex(), newRow, newRow);
    ReadingEntry entry;
    entry.id = m_nextId++;
    entry.reading = reading;
    m_readings.append(entry);
    endInsertRows();
    emit countChanged();
}

SensorReading SensorReadingModel::readingAt(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_readings.count()))
        return m_readings.at(index).reading;
    return SensorReading();
}

QVariantMap SensorReadingModel::getReading(int index) const
{
    QVariantMap result;
    if (index < 0 || index >= static_cast<int>(m_readings.count()))
        return result;

    const SensorReading &reading = m_readings.at(index).reading;
    result[QStringLiteral("readingId")] = m_readings.at(index).id;
    result[QStringLiteral("latitude")] = reading.latitude;
    result[QStringLiteral("longitude")] = reading.longitude;
    result[QStringLiteral("partectorNumber")] = reading.partectorNumber;
    result[QStringLiteral("partectorDiam")] = reading.partectorDiam;
    result[QStringLiteral("partectorMass")] = reading.partectorMass;
    result[QStringLiteral("grimmValue")] = reading.grimmValue;
    result[QStringLiteral("temperature")] = reading.temperature;
    result[QStringLiteral("humidity")] = reading.humidity;
    result[QStringLiteral("pressure")] = reading.pressure;
    result[QStringLiteral("altitude")] = reading.altitude;
    result[QStringLiteral("co2")] = reading.co2;
    result[QStringLiteral("timestamp")] = reading.timestamp;
    return result;
}

QString SensorReadingModel::formatTooltip(const SensorReading &reading) const
{
    return QStringLiteral(
        "Time: %1\n"
        "Position: %2, %3\n"
        "Altitude: %4 m\n"
        "\n"
        "PNC UFP: %5 #/cm\u00B3\n"
        "\u00D8 UFP: %6 nm\n"
        "PM0.3: %7 \u00B5g/m\u00B3\n"
        "PNC PM: %8 #/cm\u00B3\n"
        "\n"
        "Temperature: %9 \u00B0C\n"
        "Humidity: %10 %\n"
        "Pressure: %11 hPa\n"
        "CO\u2082: %12 ppm"
    ).arg(reading.timestamp.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")))
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
        emit dataChanged(index(0), index(static_cast<int>(m_readings.count()) - 1), {HazardLevelRole});
    }
}

void SensorReadingModel::startLiveUpdates()
{
    if (m_liveUpdatesConnected)
        return;

    SerialHandler *serial = qobject_cast<SerialHandler*>(
        qmlEngine(this)->singletonInstance<SerialHandler*>("ZephyrSense", "SerialHandler")
    );

    if (serial) {
        connect(serial, &SerialHandler::newReading,
                this, &SensorReadingModel::addReading);
        m_liveUpdatesConnected = true;
    }

    // Also connect to ThresholdManager if not already
    connectToThresholdManager();
}

void SensorReadingModel::stopLiveUpdates()
{
    if (!m_liveUpdatesConnected)
        return;

    SerialHandler *serial = qobject_cast<SerialHandler*>(
        qmlEngine(this)->singletonInstance<SerialHandler*>("ZephyrSense", "SerialHandler")
    );

    if (serial) {
        disconnect(serial, &SerialHandler::newReading,
                   this, &SensorReadingModel::addReading);
        m_liveUpdatesConnected = false;
    }
}

void SensorReadingModel::pruneOldReadings(int windowMinutes)
{
    if (m_readings.isEmpty())
        return;

    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-static_cast<qint64>(windowMinutes) * 60);

    // Find index of first reading to keep
    qsizetype firstToKeep = 0;
    for (qsizetype i = 0; i < m_readings.count(); ++i) {
        if (m_readings.at(i).reading.timestamp >= cutoff) {
            firstToKeep = i;
            break;
        }
        if (i == m_readings.count() - 1) {
            // All readings are old
            firstToKeep = m_readings.count();
        }
    }

    if (firstToKeep > 0) {
        beginRemoveRows(QModelIndex(), 0, static_cast<int>(firstToKeep) - 1);
        m_readings.remove(0, firstToKeep);
        endRemoveRows();
        emit countChanged();
    }
}
