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
    m_databasePath = dataPath + QStringLiteral("/zephyrsense.db");
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
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);

    if (!db.open()) {
        QString error = QStringLiteral("Failed to open database: %1").arg(db.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return false;
    }

    qDebug() << "Database opened at:" << m_databasePath;

    // Enable WAL mode and busy timeout for concurrent access
    // (IOWorker writes from worker thread, CesiumWorker reads from another thread)
    QSqlQuery pragma(db);
    if (!pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"))) {
        qWarning() << "DatabaseManager: Failed to enable WAL mode:" << pragma.lastError().text();
    }
    if (!pragma.exec(QStringLiteral("PRAGMA busy_timeout=5000"))) {
        qWarning() << "DatabaseManager: Failed to set busy_timeout:" << pragma.lastError().text();
    }

    createTables();
    return true;
}

void DatabaseManager::createTables()
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery query(db);

    // Create readings table with all sensor fields
    const QString createTableSql = QStringLiteral(R"(
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
    )");

    if (!query.exec(createTableSql)) {
        QString error = QStringLiteral("Failed to create readings table: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return;
    }

    // Create index on timestamp for efficient range queries
    const QString createIndexSql = QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
    )");

    if (!query.exec(createIndexSql)) {
        QString error = QStringLiteral("Failed to create timestamp index: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
    }

    qDebug() << "Database tables and indexes created successfully";
}

void DatabaseManager::insertReading(const SensorReading &reading)
{
    // NOTE: This slot is now deprecated. Database writes are handled by IOWorker
    // on a dedicated I/O thread for non-blocking UI performance.
    // This slot remains for API compatibility but does nothing.
    Q_UNUSED(reading)
    qCritical() << "DatabaseManager::insertReading called directly - writes should go through IOWorker";
}

