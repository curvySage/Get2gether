#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>

// PURPOSE: default constructor
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // open new connection to database and check for status.
    myconn.openConn();

    // if unable to open, display error
    if (!myconn.db.isOpen()) {
        ui->status->setText("Status: Unable to establish connection.");
        qDebug() << myconn.db.lastError().text();
    }
    else {
        ui->status->setText("Status: Connection established! ");
    }
 }

// PURPOSE: default deconstructor, user status set to offline.
MainWindow::~MainWindow()
{
    QSqlQuery query;
    query.exec("UPDATE innodb.USERS SET status = 0 WHERE username = '"+myuser+"'");

    delete ui;
}

// PURPOSE: login to dashboard
void MainWindow::on_login_button_clicked()
{
    QSqlQuery query;
    int count = 0;

    myuser = ui->username->text();
    mypass = ui->password->text();

    query.exec("SELECT * FROM innodb.USERS WHERE username='"+myuser+"' AND password='"+mypass+"'");

    // check if query matches
    while(query.next()) {
        count++;
    }

    // if record found with entered user and pass, open dashboard.
    if(count==1) {
            query.exec("UPDATE innodb.USERS SET status = 1 WHERE username = '"+myuser+"'");
            myconn.closeConn();
            this->hide();

            dashboard dash(myuser);
            dash.setModal(true);
            dash.exec();
        }
        // else display error popup message
        else {
            QMessageBox::information(this, "Login", "Username or Password is incorrect.");
        }

    }

// PURPOSE: creates new account by inserting entered user and pass into database.
void MainWindow::on_create_clicked()
{
    QString user_s, pass_s;
    QSqlQuery query;

    user_s = ui->username_create->text();
    pass_s = ui->password_create->text();

    query.exec("INSERT INTO innodb.USERS (username, password) VALUES ('"+user_s+"', '"+pass_s+"')");

    // display popup message if query was succesful.
    if (query.isActive()) {
        qDebug("Account Created");
        QMessageBox::information(this, "Login", "Account Created!");

    }
    // display error message in console.
    else {
        qDebug() << query.lastError().text();
    }
}
