#include "zephyrschemehandler.h"

#include <QDir>
#include <QFile>
#include <QUrl>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>

ZephyrSchemeHandler::ZephyrSchemeHandler(QObject *parent)
    : QWebEngineUrlSchemeHandler(parent)
{
}

void ZephyrSchemeHandler::registerScheme()
{
    QWebEngineUrlScheme scheme(QByteArrayLiteral("zephyr"));
    scheme.setSyntax(QWebEngineUrlScheme::Syntax::Host);
    scheme.setFlags(QWebEngineUrlScheme::SecureScheme
                    | QWebEngineUrlScheme::LocalAccessAllowed
                    | QWebEngineUrlScheme::CorsEnabled);
    QWebEngineUrlScheme::registerScheme(scheme);
}

void ZephyrSchemeHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl url = request->requestUrl();
    QString path = QDir::cleanPath(url.path());

    if (path.contains(QStringLiteral(".."))) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    if (path.isEmpty() || path == QStringLiteral("/"))
        path = QStringLiteral("/index.html");

    QString resourcePath = QStringLiteral(":/web") + path;
    auto *file = new QFile(resourcePath, request);

    if (!file->open(QIODevice::ReadOnly)) {
        request->fail(QWebEngineUrlRequestJob::UrlNotFound);
        delete file;
        return;
    }

    // Determine content type — explicit overrides for web-critical types
    QByteArray contentType;
    if (path.endsWith(QStringLiteral(".js")) || path.endsWith(QStringLiteral(".mjs")))
        contentType = QByteArrayLiteral("application/javascript");
    else if (path.endsWith(QStringLiteral(".css")))
        contentType = QByteArrayLiteral("text/css");
    else if (path.endsWith(QStringLiteral(".json")))
        contentType = QByteArrayLiteral("application/json");
    else if (path.endsWith(QStringLiteral(".wasm")))
        contentType = QByteArrayLiteral("application/wasm");
    else if (path.endsWith(QStringLiteral(".html")))
        contentType = QByteArrayLiteral("text/html");
    else
        contentType = m_mimeDb.mimeTypeForFile(url.fileName()).name().toLatin1();

    // Device stays alive until the request is destroyed
    connect(request, &QObject::destroyed, file, &QObject::deleteLater);

    request->reply(contentType, file);
}
