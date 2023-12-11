#include <QApplication>
#include <QWidget>

#include "democontroller.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    DemoController *dc = new DemoController;

    dc->start();
    return app.exec();
}
