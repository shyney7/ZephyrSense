#include "serialhandler.h"

#include <QDebug>
#include <cstring>

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
    , m_baudRate(115200)
{
    connect(m_serial, &QSerialPort::readyRead, this, &SerialHandler::handleReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialHandler::handleError);

    // Initial port enumeration
    refreshPorts();
}

SerialHandler::~SerialHandler()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
}

QStringList SerialHandler::availablePorts() const
{
    return m_ports;
}

bool SerialHandler::isConnected() const
{
    return m_serial->isOpen();
}

QString SerialHandler::errorString() const
{
    return m_errorString;
}

QString SerialHandler::currentPort() const
{
    return m_serial->portName();
}

int SerialHandler::baudRate() const
{
    return m_baudRate;
}

void SerialHandler::setBaudRate(int baudRate)
{
    if (m_baudRate != baudRate) {
        m_baudRate = baudRate;
        // Update serial port if already open
        if (m_serial->isOpen()) {
            m_serial->setBaudRate(m_baudRate);
        }
        emit baudRateChanged();
    }
}

void SerialHandler::refreshPorts()
{
    m_ports.clear();
    const auto portInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : portInfos) {
        QString entry = info.portName();
        if (!info.description().isEmpty()) {
            entry += " - " + info.description();
        }
        m_ports.append(entry);
    }
    emit portsChanged();
}

void SerialHandler::openPort(const QString &portName)
{
    // Close if already open
    if (m_serial->isOpen()) {
        m_serial->close();
    }

    // Parse port name (take first word before " - ")
    QString actualPortName = portName.split(" - ").first().trimmed();

    m_serial->setPortName(actualPortName);
    m_serial->setBaudRate(m_baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serial->open(QIODevice::ReadOnly)) {
        m_buffer.clear();
        m_errorString.clear();
        qDebug() << "Serial port opened:" << actualPortName << "at" << m_baudRate << "baud";
        emit connectionStateChanged(true);
    } else {
        m_errorString = m_serial->errorString();
        qWarning() << "Failed to open serial port:" << m_errorString;
        emit errorOccurred(m_errorString);
    }
}

void SerialHandler::closePort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        m_buffer.clear();
        qDebug() << "Serial port closed";
        emit connectionStateChanged(false);
    }
}

void SerialHandler::handleReadyRead()
{
    // Append incoming data to buffer
    m_buffer.append(m_serial->readAll());

    // Frame detection state machine
    // Protocol: '<' + 42 bytes data + '>' = 44 bytes total
    constexpr int DATA_SIZE = sizeof(SensorDataRaw);  // 42 bytes
    constexpr int FRAME_SIZE = DATA_SIZE + 2;         // 44 bytes with delimiters

    while (true) {
        // Find start delimiter '<'
        int startIdx = m_buffer.indexOf('<');
        if (startIdx == -1) {
            // No start delimiter found, discard all data
            m_buffer.clear();
            return;
        }

        // Discard bytes before start delimiter
        if (startIdx > 0) {
            m_buffer.remove(0, startIdx);
        }

        // Check if we have enough data for a complete frame
        if (m_buffer.size() < FRAME_SIZE) {
            return;  // Wait for more data
        }

        // Find end delimiter '>' (search after position 1)
        int endIdx = m_buffer.indexOf('>', 1);
        if (endIdx == -1) {
            // No end delimiter yet, wait for more data
            return;
        }

        // Check if we found a valid frame (exactly DATA_SIZE bytes between delimiters)
        int frameSize = endIdx - 1;  // Bytes between '<' and '>'
        if (frameSize == DATA_SIZE) {
            // Extract frame (excluding delimiters)
            QByteArray frame = m_buffer.mid(1, DATA_SIZE);
            parseFrame(frame);
        } else {
            // Invalid frame size - likely corruption
            qWarning() << "Invalid frame size:" << frameSize << "bytes, expected" << DATA_SIZE;
        }

        // Remove processed data from buffer (including delimiters)
        m_buffer.remove(0, endIdx + 1);
    }
}

void SerialHandler::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }

    m_errorString = m_serial->errorString();
    qWarning() << "Serial port error:" << error << "-" << m_errorString;

    // Handle critical errors that require closing the port
    switch (error) {
    case QSerialPort::ResourceError:
        // Device disconnected
        closePort();
        break;
    case QSerialPort::DeviceNotFoundError:
    case QSerialPort::PermissionError:
    case QSerialPort::OpenError:
        // Port cannot be used
        if (m_serial->isOpen()) {
            m_serial->close();
        }
        emit connectionStateChanged(false);
        break;
    default:
        break;
    }

    emit errorOccurred(m_errorString);
}

void SerialHandler::parseFrame(const QByteArray &frame)
{
    if (frame.size() != sizeof(SensorDataRaw)) {
        qWarning() << "Invalid frame size:" << frame.size() << "expected:" << sizeof(SensorDataRaw);
        return;
    }

    // Copy binary data to packed struct
    SensorDataRaw raw;
    std::memcpy(&raw, frame.constData(), sizeof(SensorDataRaw));

    // Create high-level reading with timestamp
    SensorReading reading(raw);

    qDebug() << "Parsed sensor reading - Temp:" << reading.temperature
             << "Humidity:" << reading.humidity
             << "Lat:" << reading.latitude
             << "Lon:" << reading.longitude;

    emit newReading(reading);
}
