#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include "connection.h"
#include "display.h"

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

/* Purpose:         Displays all of the user's associated groups into
 *                  groupsview
 * Postconditions:  All groups user is a part of is displayed in groupsview
*/
void display::updateGroupsView()
{
    qDebug("display : updateGroupsView()");
    displayResults(table, "SELECT ID AS \"ID\", name AS \"Name\" "
                                   "FROM innodb.GROUPS, innodb.GROUP_MEMBERS "
                                   "WHERE ID = groupID "
                                   "AND username = '" +id+ "'");          // populate associated groups
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

void display::updateMembersView()
{
    qDebug() << "display : updateMembersView() : group " + id;
    displayResults(table, "SELECT username AS \"Members\" "
                                        "FROM innodb.GROUP_MEMBERS "
                                        "WHERE innodb.GROUP_MEMBERS.groupID = '" +id+ "'");
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
        displayResults(table, "SELECT ID AS \"Event ID \", description AS \"Details\", date AS \"Date\", start AS \"Start\", end AS \"End\" "
                                   "FROM innodb.EVENTS "
                                   "WHERE groupID = '" +id+ "'");
}

/* Purpose:         returns the events from the current week.
 * Precondtions:    An event has its yearweek column set when inserted in the database. This is done in dialog.cpp
 * Postconditions:  reminders is uploaded with all events upcoming in current week
*/
void display::updateRemindersView()
{
    qDebug("display : updateRemindersView()");
    // get the current year and week from currentDate.
    int week = QDate::currentDate().weekNumber();  //ex: 43
    int year = QDate::currentDate().year();        //ex: 2017
    QString yearweek = QString::number(year) + QString::number(week); // ex: 201743

    // return events from the current week.
    displayResults(table,
                   "SELECT name AS \"Group\", description AS \"Details\", date AS \"Date\", start AS \"Start\", end  AS \"End\" "
                   "FROM innodb.EVENTS, innodb.GROUPS "
                   "WHERE yearweek = '"+yearweek+"' AND innodb.EVENTS.ID "
                   "IN (SELECT eventID FROM innodb.USER_EVENTS WHERE (innodb.GROUPS.ID = groupID AND username = '"+id+"'))");
}

/* Purpose:         Displays all bulletin messages in
 *                  bulletin tab
 * Postcondtions:   All bulletin messages stored in innodb.BULLETINS
 *                  are selected and displayed in bulletinview
*/
void display::updateBulletinsView()
{
    qDebug() << "display : updateBulletinsView()";
    displayResults(table, "SELECT userID AS \"User\", message AS \"Message\" "
                                     "FROM innodb.BULLETINS where groupID = '"+id+"'");
    table->resizeRowsToContents();

    // Messages are kept for a week in the database.
    // Messages are deleted every Monday at 1:00AM
    // All messages are automatically deleted using an event scheduler in the database.
}

/* Purpose:         Displays results of specified query in specified table view
 * Preconditions:   command is a valid SQL query
 *                  command is a relevant query for database: innodb
 * Postconditions:  Query results are inserted into table
*/
void display::displayResults(QTableView * table, QString command)
{
    //Error: runs this an unnecessary amount of times
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
