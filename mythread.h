#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QtCore>
#include <QThread>

// PURPOSE: This class is used to pool the database.
class MyThread : public QThread
{
    Q_OBJECT

public:
    void run();
    void performExit();
    MyThread(QObject *pObj);

signals:
    void mySignal();

private:
    //Q_OBJECT *m_pReciever;
    bool m_exit;

};

#endif // MYTHREAD_H
