#ifndef TIMESERIESCHARTMODEL_H
#define TIMESERIESCHARTMODEL_H

#include <QAbstractTableModel>
#include <QQmlEngine>
#include <QDateTime>
#include "sensorreading.h"

class TimeSeriesChartModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_ELEMENT

    // Axis bounds for QML binding
    Q_PROPERTY(qreal xMin READ xMin NOTIFY boundsChanged)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY boundsChanged)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY boundsChanged)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY boundsChanged)
    Q_PROPERTY(int dataCount READ dataCount NOTIFY dataCountChanged)

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
    int dataCount() const { return m_data.count(); }

    // QML-invokable methods
    Q_INVOKABLE void loadData(const QDateTime &start, const QDateTime &end);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void updateYBoundsForColumn(int column);

signals:
    void boundsChanged();
    void dataCountChanged();

private:
    struct DataPoint {
        qint64 timestamp;  // msecs since epoch
        qreal values[9];   // 9 sensor values
    };

    void calculateBounds();
    void calculateYBoundsForColumn(int column);

    QList<DataPoint> m_data;
    qreal m_xMin = 0;
    qreal m_xMax = 0;
    qreal m_yMin = 0;
    qreal m_yMax = 0;
    int m_activeColumn = TemperatureColumn;  // Default to temperature
};

#endif // TIMESERIESCHARTMODEL_H
