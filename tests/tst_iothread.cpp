#include <QTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include "iothread.h"
#include "ioworker.h"

class TestIOThread : public QObject
{
    Q_OBJECT

private slots:
    void init()
    {
        m_tmpDir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_tmpDir->isValid());
        m_dbPath = m_tmpDir->filePath(QStringLiteral("test.db"));
    }

    void cleanup()
    {
        if (QSqlDatabase::contains(QStringLiteral("ZephyrSenseIOWorker"))) {
            QSqlDatabase::removeDatabase(QStringLiteral("ZephyrSenseIOWorker"));
        }
        m_tmpDir.reset();
    }

    void constructor_workerNotNull()
    {
        IOThread ioThread(m_dbPath);
        QVERIFY(ioThread.worker() != nullptr);
    }

    void constructor_notRunning()
    {
        IOThread ioThread(m_dbPath);
        QCOMPARE(ioThread.isRunning(), false);
    }

    void start_startsThread()
    {
        IOThread ioThread(m_dbPath);
        ioThread.start();
        QVERIFY(ioThread.isRunning());
        ioThread.stop();
    }

    void start_emitsStartedSignal()
    {
        IOThread ioThread(m_dbPath);
        QSignalSpy spy(&ioThread, &IOThread::started);
        ioThread.start();
        QVERIFY(spy.wait(3000));
        QCOMPARE(spy.count(), 1);
        ioThread.stop();
    }

    void stop_stopsThread()
    {
        IOThread ioThread(m_dbPath);
        ioThread.start();
        QVERIFY(ioThread.isRunning());

        ioThread.stop();
        QCOMPARE(ioThread.isRunning(), false);
    }

    void stop_emitsStoppedSignal()
    {
        IOThread ioThread(m_dbPath);
        QSignalSpy startSpy(&ioThread, &IOThread::started);
        ioThread.start();
        // Wait for thread to be fully started
        QVERIFY(startSpy.wait(3000));

        QSignalSpy stopSpy(&ioThread, &IOThread::stopped);
        ioThread.stop();
        QVERIFY(stopSpy.wait(3000));
        QCOMPARE(stopSpy.count(), 1);
    }

    void stop_whenNotRunning_noOp()
    {
        IOThread ioThread(m_dbPath);
        // Should not crash
        ioThread.stop();
        QCOMPARE(ioThread.isRunning(), false);
    }

    void destructor_stopsThread()
    {
        auto ioThread = std::make_unique<IOThread>(m_dbPath);
        QSignalSpy startSpy(ioThread.get(), &IOThread::started);
        ioThread->start();
        QVERIFY(startSpy.wait(3000));
        QVERIFY(ioThread->isRunning());

        // Destructor should stop the thread
        ioThread.reset();
        // No crash = pass
    }

private:
    std::unique_ptr<QTemporaryDir> m_tmpDir;
    QString m_dbPath;
};

QTEST_GUILESS_MAIN(TestIOThread)
#include "tst_iothread.moc"
