#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QPlainTextEdit>
#include "dashboard.h"
#include "dialog.h"
#include "ui_dialog.h"

//PURPOSE: default constructor
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

//PURPOSE: default deconstructor.
Dialog::~Dialog()
{
    delete ui;
}

//PURPOSE: used to pass in user from dashboard.
void Dialog::setUser(QString u)
{
    myuser = u;
}

//Purpose: inserts event info to database when OK button is clicked.
void Dialog::on_buttonBox_accepted()
{
    dashboard dash(myuser);
    QString mydate = ui->dateEdit->date().toString();
    QString mystart = ui->start->text();
    QString myend = ui->end->text();
    QString mydesc = ui->description->toPlainText();
    QSqlQuery query;
    QString result;

    if(modeSet)
    {
        // Add new group event into EVENTS table
        query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, groupID) VALUES ('"+mydate+"','"+mystart+"', '"+myend+"','"+mydesc+"', '" +groupID+ "')");
    }
    else
    {
        // Add new event into EVENTS table
        query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, groupID) VALUES ('"+mydate+"','"+mystart+"', '"+myend+"','"+mydesc+"', 0)");
    }

    // get the current dates week and year. then set that for a new event.
    int week = ui->dateEdit->date().weekNumber();   // get the dates week. ex: 43
    int year = ui->dateEdit->date().year();         // get the dates year. ex: 2017
    QString yearweek = QString::number(year) + QString::number(week); // ex: 201743

    // Add new event into EVENTS table
    query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, yearweek) VALUES ('"+mydate+"','"+mystart+"', '"+myend+"','"+mydesc+"', '"+yearweek+"'");

    // Get newly created eventID and add new entry into USER_EVENTS
    result = getNewEventID();
    query.exec("INSERT INTO innodb.USER_EVENTS(username, eventID) VALUES ('" +myuser+ "','" +result+ "')");

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
    accepted = false;
    Dialog::close();
}

//PURPOSE: sets date edit text to chosen date
void Dialog::setDate(const QDate date)
{
    ui->dateEdit->setDate(date);
}

//PURPOSE: returns date entered in date edit
const QDate Dialog::getDate()
{
    return ui->dateEdit->date();
}

//PURPOSE: returns newly created event ID
QString Dialog::getNewEventID()
{
    QSqlQuery *query = new QSqlQuery(myconn.db);
    QString result;
    query->prepare("SELECT MAX(ID) FROM innodb.EVENTS");
    query->exec();
    query->first();

    result = query->value(0).toString();
    return result;
}

void Dialog::setMode(bool newMode)
{
    modeSet=newMode;
}

void Dialog::setGroupID(QString newID)
{
    groupID = newID;
}
