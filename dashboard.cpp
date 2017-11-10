#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "dialog.h"
#include "grouppopup.h"
#include "ui_dashboard.h"
#include "dashboard.h"
#include "invitations.h"
#include "mythread.h"


/* Purpose:         Full constructor
 * Postconditions:  Creates main dashboard ui for current user
*/
dashboard::dashboard(QString u, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dashboard)
{
    ui->setupUi(this);
    ui->calendarWidget->setGridVisible(true);           // creates calendar borders
    myuser = u;                                         // set user
    ui->userlabel->setText(u);                          // set user dashboard label

    myconn.openConn();  // connect to database

    /*-- thread -- */
    MyThread *m_pRefreshThread = new MyThread(this);
    m_pRefreshThread->start();

    /* Put this code in the dashboards deconstructor he said.
     *
     *
    m_pRefreshThread->performExit();
    while(m_pRefreshThread->isRunning())
    {
        msleep(10);
    }
    delete m_pRefreshThread;
    */

    /*-- On Load --*/
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");           // populate online "friends"
    paintEvents();          // paint calendar cells with events
    updateEventsView();     // load calendar events for curr. selected date
    updateGroupsView();
    updateBulletinsView();
    updateRemindersView();

    /*-- Signals & Slots --*/
    QObject::connect(ui->calendarWidget, SIGNAL(activated(QDate)), this, SLOT(on_addevents_clicked()));     // prompt add event for selected date when dbl-clicked
    QObject::connect(ui->calendarWidget, SIGNAL(clicked(QDate)), this, SLOT(updateEventsView()));           // update eventsview for selected date
}

/* Purpose:         Default destructor
 * Postconditions:  Destroys dashboard ui
*/
dashboard::~dashboard()
{
    //m_pRefreshThread->performExit();
    //delete m_pRefreshThread;

    delete ui;
}

/* Purpose:         Displays results of specified query in specified table view
 * Preconditions:   command is a valid SQL query
 *                  command is a relevant query for database: innodb
 * Postconditions:  Query results are inserted into table
*/
void dashboard::displayResults(QTableView * table, QString command)
{
    //QSqlQueryModel used to execute sql and traverse the results on a view table.
    QSqlQueryModel * modal = new QSqlQueryModel();
    QSqlQuery * query = new QSqlQuery(myconn.db);

    query->prepare(command);
    query->exec();
    modal->setQuery(*query);
    table->setModel(modal);
}

/* Purpose:         Paints specified date cell with specified color
 * Preconditions:   minimumDate < date < maximumDate
 *                  color is defined in Qt
 * Postconditions:  Indicated date cell's background color is changed to specified color
*/
void dashboard::paint(QDate date, QColor color)
{
    QBrush brush;
    QTextCharFormat charFormat;
    brush.setColor(color);
    charFormat = ui->calendarWidget->dateTextFormat(date);
    charFormat.setBackground(brush);
    ui->calendarWidget->setDateTextFormat(date, charFormat);
}

/* Purpose:         Updates eventsview to display current date's events owned
 *                  by current user
 * Postconditions:  Query results are displayed in eventsview
*/
// PURPOSE: Updates eventsview based on the selected event day
void dashboard::updateEventsView()
{
    displayResults(ui->eventsview, "SELECT ID AS \"Event ID\", date AS \"Date\", description AS \"Details\", start AS \"Start\", end AS \"End\" FROM innodb.USER_EVENTS, innodb.EVENTS WHERE eventID = ID AND username ='" +myuser+ "' AND date = '" +ui->calendarWidget->selectedDate().toString()+ "'");
}

void dashboard::updateGroupsView()
{
    displayResults(ui->groupsview, "SELECT ID AS \"Group ID\", name AS \"Group Name\" FROM innodb.GROUPS, innodb.GROUP_MEMBERS WHERE ID = groupID AND username = '" +myuser+ "'");          // populate associated groups
}

void dashboard::updateBulletinsView()
{
    displayResults(ui->bulletinView, "SELECT userID, message FROM innodb.BULLETINS");
}

void dashboard::updateRemindersView()
{
    // get the current year and week from currentDate.
    int week = QDate::currentDate().weekNumber();
    int year = QDate::currentDate().year();
    QString yearweek = QString::number(year) + QString::number(week);

    // return events from the same current week & year.
    displayResults(ui->reminders, "SELECT * FROM innodb.EVENTS WHERE yearweek = '"+yearweek+"'");
}

