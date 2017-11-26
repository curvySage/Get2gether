#ifndef DISPLAY_H
#define DISPLAY_H

#include <QString>
#include <QTableView>
#include <QObject>

#include "connection.h"

/* Class purpose:   Contains slots relevant to updating
 *                  a QTableView object in a dashboard
 *                  object.
*/
class display : public QObject
{
    Q_OBJECT
public:
    connection conn;

    display();
    display(connection &myconn);


private:
    void resetValues();

public slots:
    void displayResults(QTableView *table, QString command);
    void updateEventsView(QTableView *eventTable, QString username, QString date);
    void updateGroupsView(QTableView *groupTbl, QString username);
    void updateMemberEvents(QTableView *eventTable, QString groupID, QString date);
    void updateGroupEvents(QTableView *eventTable, QString groupID);
    void updateMembersView(QTableView *memberTbl, QString groupID);
    void updateRemindersView(QTableView *reminderTbl, QString username);
    void updateBulletinsView(QTableView *bulletinTbl, QString groupID);
};

#endif // DISPLAY_H
