/*****************************************************************************
 * kbvSearchTab
 * This is the widget for the search tab
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2013.01.24
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvSearchTab.h"
#include "kbvInformationDialog.h"

extern  KbvSetvalues            *settings;
extern  KbvInformationDialog    *informationDialog;

KbvSearchTab::KbvSearchTab(QWidget *parent) : KbvTabWidget(parent)
{
  searchDialog = new KbvDBSearchDialog(this);
  databaseInfo = new KbvDBInfo(this);

  //Create record info
  recordInfo = new KbvRecordInfo(this);
  
  //Create search view, model, sortmodel and thread)
  sortModel = new KbvSortFilterModel();
  searchModel = new KbvSearchModel();

  searchView = new KbvSearchView(this);
  searchView->setModels(sortModel, searchModel);
  searchView->setDatabaseInfo(databaseInfo);
  this->addWidget(searchView);

  //Pop up menu for search tab including signal connections
  popupMenu = new KbvCollectionPopUp(this, Kbv::TypeSearch);
  connect(popupMenu,    SIGNAL(copyPasteAction(QKeyEvent*)),  searchView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(popupMenu,    SIGNAL(searchClear()),           this, SLOT(searchClear()));
  connect(popupMenu,    SIGNAL(showDBAttributes()),      this, SLOT(showDBInfo()));
  connect(popupMenu,    SIGNAL(showRecordContent()),     this, SLOT(showRecordInfo()));
  connect(popupMenu,    SIGNAL(showEditor()),            this, SIGNAL(imageEdit()));
  connect(popupMenu,    SIGNAL(showBatchEditor()),       this, SIGNAL(imageBatchEdit()));
  connect(popupMenu,    SIGNAL(showMetadata()),          this, SIGNAL(imageMetadata()));

  connect(this,         SIGNAL(viewModeChanged(int)),    searchView, SLOT(changeViewMode(int)));
  connect(this,         SIGNAL(sortButtonClicked(int)),  sortModel, SLOT(sortButtonClicked(int)));
  connect(this,         SIGNAL(sortRoleChanged(int)),    sortModel, SLOT(sortRoleChanged(int)));
  connect(searchModel,  SIGNAL(enableSorting(bool)),     sortModel, SLOT(enableSorting(bool)));

  connect(searchModel,  SIGNAL(warning(QString, bool)),  this, SLOT(warnings(QString, bool)));
  connect(searchModel,  SIGNAL(infoText1(QString)),      this, SLOT(setInfoText1(QString)));
  connect(searchModel,  SIGNAL(infoText2(QString)),      this, SLOT(setInfoText2(QString)));
  connect(searchModel,  SIGNAL(infoText3(QString)),      this, SLOT(setInfoText3(QString)));
  connect(searchView,   SIGNAL(infoText2(QString)),      this, SLOT(setInfoText2(QString)));
  connect(searchView,   SIGNAL(infoText3(QString)),      this, SLOT(setInfoText3(QString)));
}

KbvSearchTab::~KbvSearchTab()
{
  //qDebug() << "KbvSearchTab::~KbvSearchTab"; //###########
  //sortModel and searchModel get deleted by searchView
  //remove database connection
  searchDB = QSqlDatabase();                  //destroy previous db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}
/*************************************************************************//*!
 * Slot: Called by collection tabs when the tabs change.
 * Set the visible flag.
 */
void    KbvSearchTab::currentTabIndex(int index)
{
  if(index == KbvConf::searchViewTabIndex)
    {
      weAreVisible = true;     
      searchView->setVisibleFlag(true);
   }
  else
    {
      weAreVisible = false;     
      searchView->setVisibleFlag(false);
    }
}
/*************************************************************************//*!
 * SLOT: Called by collectionTabs when menu search, F3 or popUp menu search
 * was activated. The info about calling database has been pased by parameter.
 * Establish a connection to the database and abort search if not successful.
 * Write database parameters into a local databaseInfo then set the database
 * to model and view. Update icon size and grid size. The search view always
 * displays results with database settings.
 * Clear the model before search. Parameters are passed in three lists:
 * keywordList: items 0-n: keywords
 * pathsList: items 0-n: paths
 * multilist: item 0: datetimeFrom as integer "JulianDay", zero when invalid
 *            item 1: datetimeTo as integer "JulianDay", zero when invalid
 *            item 2: catenation
 *            item 3: precision
 * The search itself gets processed in the searchThread.
 */
