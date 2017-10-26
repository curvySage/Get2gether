#include "dashboard.h"
#include "ui_dashboard.h"
#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include "dialog.h"


// PURPOSE: default constructor
dashboard::dashboard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dashboard)
{
    ui->setupUi(this);

    ui->userlabel->setText(myuser);

    // when dashboard is open, events and onlinelist are automatically loaded.
    myconn.openConn();
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");
    displayResults(ui->eventsview, "SELECT * FROM innodb.EVENTS");

    QObject::connect(ui->calendarWidget, SIGNAL(clicked(QDate)), this, SLOT(on_addevents_clicked()));
}

// PURPOSE: deconstructor
dashboard::~dashboard()
{
    delete ui;
}

// PURPOSE: used to pass in username from mainwindow.
void dashboard::setUser(QString u)
{
    myuser = u;
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
void dashboard::on_loadonline_clicked()
{
    displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");
}

// PURPOSE: displays results from sql query when refresh is clicked for events viewtable.
void dashboard::on_loadevents_clicked()
{
    displayResults(ui->eventsview, "SELECT * FROM innodb.EVENTS");
}

// PURPOSE: displays a new window to fill out event info when event add button is clicked.
void dashboard::on_addevents_clicked()
{
    Dialog h;
    h.setUser(myuser);
    h.setModal(true);
    h.exec();
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
            query_delete.exec("DELETE FROM innodb.EVENTS WHERE ID='"+ID_Param+"'");
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
