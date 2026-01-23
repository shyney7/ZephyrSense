#ifndef SENSORREADINGMODEL_H
#define SENSORREADINGMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDateTime>
#include "sensorreading.h"
#include "thresholdmanager.h"

class SensorReadingModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        LatitudeRole,
        LongitudeRole,
        PartectorNumberRole,
        PartectorDiamRole,
        PartectorMassRole,
        GrimmValueRole,
        TemperatureRole,
        HumidityRole,
        PressureRole,
        AltitudeRole,
        Co2Role,
        TimestampRole,
        TooltipTextRole,
        HazardLevelRole
    };

    explicit SensorReadingModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_readings.count(); }

    Q_INVOKABLE void loadFromDatabase(const QDateTime &start, const QDateTime &end);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap getReading(int index) const;

public slots:
    void addReading(const SensorReading &reading);

signals:
    void countChanged();

private:
    struct ReadingEntry {
        qint64 id;
        SensorReading reading;
    };

    QString formatTooltip(const SensorReading &reading) const;
    bool isValidCoordinate(float lat, float lon) const;
    void connectToThresholdManager();

    QList<ReadingEntry> m_readings;
    qint64 m_nextId = 1;
    bool m_thresholdManagerConnected = false;

private slots:
    void onThresholdsChanged();
};

#endif // SENSORREADINGMODEL_H
