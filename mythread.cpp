#include "display.h"
#include "mythread.h"

/*=================================================================================================================================*/
//                                          class MyThread-Specific Methods
/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Constructor       |*/
/*'------------------------'*/

// PURPOSE: constructor for creating a thread
MyThread::MyThread(QObject * pObj) {
    qDebug("MyThread : constructor");
    m_pReciever = pObj;
    m_exit = false;
    connect(this, SIGNAL(mysignal()), m_pReciever, SLOT(slot_refreshThread()));
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|        Mutators        |*/
/*'------------------------'*/

// PURPOSE: exits the thread.
void MyThread::performExit() {
    m_exit = true;
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|    Virtual Methods     |*/
/*'------------------------'*/

// PURPOSE: overrides the run function.
void MyThread::run() {
    while (!m_exit) {
        sleep(10);       // in seconds. msleep() for msecs.
        emit mysignal();
    }
}
