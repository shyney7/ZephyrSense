#include "serialhandler.h"

#include <QDebug>
#include <algorithm>
#include <array>
#include <bit>

SerialHandler::SerialHandler(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
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
            entry += QStringLiteral(" - ") + info.description();
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
    QString actualPortName = portName.split(QStringLiteral(" - ")).first().trimmed();

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
    m_buffer.append(m_serial->readAll());
    processBuffer();
}

void SerialHandler::processBuffer()
{
    // Frame detection for fixed-size binary protocol
    // Protocol: '<' + 42 bytes data + '>' = 44 bytes total
    // IMPORTANT: Binary data may contain '<' or '>' bytes, so we check
    // the end delimiter at the EXPECTED position, not by searching.
    constexpr int DATA_SIZE = sizeof(SensorDataRaw);  // 42 bytes
    constexpr int FRAME_SIZE = DATA_SIZE + 2;         // 44 bytes with delimiters
    constexpr int END_DELIMITER_POS = FRAME_SIZE - 1; // Position 43 (0-indexed)

    while (m_buffer.size() >= FRAME_SIZE) {
        // Find start delimiter '<'
        qsizetype startIdx = m_buffer.indexOf('<');
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

        // Check end delimiter at EXPECTED position (not by searching)
        // This handles binary data that may contain '<' or '>' bytes
        if (m_buffer.at(END_DELIMITER_POS) == '>') {
            // Valid frame found - extract and parse
            QByteArray frame = m_buffer.mid(1, DATA_SIZE);
            parseFrame(frame);
            // Remove processed frame from buffer
            m_buffer.remove(0, FRAME_SIZE);
        } else {
            // End delimiter not at expected position - this '<' was a false start
            // (likely a '<' byte inside previous corrupted/partial frame data)
            // Skip this '<' and search for the next potential start
            qDebug() << "Frame sync: skipping false start delimiter at position 0";
            m_buffer.remove(0, 1);
        }
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
    case QSerialPort::ReadError:
        // Device disconnected or read failure — close and reset
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
    case QSerialPort::NoError:
    case QSerialPort::WriteError:
    case QSerialPort::UnsupportedOperationError:
    case QSerialPort::UnknownError:
    case QSerialPort::TimeoutError:
    case QSerialPort::NotOpenError:
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

    // Type-safe binary deserialization via std::bit_cast (C++20)
    static_assert(std::is_trivially_copyable_v<SensorDataRaw>,
                  "SensorDataRaw must be trivially copyable for bit_cast");
    std::array<char, sizeof(SensorDataRaw)> buf;
    std::ranges::copy_n(frame.constData(), sizeof(SensorDataRaw), buf.begin());
    const SensorDataRaw raw = std::bit_cast<SensorDataRaw>(buf);

    // Create high-level reading with timestamp
    SensorReading reading(raw);

    qDebug() << "Parsed sensor reading - Temp:" << reading.temperature
             << "Humidity:" << reading.humidity
             << "Lat:" << reading.latitude
             << "Lon:" << reading.longitude;

    emit newReading(reading);
}
