#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QQmlEngine>

#include "sensorreading.h"

class SerialHandler final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY portsChanged FINAL)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStateChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred FINAL)
    Q_PROPERTY(QString currentPort READ currentPort NOTIFY connectionStateChanged FINAL)
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate NOTIFY baudRateChanged FINAL)

public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler() override;

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
    void processBuffer();
    void parseFrame(const QByteArray &frame);

    QSerialPort *m_serial;
    QByteArray m_buffer;
    QStringList m_ports;
    QString m_errorString;
    int m_baudRate = 115200;

#ifdef ZEPHYR_TESTING
public:
    void injectTestData(const QByteArray &data)
    {
        m_buffer.append(data);
        processBuffer();
    }
#endif
};

#endif // SERIALHANDLER_H
