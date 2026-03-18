#include "dockstatetracker.h"

#include <QQuickItem>
#include <utility>
#include <QQuickWindow>
#include <kddockwidgets/core/DockWidget.h>
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

    // The QML KDDW.DockWidget type is DockWidgetInstantiator, not QtQuick::DockWidget
    // directly. Access the actual dock via the "dockWidget" Q_PROPERTY.
    // String-based connect is required: DockWidgetInstantiator lacks DOCKS_EXPORT,
    // so its symbols are not exported from the KDDW DLL on Windows.
    QVariant dwVar = dockWidget->property("dockWidget");
    auto *qtquickDock = qvariant_cast<KDDockWidgets::QtQuick::DockWidget *>(dwVar);
    if (!qtquickDock) {
        qWarning() << "[DockStateTracker] dockWidget property cast failed for" << dockWidget;
        return;
    }

    auto *coreDock = qtquickDock->dockWidget();
    if (!coreDock || m_coreDocks.contains(coreDock))
        return;

    m_coreDocks.append(coreDock);

    // NOLINTBEGIN(clazy-old-style-connect)
    // DockWidgetInstantiator is not DLL-exported (missing DOCKS_EXPORT),
    // so PMF (pointer-to-member-function) connects cannot link on Windows.
    connect(dockWidget, SIGNAL(isFloatingChanged(bool)),
            this, SLOT(scheduleReevaluation()));
    connect(dockWidget, SIGNAL(isOpenChanged(bool)),
            this, SLOT(scheduleReevaluation()));
    // NOLINTEND(clazy-old-style-connect)

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
