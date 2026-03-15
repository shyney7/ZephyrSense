#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QFile>
#include <QTextStream>
#include <memory>
#include "sensorreading.h"

class IOWorker : public QObject
{
    Q_OBJECT

public:
    explicit IOWorker(QObject *parent = nullptr);
    ~IOWorker() override;

public slots:
    // Must be called after moveToThread() - initializes database connection
    void initialize(const QString &databasePath);

    // Process a reading - writes to DB and CSV (if enabled)
    void processReading(const SensorReading &reading);

    // Flush pending writes (call before shutdown)
    void flushAll();

    // CSV configuration
    void setCsvEnabled(bool enabled);
    void setCsvFilePath(const QString &path);

signals:
    void initialized(bool success);
    void databaseError(const QString &message);
    void csvError(const QString &message);

private:
    void openDatabase(const QString &path);
    void createTables();
    void insertReadingToDb(const SensorReading &reading);
    void writeReadingToCsv(const SensorReading &reading);
    void openCsvFile();
    void closeCsvFile();

    // 8-byte aligned members first
    QSqlDatabase m_db;
    QString m_databasePath;
    std::unique_ptr<QFile> m_csvFile;
    std::unique_ptr<QTextStream> m_csvStream;
    QString m_csvFilePath;

    // 1-byte members last
    bool m_dbInitialized = false;
    bool m_csvEnabled = false;

    static constexpr const char* CONNECTION_NAME = "ZephyrSenseIOWorker";
};

