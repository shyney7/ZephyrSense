// SPDX-License-Identifier: MIT
// Mock singletons for QML tests — registered as the ZephyrSense module
// so production QML files that `import ZephyrSense` resolve correctly.

#ifndef MOCKS_H
#define MOCKS_H

#include <QObject>
#include <QAbstractTableModel>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QDateTime>
#include <QPointF>
#include <QUrl>
#include <array>

class MockSerialHandler final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SerialHandler)
    QML_SINGLETON

    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY portsChanged FINAL)
    Q_PROPERTY(bool connected READ isConnected WRITE setConnected NOTIFY connectionStateChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString WRITE setErrorString NOTIFY errorOccurred FINAL)
    Q_PROPERTY(QString currentPort READ currentPort NOTIFY connectionStateChanged FINAL)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged FINAL)

public:
    explicit MockSerialHandler(QObject *parent = nullptr) : QObject(parent) {}

    QStringList availablePorts() const { return m_ports; }
    bool isConnected() const { return m_connected; }
    void setConnected(bool v) { if (m_connected != v) { m_connected = v; emit connectionStateChanged(v); } }
    QString errorString() const { return m_errorString; }
    void setErrorString(const QString &v) { if (m_errorString != v) { m_errorString = v; emit errorOccurred(v); } }
    QString currentPort() const { return m_currentPort; }
    int baudRate() const { return m_baudRate; }
    void setBaudRate(int v) { if (m_baudRate != v) { m_baudRate = v; emit baudRateChanged(); } }

    Q_INVOKABLE void openPort(const QString &portName) {
        m_currentPort = portName;
        setConnected(true);
    }
    Q_INVOKABLE void closePort() {
        m_currentPort.clear();
        setConnected(false);
    }
    Q_INVOKABLE void refreshPorts() {
        m_ports = {QStringLiteral("COM1"), QStringLiteral("COM2")};
        emit portsChanged();
    }

signals:
    void newReading(const QVariantMap &reading);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString &message);
    void portsChanged();
    void baudRateChanged();

private:
    QStringList m_ports;
    bool m_connected = false;
    QString m_errorString;
    QString m_currentPort;
    int m_baudRate = 115200;
};

class MockDatabaseManager final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(DatabaseManager)
    QML_SINGLETON

    Q_PROPERTY(QString databasePath READ databasePath CONSTANT FINAL)

public:
    explicit MockDatabaseManager(QObject *parent = nullptr) : QObject(parent) {}

    QString databasePath() const { return QStringLiteral("/tmp/mock.db"); }

    Q_INVOKABLE bool initialize() { return true; }
    Q_INVOKABLE QVariantList getReadingsInRange(const QDateTime &, const QDateTime &) {
        return QVariantList{};
    }
    Q_INVOKABLE QVariantMap getReadingById(int) { return QVariantMap{}; }
    Q_INVOKABLE QVariantList getAvailableDates() { return m_dates; }
    Q_INVOKABLE bool exportDatabase(const QUrl &) { return true; }
    Q_INVOKABLE bool importDatabase(const QUrl &) { return true; }

    void setAvailableDates(const QVariantList &dates) { m_dates = dates; }

signals:
    void databaseError(const QString &message);
    void exportCompleted(bool success);
    void importCompleted(bool success);

private:
    QVariantList m_dates;
};

class MockTimeSeriesChartModel : public QAbstractTableModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TimeSeriesChartModel)

    Q_PROPERTY(qreal xMin READ xMin NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal xMax READ xMax NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal yMin READ yMin NOTIFY boundsChanged FINAL)
    Q_PROPERTY(qreal yMax READ yMax NOTIFY boundsChanged FINAL)
    Q_PROPERTY(int dataCount READ dataCount NOTIFY dataCountChanged FINAL)
    Q_PROPERTY(int boundsCallCount READ boundsCallCount NOTIFY boundsCallCountChanged FINAL)