bool    KbvSearchTab::search(KbvDBInfo* dbInfo)
{
QStringList keywordList, pathsList;
QList<int>  multilist;
QString     name, location, msg;
QDate       dateFirst, dateLast;
int         result, catenation, precision;


  name = dbInfo->getName();
  location = dbInfo->getLocation();

  //qDebug() << "KbvSearchTab::search collection" <<name <<location; //###########

  result = searchDialog->perform(name);

  //open search dialog and get search parameters
  //then establish a connection to the database
  if(result == QDialog::Accepted)
    {
      keywordList = this->searchDialog->getKeywordList();
      pathsList = this->searchDialog->getPathsList();
      catenation = this->searchDialog->getKeywordLogic();
      precision = this->searchDialog->getSearchLogic();
      dateFirst = this->searchDialog->getDate(0);
      dateLast = this->searchDialog->getDate(1);
      //qDebug() << "KbvSearchTab::search collection" <<collection; //###########

      //1. step: close existing db connection and establish a new one
      //store db properties into a local databaseInfo. Set dbInfo to view and model.
      if((dbName != name) && (!name.isEmpty()))
        {
          dbName = name;
          dbLocation = location;
          //close existing db
          searchDB = QSqlDatabase();                  //destroy previous db-object
          QSqlDatabase::removeDatabase(dbconnection);  //close connection

          if(!this->setDBConnection(dbName, dbLocation))
            {
              msg = QString(tr("Cannot connect to database"));
              informationDialog->perform(msg, name, 1);
              return false;
            }
          collRoot = dbInfo->getRootDir();
          dbType = dbInfo->getType();
          dbVer = dbInfo->getVersion();
          dbIconSize = dbInfo->getIconSize();
          dbKeywordType = dbInfo->getKeyWordType();
          dbDescription = dbInfo->getDescription();
          databaseInfo->setDBInfo(dbType, dbIconSize, dbKeywordType, dbVer,
                                  dbName, dbDescription, dbLocation, collRoot);

          this->searchModel->setDatabase(searchDB, databaseInfo);
          this->searchView->setDatabaseInfo(databaseInfo);
          this->searchView->updateGridSize();
        }

      //2. step: prepare and perform search
      //string list keywordList: items 0-n: keywords
      //string list pathsList: items 0-n: paths
      //integer list multilist: search dates
      //item 0: datetimeFrom as integer "JulianDay", zero when invalid
      //item 1: datetimeTo as integer "JulianDay", zero when invalid
      if(dateFirst <= dateLast)
        {
          multilist.insert(0, dateFirst.toJulianDay());
          multilist.insert(1, dateLast.toJulianDay());
        }
      else
        {
          multilist <<0<<0;
        }
      multilist.insert(2, catenation);
      multilist.insert(3, precision);

      //start search and synchronise to the thread end
      //qDebug() << "KbvSearchTab::search keywordlist" << keywordList; //###########
      //qDebug() << "KbvSearchTab::search pathslist" << pathsList; //###########
      //qDebug() << "KbvSearchTab::search multilist" << multilist; //###########
      //qDebug() << "KbvSearchTab::search in" << collection; //###########
      this->searchModel->startThread(keywordList, pathsList, multilist, Kbv::search);
      return true;
    }
  return false;

}
/*************************************************************************//*!
 * Establish a connection to the database.
 */
bool  KbvSearchTab::setDBConnection(const QString name, const QString path)
{
  bool          success;

  //establish new connection
  dbconnection = "search"+dbName;
  searchDB = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
  searchDB.setHostName("host");
  searchDB.setDatabaseName(path+name+QString(dbNameExt));

  if(!searchDB.open())
    {
      //No connection possible, abort search
      searchDB = QSqlDatabase();
      QSqlDatabase::removeDatabase(dbconnection);
      success = false;
    }
  else
    {
      //db is open
      success = true;
    }
  return success;
}
/*************************************************************************//*!
 * Slot: Main menu or popup "Show database info" was activated. Display info
 * about database table description when this tab is visible, receive user
 * input of data field "comment" and update table "description".
 */
void    KbvSearchTab::showDBInfo()
{
  QString       description, stmt;
  QSqlQuery     query;
  int           result=0;
  
  stmt = QString("UPDATE OR REPLACE description SET comment = :comment");
  
  if(weAreVisible & searchModel->hasIndex(0,0,QModelIndex()))
    {
      result = this->databaseInfo->exec();
      
      if(result == QDialog::Accepted)
        {
          //write table 'description'
          searchDB.transaction();

          description = databaseInfo->getDescription(); //text or empty
          query = QSqlQuery(searchDB);
          query.prepare(stmt);
          query.bindValue(":comment", QVariant(description));
          query.exec();
          
          searchDB.commit();
        }
    }
}
/*************************************************************************//*!
 * SLOT: pop up menu of search stacked widget: show record data and edit
 * user keywords, user date and user comment in dialog KbvRecordInfo. The
 * check for tab visibility is mandatory since the call can start in popup
 * and main menu.
 * The recordInfo displays: name, path, imagesize, keywords, datechanged,
 * filesize, crc32, userkeywords, usercomment, userdate and icon.
 * User keywords, user comment and user date are editible. All communication
 * between recordInfo and collectionStack is done by signals.
 * All data are processed inside the dialog.
 * The dialog result always is of type rejected and requires no action.
 */
void    KbvSearchTab::showRecordInfo()
{
  QModelIndexList   recordIndices;
  QList<QVariant>   primaryKeys;

  if(weAreVisible)
    {
      //get selection or select all and map to model
      recordIndices = this->searchView->selectionModel()->selectedIndexes();
      if(recordIndices.size() > 0)
        {
          for(int i=0; i<recordIndices.size(); i++)
            {
              recordIndices[i] = this->sortModel->mapToSource(recordIndices[i]);
              primaryKeys.append(this->searchModel->data(recordIndices[i], Kbv::PrimaryKeyRole));
            }
          //qDebug() << "KbvStackedWidget::showRecordInfo selected" <<recordIndices.size(); //###########
          recordInfo->perform(primaryKeys, dbName, dbLocation);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Clear search result: File|Clear search result or Ctrl+F3 or pop up menu
 */
void    KbvSearchTab::searchClear()
{
  this->searchModel->clear();
  this->setInfoText1("");
  this->setInfoText2("");
  this->setInfoText3("");
}
/*************************************************************************//*!
 * Pop up menu for search tab.
 */
void    KbvSearchTab::contextMenuEvent(QContextMenuEvent *event)
{
  if(this->searchView->selectionModel()->hasSelection())
    {
      this->popupMenu->exec(event->globalPos(), true);
    }
  else
    {
      this->popupMenu->exec(event->globalPos(), false);
    }
}
/*************************************************************************//*!
 * SLOT: Warnings.
 */
void    KbvSearchTab::warnings(QString text, bool modal)
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
/****************************************************************************/
