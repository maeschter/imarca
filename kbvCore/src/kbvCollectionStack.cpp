/*****************************************************************************
 * KbvCollectionStack
 * This is the stack for collection view, model, thread and database
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.05.14
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvCollectionStack.h"
#include "kbvSetvalues.h"
#include "kbvReplaceDialog.h"
#include "kbvInformationDialog.h"

extern  KbvSetvalues          *settings;
extern  KbvReplaceDialog      *replaceDialog;
extern  KbvInformationDialog  *informationDialog;

KbvCollectionStack::KbvCollectionStack(QWidget *parent, QString dbPath, QString dbName, QString rootDir,
                                    int dbType, KbvCollectionDragDrop *dragDropHandler) : KbvTabWidget(parent)
{
  QSqlQuery query;
  QString   stmt;

  
  this->failed = false;   //failed is a info for the parent collectionTab
  this->dbVer = "0.0";
  this->dbName = dbName;
  this->dbLocation = dbPath;
  this->collRoot = rootDir;
  //qDebug() << "KbvCollectionStack::KbvCollectionStack "<<dbName <<dbLocation <<collRoot; //###########

  //Create database connection and read basic collection settings
  //Close database in destructor since we need it as long as this tab is visible
  collDBConnection = "con"+dbName;
  collDB = QSqlDatabase::addDatabase("QSQLITE", collDBConnection);
  collDB.setHostName("host");
  collDB.setDatabaseName(dbPath+dbName+QString(dbNameExt));

  //Read database settings from table description as part of databasInfo
  //If the database cannot be opened or version doesn't match, do not add database tab
  if(collDB.open())
    {
      collDB.transaction();
      stmt = QString("SELECT version, iconSize, comment, colltype, keywordtypes, rootdir FROM description");
      query = collDB.exec(stmt);
      if(query.first())
        {
          this->dbVer = query.value(0).toString();
          this->dbIconSize = (query.value(1).toInt());
          this->dbDescription = query.value(2).toString();
          this->dbType = query.value(3).toInt();
          this->dbKeywordType = query.value(4).toInt();
        }
      collDB.commit();

      //The comparasion works for strings containing figures in form of major.minor.
      if(dbVer < QString(dbMinVer))
        {
          stmt = QString(tr("Database version doesn't match!\n\n"
                            "The installed version of Imarca is %1.\n"
                            "The detected version of the database is %2.\n"
                            "The required version of the database is %3.\n\n"
                            "When you already have the newest version of Imarca please upgrade the database.\n")).arg(appVersion).arg(dbVer).arg(dbMinVer);
          informationDialog->perform(stmt, "", 2);
          this->failed = true;
          //qDebug() << "KbvCollectionStack "<<dbVer <<dbMinVer; //###########
        }
    }
  else
    {
      stmt = QString(tr("Cannot connect to database %1")).arg(dbName);
      informationDialog->perform(stmt, "", 2);
      this->failed = true;
    }
  
  //Create database info here, view and model need it
  databaseInfo = new KbvDBInfo(this);
  databaseInfo->setDBInfo(this->dbType, this->dbIconSize, this->dbKeywordType, this->dbVer,
                          this->dbName, this->dbDescription, this->dbLocation, this->collRoot);

  //Create record info
  recordInfo = new KbvRecordInfo(this);
  
  //Create model and view and set the model to view
  collSortProxy = new KbvSortFilterModel(nullptr);
  collModel = new  KbvCollectionModel(nullptr, databaseInfo);
  collView = new  KbvCollectionView(nullptr, databaseInfo); //no parent by reason of stacked widget

  collView->setModels(collSortProxy, collModel);
  collView->setDragDropHandler(dragDropHandler);

  //Create threads
  collThread = new KbvCollectionThread(nullptr, databaseInfo);
  watchThread = new KbvCollectionWatchThread(nullptr, databaseInfo, settings->dirWatchCycle);
  collModel->setThreads(collThread, watchThread);

  //Create CSV checker
  csvChecker = new KbvCsvChecker(this, databaseInfo);

  //Now set grid size and the information to be shown below the icons
  collView->updateOptions();

  //Create animation for collection tabs
  animation = new KbvBusyAnimation();

  //Pop up menu including signal connections
  popupMenu = new KbvCollectionPopUp(this, Kbv::TypeCollection);
  connect(popupMenu, SIGNAL(copyPasteAction(QKeyEvent*)),  collView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(popupMenu, SIGNAL(search()),            this, SIGNAL(search()));
  connect(popupMenu, SIGNAL(showMetadata()),      this, SIGNAL(imageMetadata()));
  connect(popupMenu, SIGNAL(showEditor()),        this, SIGNAL(imageEdit()));
  connect(popupMenu, SIGNAL(showBatchEditor()),   this, SIGNAL(imageBatchEdit()));
  connect(popupMenu, SIGNAL(showDBAttributes()),  this, SLOT(showDBInfo()));
  connect(popupMenu, SIGNAL(showRecordContent()), this, SLOT(showRecordInfo()));

  //Stack collection view and animation and add to the tab widget
  stackWidget = new QStackedWidget();
  stackWidget->addWidget(collView);
  if((dbType & Kbv::TypeCollection) || (dbType & Kbv::TypeCollectionRoot))
    {
      stackWidget->addWidget(animation);
    }
  this->addWidget(stackWidget);

  //thread, model, sort proxy and animation
  connect(this,      SIGNAL(viewModeChanged(int)),    collView,      SLOT(changeViewMode(int)));
  connect(this,      SIGNAL(sortButtonClicked(int)),  collSortProxy, SLOT(sortButtonClicked(int)));
  connect(this,      SIGNAL(sortRoleChanged(int)),    collSortProxy, SLOT(sortRoleChanged(int)));
  connect(collThread,SIGNAL(warning(QString, bool)),  this,          SLOT(warnings(QString, bool)));
  connect(collThread,SIGNAL(askForUserInput(const QString, const QString)),
                     this,SLOT(askForUserInput(QString, QString)));

  connect(collModel, SIGNAL(animationInfoText(QString, QString)), this, SLOT(setAnimationInfo(QString, QString)));
  connect(animation, SIGNAL(running(QString, bool)),  this, SLOT(showAnimation(QString, bool)));
  connect(collModel, SIGNAL(infoText1(QString)),      this, SLOT(setInfoText1(QString)));
  connect(collModel, SIGNAL(infoText2(QString)),      this, SLOT(setInfoText2(QString)));
  connect(collModel, SIGNAL(infoText3(QString)),      this, SLOT(setInfoText3(QString)));
  connect(collView,  SIGNAL(infoText2(QString)),      this, SLOT(setInfoText2(QString)));
  connect(collView,  SIGNAL(infoText3(QString)),      this, SLOT(setInfoText3(QString)));
  connect(stackWidget, SIGNAL(currentChanged(int)),   this, SLOT(endOfAnimation(int)));
}

KbvCollectionStack::~KbvCollectionStack()
{
  //qDebug() << "KbvCollectionStack::~KbvCollectionStack";//###########
  //collView and animation are removed by the stack
  //stop thread and remove database connection of thread
  delete collThread;
  delete watchThread;
  delete collSortProxy;
  delete collModel;
  
  //remove database connection of this widget
  collDB = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(collDBConnection); //close connection
}

/*************************************************************************//*!
 * Slot: receive information about the visible tab from collection tabs by
 * tab title.
 */
