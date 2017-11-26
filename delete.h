#ifndef DELETE_H
#define DELETE_H
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>
#include <connection.h>
#include <QDate>
#include "dashboard.h"
#include "ui_dashboard.h"

/* Class purpose:   Contains method that deletes an event from
 *                  users' stored data
*/
class Delete
{
public:
    Delete();
    static int Do_Delete(QString ID_Param, QString matchuser, QString currentuser, bool isGroupMode, QString TheGroup, QString GroupID);

};

#endif // DELETE_H
