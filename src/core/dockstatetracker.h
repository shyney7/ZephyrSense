#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QTimer>

namespace KDDockWidgets::Core {
class DockWidget;
}

class DockStateTracker final : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool hasDocksOutsideMainWindow READ hasDocksOutsideMainWindow NOTIFY changed FINAL)

public:
    explicit DockStateTracker(QObject *parent = nullptr);

    bool hasDocksOutsideMainWindow() const;
    Q_INVOKABLE void trackDockWidget(QObject *dockWidget);

signals:
    void changed();

private slots:
    void scheduleReevaluation();

private:
    void reevaluate();

    QList<QPointer<KDDockWidgets::Core::DockWidget>> m_coreDocks;
    QTimer m_deferTimer;
    bool m_hasDocksOutside = false;
};
