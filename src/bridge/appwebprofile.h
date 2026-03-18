#pragma once

#include <QQuickWebEngineProfile>
#include <QtQml/qqml.h>

// Foreign type wrapper: exposes a pre-configured QQuickWebEngineProfile as
// "AppWebProfile" singleton in the ZephyrSense QML module.
// Using QML_FOREIGN + QML_SINGLETON gives compile-time visibility to qmllint
// and qmlsc, unlike runtime-only qmlRegisterSingletonInstance().
struct AppWebProfileForeign
{
    Q_GADGET
    QML_FOREIGN(QQuickWebEngineProfile)
    QML_NAMED_ELEMENT(AppWebProfile)
    QML_SINGLETON

public:
    // Called by the QML engine to obtain the singleton instance.
    // CRITICAL: setInstance() must be called before engine.loadFromModule().
    static QQuickWebEngineProfile *create(QQmlEngine *engine, QJSEngine *);

    // Call from main.cpp to provide the pre-configured profile instance.
    static void setInstance(QQuickWebEngineProfile *profile);
};

