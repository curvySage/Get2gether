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
    displayObject->initValues(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
    displayObject->updateEventsView();     // load calendar events for curr. selected date
    updateGroupsView();
    updateBulletinsView();  // populates bulletins view
    updateRemindersView();  // populates reminders view
    ui->bulletinView->scrollToBottom();

    /*-- Signals & Slots --*/
    QObject::connect(ui->calendarWidget, SIGNAL(activated(QDate)), this, SLOT(on_addevents_clicked()));     // prompt add event for selected date when dbl-clicked
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
    displayObject->updateEventsView();
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
        printError(ADD_ERROR_NO_GROUP_SELECTED);

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
        updateRemindersView();
        /* If dashboard is in group mode
         *      Paint group event
         *      Update eventsview with groupevents
         * Else,
         *      Paint personal event
         *      Update personal or group calendar eventsview
        */
        if(isGroupMode)
        {
            displayObject->initValues(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
            displayObject->updateMemberEvents();
        }
        else
        {
            displayObject->initValues(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
            displayObject->updateEventsView();
        }

        paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
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
        printError(DELETE_ERROR_INVALID_MODE);

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
            QString Editdate = ui->dateEdit->date().toString();
            QString Editstart = ui->startTime->text();
            QString Editend = ui->endTime->text();
            QString Editdesc = ui->DescriptxtEdit->toPlainText();
            QSqlQuery query_update, query_count;
            int eventCount;

            // Update event details
            query_update.exec("UPDATE innodb.EVENTS "
                              "SET date='"+Editdate+
                              "',start='"+Editstart+
                              "', end='"+Editend+
                              "',description='"+Editdesc+
                              "' WHERE ID='"+ID_Param+"'");

            // To count today's events to determine if today's cell should
            // still be colored or not
            query_count.exec("SELECT COUNT(*) "
                             "FROM innodb.EVENTS, innodb.USER_EVENTS "
                             "WHERE date = '" +originalDate.toString()+
                             "' AND ID = eventiD "
                             "AND username ='" +myuser+ "'");
            query_count.first();
            eventCount = query_count.value(0).toInt();  // saves return value

            QDate modifyDate = ui->dateEdit->date();    // Store date modified in editEvent container

            /* IF date was changed
             *      Color cell appropriately
            */
            if (originalDate != modifyDate)
            {
                /* if no more events today
                 *      paint originalDate white
                 * paint modifyDate green
                 */
                if(eventCount == 0)
                    paintCell::paint(ui, originalDate, Qt::white);

                if(isGroupEvent)
                {
                    //paint(modifyDate, Qt::cyan);
                    displayObject->initValues(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
                    displayObject->updateMemberEvents();
                }
                else
                {
                    //paint(modifyDate, Qt::green);

                    if(isGroupMode)
                    {
                        displayObject->initValues(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
                        displayObject->updateMemberEvents();
                    }
                    else
                    {
                        displayObject->initValues(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
                        displayObject->updateEventsView();
                    }
                }

                paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
                clearEditInfo();
            }
        }
    }
    else
    {
       // Users cannot edit events that aren't theres
       printError(EDIT_ERROR_UNAUTH_USER);
    }

}

/* Purpose:         Allows event deletion
 * Postcondition:   Selected event is deleted from database and removed from calendar
*/
void dashboard::on_deleteEvents_clicked()
{
    QSqlQuery isGroupEventQ;                                    // To select groupID of returned event
    QString ID_Param = ui->ID_Label->text();
    bool isGroupEvent;                                          // to determine if group event

    isGroupEventQ.exec("SELECT groupID "
                       "FROM innodb.EVENTS "
                       "WHERE ID = '" + ID_Param+ "'");
    isGroupEventQ.first();
    isGroupEvent = (isGroupEventQ.value(0).toInt() != 0);       // Determine if group event (i.e. groupID != 0)

    /* If event is a group event but in personal mode,
     *      Output error
    */
    if(isGroupEvent && !isGroupMode)
    {
        printError(DELETE_ERROR_INVALID_MODE);
        return;
    }

    if(ui->NameDisplay->text().isEmpty()){
        checkNoDateEvent();
        return;
    }

    QString matchuser = ui->NameDisplay->text();                // Get event owner
    QString currentuser = myuser; // Grabbing the user's log in name.
    QString TheGroup = groupName;
    QDate currDate = ui->dateEdit->date();
    QString GroupID = groupID;


    int option = Delete::Do_Delete(ID_Param, matchuser, currentuser, isGroupMode, TheGroup, GroupID);
    if(option == 1){
        displayObject->initValues(ui->eventsview, currentuser, currDate.toString());
        displayObject->updateEventsView();
        updateRemindersView();
    }
    else if (option == 2){
        displayObject->initValues(ui->eventsview, GroupID, currDate.toString());
        displayObject->updateMemberEvents();
        updateRemindersView();
    }

    QSqlQuery query_count;
    // count today's events to determine whether today's cell
    // should still be colored or not
    query_count.exec("SELECT COUNT(*) "
                     "FROM innodb.EVENTS, innodb.USER_EVENTS "
                     "WHERE date = '" +currDate.toString()+
                     "' AND ID = eventiD AND username ='" +currentuser+ "'");
    query_count.first();
    int eventCount = query_count.value(0).toInt();

    /* If today has no more events
     *    Paint today's cell white
    */
    if(eventCount == 0)
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
        updateGroupsView();
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
        while(selectQry.next()){
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
            ui->startTime->setTime(QTime::fromString(selectQry.value(4).toString(), "hh:mm AP"));
            ui->endTime->setTime(QTime::fromString(selectQry.value(5).toString(), "hh:mm AP"));
            ui->ID_Label->setText(selectQry.value(1).toString());
        }
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
    int rowidx = index.row();

    groupID = ui->groupsview->model()->index(rowidx , 0).data().toString();
    groupName = ui->groupsview->model()->index(rowidx , 1).data().toString();
    updateCalendarName("# " + groupName);

    paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);          // paint calendar cells with events

    // where error is occurring --> updateGroupEvents()
    // click groups tab
    // click 1 group, then a 2nd = segmentation fault
    displayObject->initValues(ui->groupsview, groupID, "");
    displayObject->updateGroupEvents();    // show all group's events in eventsview
    //displayObject->initValues(ui->membersview, groupID, NULL);
    //displayObject->updateMembersView();

    // update bulletin view to group specific
    //updateBulletinsView();
}


/* Purpose:         Slot that contains functions that refreshes online/bulletin view.
 * Preconditions:   Signal is emitted.
 * Postconditions:  online/bulletin view is refreshed
*/
void dashboard::slot_refreshThread()
{
    on_loadonline_clicked();
    updateBulletinsView();
}

/*=================================================================================================================================*/

/*.------------------------.*/
/*|      Table Views       |*/
/*'------------------------'*/

/* Purpose:         Displays all of the user's associated groups into
 *                  groupsview
 * Postconditions:  All groups user is a part of is displayed in groupsview
*/
void dashboard::updateGroupsView()
{
    displayObject->displayResults(ui->groupsview, "SELECT ID AS \"ID\", name AS \"Name\" "
                                   "FROM innodb.GROUPS, innodb.GROUP_MEMBERS "
                                   "WHERE ID = groupID "
                                   "AND username = '" +myuser+ "'");          // populate associated groups
}

/* Purpose:         Displays all bulletin messages in
 *                  bulletin tab
 * Postcondtions:   All bulletin messages stored in innodb.BULLETINS
 *                  are selected and displayed in bulletinview
*/
void dashboard::updateBulletinsView()
{
    qDebug() << "dashboard : updateBulletinsView()";
    QString currentGroup = getGroupID();

    displayObject->displayResults(ui->bulletinView, "SELECT userID AS \"User\", message AS \"Message\" "
                                     "FROM innodb.BULLETINS where groupID = '"+currentGroup+"'");
    ui->bulletinView->resizeRowsToContents();

    // Messages are kept for a week in the database.
    // Messages are deleted every Monday at 1:00AM
    // All messages are automatically deleted using an event scheduler in the database.
}

/* Purpose:         returns the events from the current week.
 * Precondtions:    An event has its yearweek column set when inserted in the database. This is done in dialog.cpp
 * Postconditions:  reminders is uploaded with all events upcoming in current week
*/
void dashboard::updateRemindersView()
{
    // get the current year and week from currentDate.
    int week = QDate::currentDate().weekNumber();  //ex: 43
    int year = QDate::currentDate().year();        //ex: 2017
    QString yearweek = QString::number(year) + QString::number(week); // ex: 201743

    // return events from the current week.
    displayObject->displayResults(ui->reminders,
                   "SELECT name AS \"Group\", description AS \"Details\", date AS \"Date\", start AS \"Start\", end  AS \"End\" "
                   "FROM innodb.EVENTS, innodb.GROUPS "
                   "WHERE yearweek = '"+yearweek+"' AND innodb.EVENTS.ID "
                   "IN (SELECT eventID FROM innodb.USER_EVENTS WHERE (innodb.GROUPS.ID = groupID AND username = '"+myuser+"'))");
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
        displayObject->updateEventsView();
        paintCell::paintEvents(ui,ui->calendarWidget->selectedDate(),isGroupMode,resetStatus,myuser,groupID,myconn);
        updateCalendarName(myuser);
    }
    else
    {

        setMode(true);
        /*-- Group Signals & Slots --*/
        // Activated when groups tab is selected
        // Loads group and personal events into calendar
        //QObject::connect(ui->calendarWidget, SIGNAL(clicked(QDate)), displayObject, SLOT(updateMemberEvents(ui->eventsview, ui->calendarWidget->selectedDate().toString(), groupID)));           // update eventsview for selected date using mouse
        //QObject::connect(ui->calendarWidget, SIGNAL(selectionChanged()), displayObject, SLOT(updateMemberEvents(ui->eventsview, ui->calendarWidget->selectedDate().toString(), groupID)));       // update eventsview for selected date using keyboard
    }

    // must include when clicking away from tableview as well
    ui->groupsview->clearSelection();
    ui->groupsview->clearFocus();
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
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Error");
        MsgBox.setText("Your input is over the character limit.");
        MsgBox.exec();

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
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Error");
        MsgBox.setText("Your input is over the character limit.");
        MsgBox.exec();

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
/*|        Calendar        |*/
/*'------------------------'*/

/* Purpose:         Updates eventsview and event info fields when date selected is
 *                  changed
 * Postconditions:  Updates eventsview to selected dates relevant events
 *                  If selected date has no events, event info is reset to default values
 *                  Else, filled with event info
*/
void dashboard::on_calendarWidget_selectionChanged()
{
    clearEditInfo();

    if(isGroupMode)
    {
        displayObject->initValues(ui->eventsview, groupID, ui->calendarWidget->selectedDate().toString());
        displayObject->updateMemberEvents();
    }
    else
    {
        resetGroupAttributes();
        displayObject->initValues(ui->eventsview, myuser, ui->calendarWidget->selectedDate().toString());
        displayObject->updateEventsView();
    }

}

/* Purpose:             Disconnects group calendar connections
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

/*
 * Methods created to separate methods
*/

// Purpose: returns the value of the amount of events on target
//          owned by username
int dashboard::countEvents(QDate target, QString username)
{
    QSqlQuery queryCount_query;

    queryCount_query.exec("SELECT COUNT(*) "
                       "FROM innodb.EVENTS, innodb.USER_EVENTS "
                       "WHERE date = '" +target.toString()+
                       "' AND ID = eventID "
                       "AND username = '" +username+ "'");

    queryCount_query.next();

    return queryCount_query.value(0).toInt();
}

bool dashboard::isGroupEvent(int eventID, int &groupID)
{
    QSqlQuery groupID_query;

    groupID_query.exec("SELECT groupID "
                       "FROM innodb.EVENTS "
                       "WHERE ID = '" +QString::number(eventID)+ "'");

    groupID_query.next();

    groupID = groupID_query.value(0).toInt();

    return (groupID != 0);
}

void dashboard::printError(ErrorCode error_code)
{
    QMessageBox ErrorBox;
    ErrorBox.setWindowTitle("Woah There!");
    QString errorMsg, consoleMsg;

    switch(error_code)
    {
        case EDIT_ERROR_UNAUTH_USER:
        {
            errorMsg = "Error: you can't change other's schedules.";
            consoleMsg = "Modification failed: you can only modify events you own.";
            break;
        }
        case DELETE_ERROR_UNAUTH_USER:
        {
            errorMsg = "Error: you can't delete other's schedules.";
            consoleMsg = "Deletion failed: you can only delete events you own.";
            break;
        }
        case ADD_ERROR_NO_GROUP_SELECTED:
        {
            errorMsg = "Error: Select a group you want to add a group event.";
            consoleMsg = "Insertion failed : select group to add group event.";
            break;
        }
        case EDIT_ERROR_INVALID_MODE:
        {
            errorMsg = "Error: you can't edit group event from personal calendar.";
            consoleMsg = "Modification failed : group event must be edited from group calendar.";
            break;
        }
        case DELETE_ERROR_INVALID_MODE:
        {
            errorMsg = "Error: you can't delete group event from personal calendar.";
            consoleMsg = "Deletion failed : group event must be deleted from group calendar.";
            break;
        }
        default:
        {
            errorMsg = "Error: action causes unknown error.";
            consoleMsg = "Recent action has caused an error";
        }
    }

    ErrorBox.setText(errorMsg);
    ErrorBox.exec();
    qDebug().noquote() << consoleMsg;
}

  /* Purpose: Checks to see if there is an event selected
 * Postconditions: Returns an error if the user did not select an event to
 * delete or edit the event.
 */
void dashboard::checkNoDateEvent()
{
        QMessageBox MsgBox;
        MsgBox.setWindowTitle("Where is your event?");
        MsgBox.setText("No Event was selected. Try Again.");
        MsgBox.exec();
}

void dashboard::on_calendarWidget_clicked(const QDate &date)
{
    if(isGroupMode)
    {
        disconnect(ui->calendarWidget, SIGNAL(clicked(QDate)), displayObject, SLOT(updateEventsView()));
        disconnect(ui->calendarWidget, SIGNAL(selectionChanged()), displayObject, SLOT(updateEventsView()));
        displayObject->initValues(ui->eventsview, groupID, date.toString());
        displayObject->updateMemberEvents();
    }
    else
    {
        disconnect(ui->calendarWidget, SIGNAL(clicked(QDate)), displayObject, SLOT(updateMemberEvents()));
        disconnect(ui->calendarWidget, SIGNAL(selectionChanged()), displayObject, SLOT(updateMemberEvents()));
        displayObject->initValues(ui->eventsview, myuser, date.toString());
        displayObject->updateEventsView();
    }
}
