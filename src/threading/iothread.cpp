#include "iothread.h"
#include "ioworker.h"

#include <QDebug>

IOThread::IOThread(const QString &databasePath, QObject *parent)
    : QObject(parent)
    , m_databasePath(databasePath)
{
    m_thread = new QThread(this);
    m_worker = new IOWorker();  // No parent - will be moved to thread

    // Move worker to the dedicated I/O thread
    m_worker->moveToThread(m_thread);

    // Initialize worker when thread starts
    connect(m_thread, &QThread::started, this, [this]() {
        qDebug() << "IOThread: Thread started, initializing worker...";
        // Use invokeMethod to ensure initialization happens on the worker thread
        QMetaObject::invokeMethod(m_worker, "initialize",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, m_databasePath));
        emit started();
    });

    // Clean up worker when thread finishes
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, this, [this]() {
        qDebug() << "IOThread: Thread finished";
        emit stopped();
    });
}

IOThread::~IOThread()
{
    stop();
}

void IOThread::start()
{
    if (m_thread && !m_thread->isRunning()) {
        qDebug() << "IOThread: Starting I/O thread...";
        m_thread->start();
    }
}

void IOThread::stop()
{
    if (m_thread && m_thread->isRunning()) {
        qDebug() << "IOThread: Stopping I/O thread...";

        // Flush pending writes before stopping
        // Use BlockingQueuedConnection to ensure flush completes before we quit
        QMetaObject::invokeMethod(m_worker, "flushAll",
                                  Qt::BlockingQueuedConnection);

        m_thread->quit();
        m_thread->wait();
        qDebug() << "IOThread: I/O thread stopped";
    }
}
