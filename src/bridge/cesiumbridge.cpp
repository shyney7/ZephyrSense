#include "cesiumbridge.h"
#include "cesiumworker.h"
#include "coordinatevalidator.h"
#include "thresholdmanager.h"

#include <QDateTime>
#include <QHttpHeaders>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStandardPaths>
#include <QTimeZone>

CesiumBridge::CesiumBridge(QObject *parent, QNetworkAccessManager *networkManager)
    : QObject(parent)
{
    setObjectName(QStringLiteral("CesiumBridge"));
    m_networkManager = networkManager ? networkManager : new QNetworkAccessManager(this);
    if (networkManager && !networkManager->parent())
        networkManager->setParent(this);

    // Content URL based on build mode
#ifdef WEBDEV_MODE
    m_contentUrl = QUrl(QStringLiteral("http://localhost:5173"));
#else
    m_contentUrl = QUrl(QStringLiteral("zephyr://app/index.html"));
#endif

    // Create worker thread
    m_workerThread = new QThread(this);
    m_worker = new CesiumWorker();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::started, m_worker, [this]() {
        const QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                               + QStringLiteral("/zephyrsense.db");
        m_worker->initialize(dbPath);
    });
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &CesiumWorker::czmlGenerated,
            this, &CesiumBridge::onCzmlGenerated, Qt::QueuedConnection);

    m_workerThread->start();
}

CesiumBridge::~CesiumBridge()
{
    m_workerThread->quit();
    m_workerThread->wait();
}

QUrl CesiumBridge::contentUrl() const
{
    return m_contentUrl;
}

QString CesiumBridge::cesiumToken() const
{
    QSettings settings(QStringLiteral("ZephyrSense"), QStringLiteral("ZephyrSense"));
    return settings.value(QStringLiteral("cesiumIonToken")).toString();
}

void CesiumBridge::setCesiumToken(const QString &token)
{
    if (cesiumToken() == token)
        return;

    QSettings settings(QStringLiteral("ZephyrSense"), QStringLiteral("ZephyrSense"));
    settings.setValue(QStringLiteral("cesiumIonToken"), token);
    emit cesiumTokenChanged();
}

bool CesiumBridge::validatingToken() const
{
    return m_validatingToken;
}

bool CesiumBridge::tokenValid() const
{
    return m_tokenValid;
}

QString CesiumBridge::tokenError() const
{
    return m_tokenError;
}

void CesiumBridge::validateToken(const QString &token)
{
    if (m_validatingToken)
        return;

    if (token.trimmed().isEmpty()) {
        m_tokenValid = false;
        m_tokenError = QStringLiteral("Token cannot be empty.");
        emit tokenValidChanged();
        emit tokenErrorChanged();
        emit tokenValidationFailed(m_tokenError);
        return;
    }

    m_validatingToken = true;
    m_tokenValid = false;
    m_tokenError.clear();
    emit validatingTokenChanged();
    emit tokenValidChanged();
    emit tokenErrorChanged();

    QNetworkRequest request(
        QUrl(QStringLiteral("https://api.cesium.com/v1/assets/1/endpoint")));

    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::Authorization,
                   QString(QStringLiteral("Bearer ") + token));
    request.setHeaders(std::move(headers));
    request.setTransferTimeout(15000);

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        m_validatingToken = false;
        emit validatingTokenChanged();

        const QVariant statusVariant =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        if (statusVariant.isValid()) {
            const int statusCode = statusVariant.toInt();
            if (statusCode == 200) {
                m_tokenValid = true;
                m_tokenError.clear();
                emit tokenValidChanged();
                emit tokenErrorChanged();
                emit tokenValidationSucceeded();
            } else if (statusCode == 401 || statusCode == 403) {
                m_tokenValid = false;
                m_tokenError = QStringLiteral(
                    "Invalid token. Please check your Cesium Ion access token.");
                emit tokenValidChanged();
                emit tokenErrorChanged();
                emit tokenValidationFailed(m_tokenError);
            } else {
                m_tokenValid = false;
                m_tokenError = QStringLiteral("Unexpected response (HTTP %1). Please try again.")
                                   .arg(statusCode);
                emit tokenValidChanged();
                emit tokenErrorChanged();
                emit tokenValidationFailed(m_tokenError);
            }
        } else {
            m_tokenValid = false;
            m_tokenError = QStringLiteral("Network error: %1").arg(reply->errorString());
            emit tokenValidChanged();
            emit tokenErrorChanged();
            emit tokenValidationFailed(m_tokenError);
        }
    });
}

