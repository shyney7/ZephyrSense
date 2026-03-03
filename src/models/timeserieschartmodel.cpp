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
    return static_cast<int>(m_data.count());
}

int TimeSeriesChartModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return ColumnCount;
}

QVariant TimeSeriesChartModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_data.count()) || role != Qt::DisplayRole)
        return QVariant();

    const DataPoint &point = m_data.at(index.row());

    // Column 0 is timestamp
    if (index.column() == TimestampColumn) {
        return point.timestamp;
    }

    // Columns 1-9 are sensor values
    int sensorIndex = index.column() - 1;
    if (sensorIndex >= 0 && sensorIndex < 9) {
        return point.values[static_cast<size_t>(sensorIndex)];
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
        point.timestamp = map[QStringLiteral("timestamp")].toDateTime().toMSecsSinceEpoch();

        // Map sensor fields to values array (9 sensors, excluding lat/lon)
        point.values[0] = map[QStringLiteral("partectorNumber")].toReal();
        point.values[1] = map[QStringLiteral("partectorDiam")].toReal();
        point.values[2] = map[QStringLiteral("partectorMass")].toReal();
        point.values[3] = map[QStringLiteral("grimmValue")].toReal();
        point.values[4] = map[QStringLiteral("temperature")].toReal();
        point.values[5] = map[QStringLiteral("humidity")].toReal();
        point.values[6] = map[QStringLiteral("pressure")].toReal();
        point.values[7] = map[QStringLiteral("altitude")].toReal();
        point.values[8] = map[QStringLiteral("co2")].toReal();

        m_data.append(point);
    }

    invalidateBoundsCache();

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
    m_yMax = 100;
    invalidateBoundsCache();

    endResetModel();

    emit boundsChanged();
    emit dataCountChanged();
}

void TimeSeriesChartModel::setActiveColumn(int column)
{
    if (column < PartectorNumberColumn || column >= ColumnCount)
        return;
    if (m_activeColumn == column)
        return;
    m_activeColumn = column;
    calculateYBoundsForColumn(column);
    emit boundsChanged();
    emit activeColumnChanged();
}

QPointF TimeSeriesChartModel::getYBoundsForColumn(int column) const
{
    auto [yMin, yMax] = boundsForColumn(column);
    return QPointF(yMin, yMax);
}

qreal TimeSeriesChartModel::getYMinForColumn(int column) const
{
    return boundsForColumn(column).first;
}

qreal TimeSeriesChartModel::getYMaxForColumn(int column) const
{
    return boundsForColumn(column).second;
}

void TimeSeriesChartModel::calculateBounds()
{
    if (m_data.isEmpty()) {
        m_xMin = 0;
        m_xMax = 0;
        m_yMin = 0;
        m_yMax = 100;
        return;
    }

    // X bounds from first and last timestamp
    m_xMin = static_cast<qreal>(m_data.constFirst().timestamp);
    m_xMax = static_cast<qreal>(m_data.constLast().timestamp);

    // Y bounds for active column (default: temperature)
    calculateYBoundsForColumn(m_activeColumn);
}

void TimeSeriesChartModel::calculateYBoundsForColumn(int column)
{
    if (m_data.isEmpty()) {
        m_yMin = 0;
        m_yMax = 100;
        return;
    }

    auto [yMin, yMax] = boundsForColumn(column);
    m_yMin = yMin;
    m_yMax = yMax;
}

void TimeSeriesChartModel::invalidateBoundsCache()
{
    m_boundsCacheValid = false;
}

std::pair<qreal, qreal> TimeSeriesChartModel::boundsForColumn(int column) const
{
    if (column < PartectorNumberColumn || column >= ColumnCount) {
        return {0.0, 100.0};
    }

    rebuildBoundsCacheIfNeeded();
    return m_cachedBounds[static_cast<size_t>(column - PartectorNumberColumn)];
}

void TimeSeriesChartModel::rebuildBoundsCacheIfNeeded() const
{
    if (m_boundsCacheValid) {
        return;
    }

    if (m_data.isEmpty()) {
        m_cachedBounds.fill({0.0, 100.0});
        m_boundsCacheValid = true;
        return;
    }

    std::array<qreal, 9> minVals;
    std::array<qreal, 9> maxVals;
    minVals.fill(std::numeric_limits<qreal>::max());
    maxVals.fill(std::numeric_limits<qreal>::lowest());

    for (const DataPoint &point : m_data) {
        for (size_t i = 0; i < point.values.size(); ++i) {
            minVals[i] = qMin(minVals[i], point.values[i]);
            maxVals[i] = qMax(maxVals[i], point.values[i]);
        }
    }

    for (size_t i = 0; i < m_cachedBounds.size(); ++i) {
        const qreal minVal = minVals[i];
        const qreal maxVal = maxVals[i];

        // Add 10% padding to Y axis for better visualization.
        qreal padding = (maxVal - minVal) * 0.1;
        qreal paddedMin = minVal - padding;
        qreal paddedMax = maxVal + padding;

        // Ensure we have some range even if all values are the same.
        if (qFuzzyCompare(paddedMin, paddedMax)) {
            paddedMin = minVal - 1.0;
            paddedMax = maxVal + 1.0;
        }

        m_cachedBounds[i] = {paddedMin, paddedMax};
    }

    m_boundsCacheValid = true;
}
