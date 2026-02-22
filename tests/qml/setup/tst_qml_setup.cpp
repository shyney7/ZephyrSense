// SPDX-License-Identifier: MIT
// QML test entry point — mock singletons are registered via qt_add_qml_module
// (see mocks.h). Only _sourceDir context property is needed for file URL construction.

#include "mocks.h"

#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQuickTest>

class QmlTestSetup : public QObject
{
    Q_OBJECT

public slots:
    void applicationAvailable() {
        QQuickStyle::setStyle(QStringLiteral("Fusion"));
    }

    void qmlEngineAvailable(QQmlEngine *engine) {
        engine->rootContext()->setContextProperty(
            QStringLiteral("_sourceDir"),
            QStringLiteral(QML_SOURCE_DIR));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(ZephyrSenseQml, QmlTestSetup)
#include "tst_qml_setup.moc"
