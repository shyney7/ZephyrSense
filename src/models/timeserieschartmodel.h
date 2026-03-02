#ifndef TIMESERIESCHARTMODEL_H
#define TIMESERIESCHARTMODEL_H

#include <QAbstractTableModel>
#include <QQmlEngine>
#include <QDateTime>
#include <QPointF>
#include <array>
#include <utility>
#include "sensorreading.h"

class TimeSeriesChartModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    // Axis bounds for QML binding
    Q_PROPERTY(qreal xMin READ xMin NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY boundsChanged FINAL)
    Q_PROPERTY(int dataCount READ dataCount NOTIFY dataCountChanged FINAL)

public:
    // Column indices - timestamp first, then 9 sensors (excluding lat/lon)
    enum Columns {
        TimestampColumn = 0,
        PartectorNumberColumn,
        PartectorDiamColumn,
        PartectorMassColumn,
        GrimmValueColumn,
        TemperatureColumn,
        HumidityColumn,
        PressureColumn,
        AltitudeColumn,
        Co2Column,
        ColumnCount
    };
    Q_ENUM(Columns)

    explicit TimeSeriesChartModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Property getters
    qreal xMin() const { return m_xMin; }
    qreal xMax() const { return m_xMax; }
    qreal yMin() const { return m_yMin; }
    qreal yMax() const { return m_yMax; }
    int dataCount() const { return static_cast<int>(m_data.count()); }

    // QML-invokable methods
    Q_INVOKABLE void loadData(const QDateTime &start, const QDateTime &end);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void updateYBoundsForColumn(int column);
    // Returns per-column Y bounds as QPointF (x = paddedMin, y = paddedMax).
    // Returns (0.0, 100.0) for invalid columns or empty model data.
    Q_INVOKABLE QPointF getYBoundsForColumn(int column) const;
    Q_INVOKABLE qreal getYMinForColumn(int column) const;
    Q_INVOKABLE qreal getYMaxForColumn(int column) const;

signals:
    void boundsChanged();
    void dataCountChanged();

#ifdef ZEPHYR_TESTING
    friend class TestTimeSeriesChartModel;
#endif

private:
    struct DataPoint {
        qint64 timestamp;                // msecs since epoch
        std::array<qreal, 9> values{};   // 9 sensor values
    };

    void calculateBounds();
    void calculateYBoundsForColumn(int column);
    void invalidateBoundsCache();
    std::pair<qreal, qreal> boundsForColumn(int column) const;
    void rebuildBoundsCacheIfNeeded() const;

    QList<DataPoint> m_data;
    qreal m_xMin = 0;
    qreal m_xMax = 0;
    qreal m_yMin = 0;
    qreal m_yMax = 0;
    int m_activeColumn = TemperatureColumn;  // Default to temperature
    mutable std::array<std::pair<qreal, qreal>, 9> m_cachedBounds{};
    mutable bool m_boundsCacheValid = false;
};

#endif // TIMESERIESCHARTMODEL_H
