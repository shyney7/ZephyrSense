#include "cesiumworker.h"
#include "coordinatevalidator.h"
#include "readingsschema.h"

#include <QDateTime>
#include <QTimeZone>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QtMath>

CesiumWorker::CesiumWorker(QObject *parent)
    : QObject(parent)
{
}

CesiumWorker::~CesiumWorker()
{
    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        if (m_db.isOpen())
            m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(CONNECTION_NAME);
    }
}

void CesiumWorker::initialize(const QString &databasePath)
{
    openDatabase(databasePath);
}

void CesiumWorker::openDatabase(const QString &path)
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
        qWarning() << "CesiumWorker: Failed to open database:" << m_db.lastError().text();
        m_dbInitialized = false;
        return;
    }

    // Enable WAL mode and busy timeout for concurrent access with IOWorker writes
    QSqlQuery pragma(m_db);
    if (!pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"))) {
        qWarning() << "CesiumWorker: Failed to enable WAL mode:" << pragma.lastError().text();
    }
    if (!pragma.exec(QStringLiteral("PRAGMA busy_timeout=5000"))) {
        qWarning() << "CesiumWorker: Failed to set busy_timeout:" << pragma.lastError().text();
    }

    m_dbInitialized = createTables();
}

bool CesiumWorker::createTables()
{
    QSqlQuery query(m_db);
    if (!query.exec(QString{kReadingsTableSql})) {
        qWarning() << "CesiumWorker: Failed to create readings table:" << query.lastError().text();
        return false;
    }
    if (!query.exec(QString{kReadingsIndexSql})) {
        qWarning() << "CesiumWorker: Failed to create timestamp index:" << query.lastError().text();
    }
    return true;
}

int CesiumWorker::computeHazardFromSnapshot(
    int co2, float temperature, float humidity,
    float partectorMass, float grimmValue,
    int partectorNumber, int partectorDiam,
    float pressure, float altitude,
    const QVariantMap &thresholds)
{
    int maxHazard = 0;

    auto checkSensor = [&](const QString &prefix, double value, bool hasBidirectional = false) {
        const QString enabledKey = prefix + QStringLiteral("Enabled");
        if (!thresholds.value(enabledKey, false).toBool())
            return;

        const double warning = thresholds.value(prefix + QStringLiteral("Warning"), 0).toDouble();
        const double danger = thresholds.value(prefix + QStringLiteral("Danger"), 0).toDouble();

        if (value >= danger)
            maxHazard = qMax(maxHazard, 2);
        else if (value >= warning)
            maxHazard = qMax(maxHazard, 1);

        if (hasBidirectional) {
            const double lowWarning = thresholds.value(prefix + QStringLiteral("LowWarning"), 0).toDouble();
            const double lowDanger = thresholds.value(prefix + QStringLiteral("LowDanger"), 0).toDouble();
            if (value <= lowDanger)
                maxHazard = qMax(maxHazard, 2);
            else if (value <= lowWarning)
                maxHazard = qMax(maxHazard, 1);
        }
    };

    checkSensor(QStringLiteral("co2"), co2);
    checkSensor(QStringLiteral("temperature"), static_cast<double>(temperature), true);
    checkSensor(QStringLiteral("humidity"), static_cast<double>(humidity), true);
    checkSensor(QStringLiteral("partectorMass"), static_cast<double>(partectorMass));
    checkSensor(QStringLiteral("grimmValue"), static_cast<double>(grimmValue));
    checkSensor(QStringLiteral("partectorNumber"), partectorNumber);
    checkSensor(QStringLiteral("partectorDiam"), partectorDiam);
    checkSensor(QStringLiteral("pressure"), static_cast<double>(pressure));
    checkSensor(QStringLiteral("altitude"), static_cast<double>(altitude));

    return maxHazard;
}

