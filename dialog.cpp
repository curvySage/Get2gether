#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QPlainTextEdit>
#include "dashboard.h"
#include "dialog.h"
#include "ui_dialog.h"

/*=================================================================================================================================*/
//                                          class Dialog-Specific Methods
/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Constructor       |*/
/*'------------------------'*/

//PURPOSE: default constructor
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Destructor        |*/
/*'------------------------'*/

//PURPOSE: default deconstructor.
Dialog::~Dialog()
{
    delete ui;
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|        Mutators        |*/
/*'------------------------'*/

//PURPOSE: used to pass in user from dashboard.
void Dialog::setUser(QString u)
{
    myuser = u;
}

//PURPOSE: sets date edit text to chosen date
void Dialog::setDate(const QDate date)
{
    ui->dateEdit->setDate(date);
    this->date = date.toString();
}

//PURPOSE: sets value of groupModeSet
void Dialog::setMode(bool newMode)
{
    groupModeSet=newMode;
}

//PURPOSE: sets value of groupID
void Dialog::setGroupID(QString newID)
{
    groupID = newID;
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|       Accessors        |*/
/*'------------------------'*/

//PURPOSE: returns date entered in date edit
const QDate Dialog::getDate()
{
    return QDate::fromString(date, "ddd MMM d yyyy");
}

//PURPOSE: returns newly created event ID
QString Dialog::getNewEventID()
{
    QSqlQuery *query = new QSqlQuery(myconn.db);
    QString result;
    query->prepare("SELECT MAX(ID) "
                   "FROM innodb.EVENTS");
    query->exec();
    query->first();

    result = query->value(0).toString();
    return result;
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|    Virtual Methods     |*/
/*'------------------------'*/

//PURPOSE: Determines what's done when user selects
//         red "x-button" in dialog's corner
void Dialog::closeEvent(QCloseEvent *event)
{
    if(this->result() == QDialog::Accepted)
        accepted = true;
    else
        accepted = false;

    event->accept();
}

/*=================================================================================================================================*/
//                                                          SLOTS
/*=================================================================================================================================*/

/* Purpose:         Adds new event into database
 * Preconditions:   Values entered in text fields are valid
 * Postconditions:  New group or personal event are added into the
 *                  DB based on values specified in text fields
 * Assumptions:     innodb.EVENTS.groupID = 0 if adding a personal event
 *                  otherwise, is set to relevant group's ID
*/
void Dialog::on_buttonBox_accepted()
{
    this->accept();

    date = ui->dateEdit->date().toString();
    QString mystart = ui->start->text();
    QString myend = ui->end->text();
    QString mydesc = ui->description->toPlainText();
    QSqlQuery query;
    QString newEventID;

    // get the current dates week and year. then set that for a new event.
    int week = ui->dateEdit->date().weekNumber();   // get the dates week. ex: 43
    int year = ui->dateEdit->date().year();         // get the dates year. ex: 2017
    QString yearweek = QString::number(year) + QString::number(week); // ex: 201743

    /* If in group mode,
     *      Execute add group event query
     * Else,
     *      Execute regular personal event query
    */
    if(groupModeSet)
    {
        QSqlQuery selectMemberQ, addUserEventQ;
        QString groupMember;

        // Add new group event into EVENTS table
        query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, groupID, yearweek) "
                   "VALUES ('"+date+"','"+mystart+"', '"+myend+"','"+mydesc+"', '" +groupID+ "', '" +yearweek+ "')");
        // Get newly created eventID and add new entry into USER_EVENTS
        newEventID = getNewEventID();
        selectMemberQ.prepare("SELECT username "
                              "FROM innodb.GROUP_MEMBERS "
                              "WHERE groupID = '" +groupID+ "'");

        /* If select member query is successful
         *      Grab each member's uername
         *      Insert into database newly created event into user_event
         *          using member's username and new eventID
        */
        if(selectMemberQ.exec() && selectMemberQ.first())
        {
            do
            {
                groupMember = selectMemberQ.value(0).toString();
                addUserEventQ.exec("INSERT INTO innodb.USER_EVENTS(username, eventID) "
                                   "VALUES('" +groupMember+ "', '" +newEventID+ "')");
            }while(selectMemberQ.next());
        }
    }
    else
    {
        // Add new event into EVENTS table
        query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, groupID, yearweek) "
                   "VALUES ('"+date+"','"+mystart+"', '"+myend+"','"+mydesc+"', 0, '" +yearweek+ "')");
        newEventID = getNewEventID();

        query.exec("INSERT INTO innodb.USER_EVENTS(username, eventID) "
                   "VALUES('" +myuser+ "', '" +newEventID+ "')");
    }

    if (query.isActive()) {
        qDebug("Inserted event into database.");
    }
    else {
        qDebug() << query.lastError().text();
    }

    accepted = true;
    Dialog::close();
}

//PURPOSE: closes add events window when cancel is clicked.
void Dialog::on_buttonBox_rejected()
{
    this->reject();

    accepted = false;
    Dialog::close();
}