/* Purpose:         Paints all cells with events in database green
 * Postconditions:  Calendar cells with associated events are green
*/
void dashboard::paintEvents()
{
    QDate date;
    QSqlQuery *query = new QSqlQuery(myconn.db);    // used to query DB

    // Execute query
    query->prepare("SELECT date FROM innodb.EVENTS, innodb.USER_EVENTS WHERE eventID = ID AND username ='" +myuser+ "'");       // returns events
    query->exec();
    query->first();     // accesses first query result

    /* Paint every cell with an associated event
     * until end of query
    */
    do{
        date = QDate::fromString(query->value(0).toString(), "ddd MMM d yyyy"); // parse query to identify date
        paint(date, Qt::green);                                                 // paint parsed date cell green
      }while(query->next());                                                    // move to next query result
}

/* Purpose:         Replaces event info details with default values
 * Postconditions:  Text fields are blank, date value resetted to 01/01/00,
 *                  time fields reset to 12AM
*/
void dashboard::clearEditInfo()
{
    ui->NameDisplay->clear();
    ui->DescriptxtEdit->clear();
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->startTime->setTime(QTime(0, 0, 0, 0));
    ui->endTime->setTime(QTime(0, 0, 0, 0));
    ui->ID_Label->clear();
}

/* Purpose:         Refreshes online view showing users online
 * Postconditions:  Results of db query of online users are displayed in onlineview table
*/
void dashboard::on_loadonline_clicked()
{
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");
}

/* Purpose:         Displays new even form used to create events
 * Postocondtions:  New event is created owned by this user and is immediately
 *                  stored in database
*/
void dashboard::on_addevents_clicked()
{
    Dialog h;
    h.setDate(ui->calendarWidget->selectedDate());    // sets date edit text to currently selected date
    h.setUser(myuser);
    h.setWindowTitle("Create New Event");
    h.setModal(true);
    h.exec();

    /* If user creates event,
     *      Paint relevant cell green
     *      Update eventsview to show new event
    */
    if(h.accepted)
    {
        paint(h.getDate(), Qt::green);
        updateEventsView();
    }
}

/* Purpose:         Initializes event information form when user clicks
 *                  an event in eventsview
 * Postconditions:  Event info filled is determined by event values
 *                  stored in database
*/
void dashboard::on_eventsview_clicked(const QModelIndex &index)
{
    QString val=ui->eventsview->model()->data(index).toString();            // Grab value user clicked in eventsview

    QSqlQuery selectQry;
    selectQry.prepare("SELECT * FROM innodb.EVENTS WHERE ID='"+val+"'");

    /* If query executes,
     *      Initialize info text fields with relevant data
    */
    if(selectQry.exec()){
        while(selectQry.next()){
            ui->NameDisplay->setText(myuser);
            ui->DescriptxtEdit->setText(selectQry.value(2).toString());
            ui->dateEdit->setDate(QDate::fromString(selectQry.value(1).toString(), "ddd MMM d yyyy"));
            ui->startTime->setTime(QTime::fromString(selectQry.value(3).toString(), "hh:mm AP"));
            ui->endTime->setTime(QTime::fromString(selectQry.value(4).toString(), "hh:mm AP"));
            ui->ID_Label->setText(selectQry.value(0).toString());
        }
    }
    //To
    else {
        QMessageBox::critical(this,tr("error::"),selectQry.lastError().text());
    }
}

/* Purpose:         Updates event information with user defined values in
 *                  event info text fields
 * Postconditions:  Event is modified and replaced in database
*/
void dashboard::on_editEvents_clicked()
{
    QString matchuser = ui->NameDisplay->text();                // Get event owner
    QDate originalDate = ui->calendarWidget->selectedDate();    // To compare if date changed

    /* If current user matches event owner,
     *      Allow event modification
     * Else
     *      Print error: unauthorized to delete event
    */
    if(myuser == matchuser)
    {
        QMessageBox::StandardButton choice;
        choice = QMessageBox::warning(this,"Save Changes","Are you sure you want to change?", QMessageBox::Save | QMessageBox::Cancel);

        /* If user indicates "Save",
         *      Replace existing data in database with indicated data in event
         *      details text fields
        */
        if(choice == QMessageBox::Save)
        {
            QString Editdate = ui->dateEdit->date().toString();
            QString Editstart = ui->startTime->text();
            QString Editend = ui->endTime->text();
            QString Editdesc = ui->DescriptxtEdit->toPlainText();
            QString ID_Param = ui->ID_Label->text();
            QSqlQuery query_update, query_count;
            int eventCount;

            // Update event details
            query_update.exec("UPDATE innodb.EVENTS SET date='"+Editdate+"',start='"+Editstart+"', end='"+Editend+"',description='"+Editdesc+"' WHERE ID='"+ID_Param+"'");

            // To count today's events
            query_count.exec("SELECT COUNT(*) FROM innodb.EVENTS, innodb.USER_EVENTS WHERE date = '" +originalDate.toString()+ "' AND ID = eventiD AND username ='" +myuser+ "'");
            query_count.first();
            eventCount = query_count.value(0).toInt();  // saves return value

            updateEventsView();                         // Update eventsview with newly modified event

            QDate modifyDate = ui->dateEdit->date();

            /* IF date was changed
             *      Color cell appropriately
            */
            if (originalDate != modifyDate)
            {
                /* if no more events today
                 *      paint originalDate white
                 * paint modifyDate green
                 */
                if(eventCount == 0)
                    paint(originalDate, Qt::white);

                paint(modifyDate, Qt::green);
            }
        }
    }
    else
    {
       QMessageBox MsgBox;
       MsgBox.setWindowTitle("Woah There!");
       MsgBox.setText("Sorry, but you can't change other's schedules!");
       MsgBox.exec();
    }

}

