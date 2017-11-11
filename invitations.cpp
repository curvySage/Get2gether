#include "invitations.h"
#include "ui_invitations.h"
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>


invitations::invitations(QString myuser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::invitations)
{
    ui->setupUi(this);

    // query database to find invites
    QSqlQueryModel * modal = new QSqlQueryModel();
    QSqlQuery * query = new QSqlQuery(myconn.db);

    query->prepare("SELECT A.inviter, B.name FROM innodb.INVITES AS A, innodb.GROUPS AS B WHERE A.invitee ='"+myuser+"'AND A.groupID = B.ID");
    query->exec();
    modal->setQuery(*query);
    ui->invitesview->setModel(modal);
}

invitations::~invitations()
{
    delete ui;
}
