#ifndef DEMOVIEWER_H
# define DEMOVIEWER_H
# include <QTextEdit>

class DemoViewer : public QTextEdit
{
    Q_OBJECT

signals:
    void viewerClosed(void);

private:
    void closeEvent(QCloseEvent *);
};

#endif
