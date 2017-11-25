<<<<<<< Updated upstream
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

#include "connection.h"
=======
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>

>>>>>>> Stashed changes
#include "display.h"
#include <mythread.h>

display::display()
{
}

<<<<<<< Updated upstream

=======
display::display(connection &myconn)
{
    conn = myconn;
}

/* Purpose:         Updates eventsview to display current date's events owned
 *                  by current user
 * Postconditions:  Query results are displayed in eventsview
*/
void display::updateEventsView()
{
    qDebug() << "display : updateEventsView() : " + id + " : " + selectedDateStr;
    displayResults(table, "SELECT ID AS \"Event ID\", description AS \"Details\", date AS \"Date\", start AS \"Start\", end AS \"End\" "
                                       "FROM innodb.USER_EVENTS, innodb.EVENTS "
                                       "WHERE eventID = ID AND username ='" +id+
                                       "' AND date = '" +selectedDateStr+ "'");
}

/* Purpose:         Displays all group member's events of a selected
 *                  calendar day in eventsview
 * Preconditions:   Should be called only when dashboard is in group mode
 *                  groupID must be set (i.e. selected in groupsview)
 * Postconditions:  Updates eventsview with all group member's events
*/
void display::updateMemberEvents()
{
    qDebug() << "display : updateMemberEvents() : " + id + " : " + selectedDateStr;
    displayResults(table, "SELECT eventID AS \"Event ID\", username AS \"Owner\", description AS \"Details\", "
                                        "date AS \"Date\", start AS \"Start\", end AS \"End\" "
                                   "FROM innodb.EVENTS, innodb.USER_EVENTS "
                                   "WHERE ID = eventID AND "
                                    "date = '" +selectedDateStr+
                                    "' AND innodb.USER_EVENTS.username IN (SELECT innodb.GROUP_MEMBERS.username "
                                                                          "FROM innodb.GROUP_MEMBERS "
                                                                          "WHERE innodb.GROUP_MEMBERS.groupID = '" +id+ "') "
                                   "ORDER BY groupID");        // Update eventsview with all group member's events
}

/* Purpose:         Updates eventsview to display all group events in
 *                  eventsviews (when you click the groupID)
 * Preconditions:   groupID must be set in order to display all group events in
 *                  eventsview
 * Postconditions:  When group ID is selected in groupsview, all group's events are
 *                  loaded into eventsview
*/
void display::updateGroupEvents()
{
    qDebug() << "display : updateGroupEvents() : group " + id;
    displayResults(table, "SELECT ID AS \"Event ID \", description AS \"Details\", date AS \"Date\", start AS \"Start\", end AS \"End\" "
                                   "FROM innodb.EVENTS "
                                   "WHERE groupID = '" +id+ "'");
}

void display::updateMembersView()
{
    qDebug() << "display : updateMembersView() : group " + id;
    displayResults(table, "SELECT username AS \"Members\" "
                                        "FROM innodb.GROUP_MEMBERS "
                                        "WHERE innodb.GROUP_MEMBERS.groupID = '" +id+ "'");
}

/* Purpose:         Displays results of specified query in specified table view
 * Preconditions:   command is a valid SQL query
 *                  command is a relevant query for database: innodb
 * Postconditions:  Query results are inserted into table
*/
void display::displayResults(QTableView * table, QString command)
{
    //Error: runs this an unnecessary amount of times
    qDebug() << "display : displayResults : cmd : " + command;
    //QSqlQueryModel used to execute sql and traverse the results on a view table.

    QSqlQueryModel * modal = new QSqlQueryModel();

    QSqlQuery * query = new QSqlQuery(conn.db);
    query->exec(command);

    if(query->isActive())
    {
        modal->setQuery(*query);
        table->setModel(modal);
    }
    else
    {
        qDebug() << query->lastError().text();
    }

    //resetValues();
}

void display::initValues(QTableView *tbl, QString newID, QString date)
{
    table = tbl;
    id = newID;
    selectedDateStr = date;
}

void display::resetValues()
{
    table = NULL;
    id = selectedDateStr = "";
}
>>>>>>> Stashed changes