bool CesiumBridge::liveMode() const
{
    return m_liveMode;
}

int CesiumBridge::pendingRequestId() const
{
    return m_pendingRequestId;
}

int CesiumBridge::windowMinutes() const
{
    return m_windowMinutes;
}

void CesiumBridge::setLiveMode(bool enabled)
{
    if (enabled == m_liveMode && !m_pendingLiveSwitch)
        return;
    if (enabled && m_pendingLiveSwitch)
        return; // prevent double dispatch

    if (enabled && !m_liveMode) {
        // Switching TO live: load current window first, then enable forwarding
        m_pendingLiveSwitch = true;
        setPendingRequestId(m_pendingRequestId + 1);
        m_pendingLiveSwitchRequestId = m_pendingRequestId;
        loadRange(windowStartMsecs(),
                  QDateTime::currentMSecsSinceEpoch(),
                  m_pendingRequestId);
        return; // liveModeChanged deferred to onCzmlGenerated
    }

    m_liveMode = enabled;
    m_pendingLiveSwitch = false;
    m_pendingLiveSwitchRequestId = -1;
    emit liveModeChanged(m_liveMode);
}

void CesiumBridge::setPendingRequestId(int id)
{
    if (m_pendingRequestId == id)
        return;
    m_pendingRequestId = id;
    emit pendingRequestIdChanged(m_pendingRequestId);
}

void CesiumBridge::setWindowMinutes(int minutes)
{
    if (m_windowMinutes == minutes)
        return;
    m_windowMinutes = minutes;
    emit windowMinutesChanged(m_windowMinutes);
}

void CesiumBridge::setThresholdManager(ThresholdManager *manager)
{
    m_thresholdManager = manager;
}

void CesiumBridge::setJsReady(bool ready)
{
    if (m_jsReady.value() == ready)
        return;

    m_jsReady = ready; // auto-emits jsReadyChanged()

    if (ready && m_queuedLoad) {
        const auto req = *m_queuedLoad;
        m_queuedLoad.reset();
        loadRange(req.startMsecs, req.endMsecs, req.requestId);
    }
}

void CesiumBridge::loadRange(qint64 startMsecs, qint64 endMsecs, int requestId)
{
    if (!m_jsReady.value()) {
        m_queuedLoad = PendingLoadRequest{startMsecs, endMsecs, requestId};
        return;
    }

    QVariantMap thresholdSnapshot;
    if (m_thresholdManager)
        thresholdSnapshot = m_thresholdManager->getThresholds();

    QMetaObject::invokeMethod(m_worker, "generateCzml",
        Qt::QueuedConnection,
        Q_ARG(qint64, startMsecs),
        Q_ARG(qint64, endMsecs),
        Q_ARG(int, requestId),
        Q_ARG(QVariantMap, thresholdSnapshot));
}

QString CesiumBridge::getThresholdConfig()
{
    if (!m_thresholdManager)
        return QStringLiteral("{}");

    const QVariantMap thresholds = m_thresholdManager->getThresholds();
    return QString::fromUtf8(
        QJsonDocument(QJsonObject::fromVariantMap(thresholds)).toJson(QJsonDocument::Compact));
}

