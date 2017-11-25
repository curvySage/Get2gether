#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>
#include <QTableView>
#include <connection.h>
#include <dialog.h>
#include <QThread>
#include <QtCore>
#include <mythread.h>
#include "display.h"

#include "display.h"

//error codes
typedef enum {
    EDIT_ERROR_UNAUTH_USER = -1,
    DELETE_ERROR_UNAUTH_USER = -2,
    ADD_ERROR_NO_GROUP_SELECTED = -3,
    EDIT_ERROR_INVALID_MODE = -4,
    DELETE_ERROR_INVALID_MODE = -5
}ErrorCode;

namespace Ui {
class dashboard;
}

/* Class purpose:   Creates user's main dashboard
*/
class dashboard : public QDialog
{
    Q_OBJECT

public:
<<<<<<< HEAD
    MyThread *m_pRefreshThread;
    display *displayObject;
    connection myconn;
=======
    explicit dashboard(QString u, QWidget *parent = 0);
    ~dashboard();
>>>>>>> d9c9ef1feba9fecc7c8c911c22d6127761b59123
    QString myuser;
    QString groupID;
    QString groupName;
    MyThread *m_pRefreshThread;
    display *displayObject;
    connection myconn;
    QString getGroupID();
    bool getMode();
    bool isGroupMode;               // to indicate group/personal mode
    bool resetStatus;
    void setMode(bool isGroup);
    void setGroupID(const QModelIndex &groupID);
    void setGroupName();
    void updateCalendarName(QString name);

private:
    Ui::dashboard *ui;

protected:
    void closeEvent(QCloseEvent * e);

signals:
    void cellChanged(QTableView *tbl, QString user, QString date);

private slots:
    void on_loadonline_clicked();
    void on_addevents_clicked();
    void on_editEvents_clicked();
    void on_deleteEvents_clicked();
<<<<<<< HEAD
=======
    void on_createGroup_clicked();
    void on_sendButton_clicked();
    void on_homeButton_clicked();
    void on_groupsview_clicked(const QModelIndex &index);
    void on_eventsview_clicked(const QModelIndex &index);
    void on_calendarWidget_selectionChanged();
    void on_messageBox_textChanged();
    void on_networktabs_currentChanged(int index);
    //void updateEventsView();
    void updateGroupsView();
    void updateBulletinsView();
    void updateRemindersView();
<<<<<<< Updated upstream
    void updateMemberEvents();
    void updateGroupEvents();
>>>>>>> d9c9ef1feba9fecc7c8c911c22d6127761b59123
=======
    //void updateMemberEvents();
    //void updateGroupEvents();
>>>>>>> Stashed changes
    void paint(QDate date, QColor color);
    //void paintEvents();
    void clearEditInfo();
    //void displayResults(QTableView * table, QString);
    void resetGroupAttributes();
<<<<<<< HEAD
    void updateBulletinsView();
    //void on_onlineview_clicked(const QModelIndex &index);
    void updateGroupsView();
    void updateRemindersView();
    void displayResults(QTableView *table, QString command);
    void updateEventsView();
    void updateMemberEvents();
    void updateGroupEvents();

    // new methods created for simplification
    int countEvents(QDate target, QString username);
    bool isGroupEvent(int eventID, int &groupID);
    void printError(ErrorCode error_code);
=======
    void slot_refreshThread();
    void checkNoDateEvent();
    //void on_onlineview_clicked(const QModelIndex &index);
<<<<<<< Updated upstream
    void on_DescriptxtEdit_textChanged();
>>>>>>> d9c9ef1feba9fecc7c8c911c22d6127761b59123
=======
    void on_calendarWidget_clicked(const QDate &date);
>>>>>>> Stashed changes
};

#endif // DASHBOARD_H
