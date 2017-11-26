#ifndef PAINTCELL_H
#define PAINTCELL_H

#include <QDialog>
#include <QTableView>
#include "connection.h"
//#include <connection.h>
#include <dialog.h>
#include <QThread>
#include <QtCore>
#include <mythread.h>
#include "dashboard.h"
#include <QString>
#include <QColor>
#include <QIcon>
#include "ui_dashboard.h"

/* Class purpose:   Contains methods relevant the
 *                  painting of a calendar widget
 *                  located on the dashboard object
 * Assumptions:     Cyan = group event
 *                  Green = personal event
 *                  White = default color/no event
*/
class paintCell
{
public:
    paintCell();
    static void paintEvents(Ui::dashboard *ui,QDate date,bool isGroupMode,bool resetStatus,QString myuser,QString groupID,connection &newconn);
    static void paint(Ui::dashboard *ui,QDate date, QColor color);
};
#endif // PAINTCELL_H
