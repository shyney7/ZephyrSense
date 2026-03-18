#include "dockstatetracker.h"

#include <QQuickItem>
#include <utility>
#include <QQuickWindow>
#include <kddockwidgets/core/DockWidget.h>
#include <DockWidgetInstantiator.h>
#include <kddockwidgets/qtquick/views/DockWidget.h>

DockStateTracker::DockStateTracker(QObject *parent)
    : QObject(parent)
{
    m_deferTimer.setSingleShot(true);
    connect(&m_deferTimer, &QTimer::timeout, this, &DockStateTracker::reevaluate);
}

bool DockStateTracker::hasDocksOutsideMainWindow() const
{
    return m_hasDocksOutside;
}

void DockStateTracker::trackDockWidget(QObject *dockWidget)
{
    if (!dockWidget)
        return;

    // The QML KDDW.DockWidget is a DockWidgetInstantiator, not QtQuick::DockWidget
    // directly. Cast to the concrete type for type-safe signal connections.
    auto *instantiator = qobject_cast<KDDockWidgets::DockWidgetInstantiator *>(dockWidget);
    if (!instantiator) {
        qWarning() << "[DockStateTracker] not a DockWidgetInstantiator:" << dockWidget;
        return;
    }

    auto *qtquickDock = instantiator->dockWidget();
    if (!qtquickDock) {
        qWarning() << "[DockStateTracker] dockWidget() returned null for" << dockWidget;
        return;
    }

    auto *coreDock = qtquickDock->dockWidget();
    if (!coreDock || m_coreDocks.contains(coreDock))
        return;

    m_coreDocks.append(coreDock);

    connect(instantiator, &KDDockWidgets::DockWidgetInstantiator::isFloatingChanged,
            this, &DockStateTracker::scheduleReevaluation);
    connect(instantiator, &KDDockWidgets::DockWidgetInstantiator::isOpenChanged,
            this, &DockStateTracker::scheduleReevaluation);

    // Connect to the actual QQuickItem's windowChanged signal — fires when the
    // dock moves between the main window and a floating window
    connect(qtquickDock, &QQuickItem::windowChanged,
            this, &DockStateTracker::scheduleReevaluation);
}

void DockStateTracker::scheduleReevaluation()
{
    m_deferTimer.start(0);
}

void DockStateTracker::reevaluate()
{
    bool hasAny = false;
    for (const auto &ptr : std::as_const(m_coreDocks)) {
        if (ptr && ptr->isOpen() && !ptr->isInMainWindow()) {
            hasAny = true;
            break;
        }
    }

    if (hasAny != m_hasDocksOutside) {
        m_hasDocksOutside = hasAny;
        emit changed();
    }
}