QVariantList DatabaseManager::getReadingsInRange(const QDateTime &start, const QDateTime &end)
{
    QVariantList results;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    if (!db.isOpen()) {
        emit databaseError(QStringLiteral("Database not open"));
        return results;
    }

    QSqlQuery query(db);
    query.setForwardOnly(true);  // Memory efficient for large result sets

    query.prepare(QStringLiteral(R"(
        SELECT id, timestamp, partectorNumber, partectorDiam, partectorMass,
               grimmValue, temperature, humidity, pressure,
               altitude, latitude, longitude, co2
        FROM readings
        WHERE timestamp BETWEEN ? AND ?
        ORDER BY timestamp ASC
    )"));

    query.addBindValue(start.toMSecsSinceEpoch());
    query.addBindValue(end.toMSecsSinceEpoch());

    if (!query.exec()) {
        QString error = QStringLiteral("Failed to query readings: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return results;
    }

    while (query.next()) {
        QVariantMap reading;
        reading[QStringLiteral("id")] = query.value(0).toInt();  // Database ID
        reading[QStringLiteral("timestamp")] = QDateTime::fromMSecsSinceEpoch(query.value(1).toLongLong());
        reading[QStringLiteral("partectorNumber")] = query.value(2).toInt();
        reading[QStringLiteral("partectorDiam")] = query.value(3).toInt();
        reading[QStringLiteral("partectorMass")] = query.value(4).toDouble();
        reading[QStringLiteral("grimmValue")] = query.value(5).toDouble();
        reading[QStringLiteral("temperature")] = query.value(6).toDouble();
        reading[QStringLiteral("humidity")] = query.value(7).toDouble();
        reading[QStringLiteral("pressure")] = query.value(8).toDouble();
        reading[QStringLiteral("altitude")] = query.value(9).toDouble();
        reading[QStringLiteral("latitude")] = query.value(10).toDouble();
        reading[QStringLiteral("longitude")] = query.value(11).toDouble();
        reading[QStringLiteral("co2")] = query.value(12).toInt();
        results.append(reading);
    }

    return results;
}

QVariantMap DatabaseManager::getReadingById(int id)
{
    QVariantMap result;

    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    if (!db.isOpen()) {
        qWarning() << "Database not open";
        return result;
    }

    QSqlQuery query(db);
    query.prepare(QStringLiteral(R"(
        SELECT id, timestamp, partectorNumber, partectorDiam, partectorMass,
               grimmValue, temperature, humidity, pressure,
               altitude, latitude, longitude, co2
        FROM readings
        WHERE id = ?
    )"));

    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "Failed to get reading by ID:" << query.lastError().text();
        return result;
    }

    if (query.next()) {
        result[QStringLiteral("id")] = query.value(0).toInt();
        result[QStringLiteral("timestamp")] = QDateTime::fromMSecsSinceEpoch(query.value(1).toLongLong());
        result[QStringLiteral("partectorNumber")] = query.value(2).toInt();
        result[QStringLiteral("partectorDiam")] = query.value(3).toInt();
        result[QStringLiteral("partectorMass")] = query.value(4).toDouble();
        result[QStringLiteral("grimmValue")] = query.value(5).toDouble();
        result[QStringLiteral("temperature")] = query.value(6).toDouble();
        result[QStringLiteral("humidity")] = query.value(7).toDouble();
        result[QStringLiteral("pressure")] = query.value(8).toDouble();
        result[QStringLiteral("altitude")] = query.value(9).toDouble();
        result[QStringLiteral("latitude")] = query.value(10).toDouble();
        result[QStringLiteral("longitude")] = query.value(11).toDouble();
        result[QStringLiteral("co2")] = query.value(12).toInt();
    }

    return result;
}

bool DatabaseManager::exportDatabase(const QUrl &destination)
{
    QString destPath = destination.toLocalFile();
    if (destPath.isEmpty()) {
        emit databaseError(QStringLiteral("Invalid export destination"));
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
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
        qWarning() << "Failed to reopen database after export";
    }

    if (!success) {
        QString error = QStringLiteral("Failed to export database to: %1").arg(destPath);
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
        emit databaseError(QStringLiteral("Invalid import source"));
        emit importCompleted(false);
        return false;
    }

    if (!QFile::exists(sourcePath)) {
        emit databaseError(QStringLiteral("Import file does not exist"));
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
    QString backupPath = m_databasePath + QStringLiteral(".backup");
    bool hadExisting = QFile::exists(m_databasePath);
    if (hadExisting) {
        QFile::remove(backupPath);  // Remove old backup if exists
        if (!QFile::rename(m_databasePath, backupPath)) {
            emit databaseError(QStringLiteral("Failed to backup current database"));
            // Try to reopen original
            QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), CONNECTION_NAME);
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
        emit databaseError(QStringLiteral("Failed to import database"));
    }

    // Reopen the connection
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), CONNECTION_NAME);
    db.setDatabaseName(m_databasePath);
    if (!db.open()) {
        qWarning() << "Failed to reopen database after import";
        emit databaseError(QStringLiteral("Failed to reopen database after import"));
        success = false;
    }

    emit importCompleted(success);
    return success;
}

QVariantList DatabaseManager::getAvailableDates()
{
    QVariantList dates;
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    if (!db.isOpen()) {
        qWarning() << "Database not open for getAvailableDates";
        return dates;
    }

    QSqlQuery query(db);
    query.setForwardOnly(true);

    // Query distinct dates (day precision) from readings table
    // timestamp is stored as milliseconds since epoch
    if (!query.exec(QStringLiteral(R"(
        SELECT DISTINCT date(timestamp / 1000, 'unixepoch', 'localtime') as date
        FROM readings
        ORDER BY date DESC
    )"))) {
        qWarning() << "Failed to query available dates:" << query.lastError().text();
        return dates;
    }

    while (query.next()) {
        dates.append(query.value(0).toString());  // Format: "2026-01-25"
    }

    return dates;
}