/* Purpose:         Allows event deletion
 * Postcondition:   Selected event is deleted from database and removed from calendar
*/
void dashboard::on_deleteEvents_clicked()
{
    QString matchuser = ui->NameDisplay->text();        // to compare event owner

    /* If username is event owner,
     *      Allow event deletion
     * Else,
     *      Print error: unauthorized deletion
    */
    if(myuser == matchuser) {
        QMessageBox::StandardButton choice;
        choice = QMessageBox::warning(this,"Delete Event?", "Are you sure you want to delete your event?",QMessageBox::Yes | QMessageBox::No);

        /* If user selects Yes,
         *      Query database removing selected event
        */
        if (choice == QMessageBox::Yes) {
            QString ID_Param = ui->ID_Label->text();
            QSqlQuery query_delete, query_count;
            QDate currDate = ui->dateEdit->date();
            int eventCount;

            // update EVENTS table
            query_delete.exec("DELETE FROM innodb.EVENTS WHERE ID='"+ID_Param+"'");
            // update USER_EVENTS table
            query_delete.exec("DELETE FROM innodb.USER_EVENTS WHERE eventID ='" +ID_Param+ "' AND username ='" +myuser+ "'");

            // count today's events
            query_count.exec("SELECT COUNT(*) FROM innodb.EVENTS, innodb.USER_EVENTS WHERE date = '" +currDate.toString()+ "' AND ID = eventiD AND username ='" +myuser+ "'");
            query_count.first();
            eventCount = query_count.value(0).toInt();

            /* If today has no more events
             *    Paint today's cell white
            */
            if(eventCount == 0)
                paint(currDate, Qt::white);     // paint back to default color

            clearEditInfo();                // clear edit info
            updateEventsView();             // update eventsview with removed event
        }
    }
    else {
        QMessageBox MsgBox1;
        MsgBox1.setWindowTitle("Wait a Second!");
        MsgBox1.setText("Sorry, but you can't delete other's schedules!");
        MsgBox1.exec();
    }
}

/* Purpose:         Updates eventsview and event info fields when date selected is
 *                  changed
 * Postconditions:  Updates eventsview to selected dates relevant events
 *                  If selected date has no events, event info is reset to default values
 *                  Else, filled with event info
*/
void dashboard::on_calendarWidget_selectionChanged()
{
    clearEditInfo();
    updateEventsView();
}

/* Purpose:         Displays group members of group clicked in groups view
 * Postconditions:  Query is determined by group clicked; results of query
 *                  populate membersview
*/
void dashboard::on_groupsview_clicked(const QModelIndex &index)
{
    QString val=ui->groupsview->model()->data(index).toString();        // Grab group ID
    displayResults(ui->membersview, "SELECT username AS \"Members\" FROM innodb.GROUP_MEMBERS, innodb.GROUPS WHERE innodb.GROUP_MEMBERS.groupID = innodb.GROUPS.ID AND innodb.GROUPS.ID ='" +val+ "'");
}

/* Purpose:         Creates new Create Group form when Create Group button
 *                  is clicked
 * Postconditions:  New Create Group dialog pops up when Create Group clicked
*/
void dashboard::on_createGroup_clicked()
{
    GroupPopUp createGroup;
    createGroup.setUser(myuser);
    createGroup.setModal(true);
    createGroup.setWindowTitle("Create Group");
    createGroup.loadAddFriendsList();
    createGroup.exec();

    if(createGroup.accepted)
    {
        updateGroupsView();
    }
}

void dashboard::on_invites_button_clicked()
{
    invitations invites(myuser);
    invites.setModal(true);
    invites.setWindowTitle("Invitations");
    invites.exec();
}

void dashboard::on_sendButton_clicked()
{
    QString message = ui->messageBox->toPlainText();
    QString current = QDate::currentDate().toString();
    QSqlQuery query;

    query.exec("INSERT INTO innodb.BULLETINS(date, userID, message) VALUES ('"+current+"','"+myuser+"','"+message+"')");
    if (query.isActive()) {
        qDebug("Inserted message into database.");
    }
    else {
        qDebug() << query.lastError().text();
    }
}
