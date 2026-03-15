#ifndef CESIUMWORKER_H
#define CESIUMWORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>

class CesiumWorker : public QObject
{
    Q_OBJECT

public:
    explicit CesiumWorker(QObject *parent = nullptr);
    ~CesiumWorker() override;

    static int computeHazardFromSnapshot(int co2, float temperature, float humidity,
                                         float partectorMass, float grimmValue,
                                         int partectorNumber, int partectorDiam,
                                         float pressure, float altitude,
                                         const QVariantMap &thresholds);

public slots:
    void initialize(const QString &databasePath);
    void generateCzml(qint64 startMsecs, qint64 endMsecs, int requestId,
                      const QVariantMap &thresholds);

signals:
    void czmlGenerated(const QString &czmlJson, int requestId);

private:
    void openDatabase(const QString &path);
    bool createTables();
    static bool isValidCoordinate(double lat, double lon);

    QSqlDatabase m_db;
    bool m_dbInitialized = false;

    static constexpr const char* CONNECTION_NAME = "ZephyrSenseCesium";
};

#endif // CESIUMWORKER_H
