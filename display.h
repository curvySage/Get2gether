#ifndef DISPLAY_H
#define DISPLAY_H

#include <QString>
#include <QTableView>
#include <QObject>

#include "connection.h"

class display : public QObject
{
    Q_OBJECT
public:
    connection conn;
    QTableView *table;
    QString id;
    QString selectedDateStr;

    display();
    display(connection &myconn);

private:
    void resetValues();

public slots:
    void displayResults(QTableView *table, QString command);
    void updateEventsView();
    void updateMemberEvents();
    void updateGroupEvents();
    void updateMembersView();
    void initValues(QTableView *tbl, QString newID, QString date);
};

#endif // DISPLAY_H
