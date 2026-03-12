#ifndef CESIUMBRIDGE_H
#define CESIUMBRIDGE_H

#include <QObject>
#include <QQmlEngine>
#include <QThread>
#include <QUrl>
#include "sensorreading.h"

class QNetworkAccessManager;
class QNetworkReply;
class CesiumWorker;
class ThresholdManager;

class CesiumBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QUrl contentUrl READ contentUrl CONSTANT FINAL)
    Q_PROPERTY(QString cesiumToken READ cesiumToken WRITE setCesiumToken NOTIFY cesiumTokenChanged FINAL)
    Q_PROPERTY(bool liveMode READ liveMode WRITE setLiveMode NOTIFY liveModeChanged FINAL)
    Q_PROPERTY(int pendingRequestId READ pendingRequestId
               WRITE setPendingRequestId NOTIFY pendingRequestIdChanged FINAL)
    Q_PROPERTY(int windowMinutes READ windowMinutes
               WRITE setWindowMinutes NOTIFY windowMinutesChanged FINAL)
    Q_PROPERTY(bool validatingToken READ validatingToken NOTIFY validatingTokenChanged FINAL)
    Q_PROPERTY(bool tokenValid READ tokenValid NOTIFY tokenValidChanged FINAL)
    Q_PROPERTY(QString tokenError READ tokenError NOTIFY tokenErrorChanged FINAL)

public:
    explicit CesiumBridge(QObject *parent = nullptr);
    ~CesiumBridge() override;

    QUrl contentUrl() const;
    QString cesiumToken() const;
    bool liveMode() const;
    int pendingRequestId() const;
    int windowMinutes() const;
    bool validatingToken() const;
    bool tokenValid() const;
    QString tokenError() const;

    void setCesiumToken(const QString &token);
    void setLiveMode(bool enabled);
    void setPendingRequestId(int id);
    void setWindowMinutes(int minutes);
    void setThresholdManager(ThresholdManager *manager);

    Q_INVOKABLE void loadRange(qint64 startMsecs, qint64 endMsecs, int requestId);
    Q_INVOKABLE QString getThresholdConfig();
    Q_INVOKABLE void validateToken(const QString &token);

signals:
    void czmlReady(const QString &czmlJson, int requestId);
    void czmlPacket(const QString &czmlJson);
    void cesiumTokenChanged();
    void liveModeChanged(bool liveMode);
    void pendingRequestIdChanged(int pendingRequestId);
    void windowMinutesChanged(int windowMinutes);
    void thresholdsChanged(const QVariantMap &config);
    void validatingTokenChanged();
    void tokenValidChanged();
    void tokenErrorChanged();
    void tokenValidationSucceeded();
    void tokenValidationFailed(const QString &error);

public slots:
    void onNewReading(const SensorReading &reading);
    void onThresholdsChanged();

private slots:
    void onCzmlGenerated(const QString &czmlJson, int requestId);

private:
    qint64 windowStartMsecs() const;

    QUrl m_contentUrl;
    ThresholdManager *m_thresholdManager = nullptr;
    QNetworkAccessManager *m_networkManager = nullptr;
    QThread *m_workerThread = nullptr;
    CesiumWorker *m_worker = nullptr;
    QString m_tokenError;
    int m_pendingRequestId = -1;
    int m_pendingLiveSwitchRequestId = -1;
    int m_windowMinutes = 60;
    bool m_liveMode = true;
    bool m_pendingLiveSwitch = false;
    bool m_validatingToken = false;
    bool m_tokenValid = false;
};

#endif // CESIUMBRIDGE_H
