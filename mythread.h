#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QtCore>
#include <QThread>
#include <QTableView>

// PURPOSE: This class is used to run a thread.
class MyThread : public QThread
{
    Q_OBJECT

public:
    void run();
    void performExit();
    MyThread(QObject *pObj);

signals:
    void mysignal();

private:
    QObject *m_pReciever;
    bool m_exit;

};

#endif // MYTHREAD_H
