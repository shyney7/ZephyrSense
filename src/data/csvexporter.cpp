#include "csvexporter.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
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
    if (!m_enabled || m_filePath.isEmpty()) {
        return;
    }

    // Check if file exists and has content before opening
    QFileInfo fileInfo(m_filePath);
    bool needsHeader = !fileInfo.exists() || fileInfo.size() == 0;

    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QString error = QString("Failed to open CSV file: %1").arg(file.errorString());
        qWarning() << "CsvExporter:" << error;
        emit exportError(error);
        return;
    }

    QTextStream stream(&file);

    // Write header if this is a new/empty file
    if (needsHeader) {
        stream << "timestamp,partector_number,partector_diam,partector_mass,"
               << "grimm_value,temperature,humidity,pressure,"
               << "altitude,latitude,longitude,co2\n";
    }

    // Write data row
    stream << reading.timestamp.toString(Qt::ISODate) << ","
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

    stream.flush();
    file.close();
}
