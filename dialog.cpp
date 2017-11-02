#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QPlainTextEdit>
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
    QString mydate = ui->dateEdit->date().toString();
    QString mystart = ui->start->text();
    QString myend = ui->end->text();
    QString mydesc = ui->description->toPlainText();

    QSqlQuery query;
    query.exec("INSERT INTO innodb.EVENTS (date, start, end, description, owner) VALUES ('"+mydate+"','"+mystart+"', '"+myend+"','"+mydesc+"', '"+myuser+"')");

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
