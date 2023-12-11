#include "democontroller.h"

DemoController::DemoController()
{
    m_viewer = new DemoViewer;
    m_pipewire = new QPipewireHandler;

    connect(m_pipewire, &QPipewireHandler::evInitComplete,
            this, &DemoController::onInitComplete);
    connect(m_pipewire, &QPipewireHandler::evItemRemoved,
            this, &DemoController::onItemRemoved);
    connect(m_pipewire, &QPipewireHandler::evLinkAdded,
            this, &DemoController::onLinkAdded);
    connect(m_pipewire, &QPipewireHandler::evNodeAdded,
            this, &DemoController::onNodeAdded);
    connect(m_pipewire, &QPipewireHandler::evPortAdded,
            this, &DemoController::onPortAdded);
    connect(m_viewer, &DemoViewer::viewerClosed,
            this, &DemoController::onViewerClosed);
}

void DemoController::onLinkAdded(PWLink *l)
{
    QString s = QString("Link added: id %1, serial %2:"
            " Source node %3 (port %4) => dst node %5 (port %6)")
            .arg(l->id)
            .arg(l->serial)
            .arg(l->nodeOutId)
            .arg(l->portOutId)
            .arg(l->inNodeId)
            .arg(l->inPortId);

    m_viewer->append(s);
    delete l;
}

void DemoController::onNodeAdded(PWNode *n)
{
    QString s = QString("Node added: id %1, serial %2")
            .arg(n->id)
            .arg(n->serial);

    m_viewer->append(s);
    delete n;
}

void DemoController::onPortAdded(PWPort *p)
{
    QString s = QString("Port added: id %1, serial %2")
            .arg(p->id)
            .arg(p->serial);

    m_viewer->append(s);
    delete p;
}

void DemoController::onInitComplete(void)
{
    m_viewer->append("Initial burst complete.");
}

void DemoController::onItemRemoved(uint id)
{
    QString s = QString("Item #%1 removed.")
            .arg(id);

    m_viewer->append(s);
}

void DemoController::onViewerClosed(void)
{
    m_pipewire->stop();
}

void DemoController::start(void)
{
    m_viewer->show();
    m_pipewire->start();
}
