#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include <QDateTime>
#include <QVariantList>
#include "sensorreading.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString databasePath READ databasePath CONSTANT)

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    static constexpr const char* CONNECTION_NAME = "ZephyrSense";

    QString databasePath() const { return m_databasePath; }

    Q_INVOKABLE bool initialize();
    Q_INVOKABLE bool exportDatabase(const QUrl &destination);
    Q_INVOKABLE bool importDatabase(const QUrl &source);
    Q_INVOKABLE QVariantList getReadingsInRange(const QDateTime &start, const QDateTime &end);

public slots:
    void insertReading(const SensorReading &reading);

signals:
    void databaseError(const QString &message);
    void exportCompleted(bool success);
    void importCompleted(bool success);

private:
    void createTables();
    QString m_databasePath;
};

#endif // DATABASEMANAGER_H
