#include "dashboard.h"
#include "ui_dashboard.h"
#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>


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
