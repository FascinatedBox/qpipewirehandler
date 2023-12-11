#ifndef DEMOCONTROLLER_H
# define DEMOCONTROLLER_H
# include "demoviewer.h"

# include "qpipewirehandler.h"

class DemoController : public QObject
{
    Q_OBJECT

public:
    DemoController(void);
    void start(void);

private slots:
    void onInitComplete(void);
    void onItemRemoved(uint);
    void onLinkAdded(PWLink *link);
    void onNodeAdded(PWNode *node);
    void onPortAdded(PWPort *port);
    void onViewerClosed(void);

private:
    QPipewireHandler *m_pipewire;
    DemoViewer *m_viewer;
};

#endif
