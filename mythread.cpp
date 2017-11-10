#include "mythread.h"

MyThread::MyThread(QObject * pObj) {
    m_pReciever = pObj;
    m_exit = false;

    // We can use the same thread for updating multiple views by creating a slot
    // in dashboard that calls the update view functions.

    // This one just updates the bulletin view.
    connect(this, SIGNAL(mysignal()), m_pReciever, SLOT(updateBulletinsView()));
}

// PURPOSE: overrides the run function.
void MyThread::run() {
    while (!m_exit) {
        sleep(5);       // in seconds. msleep() for msecs.
        emit mysignal();
    }
}

// PURPOSE: exits the thread.
void MyThread::performExit() {
    m_exit = true;
}

