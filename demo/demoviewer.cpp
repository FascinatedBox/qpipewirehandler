#include "demoviewer.h"

void DemoViewer::closeEvent(QCloseEvent *event)
{
    emit viewerClosed();
}