void    KbvCollectionStack::currentTabTitle(QString title)
{
  //qDebug() << "KbvCollectionStack::currentTabTitle"<<collDBName << title; //###########
  if(title == dbName)
    {
      weAreVisible = true;
      this->collView->tabIsVisible(true);
      this->collModel->tabIsVisible(true);
    }
  else
    {
      weAreVisible = false;
      this->collView->tabIsVisible(false);
      this->collModel->tabIsVisible(false);
    }
}
/*************************************************************************//*!
 * Main menu or popup "Show database info" was activated. Display info about
 * database (table description) when this tab is visible and receive user input
 * of data field "comment" for table "description".
 */
void    KbvCollectionStack::showDBInfo()
{
  QString       description, stmt;
  QSqlQuery     query;
  int           result=0;
  
  stmt = QString("UPDATE OR REPLACE description SET comment = :comment");
  
  if(weAreVisible)
    {
      result = this->databaseInfo->exec();
      
      if(result == QDialog::Accepted)
        {
          description = databaseInfo->getDescription(); //text or empty
      
          query = QSqlQuery(collDB);
          collDB.transaction();
          
          //write table 'description'
          query.prepare(stmt);
          query.bindValue(":comment", QVariant(description));
          query.exec();
          
          collDB.commit();
        }
    }
}
/*************************************************************************//*!
 * Search function of KbvCollectionTabs.
 * Return a pointer to the databaseInfo.
 */
KbvDBInfo*  KbvCollectionStack::getDatabaseInfo()
{
  return databaseInfo;
}

