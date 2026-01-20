#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QQmlEngine>

#include "../core/sensorreading.h"

class SerialHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY portsChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)
    Q_PROPERTY(QString currentPort READ currentPort NOTIFY connectionStateChanged)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged)

public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    // Property getters
    QStringList availablePorts() const;
    bool isConnected() const;
    QString errorString() const;
    QString currentPort() const;
    int baudRate() const;

    // Property setter
    void setBaudRate(int baudRate);

    // QML invokable methods
    Q_INVOKABLE void openPort(const QString &portName);
    Q_INVOKABLE void closePort();
    Q_INVOKABLE void refreshPorts();

signals:
    void newReading(const SensorReading &reading);
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString &message);
    void portsChanged();
    void baudRateChanged();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    void parseFrame(const QByteArray &frame);

    QSerialPort *m_serial;
    QByteArray m_buffer;
    QStringList m_ports;
    QString m_errorString;
    int m_baudRate = 115200;
};

#endif // SERIALHANDLER_H
