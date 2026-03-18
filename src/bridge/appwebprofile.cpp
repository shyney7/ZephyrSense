#include "appwebprofile.h"

#include <QJSEngine>
#include <QQmlEngine>
#include <QThread>

static QQuickWebEngineProfile *s_instance = nullptr;
static QJSEngine *s_engine = nullptr;

void AppWebProfileForeign::setInstance(QQuickWebEngineProfile *profile)
{
    Q_ASSERT_X(profile, "AppWebProfileForeign::setInstance",
               "profile must not be null");
    Q_ASSERT_X(!s_instance || s_instance == profile, "AppWebProfileForeign::setInstance",
               "AppWebProfile singleton instance cannot be replaced");
    s_instance = profile;
}

QQuickWebEngineProfile *AppWebProfileForeign::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT_X(s_instance, "AppWebProfileForeign::create",
               "WebEngine profile not initialized — call setInstance() before loading QML");
    Q_ASSERT_X(engine->thread() == s_instance->thread(), "AppWebProfileForeign::create",
               "QML engine and AppWebProfile must live on the same thread");

    if (s_engine)
        Q_ASSERT_X(engine == s_engine, "AppWebProfileForeign::create",
                   "AppWebProfile singleton is only supported with one QML engine");
    else
        s_engine = engine;

    // C++ side manages lifetime via QObject parent (QApplication).
    // Prevent the QML engine from taking ownership and double-deleting.
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}
