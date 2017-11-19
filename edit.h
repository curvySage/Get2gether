#ifndef EDIT_H
#define EDIT_H

#include <QString>
#include "connection.h"

/*
 * Purpose: To edit event
*/
class edit
{

public:
    /*
     * Attributes
    */
    QString eventOwner;
    bool dateModified;
    int eventID;
    QString originalDate;
    QString submittedDate;
    QString newStartTime;
    QString newEndTime;
    QString newDescription;

    /*
     * Methods
    */
    edit();
    edit(QString newOwner, int id, QString oDate, QString sDate, QString start, QString end, QString desc);
    ~edit();
    bool update(const connection &dbConn);
};

#endif // EDIT_H
