#ifndef ZEPHYRSCHEMEHANDLER_H
#define ZEPHYRSCHEMEHANDLER_H

#include <QWebEngineUrlSchemeHandler>
#include <QMimeDatabase>

class ZephyrSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    explicit ZephyrSchemeHandler(QObject *parent = nullptr);

    // Register the "zephyr" scheme — MUST be called before QApplication
    static void registerScheme();

protected:
    void requestStarted(QWebEngineUrlRequestJob *request) override;

private:
    QMimeDatabase m_mimeDb;
};

#endif // ZEPHYRSCHEMEHANDLER_H
