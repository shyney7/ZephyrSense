#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "src/core/sensorreading.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Register SensorReading for use in signal/slot and QML
    qRegisterMetaType<SensorReading>("SensorReading");

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("ZephyrSense", "Main");

    return app.exec();
}
