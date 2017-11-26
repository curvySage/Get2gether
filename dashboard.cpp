#include <connection.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableWidget>
#include <QtGui/QTextCharFormat>
#include <QSignalMapper>

#include "dialog.h"
#include "grouppopup.h"
#include "ui_dashboard.h"
#include "dashboard.h"
#include "invitations.h"
#include "mythread.h"
#include "paintCell.h"
#include "delete.h"
#include "error.h"

/*=================================================================================================================================*/
//                                          class Dashboard-Specific Methods
/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Constructor       |*/
/*'------------------------'*/

/* Purpose:         Full constructor
 * Postconditions:  Creates main dashboard ui for current user
*/
dashboard::dashboard(QString u, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dashboard)
{
    /*-- Initialize dashboard attributes --*/
    myuser = u;                                         // set user
    isGroupMode = false;
    groupID = "0";
    resetStatus = false;
    displayObject = new display(myconn);

    /*-- Initialize dashboard --*/
    ui->setupUi(this);
    ui->calendarWidget->setGridVisible(true);           // creates calendar borders
    updateCalendarName(myuser);                     // set user dashboard label
    myconn.openConn();                                  // connect to database

    /*-- Thread -- */
    m_pRefreshThread = new MyThread(this);
    m_pRefreshThread->start();

    /*-- On Load --*/
    displayObject->displayResults(ui->onlineview, "SELECT username FROM innodb.USERS where status = 1");           // populate online "friends"
    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);          // paint calendar cells with events
    displayObject->updateEventsView(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
    displayObject->updateGroupsView(ui->groupsview, myuser);
    displayObject->updateBulletinsView(ui->bulletinView, groupID);
    displayObject->updateRemindersView(ui->reminders, myuser);
    ui->bulletinView->scrollToBottom();

    /*-- Signals & Slots --*/
    connect(ui->calendarWidget, SIGNAL(activated(QDate)), this, SLOT(on_addevents_clicked()));     // prompt add event for selected date when dbl-clicked
    connect(this, SIGNAL(updateReminder(QTableView*,QString)), displayObject, SLOT(updateRemindersView(QTableView*,QString)));
    connect(this, SIGNAL(updateBulletins(QTableView*,QString)), displayObject, SLOT(updateBulletinsView(QTableView*,QString)));
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Destructor        |*/
/*'------------------------'*/

/* Purpose:         Deletes dashboard, stops thread, and sets user to offline
 * Postconditions:  user is set offline, thread is stopped, then program exits.
*/
dashboard::~dashboard()
{
    QSqlQuery query;
    query.exec("UPDATE innodb.USERS SET status = 0 WHERE username = '"+myuser+"'");
    query.finish();

    m_pRefreshThread->performExit();
    while(m_pRefreshThread->isRunning())
    {
        QThread::msleep(10);
    }
    delete m_pRefreshThread;

    delete ui;
}

/* Purpose:         overrides the exit button for dashboard to add confirmation exit box.
 * Postconditions:  message box popups up to confirm exit action.
*/
void dashboard::closeEvent(QCloseEvent * e) {
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Confirm",
                                                                tr("Are you sure you want to exit?\n"),
                                                                QMessageBox::Cancel | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        e->ignore();
    } else {
        e->accept();
    }
}


/*=================================================================================================================================*/

/*.------------------------.*/
/*|        Mutators        |*/
/*'------------------------'*/

/* Purpose:         Determines group mode setting
 * Preconditions:   isGroup should be used to indicate
 *                  whether dashboard is in group mode
 *                  or not
 * Postconditions:  Sets isGroupMode to group mode
*/
void dashboard::setMode(bool isGroup)
{
    isGroupMode = isGroup;
}

/* Purpose:         Stores group ID of group selected
 * Preconditions:   group ID is set when selecting groupID
 *                  in groupsview
 * Postconditions:  Sets groupID to selected groupsview ID
*/
void dashboard::setGroupID(const QModelIndex &index)
{
    QString val=ui->groupsview->model()->data(index).toString();    // Grab groupID clicked in groupsview
    groupID = val;                                                  // Set groupID to selected groupsview ID
}

