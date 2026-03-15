#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTemporaryDir>
#include <QDebug>
#include "sensorreading.h"

// ─── SensorReadingBuilder ────────────────────────────────────────────────────
// Fluent API for constructing test SensorReading objects.

class SensorReadingBuilder
{
public:
    SensorReadingBuilder &withCoordinates(float lat, float lon)
    {
        m_reading.latitude = lat;
        m_reading.longitude = lon;
        return *this;
    }

    SensorReadingBuilder &withTemperature(float t)
    {
        m_reading.temperature = t;
        return *this;
    }

    SensorReadingBuilder &withHumidity(float h)
    {
        m_reading.humidity = h;
        return *this;
    }

    SensorReadingBuilder &withPressure(float p)
    {
        m_reading.pressure = p;
        return *this;
    }

    SensorReadingBuilder &withAltitude(float a)
    {
        m_reading.altitude = a;
        return *this;
    }

    SensorReadingBuilder &withCo2(int co2)
    {
        m_reading.co2 = co2;
        return *this;
    }

    SensorReadingBuilder &withPartectorNumber(int n)
    {
        m_reading.partectorNumber = n;
        return *this;
    }

    SensorReadingBuilder &withPartectorDiam(int d)
    {
        m_reading.partectorDiam = d;
        return *this;
    }

    SensorReadingBuilder &withPartectorMass(float m)
    {
        m_reading.partectorMass = m;
        return *this;
    }

    SensorReadingBuilder &withGrimmValue(float g)
    {
        m_reading.grimmValue = g;
        return *this;
    }

    SensorReadingBuilder &withTimestamp(const QDateTime &ts)
    {
        m_reading.timestamp = ts;
        return *this;
    }

    SensorReadingBuilder &withAllSensors()
    {
        m_reading.partectorNumber = 1000;
        m_reading.partectorDiam = 50;
        m_reading.partectorMass = 2.5f;
        m_reading.grimmValue = 1.2f;
        m_reading.temperature = 22.5f;
        m_reading.humidity = 55.0f;
        m_reading.pressure = 1013.25f;
        m_reading.altitude = 150.0f;
        m_reading.latitude = 48.1f;
        m_reading.longitude = 11.5f;
        m_reading.co2 = 450;
        m_reading.timestamp = QDateTime::currentDateTime();
        return *this;
    }

    SensorReading build() const { return m_reading; }

private:
    SensorReading m_reading;
};

// ─── TestDatabaseHelper ──────────────────────────────────────────────────────
// Encapsulates QTemporaryDir + SQLite schema creation for test databases.

class TestDatabaseHelper
{
public:
    explicit TestDatabaseHelper(const QString &connectionName = QStringLiteral("TestDB"))
        : m_connectionName(connectionName)
    {
    }

    ~TestDatabaseHelper()
    {
        if (QSqlDatabase::contains(m_connectionName)) {
            QSqlDatabase::database(m_connectionName).close();
            QSqlDatabase::removeDatabase(m_connectionName);
        }
    }

    // Non-copyable
    TestDatabaseHelper(const TestDatabaseHelper &) = delete;
    TestDatabaseHelper &operator=(const TestDatabaseHelper &) = delete;

    bool initialize()
    {
        if (!m_tmpDir.isValid())
            return false;

        m_dbPath = m_tmpDir.filePath(QStringLiteral("test.db"));

        QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
        db.setDatabaseName(m_dbPath);
        if (!db.open()) {
            qWarning() << "TestDatabaseHelper: Failed to open DB:" << db.lastError().text();
            return false;
        }

        QSqlQuery query(db);
        bool ok = query.exec(QStringLiteral(R"(
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
        )"));
        if (!ok)
            qWarning() << "TestDatabaseHelper: Table creation failed:" << query.lastError().text();

        query.exec(QStringLiteral(R"(
            CREATE INDEX IF NOT EXISTS idx_timestamp ON readings(timestamp)
        )"));

        return ok;
    }

    bool insertReading(const SensorReading &reading)
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"(
            INSERT INTO readings (timestamp, partectorNumber, partectorDiam, partectorMass,
                                  grimmValue, temperature, humidity, pressure,
                                  altitude, latitude, longitude, co2)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
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
        return query.exec();
    }

    int rowCount() const
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        QSqlQuery query(db);
        if (query.exec(QStringLiteral("SELECT COUNT(*) FROM readings")) && query.next())
            return query.value(0).toInt();
        return -1;
    }

    QString dbPath() const { return m_dbPath; }
    QString connectionName() const { return m_connectionName; }
    QString tempPath() const { return m_tmpDir.path(); }

private:
    QTemporaryDir m_tmpDir;
    QString m_dbPath;
    QString m_connectionName;
};

// ─── FrameBuilder ────────────────────────────────────────────────────────────
// Constructs binary serial frames for SerialHandler testing.

class FrameBuilder
{
public:
    FrameBuilder &withRaw(const SensorDataRaw &raw)
    {
        m_raw = raw;
        return *this;
    }

    FrameBuilder &withDefaults()
    {
        m_raw = SensorDataRaw{};
        m_raw.partectorNumber = 1000;
        m_raw.partectorDiam = 50;
        m_raw.partectorMass = 2.5f;
        m_raw.grimmValue = 1.2f;
        m_raw.temperature = 22.5f;
        m_raw.humidity = 55.0f;
        m_raw.pressure = 1013.25f;
        m_raw.altitude = 150.0f;
        m_raw.latitude = 48.1f;
        m_raw.longitude = 11.5f;
        m_raw.co2 = 450;
        return *this;
    }

    FrameBuilder &withTemperature(float t)
    {
        m_raw.temperature = t;
        return *this;
    }

    FrameBuilder &withPartectorNumber(int32_t n)
    {
        m_raw.partectorNumber = n;
        return *this;
    }

    FrameBuilder &withCo2(uint16_t c)
    {
        m_raw.co2 = c;
        return *this;
    }

    // Returns a complete valid frame: '<' + 42-byte payload + '>'
    QByteArray buildFrame() const
    {
        QByteArray frame;
        frame.reserve(sizeof(SensorDataRaw) + 2);
        frame.append('<');
        frame.append(reinterpret_cast<const char*>(&m_raw), sizeof(SensorDataRaw));
        frame.append('>');
        return frame;
    }

    // Returns a frame with wrong end delimiter
    QByteArray buildCorruptedFrame() const
    {
        QByteArray frame;
        frame.reserve(sizeof(SensorDataRaw) + 2);
        frame.append('<');
        frame.append(reinterpret_cast<const char*>(&m_raw), sizeof(SensorDataRaw));
        frame.append('!');  // Wrong delimiter
        return frame;
    }

    // Returns the first N bytes of a valid frame
    QByteArray buildPartialFrame(int bytes) const
    {
        QByteArray full = buildFrame();
        return full.left(bytes);
    }

    const SensorDataRaw &raw() const { return m_raw; }

private:
    SensorDataRaw m_raw = {};
};

