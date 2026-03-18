#include "ioworker.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDebug>

IOWorker::IOWorker(QObject *parent)
    : QObject(parent)
{
}

IOWorker::~IOWorker()
{
    flushAll();
    closeCsvFile();

    // Close database connection
    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        {
            QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(CONNECTION_NAME);
    }
}

void IOWorker::initialize(const QString &databasePath)
{
    m_databasePath = databasePath;
    openDatabase(databasePath);
    emit initialized(m_dbInitialized);
}

void IOWorker::openDatabase(const QString &path)
{
    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        m_db = QSqlDatabase::database(CONNECTION_NAME);
        if (m_db.isOpen()) {
            m_dbInitialized = true;
            return;
        }
    }

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), CONNECTION_NAME);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        QString error = QStringLiteral("IOWorker: Failed to open database: %1").arg(m_db.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        m_dbInitialized = false;
        return;
    }

    qDebug() << "IOWorker: Database opened at:" << path;

    // Enable WAL mode for better concurrent read/write performance
    // (DatabaseManager reads from main thread, IOWorker writes from worker thread)
    QSqlQuery walQuery(m_db);
    if (!walQuery.exec(QStringLiteral("PRAGMA journal_mode=WAL"))) {
        qWarning() << "IOWorker: Failed to enable WAL mode:" << walQuery.lastError().text();
    }

    QSqlQuery timeoutQuery(m_db);
    if (!timeoutQuery.exec(QStringLiteral("PRAGMA busy_timeout=5000"))) {
        qWarning() << "IOWorker: Failed to set busy_timeout:" << timeoutQuery.lastError().text();
    }

    createTables();
    m_dbInitialized = true;
}

void IOWorker::createTables()
{
    QSqlQuery query(m_db);

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
        QString error = QStringLiteral("IOWorker: Failed to create readings table: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
        return;
    }

    const QString createIndexSql = QStringLiteral(R"(
        CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
    )");

    if (!query.exec(createIndexSql)) {
        QString error = QStringLiteral("IOWorker: Failed to create timestamp index: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
    }

    qDebug() << "IOWorker: Database tables and indexes ready";
}

void IOWorker::processReading(const SensorReading &reading)
{
    // Write to database
    if (m_dbInitialized) {
        insertReadingToDb(reading);
    } else {
        static bool warnedDbNotInit = false;
        if (!warnedDbNotInit) {
            qWarning() << "IOWorker: Database not initialized - readings will not be saved to database";
            warnedDbNotInit = true;
        }
    }

    // Write to CSV if enabled
    if (m_csvEnabled && !m_csvFilePath.isEmpty()) {
        writeReadingToCsv(reading);
    }
}

void IOWorker::insertReadingToDb(const SensorReading &reading)
{
    if (!m_db.isOpen()) {
        emit databaseError(QStringLiteral("IOWorker: Database not open"));
        return;
    }

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(R"(
        INSERT INTO readings (
            timestamp, partectorNumber, partectorDiam, partectorMass,
            grimmValue, temperature, humidity, pressure,
            altitude, latitude, longitude, co2
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )"));

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
        QString error = QStringLiteral("IOWorker: Failed to insert reading: %1").arg(query.lastError().text());
        qWarning() << error;
        emit databaseError(error);
    }
}

void IOWorker::writeReadingToCsv(const SensorReading &reading)
{
    // Open file if not already open
    if (!m_csvFile || !m_csvFile->isOpen()) {
        openCsvFile();
    }

    if (!m_csvStream) {
        return;
    }

    // Write data row
    *m_csvStream << reading.timestamp.toString(Qt::ISODate) << ","
                 << reading.partectorNumber << ","
                 << reading.partectorDiam << ","
                 << reading.partectorMass << ","
                 << reading.grimmValue << ","
                 << reading.temperature << ","
                 << reading.humidity << ","
                 << reading.pressure << ","
                 << reading.altitude << ","
                 << reading.latitude << ","
                 << reading.longitude << ","
                 << reading.co2 << "\n";

    // Flush periodically (every write at 2s interval is fine)
    m_csvStream->flush();
}

void IOWorker::openCsvFile()
{
    if (m_csvFilePath.isEmpty()) {
        return;
    }

    // Check if file exists and has content
    QFileInfo fileInfo(m_csvFilePath);
    bool needsHeader = !fileInfo.exists() || fileInfo.size() == 0;

    m_csvFile = std::make_unique<QFile>(m_csvFilePath);
    if (!m_csvFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QString error = QStringLiteral("IOWorker: Failed to open CSV file: %1").arg(m_csvFile->errorString());
        qWarning() << error;
        emit csvError(error);
        m_csvFile.reset();
        return;
    }

    m_csvStream = std::make_unique<QTextStream>(m_csvFile.get());

    // Write header if this is a new/empty file
    if (needsHeader) {
        *m_csvStream << "timestamp,partector_number,partector_diam,partector_mass,"
                     << "grimm_value,temperature,humidity,pressure,"
                     << "altitude,latitude,longitude,co2\n";
        m_csvStream->flush();
    }

    qDebug() << "IOWorker: CSV file opened:" << m_csvFilePath;
}

void IOWorker::closeCsvFile()
{
    if (m_csvStream) {
        m_csvStream->flush();
        m_csvStream.reset();
    }
    if (m_csvFile) {
        m_csvFile->close();
        m_csvFile.reset();
    }
}

void IOWorker::flushAll()
{
    if (m_csvStream) {
        m_csvStream->flush();
    }
    qDebug() << "IOWorker: Flushed all pending writes";
}

void IOWorker::setCsvEnabled(bool enabled)
{
    if (m_csvEnabled == enabled) {
        return;
    }

    m_csvEnabled = enabled;
    qDebug() << "IOWorker: CSV enabled =" << m_csvEnabled;

    if (!m_csvEnabled) {
        closeCsvFile();
    }
}

void IOWorker::setCsvFilePath(const QString &path)
{
    if (m_csvFilePath == path) {
        return;
    }

    // Close existing file before switching
    closeCsvFile();

    m_csvFilePath = path;
    qDebug() << "IOWorker: CSV file path =" << m_csvFilePath;

    // File will be opened on next write if enabled
}
