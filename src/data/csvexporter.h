#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QObject>
#include <QQmlEngine>
#include <QUrl>
#include "sensorreading.h"

class CsvExporter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)

public:
    explicit CsvExporter(QObject *parent = nullptr);

    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path);

    Q_INVOKABLE void setFilePathFromUrl(const QUrl &url);

public slots:
    void appendReading(const SensorReading &reading);

signals:
    void enabledChanged();
    void filePathChanged();
    void exportError(const QString &message);

private:
    bool m_enabled = false;
    QString m_filePath;
};

#endif // CSVEXPORTER_H
