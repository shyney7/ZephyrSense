#include "csvexporter.h"

#include <QDebug>

CsvExporter::CsvExporter(QObject *parent)
    : QObject(parent)
{
}

void CsvExporter::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged();
        qDebug() << "CsvExporter: enabled =" << m_enabled;
    }
}

void CsvExporter::setFilePath(const QString &path)
{
    if (m_filePath != path) {
        m_filePath = path;
        emit filePathChanged();
        qDebug() << "CsvExporter: filePath =" << m_filePath;
    }
}

void CsvExporter::setFilePathFromUrl(const QUrl &url)
{
    setFilePath(url.toLocalFile());
}

void CsvExporter::appendReading(const SensorReading &reading)
{
    // NOTE: This slot is now deprecated. CSV writes are handled by IOWorker
    // on a dedicated I/O thread for non-blocking UI performance.
    // This slot remains for API compatibility but does nothing.
    // Configuration changes (enabled, filePath) are relayed to IOWorker via signals.
    Q_UNUSED(reading)
}
