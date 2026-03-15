#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class MockNetworkReply : public QNetworkReply
{
public:
    explicit MockNetworkReply(QObject *parent = nullptr)
        : QNetworkReply(parent)
    {
        open(QIODevice::ReadOnly);
    }

    void abort() override {}

    void finish(int httpStatus)
    {
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, httpStatus);
        setFinished(true);
        emit finished();
    }

    void finishWithError(NetworkError error, const QString &errorString)
    {
        setError(error, errorString);
        setFinished(true);
        emit finished();
    }

protected:
    qint64 readData(char *, qint64) override { return -1; }
    qint64 writeData(const char *, qint64) override { return -1; }
};

class MockNetworkAccessManager : public QNetworkAccessManager
{
public:
    using QNetworkAccessManager::QNetworkAccessManager;

    MockNetworkReply *lastReply() const { return m_lastReply; }
    int requestCount() const { return m_requestCount; }

protected:
    QNetworkReply *createRequest(Operation /*op*/,
                                 const QNetworkRequest & /*request*/,
                                 QIODevice * /*outgoingData*/ = nullptr) override
    {
        ++m_requestCount;
        m_lastReply = new MockNetworkReply(this);
        return m_lastReply;
    }

private:
    MockNetworkReply *m_lastReply = nullptr;
    int m_requestCount = 0;
};
