#include "mythread.h"

MyThread::MyThread(QObject * pObj) {
    m_pReciever = pObj;
    m_exit = false;
    connect(this, SIGNAL(mysignal()), m_pReciever, SLOT(on_loadonline_clicked()));
}

void MyThread::run() {
    while (!m_exit) {
        msleep(1000);
        emit mysignal();
    }
}


void MyThread::performExit() {
    m_exit = true;
}