/* Purpose:         Stores group Name of group selected
 * Preconditions:   group ID is set when selecting groupID
 *                  in groupsview
 * Postconditions:  Sets groupName to selected group's name
*/
void dashboard::setGroupName()
{
    QSqlQuery groupNameQ;

    groupNameQ.prepare("SELECT name FROM innodb.GROUPS WHERE ID = '" +groupID+ "'");

    if(groupNameQ.exec() && groupNameQ.first())
        groupName = groupNameQ.value(0).toString();
}

/* Purpose:         Updates the calendar label and centers it.
 * Postconditions:  Sets calendarName to selected group's name
*/
void dashboard::updateCalendarName(QString name)
{
    ui->calendarName->setText(name);
    ui->calendarName->setAlignment(Qt::AlignCenter);
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|       Accessors        |*/
/*'------------------------'*/

/* Purpose:         Accesses value of groupID
 * Preconditions:   By default, groupID = "0", otherwise is
 *                  set when a groupID is selected in groupsview
 * Postconditions:  Returns groupID (an int value) in type QString
*/
QString dashboard::getGroupID()
{
    return groupID;
}

/* Purpose:         Accesses value of isGroupMode
 * Preconditions:   By default, isGroupMode = false, otherwise is
 *                  set when tab changed to "Group"
 * Postconditions:  Returns isGroupMode in type bool
*/
bool dashboard::getMode()
{
    return isGroupMode;
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|     Helper Methods     |*/
/*'------------------------'*/

/* Purpose:         Returns the total number of events occurring
 *                  on a specified day
 * Postconditions:  Depending on if on group mode determines the
 *                      query executed
 *                  Returns number of total events occurring on date
 *                      target in type int
*/
// Purpose: returns the value of the amount of events on target
//          owned by username
int dashboard::countEvents(QDate target)
{
    QSqlQuery queryCount_query;

    if(isGroupMode) {
        queryCount_query.prepare("SELECT COUNT(*) "
                                 "FROM innodb.EVENTS, innodb.USER_EVENTS "
                                 "WHERE date = '" +target.toString()+ "' "
                                 "AND ID = eventID "
                                 "AND username 	IN(SELECT username "
                                                 "FROM innodb.GROUP_MEMBERS "
                                                 "WHERE groupID = '" +groupID+ "') ");
    }
    else {
        queryCount_query.prepare("SELECT COUNT(*) "
                        "FROM innodb.EVENTS, innodb.USER_EVENTS "
                        "WHERE date = '" +target.toString()+
                        "' AND ID = eventID "
                        "AND username = '" +myuser+ "'");
    }

    queryCount_query.exec();
    queryCount_query.next();

    if(!queryCount_query.isActive())
    {
        qDebug() << queryCount_query.lastError().text();
    }

    return queryCount_query.value(0).toInt();
}



  /* Purpose:           Checks to see if there is an event selected
   * Postconditions:    Returns an error if the user did not select an event to
   *                    delete or edit the event.
 */
void dashboard::checkNoDateEvent()
{
        error::printError(DELETE_NO_DATE_SELECTED);
}

/*=================================================================================================================================*/
//                                                          SLOTS
/*=================================================================================================================================*/

/*.------------------------.*/
/*|        Buttons         |*/
/*'------------------------'*/

/* Purpose:         When clicking the home button display the
 *                  personal events and not show the user's group
 *                  events in the calendar.
 *                  Also display the user's personal calendar label.
 * Postcondtions:   Repopulates calendar with user's group and
 *                  personal events
 *                  Updates dashboard label
*/
void dashboard::on_homeButton_clicked()
{
    //Set the tab to Online if on group tab
    if(ui->networktabs->currentIndex() == 1){
        resetStatus=true;
        resetGroupAttributes();
        ui->networktabs->setCurrentIndex(0);                 // Reset set group attributes back to default
    }

    //Refresh Online Table
    displayObject->displayResults(ui->onlineview, "SELECT username "
                                   "FROM innodb.USERS "
                                   "WHERE status = 1");
    //Update Userlabel to the username
    updateCalendarName(myuser);
    displayObject->updateEventsView(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);                          // Paint personal and group events appropriately
}

/* Purpose:         Displays new even form used to create events
 * Postcondtions:   New event is created owned by this user and is immediately
 *                  stored in database
*/
void dashboard::on_addevents_clicked()
{

    /* If in group mode but user hasn't selected a group calendar
     *      Output error
    */
    if(this->groupID == "0" && isGroupMode)
    {
        error::printError(ADD_ERROR_NO_GROUP_SELECTED);

        return;
    }

    Dialog h;
    h.setDate(ui->calendarWidget->selectedDate());    // sets date edit text to currently selected date
    h.setUser(myuser);
    h.setWindowTitle("Create New Event");
    h.setModal(true);
    h.setMode(isGroupMode);
    h.setGroupID(groupID);
    h.exec();

    /* If user creates event,
     *      Paint relevant cell green
     *      Update eventsview to show new event
    */
    if(h.accepted)
    {
        emit updateReminder(ui->reminders, myuser);

        /* If dashboard is in group mode
         *      Paint group event
         *      Update eventsview with groupevents
         * Else,
         *      Paint personal event
         *      Update personal or group calendar eventsview
        */
        if(isGroupMode)
        {
            displayObject->updateMemberEvents(ui->eventsview, groupID, h.getDate().toString());
        }
        else
        {
            displayObject->updateEventsView(ui->eventsview, myuser, h.getDate().toString());
        }

        paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
    }
    else
    {
        qDebug("Insertion of event canceled.");
    }
}

/* Purpose:         Updates event information with user defined values in
 *                  event info text fields
 * Postconditions:  Event is modified and replaced in database
*/
void dashboard::on_editEvents_clicked()
{
    if(ui->NameDisplay->text().isEmpty()){
        checkNoDateEvent();
        return;
    }
    QString matchuser = ui->NameDisplay->text();                // Get event owner
    QDate originalDate = ui->calendarWidget->selectedDate();    // To compare if date changed
    QString ID_Param = ui->ID_Label->text();
    QSqlQuery isGroupEventQ;                                    // To select groupID of events table event
    bool isGroupEvent;                                          // To determine if selected event is a group event

    isGroupEventQ.exec("SELECT groupID "
                       "FROM innodb.EVENTS "
                       "WHERE ID = '" + ID_Param+ "'");
    isGroupEventQ.first();
    isGroupEvent = (isGroupEventQ.value(0).toInt() != 0);       // Personal events have attribute groupID = 0

    /* If event is a group event but in personal mode,
     *      Output error
    */
    if(isGroupEvent && !isGroupMode)
    {
        error::printError(DELETE_ERROR_INVALID_MODE);

        return;
    }

    /* If current user matches event owner or group event owner,
     *      Allow event modification
     * Else
     *      Print error: unauthorized to delete event
    */
    if(myuser == matchuser || groupName == matchuser)
    {
        QMessageBox::StandardButton choice;
        choice = QMessageBox::warning(this,"Save Changes","Are you sure you want to change?", QMessageBox::Save | QMessageBox::Cancel);

        /* If user indicates "Save",
         *      Replace existing data in database with indicated data in event
         *      details text fields
        */
        if(choice == QMessageBox::Save)
        {
            QDate Editdate = ui->dateEdit->date();
            QString Editstart = ui->startTime->text();
            QString Editend = ui->endTime->text();
            QString Editdesc = ui->DescriptxtEdit->toPlainText();
            QString Edityearweek;
            QSqlQuery query_update;

            /* IF date was changed
             *      Color cell appropriately
            */
            if (originalDate != Editdate)
            {
                int year = Editdate.year();
                int week = Editdate.weekNumber();
                Edityearweek = QString::number(year) + QString::number(week);
                /* if this event is the only event on originalDate
                 *      paint originalDate white
                 */
                if(countEvents(originalDate) == 1)
                     paintCell::paint(ui, originalDate, Qt::white);
            }

            // Update event details
            query_update.exec("UPDATE innodb.EVENTS "
                              "SET date='"+Editdate.toString()+
                              "',start='"+Editstart+
                              "', end='"+Editend+
                              "',description='"+Editdesc+
                              "',yearweek='"+Edityearweek+
                              "' WHERE ID='"+ID_Param+"'");

            if(query_update.isActive()) {
                qDebug().noquote() << "Modification of Event " +ID_Param+ " successful.";
            }
            else {
                qDebug() << query_update.lastError().text();
            }

            emit updateReminder(ui->reminders, myuser);

            if(isGroupEvent || isGroupMode)
            {
                displayObject->updateMemberEvents(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
            }
            else
            {
                displayObject->updateEventsView(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
            }

            paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
            clearEditInfo();
        }
        else {
            qDebug().noquote() << "Modification of Event " +ID_Param+ " canceled.";
        }

    }
    else
    {
       // Users cannot edit events that aren't theres
       error::printError(EDIT_ERROR_UNAUTH_USER);
    }

}

/* Purpose:         Allows event deletion
 * Postcondition:   Selected event is deleted from database and removed from calendar
*/
void dashboard::on_deleteEvents_clicked()
{
    if(ui->NameDisplay->text().isEmpty()){
        checkNoDateEvent();
        return;
    }

    QString ID_Param = ui->ID_Label->text();
    QString matchuser = ui->NameDisplay->text();                // Get event owner
    QString currentuser = myuser; // Grabbing the user's log in name.
    QString TheGroup = groupName;
    QDate currDate = ui->dateEdit->date();
    QString GroupID = groupID;

    int option = Delete::Do_Delete(ID_Param, matchuser, currentuser, isGroupMode, TheGroup, GroupID);

    if(option == 1){
        displayObject->updateEventsView(ui->eventsview, myuser, currDate.toString());
        emit updateReminder(ui->reminders, myuser);
    }
    else if (option == 2){
        displayObject->updateMemberEvents(ui->eventsview, groupID, currDate.toString());
        emit updateReminder(ui->reminders, myuser);
    }

    /* If today has no more events
     *    Paint today's cell white
    */

    if(countEvents(currDate) == 0)
        paintCell::paint(ui, currDate, Qt::white);     // paint back to default color

    clearEditInfo();                // clear edit info
}

/* Purpose:         slot for when user sends a message
 * Postcondtions:   Adds entry into inodb.BULLETINS determined by
 *                  user-specified message in message box
*/
void dashboard::on_sendButton_clicked()
{
    QString message = ui->messageBox->toPlainText();
    QSqlQuery query;
    QDateTime date = QDateTime::currentDateTime();

    query.prepare("INSERT INTO innodb.BULLETINS(date, userID, message, groupID) VALUES (:val,'"+myuser+"','"+message+"', :group)");
    query.bindValue(":val",date);
    query.bindValue(":group", getGroupID());
    query.exec();

    if (query.isActive()) {
        qDebug("Inserted message into database.");
    }
    else {
        qDebug() << query.lastError().text();
    }

    // clear message box after message is sent
    ui->messageBox->clear();
    ui->bulletinView->scrollToBottom();
}


/* Purpose:         Creates new Create Group form when Create Group button
 *                  is clicked
 * Postconditions:  New Create Group dialog pops up when Create Group clicked
*/
void dashboard::on_createGroup_clicked()
{
    GroupPopUp createGroup;
    createGroup.setUser(myuser);
    createGroup.setModal(true);
    createGroup.setWindowTitle("Create Group");
    createGroup.loadAddFriendsList();
    createGroup.exec();

    if(createGroup.accepted)
    {
        displayObject->updateGroupsView(ui->groupsview, myuser);
    }
}

/* Purpose:         Refreshes online view showing users online
 * Postconditions:  Results of db query of online users are displayed in onlineview table
*/
void dashboard::on_loadonline_clicked()
{
    displayObject->displayResults(ui->onlineview, "SELECT username "
                                   "FROM innodb.USERS "
                                   "WHERE status = 1");
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Table Views       |*/
/*'------------------------'*/


/* Purpose:         Initializes event information form when user clicks
 *                  an event in eventsview
 * Postconditions:  Event info filled is determined by event values
 *                  stored in database
*/
void dashboard::on_eventsview_clicked(const QModelIndex &index)
{
    int rowidx = index.row();

    QString val=ui->eventsview->model()->index(rowidx , 0).data().toString();            // Grab value user clicked in eventsview
    bool isGroupEvent;
    QSqlQuery selectQry;
    selectQry.prepare("SELECT username, ID, date, description, start, end, groupID "
                      "FROM innodb.EVENTS, innodb.USER_EVENTS "
                      "WHERE ID = eventID AND ID ='" +val+ "'");

    /* If query executes,
     *      Initialize info text fields with relevant data
    */
    if(selectQry.exec() && selectQry.first()){
            isGroupEvent = (selectQry.value(6).toInt() != 0);
            if(isGroupEvent)
            {
                QSqlQuery groupNameQ;
                QString groupName;

                groupNameQ.prepare("SELECT name FROM innodb.GROUPS WHERE ID = '" +selectQry.value(6).toString()+ "'");

                if(groupNameQ.exec() && groupNameQ.first())
                {
                    groupName = groupNameQ.value(0).toString();
                }

                ui->NameDisplay->setText(groupName);
            }
            else
            {
                ui->NameDisplay->setText(selectQry.value(0).toString());
            }

            ui->DescriptxtEdit->setText(selectQry.value(3).toString());
            ui->dateEdit->setDate(QDate::fromString(selectQry.value(2).toString(), "ddd MMM d yyyy"));
            ui->startTime->setTime(QTime::fromString(selectQry.value(4).toString(), "h:mm AP"));
            ui->endTime->setTime(QTime::fromString(selectQry.value(5).toString(), "h:mm AP"));
            ui->ID_Label->setText(selectQry.value(1).toString());
    }
    //To
    else {
        QMessageBox::critical(this,tr("error::"),selectQry.lastError().text());
    }
}

/* Purpose:         Displays group members of group clicked in groups view
 * Postconditions:  Query is determined by group clicked; results of query
 *                  populate membersview
*/
void dashboard::on_groupsview_clicked(const QModelIndex &index)
{
    // reset paint
    resetStatus = true;
    resetStatus = false;

    int rowidx = index.row();

    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,true,myuser,groupID,myconn);          // resets calendar cell color to default

    groupID = ui->groupsview->model()->index(rowidx , 0).data().toString();
    groupName = ui->groupsview->model()->index(rowidx , 1).data().toString();
    updateCalendarName("# " + groupName);

    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);          // paint calendar cells with events
    displayObject->updateGroupEvents(ui->eventsview, groupID);
    displayObject->updateMembersView(ui->membersview, groupID);
    emit updateBulletins(ui->bulletinView, groupID);
}


/* Purpose:         Slot that contains functions that refreshes online/bulletin view.
 * Preconditions:   Signal is emitted.
 * Postconditions:  online/bulletin view is refreshed
*/
void dashboard::slot_refreshThread()
{
    on_loadonline_clicked();
    emit updateBulletins(ui->bulletinView, groupID);
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|   Dashboard Elements   |*/
/*'------------------------'*/

/* Purpose:         Replaces event info details with default values
 * Postconditions:  Text fields are blank, date value resetted to 01/01/00,
 *                  time fields reset to 12AM
*/
void dashboard::clearEditInfo()
{
    ui->NameDisplay->clear();
    ui->DescriptxtEdit->clear();
    ui->dateEdit->setDate(QDate(2000, 1, 1));
    ui->startTime->setTime(QTime(0, 0, 0, 0));
    ui->endTime->setTime(QTime(0, 0, 0, 0));
    ui->ID_Label->clear();
}

/* Purpose:         Determines which mode dashboard is in:
 *                  group or personal
*/
void dashboard::on_networktabs_currentChanged(int index)
{
    // If tab index = 0 ("Friends"), set isGroupMode false
    if(index == 0)
    {
        resetGroupAttributes();
        setMode(false);
        displayObject->updateEventsView(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
        paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
        updateCalendarName(myuser);
        // must include when clicking away from tableview as well
        ui->groupsview->clearSelection();
        ui->groupsview->clearFocus();
    }
    else
    {
        setMode(true);
    }
}

/* Purpose:         slot for when user types a message in bulletin.
 *                  limits message length and updates remaining characters.
 * Postconditions:  message box is updated as user types into given text
 *                  field
*/
void dashboard::on_messageBox_textChanged()
{
    int MAX = 100;

    // get message length. then update display count.
    QString lettercount = QString::number(ui->messageBox->toPlainText().length());
    ui->count->setText(lettercount + " / " + QString::number(MAX));
    // if message is over 100, limit it

    // validation check if user pastes in input that is over 100.
    if (ui->messageBox->toPlainText().length() > MAX + 1) {
        error::printError(TXTBOX_MAX_CHAR_REACHED);

        ui->messageBox->clear();
    }

    // validation check if users types over 100 characters.
    if (ui->messageBox->toPlainText().length() > MAX) {
        ui->messageBox->textCursor().deletePreviousChar();
    }

    // if message is 100, change label to red, else back to black.
    if (ui->messageBox->toPlainText().length() == MAX) {
        ui->count->setStyleSheet("color: red;");
    }
    else {
        ui->count->setStyleSheet("color: black;");
    }
}

/* Purpose:         slot for when user edits event descriptor
 *                  limits message length and updates remaining characters.
 * Postconditions:  descriptor is updated as user types into given text
 *                  field
*/
void dashboard::on_DescriptxtEdit_textChanged()
{
    int MAX = 50;

    // validation check if user pastes in input that is over 100.
    if (ui->DescriptxtEdit->toPlainText().length() > MAX + 1) {
        error::printError(TXTBOX_MAX_CHAR_REACHED);

        ui->DescriptxtEdit->clear();
    }

    // validation check if users types over 100 characters.
    if (ui->DescriptxtEdit->toPlainText().length() > MAX) {
        ui->DescriptxtEdit->textCursor().deletePreviousChar();
    }

    // if message is 100, change label to red, else back to black.
    if (ui->DescriptxtEdit->toPlainText().length() == MAX) {
        ui->DescriptxtEdit->setStyleSheet("color: red;");
    }
    else {
        ui->DescriptxtEdit->setStyleSheet("color: black;");
    }
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|        Calendar        |*/
/*'------------------------'*/

/* Purpose:         Emits signal to update relevant table views whenever
 *                  a new calendar date cell is selected
 * Postconditions:  Eventsview is updated to contain all events relevant
 *                  to chosen cell date
*/
void dashboard::on_calendarWidget_selectionChanged()
{
    clearEditInfo();

    if(isGroupMode)
    {
        emit updateTable(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
    }
    else
    {
        resetGroupAttributes();
        emit updateTable(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
    }

}

/* Purpose:         Emits signal to update relevant table views whenever
 *                  a new calendar date cell is clicked
 * Postconditions:  Determines calendar signal-slot connection based on mode
 *                  Eventsview is updated to contain all events relevant
 *                      to chosen cell date
*/
void dashboard::on_calendarWidget_clicked(const QDate &date)
{
    if(isGroupMode)
    {
        disconnect(this, SIGNAL(updateTable(QTableView*,QString,QString)), displayObject, SLOT(updateEventsView(QTableView*, QString, QString)));
        connect(this, SIGNAL(updateTable(QTableView*,QString,QString)), displayObject, SLOT(updateMemberEvents(QTableView*, QString, QString)));
        emit updateTable(ui->eventsview, groupID, date.toString());
    }
    else
    {
        disconnect(this, SIGNAL(updateTable(QTableView*,QString,QString)), displayObject, SLOT(updateMemberEvents(QTableView*, QString, QString)));
        connect(this, SIGNAL(updateTable(QTableView*,QString,QString)), displayObject, SLOT(updateEventsView(QTableView*, QString, QString)));
        emit updateTable(ui->eventsview, myuser, date.toString());
    }
}

/* Purpose:             Resets all group calendar functionality/attributes
 * Preconditions:       Should be called when dashboard is in
 *                      personal mode (!isGroupMode)
 * Postconditions:      Calendar functionality returns to default
 *                      abilities: show only logged in user's events
 *                      when a calendar date is selected
*/
void dashboard::resetGroupAttributes()
{
    resetStatus = true;
    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
    resetStatus = false;
    groupID = "0";     // Reset groupID
    groupName = "";
}




