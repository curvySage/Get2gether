#include "mainwindow.h"
#include "mythread.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    //MyThread mthread;
    //mthread.start();

    w.show();

    return a.exec();
}
