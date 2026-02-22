#ifndef IOTHREAD_H
#define IOTHREAD_H

#include <QObject>
#include <QThread>

class IOWorker;

class IOThread : public QObject
{
    Q_OBJECT

public:
    explicit IOThread(const QString &databasePath, QObject *parent = nullptr);
    ~IOThread() override;

    void start();
    void stop();

    IOWorker* worker() const { return m_worker; }
    bool isRunning() const { return m_thread && m_thread->isRunning(); }

signals:
    void started();
    void stopped();

private:
    QThread *m_thread = nullptr;
    IOWorker *m_worker = nullptr;
    QString m_databasePath;
};

#endif // IOTHREAD_H
