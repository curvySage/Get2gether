#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QDialog>
#include <QTableView>
#include <connection.h>
#include <dialog.h>
namespace Ui {
class dashboard;
}

// PURPOSE: This class displays the dashboard.
class dashboard : public QDialog
{
    Q_OBJECT

public:
    explicit dashboard(QWidget *parent = 0);
    ~dashboard();

    connection myconn;
    QString myuser;
    void setUser(QString u);
    void displayResults(QTableView * table, QString);

private slots:
    void on_loadonline_clicked();
    void on_loadevents_clicked();
    void on_addevents_clicked();

    void on_eventsview_activated(const QModelIndex &index);

private:
    Ui::dashboard *ui;
};

#endif // DASHBOARD_H