void CesiumWorker::generateCzml(qint64 startMsecs, qint64 endMsecs, int requestId,
                                 const QVariantMap &thresholds)
{
    QJsonArray czml;

    // Document packet with clock
    const QString startIso = QDateTime::fromMSecsSinceEpoch(startMsecs, QTimeZone::UTC)
                                 .toString(Qt::ISODate);
    const QString endIso = QDateTime::fromMSecsSinceEpoch(endMsecs, QTimeZone::UTC)
                               .toString(Qt::ISODate);
    const QString interval = startIso + QStringLiteral("/") + endIso;

    QJsonObject docPacket;
    docPacket[QStringLiteral("id")] = QStringLiteral("document");
    docPacket[QStringLiteral("name")] = QStringLiteral("ZephyrSense Sensor Data");
    docPacket[QStringLiteral("version")] = QStringLiteral("1.0");

    QJsonObject clock;
    clock[QStringLiteral("interval")] = interval;
    clock[QStringLiteral("currentTime")] = startIso;
    clock[QStringLiteral("multiplier")] = 1;
    clock[QStringLiteral("range")] = QStringLiteral("LOOP_STOP");
    clock[QStringLiteral("step")] = QStringLiteral("SYSTEM_CLOCK_MULTIPLIER");
    docPacket[QStringLiteral("clock")] = clock;

    czml.append(docPacket);

    // Query readings in time range
    if (!m_dbInitialized) {
        emit czmlGenerated(QString::fromUtf8(QJsonDocument(czml).toJson(QJsonDocument::Compact)),
                           requestId);
        return;
    }

    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    query.prepare(QStringLiteral(R"(
        SELECT id, timestamp, partectorNumber, partectorDiam, partectorMass,
               grimmValue, temperature, humidity, pressure, altitude,
               latitude, longitude, co2
        FROM readings
        WHERE timestamp >= ? AND timestamp <= ?
        ORDER BY timestamp ASC
    )"));
    query.addBindValue(startMsecs);
    query.addBindValue(endMsecs);

    if (!query.exec()) {
        qWarning() << "CesiumWorker: Query failed:" << query.lastError().text();
        emit czmlGenerated(QString::fromUtf8(QJsonDocument(czml).toJson(QJsonDocument::Compact)),
                           requestId);
        return;
    }

    // Hazard color RGBA arrays
    auto makeColorArray = [](int r, int g, int b, int a) {
        return QJsonArray{ r, g, b, a };
    };
    const QJsonArray colorNormal = makeColorArray(76, 175, 80, 255);
    const QJsonArray colorWarning = makeColorArray(255, 152, 0, 255);
    const QJsonArray colorDanger = makeColorArray(244, 67, 54, 255);

    QJsonArray flightPathCoords;

    while (query.next()) {
        const int readingId = query.value(0).toInt();
        const qint64 timestamp = query.value(1).toLongLong();
        const int partectorNumber = query.value(2).toInt();
        const int partectorDiam = query.value(3).toInt();
        const float partectorMass = query.value(4).toFloat();
        const float grimmValue = query.value(5).toFloat();
        const float temperature = query.value(6).toFloat();
        const float humidity = query.value(7).toFloat();
        const float pressure = query.value(8).toFloat();
        const float altitude = query.value(9).toFloat();
        const double latitude = query.value(10).toDouble();
        const double longitude = query.value(11).toDouble();
        const int co2 = query.value(12).toInt();

        if (!isValidCoordinate(latitude, longitude))
            continue;

        // Compute hazard from threshold snapshot
        const int hazard = computeHazardFromSnapshot(
            co2, temperature, humidity, partectorMass, grimmValue,
            partectorNumber, partectorDiam, pressure, altitude, thresholds);

        const QJsonArray &pointColor = (hazard == 2) ? colorDanger
                                     : (hazard == 1) ? colorWarning
                                                     : colorNormal;

        const QString hazardLabel = (hazard == 2) ? QStringLiteral("Danger")
                                  : (hazard == 1) ? QStringLiteral("Warning")
                                                  : QStringLiteral("Normal");
        const QString hazardHtmlColor = (hazard == 2) ? QStringLiteral("#F44336")
                                      : (hazard == 1) ? QStringLiteral("#FF9800")
                                                      : QStringLiteral("#4CAF50");

        // Build timestamp string
        const QString tsStr = QDateTime::fromMSecsSinceEpoch(timestamp, QTimeZone::UTC)
                                  .toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

        // Build description HTML
        const QString description = QStringLiteral(
            "<table style='width:100%'>"
            "<tr><th>Timestamp</th><td>%1</td></tr>"
            "<tr><th>PNC UFP</th><td>%2 #/cm\u00B3</td></tr>"
            "<tr><th>\u00D8 UFP</th><td>%3 nm</td></tr>"
            "<tr><th>PM0.3</th><td>%4 \u00B5g/m\u00B3</td></tr>"
            "<tr><th>PNC PM</th><td>%5 #/cm\u00B3</td></tr>"
            "<tr><th>Temperature</th><td>%6 \u00B0C</td></tr>"
            "<tr><th>Humidity</th><td>%7 %</td></tr>"
            "<tr><th>Pressure</th><td>%8 hPa</td></tr>"
            "<tr><th>Altitude</th><td>%9 m</td></tr>"
            "<tr><th>CO\u2082</th><td>%10 ppm</td></tr>"
            "<tr><th>Hazard</th><td style='color:%11'>%12</td></tr>"
            "</table>")
            .arg(tsStr)
            .arg(partectorNumber)
            .arg(partectorDiam)
            .arg(static_cast<double>(partectorMass), 0, 'f', 2)
            .arg(static_cast<double>(grimmValue), 0, 'f', 2)
            .arg(static_cast<double>(temperature), 0, 'f', 1)
            .arg(static_cast<double>(humidity), 0, 'f', 1)
            .arg(static_cast<double>(pressure), 0, 'f', 1)
            .arg(static_cast<double>(altitude), 0, 'f', 1)
            .arg(co2)
            .arg(hazardHtmlColor, hazardLabel);

        // Position: [longitude, latitude, altitude] (CesiumJS convention: lon first)
        QJsonArray cartographicDegrees = { longitude, latitude, static_cast<double>(altitude) };

        QJsonObject position;
        position[QStringLiteral("cartographicDegrees")] = cartographicDegrees;

        QJsonObject pointStyle;
        pointStyle[QStringLiteral("pixelSize")] = 10;
        QJsonObject colorObj;
        colorObj[QStringLiteral("rgba")] = pointColor;
        pointStyle[QStringLiteral("color")] = colorObj;

        QJsonObject pointPacket;
        pointPacket[QStringLiteral("id")] = QStringLiteral("reading-%1").arg(readingId);
        pointPacket[QStringLiteral("name")] = QStringLiteral("Reading #%1").arg(readingId);
        pointPacket[QStringLiteral("description")] = description;
        pointPacket[QStringLiteral("position")] = position;
        pointPacket[QStringLiteral("point")] = pointStyle;

        czml.append(pointPacket);

        // Collect for flight path
        flightPathCoords.append(longitude);
        flightPathCoords.append(latitude);
        flightPathCoords.append(static_cast<double>(altitude));
    }

    // Flight path polyline (only if we have valid coordinates)
    if (!flightPathCoords.isEmpty()) {
        QJsonObject flightPath;
        flightPath[QStringLiteral("id")] = QStringLiteral("flight-path");
        flightPath[QStringLiteral("name")] = QStringLiteral("Flight Path");

        QJsonObject positions;
        positions[QStringLiteral("cartographicDegrees")] = flightPathCoords;

        QJsonObject material;
        QJsonObject solidColor;
        QJsonObject lineColor;
        lineColor[QStringLiteral("rgba")] = QJsonArray{ 33, 150, 243, 255 };
        solidColor[QStringLiteral("color")] = lineColor;
        material[QStringLiteral("solidColor")] = solidColor;

        QJsonObject polyline;
        polyline[QStringLiteral("positions")] = positions;
        polyline[QStringLiteral("material")] = material;
        polyline[QStringLiteral("width")] = 3;
        polyline[QStringLiteral("clampToGround")] = false;
        polyline[QStringLiteral("arcType")] = QStringLiteral("NONE");

        flightPath[QStringLiteral("polyline")] = polyline;
        czml.append(flightPath);
    }

    emit czmlGenerated(QString::fromUtf8(QJsonDocument(czml).toJson(QJsonDocument::Compact)),
                       requestId);
}
