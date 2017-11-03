#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>
#include <QTableView>
#include <connection.h>
#include <dialog.h>
namespace Ui {
class dashboard;
}

/* Class purpose:   Creates user's main dashboard
*/
class dashboard : public QDialog
{
    Q_OBJECT

public:
    connection myconn;
    QString myuser;

    explicit dashboard(QString u, QWidget *parent = 0);
    ~dashboard();
    void displayResults(QTableView * table, QString);
    void paint(QDate date, QColor color);

private slots:
    void on_loadonline_clicked();
    void on_addevents_clicked();
    void on_eventsview_clicked(const QModelIndex &index);
    void on_editEvents_clicked();
    void on_deleteEvents_clicked();
    void updateEventsView();
    void paintEvents();
    void clearEditInfo();
    void on_calendarWidget_selectionChanged();
    void on_groupsview_clicked(const QModelIndex &index);

    void on_createGroup_clicked();

private:
    Ui::dashboard *ui;
};

#endif // DASHBOARD_H
