#include "dashboard.h"
#include "ui_dashboard.h"
#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>            // used to paint cell
#include "dialog.h"


// PURPOSE: default constructor
dashboard::dashboard(QString u, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dashboard)
{
    ui->setupUi(this);

    myuser = u;
    ui->userlabel->setText(u);

    // when dashboard is open, events and onlinelist are automatically loaded.
    myconn.openConn();
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");

    /*-- ON LOAD --*/
    paintEvents();
    updateEventsView();

    /*-- IF USER DOUBLE CLICKS/PRESSES ENTER, POP UP ADD EVENT DIALOG --*/
    QObject::connect(ui->calendarWidget, SIGNAL(activated(QDate)), this, SLOT(on_addevents_clicked()));

    /*-- IF USER CLICKS/SELECTS DATE, SHOW ASSOCIATED EVENTS IN EVENTSVIEW --*/
    // displays all the associated events when day is clicked
    QObject::connect(ui->calendarWidget, SIGNAL(clicked(QDate)), this, SLOT(updateEventsView()));
    QObject::connect(ui->calendarWidget, SIGNAL(selectionChanged()), this, SLOT(updateEventsView()));
}

// PURPOSE: deconstructor
dashboard::~dashboard()
{
    delete ui;
}

// PURPOSE: executes sql command and displays the results.
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

// PURPOSE: displays results from sql query when refresh is clicked for online viewtable.
// CURRENT STATUS: need to click refresh to update calendar for deleted days
//                 and to refresh eventsview
void dashboard::on_loadonline_clicked()
{
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");
}


// PURPOSE: displays a new window to fill out event info when event add button is clicked.
void dashboard::on_addevents_clicked()
{
    Dialog h;
    QDate selectedDay = ui->calendarWidget->selectedDate();
    h.setDate(selectedDay); // sets date edit text to currently selected date
    h.setUser(myuser);
    h.setModal(true);
    h.exec();

    // to avoid refresh, paint automatically and load event
    if(h.accepted)
    {
        paint(h.getDate(), Qt::green);
        updateEventsView();
    }
}

//Purpose: When the user selected an row from the events table, the information will be displayed on the left for the user to edit.
void dashboard::on_eventsview_activated(const QModelIndex &index)
{
   QString val=ui->eventsview->model()->data(index).toString();

   QSqlQuery selectQry;
   selectQry.prepare("SELECT * FROM innodb.EVENTS WHERE ID='"+val+"'");
   //Display the information into the Left field boxes.
   if(selectQry.exec()){
       while(selectQry.next()){
           ui->NameDisplay->setText(selectQry.value(5).toString());
           ui->DescriptxtEdit->setText(selectQry.value(2).toString());
           ui->DateTxt->setText(selectQry.value(1).toString());
           ui->StartTimeTxt->setText(selectQry.value(3).toString());
           ui->EndTimeTxt->setText(selectQry.value(4).toString());
           ui->ID_Label->setText(selectQry.value(0).toString());
       }
   }
   //To
   else {
       QMessageBox::critical(this,tr("error::"),selectQry.lastError().text());
   }
}

//Purpose: When the user hits the edit button, the user can update his/her information.
void dashboard::on_editEvents_clicked()
{
    QString matchuser = ui->NameDisplay->text();
    if(myuser == matchuser) {
    QMessageBox::StandardButton choice;
    choice = QMessageBox::warning(this,"Save Changes","Are you sure you want to change?", QMessageBox::Save | QMessageBox::Cancel);
    //If user clicks save then the information will change.
    if(choice == QMessageBox::Save){
        QString Editdate = ui->DateTxt->toPlainText();
        QString Editstart = ui->StartTimeTxt->toPlainText();
        QString Editend = ui->EndTimeTxt->toPlainText();
        QString Editdesc = ui->DescriptxtEdit->toPlainText();
        QString ID_Param = ui->ID_Label->text();
        QSqlQuery query_update;
        query_update.exec("UPDATE innodb.EVENTS SET date='"+Editdate+"',start='"+Editstart+"', end='"+Editend+"',description='"+Editdesc+"' WHERE ID='"+ID_Param+"' AND owner='"+myuser+"'");
     }
    }
    //Tells the user he/she can not change other's schedules.
    else {
       QMessageBox MsgBox;
       MsgBox.setWindowTitle("Woah There!");
       MsgBox.setText("Sorry, but you can't change other's schedules!");
       MsgBox.exec();
    }

}

//Purpose to delete your event from the database and not other's
void dashboard::on_deleteEvents_clicked()
{
    QString matchuser = ui->NameDisplay->text();
    if(myuser == matchuser) {
        QMessageBox::StandardButton choice;
        choice = QMessageBox::warning(this,"Delete Event?", "Are you sure you want to delete your event?",QMessageBox::Yes | QMessageBox::No);
        if (choice == QMessageBox::Yes) {
            QString ID_Param = ui->ID_Label->text();
            QSqlQuery query_delete;
            QDate currDate = QDate::fromString(ui->DateTxt->toPlainText(), "ddd MMM d yyyy");
            query_delete.exec("DELETE FROM innodb.EVENTS WHERE ID='"+ID_Param+"'");

            // paint back to default color
            paint(currDate, Qt::white);
            updateEventsView(); // auto. refresh cell events
        }
    }
    //Tell the user he/she can not delete other's schedules.
    else {
        QMessageBox MsgBox1;
        MsgBox1.setWindowTitle("Wait a Second!");
        MsgBox1.setText("Sorry, but you can't delete other's schedules!");
        MsgBox1.exec();
    }
}

// PURPOSE: Updates eventsview based on the selected event day
void dashboard::updateEventsView()
{
    //QSqlQueryModel used to execute sql and traverse the results on a view table.
    QSqlQueryModel * modal = new QSqlQueryModel();
    QSqlQuery * query = new QSqlQuery(myconn.db);

    query->prepare("SELECT * FROM innodb.EVENTS WHERE DATE ='" +ui->calendarWidget->selectedDate().toString() + "' AND owner ='" +myuser+ "'"); //
    query->exec();
    modal->setQuery(*query);
    ui->eventsview->setModel(modal);
}

// PURPOSE: changes all calendar days with an event associated with it
//          to the color green
void dashboard::paintEvents()
{
    QDate date;
    QSqlQuery *query = new QSqlQuery(myconn.db);    // used to query DB
    bool toContinue;        // used to control do-while

    // exec. query
    query->prepare("SELECT date FROM innodb.EVENTS WHERE owner ='" +myuser+ "'");
    query->exec();
    query->first();     // accesses first query result

    // for every day with an event, paint different color
    do{
        // parse QDate from query date result
        date = QDate::fromString(query->value(0).toString(), "ddd MMM d yyyy");

        // paints event day green
        paint(date, Qt::green);

        // determines if query is still valid
        toContinue = (query->isActive() && query->isSelect());

      }while(query->next());    // move to next query result
}

// used to paint specified day cell with specified color
void dashboard::paint(QDate date, QColor color)
{
    QBrush brush;
    QTextCharFormat charFormat;
    brush.setColor(color);
    charFormat = ui->calendarWidget->dateTextFormat(date);
    charFormat.setBackground(brush);
    ui->calendarWidget->setDateTextFormat(date, charFormat);
}