/*************************************************************************//*!
 * SLOT: pop up menu of collection stacked widget: show record data and edit
 * user keywords, user date and user comment in dialog KbvRecordInfo. The
 * check for tab visibility is mandatory since the call can start in popup
 * and main menu.
 * The recordInfo displays: name, path, imagesize, keywords, datechanged,
 * filesize, crc32, userkeywords, usercomment, userdate and icon.
 * User eywords, user comment and user date are editible.
 * All data are processed inside the dialog.
 * The dialog result always is of type rejected and requires no action.
 */
void    KbvCollectionStack::showRecordInfo()
{
  QModelIndexList   recordIndices;
  QList<QVariant>   primaryKeys;

  if(weAreVisible)
    {
      //get selection or select all and map to model
      recordIndices = this->collView->selectionModel()->selectedIndexes();
      if(recordIndices.size() > 0)
        {
          for(int i=0; i<recordIndices.size(); i++)
            {
              recordIndices[i] = this->collSortProxy->mapToSource(recordIndices[i]);
              primaryKeys.append(this->collModel->data(recordIndices[i], Kbv::PrimaryKeyRole));
            }
          //qDebug() << "KbvCollectionStack::showRecordInfo" <<dbName <<dbLocation <<recordIndices.size(); //###########
          recordInfo->perform(primaryKeys, dbName, dbLocation);
        }
    }
}
/*************************************************************************//*!
 * Collection root directory.
 */
QString    KbvCollectionStack::getCollectionRoot()
{
  return  collRoot;
}
/*************************************************************************//*!
 * Pop up menu.
 */
void    KbvCollectionStack::contextMenuEvent(QContextMenuEvent *event)
{
  if(this->collView->selectionModel()->hasSelection())
    {
      this->popupMenu->exec(event->globalPos(), true);
    }
  else
    {
      this->popupMenu->exec(event->globalPos(), false);
    }
}
/*************************************************************************//*!
 * Slot: Insert function in thread needs input from user.
 * Called from collection thread. Opens replace dialog and sends user decision
 * to thread.
 */
void    KbvCollectionStack::askForUserInput(QString text1, QString text2)
{
  int   retval;

  retval = replaceDialog->perform(text1, text2);
  collThread->getUserInput(retval);
}
/*************************************************************************//*!
 * SLOT: Warnings. Required by thread.
 */
void    KbvCollectionStack::warnings(QString text, bool modal)
{
  if(modal)
    {
      informationDialog->perform(text, "", 1);
    }
  else
    {
      informationDialog->setCaptionAndIcon(text, "", 1);
      informationDialog->show();
    }
}
/*************************************************************************//*!
 * Slot: display the "busy animation" instead of collection tab. The animation
 * only is available for collections. Parameter title is the related
 * collection tab and database name too. The animation gets hidden on param
 * show=false when the finish button was pressed.
 */
void    KbvCollectionStack::showAnimation(QString title, bool show)
{
  QString           str;

  //qDebug() << "KbvCollectionStack::showAnimation" << title << dbType; //###########
  if ((dbName == title) && ((dbType & Kbv::TypeCollection) || (dbType & Kbv::TypeCollectionRoot)))
    {
      str = QString(tr("Working on database: %1").arg(title));
      if(show)
        {
          this->stackWidget->setCurrentIndex(1);     //animation visible
          animation->run(title, true);
          animation->setText1(str);
        }
      else
        {
          animation->run(title, false);
          animation->setText1("");
          this->stackWidget->setCurrentIndex(0);    //collection view visible
        }
    }
}
/*************************************************************************//*!
 * Slot: end of import or update thread. Display result overview in text
 * label 2 of "busy animation" and display the "finish" button.
 */
void    KbvCollectionStack::setAnimationInfo(QString title, QString info)
{
  //qDebug() << "KbvCollectionStack::setAnimationInfo" <<title <<info; //###########
  if (dbName == title)
    {
      this->stackWidget->setCurrentIndex(1);     //animation visible
      animation->setText2(info);
    }
}
/*************************************************************************//*!
 * Slot: triggerd by signal currentChanged. The "busy animation" had finished
 * and the animation widget has been closed by the "finish" button.
 * The collView is visible now so we can force the model to read the actual
 * branch which was updated.
 */
void    KbvCollectionStack::endOfAnimation(int index)
{
  if (index == 0)
    {
      //qDebug() << "KbvCollectionStack::endOfAnimation" <<collDBName; //###########
      this->collModel->readFromDB(dbName, this->collModel->getActualBranch());
    }
}

/****************************************************************************/