public:
    explicit MockTimeSeriesChartModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent)
    {
        m_bounds.fill(QPointF(0.0, 100.0));
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return 0;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return 10;
    }

    QVariant data(const QModelIndex &, int = Qt::DisplayRole) const override
    {
        return QVariant();
    }

    Q_INVOKABLE QPointF getYBoundsForColumn(int column)
    {
        ++m_boundsCallCount;
        emit boundsCallCountChanged();

        if (column < 1 || column > 9) {
            return QPointF(0.0, 100.0);
        }

        return m_bounds[static_cast<size_t>(column - 1)];
    }

    Q_INVOKABLE void setBoundsForColumn(int column, qreal minValue, qreal maxValue)
    {
        if (column < 1 || column > 9) {
            return;
        }
        m_bounds[static_cast<size_t>(column - 1)] = QPointF(minValue, maxValue);
    }

    Q_INVOKABLE void emitBoundsChanged() { emit boundsChanged(); }

    Q_INVOKABLE void setXRange(qreal minValue, qreal maxValue)
    {
        m_xMin = minValue;
        m_xMax = maxValue;
        emit boundsChanged();
    }

    Q_INVOKABLE void resetForTest()
    {
        m_bounds.fill(QPointF(0.0, 100.0));
        if (m_boundsCallCount != 0) {
            m_boundsCallCount = 0;
            emit boundsCallCountChanged();
        }
    }

    int boundsCallCount() const { return m_boundsCallCount; }
    qreal xMin() const { return m_xMin; }
    qreal xMax() const { return m_xMax; }
    qreal yMin() const { return 0.0; }
    qreal yMax() const { return 100.0; }
    int dataCount() const { return 0; }

signals:
    void boundsChanged();
    void dataCountChanged();
    void boundsCallCountChanged();

private:
    std::array<QPointF, 9> m_bounds{};
    int m_boundsCallCount = 0;
    qreal m_xMin = 0.0;
    qreal m_xMax = 0.0;
};

class MockCsvExporter final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CsvExporter)
    QML_SINGLETON

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged FINAL)

public:
    explicit MockCsvExporter(QObject *parent = nullptr) : QObject(parent) {}

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool v) { if (m_enabled != v) { m_enabled = v; emit enabledChanged(); } }
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &v) { if (m_filePath != v) { m_filePath = v; emit filePathChanged(); } }

    Q_INVOKABLE void setFilePathFromUrl(const QUrl &) {}

signals:
    void enabledChanged();
    void filePathChanged();
    void exportError(const QString &message);

private:
    bool m_enabled = false;
    QString m_filePath;
};

class MockThresholdManager final : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ThresholdManager)
    QML_SINGLETON

public:
    explicit MockThresholdManager(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QString getColorForSensor(const QString &, qreal) const {
        return QStringLiteral("#4CAF50");
    }
    Q_INVOKABLE qreal getSweepAngleForSensor(const QString &sensorKey, qreal value,
                                             qreal minValue, qreal fallbackMax) const
    {
        const auto legacyLinearSweep = [value, minValue, fallbackMax]() -> qreal {
            if (minValue >= fallbackMax) {
                return 0.0;
            }
            const qreal clampedValue = qBound(minValue, value, fallbackMax);
            return ((clampedValue - minValue) / (fallbackMax - minValue)) * 360.0;
        };

        if (sensorKey == QLatin1String("co2")) {
            if (m_co2Warning <= minValue || m_co2Danger <= m_co2Warning || m_co2Danger <= 0.0) {
                return legacyLinearSweep();
            }

            const qreal bufferedMax = m_co2Danger * 1.25;
            if (value <= m_co2Warning) {
                const qreal normalizedA = qBound(0.0, (value - minValue) / (m_co2Warning - minValue), 1.0);
                return normalizedA * 180.0;
            }

            if (value <= m_co2Danger) {
                const qreal normalizedB = qBound(0.0, (value - m_co2Warning) / (m_co2Danger - m_co2Warning), 1.0);
                return 180.0 + (normalizedB * 90.0);
            }

            const qreal normalizedC = qBound(0.0, (value - m_co2Danger) / (bufferedMax - m_co2Danger), 1.0);
            return 270.0 + (normalizedC * 90.0);
        }

        // Legacy fallback behavior for unknown keys / empty sensorKey.
        return legacyLinearSweep();
    }
    Q_INVOKABLE bool isSensorEnabledForKey(const QString &) const { return true; }
    Q_INVOKABLE void resetToDefaults() { emit thresholdsChanged(); }
    Q_INVOKABLE void setCo2Thresholds(qreal warning, qreal danger)
    {
        m_co2Warning = warning;
        m_co2Danger = danger;
        emit thresholdsChanged();
    }
    Q_INVOKABLE void emitThresholdsChanged() { emit thresholdsChanged(); }

signals:
    void thresholdsChanged();

private:
    qreal m_co2Warning = 1000.0;
    qreal m_co2Danger = 2000.0;
};

#endif // MOCKS_H
