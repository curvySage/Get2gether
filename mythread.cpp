#include "mythread.h"

MyThread::MyThread(QObject * pObj) {
    //m_pReciever = pObj;
    m_exit = false;
    //connect(this, signals(mySignal()), m_pReciever, SLOT(//button));
}

void MyThread::run() {
    while (!m_exit) {
        emit mySignal();
        sleep(3000);
    }
}

void MyThread::performExit() {
    m_exit = true;
}

