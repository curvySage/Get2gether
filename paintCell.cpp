#include "paintCell.h"
#include "connection.h"
//#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>
#include <QString>
#include <QColor>
#include "dashboard.h"
#include "ui_dashboard.h"
#include <QIcon>

paintCell::paintCell()
{
    connection myconn;          //connect to database
    myconn.openConn();

  // paintCell();
}

/* Purpose:         Paints specified date cell with specified color
 * Preconditions:   minimumDate < date < maximumDate
 *                  color is defined in Qt
 * Postconditions:  Indicated date cell's background color is changed to specified color
*/
void paintCell::paint(Ui::dashboard *ui,QDate date, QColor color)
{
    qDebug("paintCell : paint");
    QBrush brush;
    QTextCharFormat charFormat;
    brush.setColor(color);
    charFormat = ui->calendarWidget->dateTextFormat(date);
    charFormat.setBackground(brush);
    ui->calendarWidget->setDateTextFormat(date, charFormat);
}


void paintCell::paintEvents(Ui::dashboard *ui,QDate date,bool isGroupMode,bool resetStatus,QString myuser,QString groupID,connection &newconn)
{
    qDebug("paintCell : paintEvents");
    QSqlQuery *query = new QSqlQuery(newconn.db);    // used to query DB
    bool isGroupEvent, isThisGroupEvent;

    /* If in personal mode,
     *      Prepare query using only logged in user
     * Else in group mode,
     *      Select each group member's username in group
     *      Accumulate union select query based on current group
     *          member and selected calendar day
    */
    if(!isGroupMode)
    {
        query->prepare("SELECT date, groupID "
                       "FROM innodb.EVENTS, innodb.USER_EVENTS "
                       "WHERE eventID = ID AND username ='" +myuser+
                       "' ORDER BY groupID");       // returns events
    }
    else
    {
        QSqlQuery selectMemberQ;
        QString groupMember, qResult;

        selectMemberQ.prepare("SELECT username "
                              "FROM innodb.GROUP_MEMBERS "
                              "WHERE groupID = '" +groupID+
                              "' AND username != '" +myuser+ "'");

        qResult = "(SELECT date, groupID "
                  "FROM innodb.EVENTS, innodb.USER_EVENTS "
                  "WHERE username = '" +myuser+ "' AND eventID = ID) ";

        /* If member select query executes,
         *      Grab each member's username
         *      Accumulate union select query for current member
         *          grabbing event date to paint
        */
        if(selectMemberQ.exec() && selectMemberQ.first())
        {
            do
            {
                groupMember = selectMemberQ.value(0).toString();
                qResult += "UNION (SELECT date, groupID "
                           "FROM innodb.EVENTS, innodb.USER_EVENTS "
                           "WHERE username = '" +groupMember+
                           "' AND eventID = ID) ";
            }while(selectMemberQ.next());

            qResult += "ORDER BY groupID";      // Order by groupID to have all group events at the end
                                                //  of the query so that group events are painted cyan
                                                //  even if personal events happen to be on the same day
            query->prepare(qResult);            // Prepare paint query using accumulated query
        }
    }

    query->exec();

    /* Paint every cell with an associated event
     * until end of query
    */
    while(query->next()) {
        date = QDate::fromString(query->value(0).toString(), "ddd MMM d yyyy"); // parse query to identify date
        isGroupEvent = (query->value(1).toInt() != 0);
        isThisGroupEvent = isGroupEvent && (query->value(1).toString() == groupID);

        /* If dashboard is in group mode,
         *      If event to color is selected group's event,
         *          Paint cyan
         *      Else, it's group member's personal event,
         *          Paint green
         * Else, in personal mode
         *      If event is a group event,
         *          Paint cyan
         *      Else, personal event only
         *          Paint green
        */
        if(isGroupMode)
        {
            if(resetStatus == true)
            {
                paint(ui,date, Qt::white);
            }
            else
            {
                if(isThisGroupEvent) paint(ui,date, Qt::cyan);
                else paint(ui,date, Qt::green);
            }
        }
        else
        {
            if(isGroupEvent) paint(ui,date, Qt::cyan);
            else paint(ui,date, Qt::green);                                                 // paint parsed date cell green
        }

    }//while(query->next());                                                    // move to next query result
}

