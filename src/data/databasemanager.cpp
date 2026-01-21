#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // Set up database path in app data location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_databasePath = dataPath + "/zephyrsense.db";
}

DatabaseManager::~DatabaseManager()
{
    // Close the connection if it exists
    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        QSqlDatabase::database(CONNECTION_NAME).close();
        QSqlDatabase::removeDatabase(CONNECTION_NAME);
    }
}

bool DatabaseManager::initialize()
{
    // Check if already connected
    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
        if (db.isOpen()) {
            return true;
        }
    }

    // Create new connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);

    if (!db.open()) {
        QString error = QString("Failed to open database: %1").arg(db.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return false;
    }

    qDebug() << "Database opened at:" << m_databasePath;
    createTables();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(db);

    // Create readings table with all sensor fields
    const QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            partectorNumber INTEGER,
            partectorDiam INTEGER,
            partectorMass REAL,
            grimmValue REAL,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            altitude REAL,
            latitude REAL,
            longitude REAL,
            co2 INTEGER
        )
    )";

    if (!query.exec(createTableSql)) {
        QString error = QString("Failed to create readings table: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return;
    }

    // Create index on timestamp for efficient range queries
    const QString createIndexSql = R"(
        CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
    )";

    if (!query.exec(createIndexSql)) {
        QString error = QString("Failed to create timestamp index: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
    }

    qDebug() << "Database tables and indexes created successfully";
}

void DatabaseManager::insertReading(const SensorReading &reading)
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    if (!db.isOpen()) {
        emit databaseError("Database not open");
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO readings (
            timestamp, partectorNumber, partectorDiam, partectorMass,
            grimmValue, temperature, humidity, pressure,
            altitude, latitude, longitude, co2
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");

    // Store timestamp as milliseconds since epoch (INTEGER)
    query.addBindValue(reading.timestamp.toMSecsSinceEpoch());
    query.addBindValue(reading.partectorNumber);
    query.addBindValue(reading.partectorDiam);
    query.addBindValue(static_cast<double>(reading.partectorMass));
    query.addBindValue(static_cast<double>(reading.grimmValue));
    query.addBindValue(static_cast<double>(reading.temperature));
    query.addBindValue(static_cast<double>(reading.humidity));
    query.addBindValue(static_cast<double>(reading.pressure));
    query.addBindValue(static_cast<double>(reading.altitude));
    query.addBindValue(static_cast<double>(reading.latitude));
    query.addBindValue(static_cast<double>(reading.longitude));
    query.addBindValue(reading.co2);

    if (!query.exec()) {
        QString error = QString("Failed to insert reading: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
    }
}

QVariantList DatabaseManager::getReadingsInRange(const QDateTime &start, const QDateTime &end)
{
    QVariantList results;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    if (!db.isOpen()) {
        emit databaseError("Database not open");
        return results;
    }

    QSqlQuery query(db);
    query.setForwardOnly(true);  // Memory efficient for large result sets

    query.prepare(R"(
        SELECT timestamp, partectorNumber, partectorDiam, partectorMass,
               grimmValue, temperature, humidity, pressure,
               altitude, latitude, longitude, co2
        FROM readings
        WHERE timestamp BETWEEN ? AND ?
        ORDER BY timestamp ASC
    )");

    query.addBindValue(start.toMSecsSinceEpoch());
    query.addBindValue(end.toMSecsSinceEpoch());

    if (!query.exec()) {
        QString error = QString("Failed to query readings: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return results;
    }

    while (query.next()) {
        QVariantMap reading;
        reading["timestamp"] = QDateTime::fromMSecsSinceEpoch(query.value(0).toLongLong());
        reading["partectorNumber"] = query.value(1).toInt();
        reading["partectorDiam"] = query.value(2).toInt();
        reading["partectorMass"] = query.value(3).toDouble();
        reading["grimmValue"] = query.value(4).toDouble();
        reading["temperature"] = query.value(5).toDouble();
        reading["humidity"] = query.value(6).toDouble();
        reading["pressure"] = query.value(7).toDouble();
        reading["altitude"] = query.value(8).toDouble();
        reading["latitude"] = query.value(9).toDouble();
        reading["longitude"] = query.value(10).toDouble();
        reading["co2"] = query.value(11).toInt();
        results.append(reading);
    }

    return results;
}

bool DatabaseManager::exportDatabase(const QUrl &destination)
{
    QString destPath = destination.toLocalFile();
    if (destPath.isEmpty()) {
        emit databaseError("Invalid export destination");
        emit exportCompleted(false);
        return false;
    }

    // Close the connection before copying
    {
        QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(CONNECTION_NAME);

    // Copy the database file
    bool success = QFile::copy(m_databasePath, destPath);

    // Reopen the connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
        qWarning() << "Failed to reopen database after export";
    }

    if (!success) {
        QString error = QString("Failed to export database to: %1").arg(destPath);
        qWarning() << error;
        emit databaseError(error);
    }

    emit exportCompleted(success);
    return success;
}

bool DatabaseManager::importDatabase(const QUrl &source)
{
    QString sourcePath = source.toLocalFile();
    if (sourcePath.isEmpty()) {
        emit databaseError("Invalid import source");
        emit importCompleted(false);
        return false;
    }

    if (!QFile::exists(sourcePath)) {
        emit databaseError("Import file does not exist");
        emit importCompleted(false);
        return false;
    }

    // Close the connection before importing
    {
        QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(CONNECTION_NAME);

    // Backup current database
    QString backupPath = m_databasePath + ".backup";
    bool hadExisting = QFile::exists(m_databasePath);
    if (hadExisting) {
        QFile::remove(backupPath);  // Remove old backup if exists
        if (!QFile::rename(m_databasePath, backupPath)) {
            emit databaseError("Failed to backup current database");
            // Try to reopen original
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
            db.setDatabaseName(m_databasePath);
            db.open();
            emit importCompleted(false);
            return false;
        }
    }

    // Copy import file to database location
    bool success = QFile::copy(sourcePath, m_databasePath);

    if (success) {
        // Remove backup on success
        if (hadExisting) {
            QFile::remove(backupPath);
        }
    } else {
        // Restore backup on failure
        if (hadExisting) {
            QFile::rename(backupPath, m_databasePath);
        }
        emit databaseError("Failed to import database");
    }

    // Reopen the connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
        qWarning() << "Failed to reopen database after import";
        emit databaseError("Failed to reopen database after import");
        success = false;
    }

    emit importCompleted(success);
    return success;
}
