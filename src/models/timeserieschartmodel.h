#pragma once

#include <QAbstractTableModel>
#include <QQmlEngine>
#include <QDateTime>
#include <QPointF>
#include <array>
#include <utility>

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
    Q_PROPERTY(int activeColumn READ activeColumn WRITE setActiveColumn NOTIFY activeColumnChanged FINAL)

    // Per-column Y bounds as QPointF (x = paddedMin, y = paddedMax) — FINAL for qmllint
    Q_PROPERTY(QPointF boundsCol1 READ boundsCol1 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol2 READ boundsCol2 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol3 READ boundsCol3 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol4 READ boundsCol4 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol5 READ boundsCol5 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol6 READ boundsCol6 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol7 READ boundsCol7 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol8 READ boundsCol8 NOTIFY boundsChanged FINAL)
    Q_PROPERTY(QPointF boundsCol9 READ boundsCol9 NOTIFY boundsChanged FINAL)

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
    int activeColumn() const { return m_activeColumn; }
    void setActiveColumn(int column);

    // Per-column bounds getters (1-indexed to match Columns enum)
    QPointF boundsCol1() const { auto b = boundsForColumn(1); return QPointF(b.first, b.second); }
    QPointF boundsCol2() const { auto b = boundsForColumn(2); return QPointF(b.first, b.second); }
    QPointF boundsCol3() const { auto b = boundsForColumn(3); return QPointF(b.first, b.second); }
    QPointF boundsCol4() const { auto b = boundsForColumn(4); return QPointF(b.first, b.second); }
    QPointF boundsCol5() const { auto b = boundsForColumn(5); return QPointF(b.first, b.second); }
    QPointF boundsCol6() const { auto b = boundsForColumn(6); return QPointF(b.first, b.second); }
    QPointF boundsCol7() const { auto b = boundsForColumn(7); return QPointF(b.first, b.second); }
    QPointF boundsCol8() const { auto b = boundsForColumn(8); return QPointF(b.first, b.second); }
    QPointF boundsCol9() const { auto b = boundsForColumn(9); return QPointF(b.first, b.second); }

    // QML-invokable methods
    Q_INVOKABLE void loadData(const QDateTime &start, const QDateTime &end);
    Q_INVOKABLE void clear();
    // Returns per-column Y bounds as QPointF (x = paddedMin, y = paddedMax).
    // Returns (0.0, 100.0) for invalid columns or empty model data.
    Q_INVOKABLE QPointF getYBoundsForColumn(int column) const;
    Q_INVOKABLE qreal getYMinForColumn(int column) const;
    Q_INVOKABLE qreal getYMaxForColumn(int column) const;

signals:
    void boundsChanged();
    void dataCountChanged();
    void activeColumnChanged();

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
    qreal m_yMax = 100;
    int m_activeColumn = TemperatureColumn;  // Default to temperature
    mutable std::array<std::pair<qreal, qreal>, 9> m_cachedBounds{};
    mutable bool m_boundsCacheValid = false;
};