void CesiumBridge::onNewReading(const SensorReading &reading)
{
    if (!m_liveMode)
        return;

    if (!m_jsReady.value())
        return;

    if (!isValidCoordinate(reading.latitude, reading.longitude))
        return;

    // Build single-point CZML packet
    QVariantMap thresholdSnapshot;
    if (m_thresholdManager)
        thresholdSnapshot = m_thresholdManager->getThresholds();

    const int hazard = CesiumWorker::computeHazardFromSnapshot(
        reading.co2, reading.temperature, reading.humidity,
        reading.partectorMass, reading.grimmValue,
        reading.partectorNumber, reading.partectorDiam,
        reading.pressure, reading.altitude, thresholdSnapshot);

    const QJsonArray pointColor = (hazard == 2) ? QJsonArray{ 244, 67, 54, 255 }
                                : (hazard == 1) ? QJsonArray{ 255, 152, 0, 255 }
                                                : QJsonArray{ 76, 175, 80, 255 };

    const QString tsStr = reading.timestamp.toUTC()
                              .toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));

    const QString hazardLabel = (hazard == 2) ? QStringLiteral("Danger")
                              : (hazard == 1) ? QStringLiteral("Warning")
                                              : QStringLiteral("Normal");
    const QString hazardHtmlColor = (hazard == 2) ? QStringLiteral("#F44336")
                                  : (hazard == 1) ? QStringLiteral("#FF9800")
                                                  : QStringLiteral("#4CAF50");

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
        .arg(reading.partectorNumber)
        .arg(reading.partectorDiam)
        .arg(static_cast<double>(reading.partectorMass), 0, 'f', 2)
        .arg(static_cast<double>(reading.grimmValue), 0, 'f', 2)
        .arg(static_cast<double>(reading.temperature), 0, 'f', 1)
        .arg(static_cast<double>(reading.humidity), 0, 'f', 1)
        .arg(static_cast<double>(reading.pressure), 0, 'f', 1)
        .arg(static_cast<double>(reading.altitude), 0, 'f', 1)
        .arg(reading.co2)
        .arg(hazardHtmlColor, hazardLabel);

    const qint64 nowMs = reading.timestamp.toMSecsSinceEpoch();

    QJsonObject position;
    position[QStringLiteral("cartographicDegrees")] = QJsonArray{
        static_cast<double>(reading.longitude),
        static_cast<double>(reading.latitude),
        static_cast<double>(reading.altitude)
    };

    QJsonObject colorObj;
    colorObj[QStringLiteral("rgba")] = pointColor;

    QJsonObject pointStyle;
    pointStyle[QStringLiteral("pixelSize")] = 10;
    pointStyle[QStringLiteral("color")] = colorObj;

    const QString readingId = QStringLiteral("live-%1").arg(nowMs);
    QJsonObject pointPacket;
    pointPacket[QStringLiteral("id")] = readingId;
    pointPacket[QStringLiteral("name")] = QStringLiteral("Live Reading");
    pointPacket[QStringLiteral("description")] = description;
    pointPacket[QStringLiteral("position")] = position;
    pointPacket[QStringLiteral("point")] = pointStyle;

    QJsonArray czml;
    czml.append(pointPacket);

    emit czmlPacket(QString::fromUtf8(QJsonDocument(czml).toJson(QJsonDocument::Compact)));
}

void CesiumBridge::onCzmlGenerated(const QString &czmlJson, int requestId)
{
    if (m_pendingLiveSwitch && requestId == m_pendingLiveSwitchRequestId) {
        m_liveMode = true;
        m_pendingLiveSwitch = false;
        m_pendingLiveSwitchRequestId = -1;
        emit liveModeChanged(true);
    }
    emit czmlReady(czmlJson, requestId);
}

void CesiumBridge::onThresholdsChanged()
{
    if (!m_thresholdManager)
        return;
    emit thresholdsChanged(m_thresholdManager->getThresholds());
}

qint64 CesiumBridge::windowStartMsecs() const
{
    return QDateTime::currentMSecsSinceEpoch() - static_cast<qint64>(m_windowMinutes) * 60 * 1000;
}
