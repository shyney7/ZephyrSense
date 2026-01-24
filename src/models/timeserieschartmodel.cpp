#include "timeserieschartmodel.h"
#include "databasemanager.h"
#include <QDebug>
#include <QVariantMap>
#include <limits>

TimeSeriesChartModel::TimeSeriesChartModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TimeSeriesChartModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.count();
}

int TimeSeriesChartModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant TimeSeriesChartModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count() || role != Qt::DisplayRole)
        return QVariant();

    const DataPoint &point = m_data.at(index.row());

    // Column 0 is timestamp
    if (index.column() == TimestampColumn) {
        return point.timestamp;
    }

    // Columns 1-9 are sensor values
    int sensorIndex = index.column() - 1;
    if (sensorIndex >= 0 && sensorIndex < 9) {
        return point.values[sensorIndex];
    }

    return QVariant();
}

void TimeSeriesChartModel::loadData(const QDateTime &start, const QDateTime &end)
{
    // Get DatabaseManager singleton from QML engine
    QQmlEngine *engine = qmlEngine(this);
    if (!engine) {
        qWarning() << "TimeSeriesChartModel: QML engine not available, cannot load data";
        return;
    }

    auto *dbManager = engine->singletonInstance<DatabaseManager*>("ZephyrSense", "DatabaseManager");
    if (!dbManager) {
        qWarning() << "TimeSeriesChartModel: DatabaseManager singleton not available";
        return;
    }

    beginResetModel();

    // Clear existing data
    m_data.clear();

    // Get readings from database
    QVariantList readings = dbManager->getReadingsInRange(start, end);
    qDebug() << "TimeSeriesChartModel: Loaded" << readings.count() << "readings from" << start << "to" << end;

    // Convert to DataPoint structs
    for (const QVariant &v : readings) {
        QVariantMap map = v.toMap();

        DataPoint point;
        point.timestamp = map["timestamp"].toDateTime().toMSecsSinceEpoch();

        // Map sensor fields to values array (9 sensors, excluding lat/lon)
        point.values[0] = map["partectorNumber"].toReal();
        point.values[1] = map["partectorDiam"].toReal();
        point.values[2] = map["partectorMass"].toReal();
        point.values[3] = map["grimmValue"].toReal();
        point.values[4] = map["temperature"].toReal();
        point.values[5] = map["humidity"].toReal();
        point.values[6] = map["pressure"].toReal();
        point.values[7] = map["altitude"].toReal();
        point.values[8] = map["co2"].toReal();

        m_data.append(point);
    }

    // Calculate bounds
    calculateBounds();

    endResetModel();

    emit boundsChanged();
    emit dataCountChanged();
}

void TimeSeriesChartModel::clear()
{
    beginResetModel();

    m_data.clear();
    m_xMin = 0;
    m_xMax = 0;
    m_yMin = 0;
    m_yMax = 0;

    endResetModel();

    emit boundsChanged();
    emit dataCountChanged();
}

void TimeSeriesChartModel::updateYBoundsForColumn(int column)
{
    if (column < PartectorNumberColumn || column >= ColumnCount) {
        qWarning() << "TimeSeriesChartModel: Invalid column for Y bounds:" << column;
        return;
    }

    m_activeColumn = column;
    calculateYBoundsForColumn(column);
    emit boundsChanged();
}

void TimeSeriesChartModel::calculateBounds()
{
    if (m_data.isEmpty()) {
        m_xMin = 0;
        m_xMax = 0;
        m_yMin = 0;
        m_yMax = 0;
        return;
    }

    // X bounds from first and last timestamp
    m_xMin = m_data.first().timestamp;
    m_xMax = m_data.last().timestamp;

    // Y bounds for active column (default: temperature)
    calculateYBoundsForColumn(m_activeColumn);
}

void TimeSeriesChartModel::calculateYBoundsForColumn(int column)
{
    if (m_data.isEmpty()) {
        m_yMin = 0;
        m_yMax = 0;
        return;
    }

    int sensorIndex = column - 1;  // Column 0 is timestamp, sensors start at 1
    if (sensorIndex < 0 || sensorIndex >= 9) {
        qWarning() << "TimeSeriesChartModel: Invalid sensor index:" << sensorIndex;
        return;
    }

    qreal minVal = std::numeric_limits<qreal>::max();
    qreal maxVal = std::numeric_limits<qreal>::lowest();

    for (const DataPoint &point : m_data) {
        qreal value = point.values[sensorIndex];
        if (value < minVal) minVal = value;
        if (value > maxVal) maxVal = value;
    }

    // Add 10% padding to Y axis for better visualization
    qreal padding = (maxVal - minVal) * 0.1;
    m_yMin = minVal - padding;
    m_yMax = maxVal + padding;

    // Ensure we have some range even if all values are the same
    if (qFuzzyCompare(m_yMin, m_yMax)) {
        m_yMin -= 1;
        m_yMax += 1;
    }
}
