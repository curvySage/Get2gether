#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "edit.h"

edit::edit()
{
    connection myconn;          //connect to database
    myconn.openConn();
    eventOwner = originalDate = submittedDate = newStartTime = newEndTime = newDescription = "";
    eventID = -1;
    dateModified = false;
}

edit::~edit()
{

}

edit::edit(QString newOwner, int id, QString oDate, QString sDate, QString start, QString end, QString desc)
{
    connection myconn;          //connect to database
    myconn.openConn();

    eventOwner = newOwner;
    eventID = id;
    originalDate = oDate;
    submittedDate = sDate;
    dateModified = (oDate.compare(sDate) != 0);
    newStartTime = start;
    newEndTime = end;
    newDescription = desc;
}

bool edit::update(const connection &dbConn)
{
    QSqlQuery *editQuery = new QSqlQuery(dbConn.db);

    editQuery->exec("UPDATE innodb.EVENTS "
                    "SET date='"+submittedDate+
                    "',start='"+newStartTime+
                    "', end='"+newEndTime+
                    "',description='"+newDescription+
                    "' WHERE ID='"+QString::number(eventID)+"'");

    if(editQuery->isActive())
    {
        qDebug().noquote() << "Event " +QString::number(eventID)+ " successfully modified.";
        return true;
    }
    else
    {
        qDebug() << editQuery->lastError().text();
        return false;
    }

    editQuery->clear();
}
