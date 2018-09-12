/*****************************************************************************
 * kbvCollectionTabs
 * This is the tab widget for file, search, album and collection tabs
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2011.10.11
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvMainMenu.h"
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvPluginInterfaces.h"
#include "kbvCollectionTabs.h"
#include "kbvCsvChecker.h"

extern  KbvMainMenu               *mainMenu;
extern  KbvSetvalues              *settings;
extern  KbvInformationDialog      *informationDialog;
extern  KbvIptcExifXmpInterface   *metadataPlugin;
extern  KbvImageEditorInterface   *imageEditPlugin;

KbvCollectionTabs::KbvCollectionTabs(QWidget *parent) : QTabWidget(parent)
{
  messageNoConnect = QString(tr("Can't open database.\n"));
  messageNoConnectReason = QString(tr("Possible reasons:\nUnsufficient access rights.\n"
                                      "File is not a SQLite 3 database or corrupt."));

  //Tab bar with mouse double click event
  collTabBar = new KbvTabBar();
  this->setTabBar(collTabBar);

  //Note:
  //Tabs of fileView and searchView always are addressed by index
  //Tabs of albums/collections always are addressed by their title
  //Create file list view and model
  fileViewTab = new KbvFileTab(this);

  //Create search result tab (searchView, sortModel, searchModel and searchThread)
  searchViewTab = new KbvSearchTab(this);

  //insert file view tab and search view tab as first tabs
  this->insertTab(KbvConf::fileViewTabIndex,   fileViewTab,   QIcon(":/kbv/icons/eye.png"), "");
  this->insertTab(KbvConf::searchViewTabIndex, searchViewTab, QIcon(":/kbv/icons/folder-image.png"), "");
  
  //Manage drag'n'drop, set info labels
  dndHandler = new KbvCollectionDragDrop(nullptr);
  connect(dndHandler,             SIGNAL(infoText2(QString)), fileViewTab,  SLOT(setInfoText2(QString)));
  connect(dndHandler,             SIGNAL(infoText2(QString)), searchViewTab,SLOT(setInfoText2(QString)));
  connect(fileViewTab->fileModel, SIGNAL(infoText2(QString)), searchViewTab,SLOT(setInfoText2(QString)));
  
  connect(fileViewTab,    SIGNAL(imageEdit()),        this, SLOT(openImageEditor()));
  connect(fileViewTab,    SIGNAL(imageBatchEdit()),   this, SLOT(openBatchEditor()));
  connect(fileViewTab,    SIGNAL(imageMetadata()),    this, SLOT(openMetaDataEditor()));
  connect(searchViewTab,  SIGNAL(imageEdit()),        this, SLOT(openImageEditor()));
  connect(searchViewTab,  SIGNAL(imageBatchEdit()),   this, SLOT(openBatchEditor()));
  connect(searchViewTab,  SIGNAL(imageMetadata()),    this, SLOT(openMetaDataEditor()));

  //Plugin libKbvMetadata for image meta data IPTC, Exif, XMP
  //Plugin libKbvImageEditor for image manipulation
  connect(mainMenu, SIGNAL(menuMetadata()),     this, SLOT(openMetaDataEditor()));
  connect(mainMenu, SIGNAL(menuIptcTemplate()), this, SLOT(openIptcTemplate()));
  connect(mainMenu, SIGNAL(menuImageEdit()),    this, SLOT(openImageEditor()));
  connect(mainMenu, SIGNAL(menuBatchEdit()),    this, SLOT(openBatchEditor()));

  //connect all remaining members
  connect(mainMenu, SIGNAL(menuCopyCutPaste(QKeyEvent*)), fileViewTab->fileView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(mainMenu, SIGNAL(menuRename()),                 fileViewTab->fileView, SLOT(renameFromMenu()));
  connect(mainMenu, SIGNAL(menuShowSlides()),             fileViewTab->fileView, SLOT(displaySlideShow()));
  connect(this,     SIGNAL(settingsChanged()),            fileViewTab->fileView, SLOT(updateOptions()));
  connect(this,     SIGNAL(currentChanged(int)),          fileViewTab->fileView, SLOT(currentTabIndex(int)));
  connect(this,     SIGNAL(ctrlAKeyAction(QKeyEvent*)),   fileViewTab->fileView, SLOT(keyPressEvent(QKeyEvent*)));

  connect(mainMenu, SIGNAL(menuCopyCutPaste(QKeyEvent*)), searchViewTab->searchView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(mainMenu, SIGNAL(menuRename()),                 searchViewTab->searchView, SLOT(renameFromMenu()));
  connect(mainMenu, SIGNAL(menuShowSlides()),             searchViewTab->searchView, SLOT(displaySlideShow()));
  connect(mainMenu, SIGNAL(menuFindClear()),              searchViewTab, SLOT(searchClear()));
  connect(mainMenu, SIGNAL(menuDBInfo()),                 searchViewTab, SLOT(showDBInfo()));
  connect(mainMenu, SIGNAL(menuRecordInfo()),             searchViewTab, SLOT(showRecordInfo()));
  connect(this,     SIGNAL(currentChanged(int)),          searchViewTab, SLOT(currentTabIndex(int)));
  connect(this,     SIGNAL(ctrlAKeyAction(QKeyEvent*)),   searchViewTab->searchView, SLOT(keyPressEvent(QKeyEvent*)));

  connect(mainMenu, SIGNAL(menuFind()),           this,  SLOT(searchCollection()));
  connect(mainMenu, SIGNAL(menuCsvCheck()),       this,  SLOT(checkCollection()));
  connect(this,     SIGNAL(currentChanged(int)),  this,  SLOT(currentTabTitle(int)));

  //close tab on double click
  connect(collTabBar, SIGNAL(closeTab(int)),     this, SLOT(closeCollectionTab(int)));

  //activate file view: exact result only on deactivate/activate!!!
  this->setCurrentIndex(KbvConf::searchViewTabIndex);
  this->setCurrentIndex(KbvConf::fileViewTabIndex);
}

KbvCollectionTabs::~KbvCollectionTabs()
{
  //qDebug() << "KbvCollectionTabs::~KbvCollectionTabs";//###########
  delete  dndHandler;
  while(this->count() > 0)
    {
      delete this->widget(0); //fileViewTab, searchViewTab, all collTabs
    }
}

/*************************************************************************//*!
 * SLOT: Open IPTC template in static plugin libImageMetadata.\n
 * When main menu Image|IPTC template is selected: open template editor.
 */
void  KbvCollectionTabs::openIptcTemplate()
{
  //qDebug() << "KbvCollectionTabswidget::openIptcTemplate";//###########
  if(metadataPlugin)
    {
      metadataPlugin->openIptcTemplate();
    }
}
/*************************************************************************//*!
 * SLOT: Open meta data editor in static plugin libImageMetadata.\n
 * When main menu Image|Meta Data is selected: open image editor with
 * selected files.
 */
void  KbvCollectionTabs::openMetaDataEditor()
{
  QList<QPair<QString, QString> >   files;

  this->selectedFilesList(files);
  //qDebug() << "KbvCollectionTabswidget::openIptcEditor"<<i <<filelist;//###########
  if(metadataPlugin)
    {
      metadataPlugin->openEditor(files);
    }     
}
/*************************************************************************//*!
 * SLOT: Open image editor in static plugin libKbvImageEditor with selected
 * files when main menu Image|Edit is selected or F7 pressed.
 */
void  KbvCollectionTabs::openImageEditor()
{
  QList<QPair<QString, QString> >   files;

  this->selectedFilesList(files);
  //qDebug() << "KbvCollectionTabs::openImageEditor" <<files; //###########
  if(imageEditPlugin)
    {
      imageEditPlugin->openEditor(files);
    }
}
/*************************************************************************//*!
 * SLOT: Open batch editor in static plugin libKbvImageEditor with selected
 * files when main menu Image|Batch is activated.
 */
void  KbvCollectionTabs::openBatchEditor()
{
  QList<QPair<QString, QString> >   files;

  this->selectedFilesList(files);
  //qDebug() << "KbvCollectionTabs::openBatchEditor" <<files; //###########
  if(imageEditPlugin)
    {
      imageEditPlugin->openBatchEditor(files);
    }
}
/*************************************************************************//*!
 * Collect file names from selected indicees in fileView, searchView or collView
 * depending on which view is the current one.
 */
void  KbvCollectionTabs::selectedFilesList(QList<QPair<QString, QString> > &files)
{
  QPair<QString, QString>         file;
  QModelIndexList                 selIndices;

  int i = this->currentIndex();
  if(i == KbvConf::fileViewTabIndex)
    {
      selIndices = this->fileViewTab->fileView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              file.first  = this->fileViewTab->fileView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              file.second = this->fileViewTab->fileView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              files.append(file);
            }
        }
    }
  if(i == KbvConf::searchViewTabIndex)
    {
      selIndices = this->searchViewTab->searchView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              file.first  = this->searchViewTab->searchView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              file.second = this->searchViewTab->searchView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              files.append(file);
            }
        }
    }
  if(i >= KbvConf::collectionViewTabIndex)
    {
      KbvCollectionStack *collTab = qobject_cast<KbvCollectionStack*>(this->widget(i));
      selIndices = collTab->collView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              file.first  = collTab->collView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              file.second = collTab->collView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              files.append(file);
            }
        }
    }
}
/*************************************************************************//*!
 * SLOT: switches file view pane depending on tab of left splitter area\n
 * dir tree tab is active -> file view tab is needed
 */
void  KbvCollectionTabs::activateTab(int index)
{
  //qDebug() << "KbvCollectionTabs::activateTab" << index;//###########
  switch (index)
  {
    case (KbvConf::DirTreeTab):
      {
        this->setCurrentIndex(KbvConf::fileViewTabIndex);
        break;
      }
  }
}
/*************************************************************************//*!
 * SLOT: Inform the collection tabs about which of them is the active tab.
 * Triggerd by signal QTabWidget::currentChanged(int index)
 */
void    KbvCollectionTabs::currentTabTitle(int index)
{
  QString title;

  title = this->tabText(index);
  //qDebug() << "KbvCollectionTabs::currentTabTitle" << index << title; //###########
  emit  currentTitle(title);
}
/************************************************************************//*!
 * The scheduler decides where the drop has to be performed:
 * When drop on a collection view happens the drop event calls directly the
 * dndHandler of collectionTabs since the target db and branch are visible and
 * mime data can be added to the related dir, database and model. Hence, when
 * the drop has finished the collection view displays the actual state.
 * When the drop takes place on the collectionTreeView we must consider two
 * cases:
 * The target is an open album: visible = true
 * The target is an open collection and the drop path is shown: visible = true
 * Only the case of an open database requires a pointer for update of the model.
 * In all cases the dropped files must be added/moved to the dir and to the DB.
 */
void    KbvCollectionTabs::dropDataScheduler(const QMimeData *mimeData, Qt::DropAction action,
                                            QString path, KbvDBInfo *info, bool visible)
{
  KbvCollectionStack  *collTab = 0;
  KbvCollectionModel  *model = 0;
  int                 index, tabCount;
  bool                open;

  //check for tab index of collection/album
  open = false;
  tabCount = this->count();
  for(index=2; index<tabCount; index++)
    {
      if(this->tabText(index) == info->getName())
        {
          open = true;
          break;
        }
    }

  //qDebug() << "KbvCollectionTabs::dropDataScheduler tab no." <<index <<info->getName() <<path <<open <<visible;//###########
  if(open)
    {
      collTab = qobject_cast<KbvCollectionStack*>(this->widget(index));
      model = collTab->collModel;
    }
  //qDebug() << "KbvCollectionTabs::dropDataScheduler" <<model;//###########

  if(visible)
    {
      this->dndHandler->dropMimeData(mimeData, action, path, info, model);
    }
  else
    {
      this->dndHandler->dropMimeData(mimeData, action, path, info, 0);
    }
}
/*************************************************************************//*!
 * SLOT: Called by main menu or ctrl+F.
 * Prepare a list of names of open databases for searchView. The current
 * database is the first entry. The search tab will be set active.
 * Display a message when no database is open.
 */
void    KbvCollectionTabs::searchCollection()
{
  KbvCollectionStack  *collView;
  KbvDBInfo           *dbInfo;
  int                 current, count;

  //qDebug() << "KbvCollectionTabs::searchCollection";//###########

  //Consider only tab counts > 1, since 0=fileView and 1=searchView
  count = this->count();

  if(count > 2)
    {
      current = this->currentIndex();
      //When no collection is visible, use the first one
      if(current < 2)
        {
          current = 2;
        }
      //get a pointer to the database info of collection view tab
      collView = qobject_cast<KbvCollectionStack*>(this->widget(current));
      dbInfo = collView->getDatabaseInfo();

      //Start search in searchView--
      if(this->searchViewTab->search(dbInfo))
        {
          this->setCurrentIndex(KbvConf::searchViewTabIndex);
        }
    }
  else
    {
      informationDialog->perform(QString(tr("No database open")),
                          QString(tr("Please open a database.")), 0);
    }
}
/*************************************************************************//*!
 * SLOT: Called by main.
 * Get a pointer and show the CSV checker.
 * Display a message when no database is selected.
 */
void    KbvCollectionTabs::checkCollection()
{
  KbvCollectionStack  *collStack = 0;
  int                 current;

  //Consider only indices > 1, since 0=fileView and 1=searchView
  current = this->currentIndex();
  //qDebug() << "KbvCollectionTabs::checkCollection" <<this->tabText(current); //###########

  if(current >= 2)
    {
      //start checker
      collStack = qobject_cast<KbvCollectionStack*>(this->currentWidget());
      collStack->csvChecker->show();
    }
  else
    {
      informationDialog->perform(QString(tr("No database open")),
                          QString(tr("Please open a database.")), 0);
    }
}
/*************************************************************************//*!
 * SLOT: adds a collection/album tab "title" or displays a collection branch
 * (called by collection tree view).
 * When the tab is already available nothing is added but the tab becomes
 * active. In case of a collection we have to read the selected branch since
 * this may have changed.
 * When the database is no SQLite3 type or corrupt nothing will be added.
 * The parameter "type" is used only to determine the appropriate tab icon.
 */
void    KbvCollectionTabs::addCollectionTab(QString title, QString branch, int type, QString location, QString rootDir)
{
  int       i, tabCount;
  
  //TODO: Einmalige große Verzögerung beim Programmstart
  QTime t;    //====================
  t.start();  //====================

  
  //Check for already existing tab (album or collection). The generated signal
  //has no effect on albums. Ignore: tab 0 = file view, tab 1 = search view
  tabCount = this->count();
  for(i=2; i<tabCount; i++)
    {
      if (this->tabText(i) == title)
        {
          //qDebug() << "KbvCollectionTabs::addCollectionTab exists" << title << branch << i;//###########
          //Tab already exists, branch selected in collection tree
          //Set affected tab visible and read selected branch
          this->setCurrentIndex(i);           //emits currentChanged -> currentTitle
          emit dbReadBranch(title, branch);
          return;
        }
    }

  //add new collection tab ------------------------------------------------------------
  //verify database, create tab and populate
  if(!checkForSQLiteDB(location + title + QString(dbNameExt)))
    {
      informationDialog->perform(messageNoConnect, messageNoConnectReason, 1);
      return;
    }

  //Each collection tab contains a stacked widget with collection view at
  //index 0 and busy animation at index 1; Album tabs only contain the view.
  collViewTab = new KbvCollectionStack(this, location, title, rootDir, type, dndHandler);
  if(collViewTab->failed)
    {
      //qDebug() << "KbvCollectionTabs::addCollectionTab version";//###########
      delete collViewTab;
      return;
    }

  //qDebug() << "KbvCollectionTabs::addCollectionTab stack" <<t.elapsed(); //====================
  //Set all connections so all is working when we add the new tab widget
  connect(this, SIGNAL(settingsChanged()),      collViewTab->collView, SLOT(updateOptions()));
  //from main menu
  connect(mainMenu, SIGNAL(menuCopyCutPaste(QKeyEvent*)), collViewTab->collView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(mainMenu, SIGNAL(menuRename()),       collViewTab->collView,  SLOT(renameFromMenu()));
  connect(mainMenu, SIGNAL(menuShowSlides()),   collViewTab->collView,  SLOT(displaySlideShow()));
  connect(mainMenu, SIGNAL(menuDBInfo()),       collViewTab,            SLOT(showDBInfo()));
  connect(mainMenu, SIGNAL(menuRecordInfo()),   collViewTab,            SLOT(showRecordInfo()));

  connect(this,     SIGNAL(ctrlAKeyAction(QKeyEvent*)), collViewTab->collView, SLOT(keyPressEvent(QKeyEvent*)));
  
  //from collection tree view
  connect(this, SIGNAL(dbReadBranch(QString, QString)), collViewTab->collView,  SLOT(setActualBranch(QString, QString)));
  connect(this, SIGNAL(dbReadBranch(QString, QString)), collViewTab->collModel, SLOT(readFromDB(QString, QString)));
  connect(this, SIGNAL(renameDBBranch(QString)),        collViewTab->collModel, SLOT(renameDBBranch(QString)));

  //show progress of drag'n'drop
  connect(dndHandler, SIGNAL(infoText1(QString)),       collViewTab, SLOT(setInfoText1(QString)));
  connect(dndHandler, SIGNAL(infoText2(QString)),       collViewTab, SLOT(setInfoText2(QString)));
  connect(dndHandler, SIGNAL(infoText3(QString)),       collViewTab, SLOT(setInfoText3(QString)));

  connect(this,        SIGNAL(currentTitle(QString)),   collViewTab, SLOT(currentTabTitle(QString)));
  connect(collViewTab, SIGNAL(search()),                this, SLOT(searchCollection()));
  connect(collViewTab, SIGNAL(imageEdit()),             this, SLOT(openImageEditor()));
  connect(collViewTab, SIGNAL(imageBatchEdit()),        this, SLOT(openBatchEditor()));
  connect(collViewTab, SIGNAL(imageMetadata()),         this, SLOT(openMetaDataEditor()));
  
  //connection to imageEditor possible at this point
  if(imageEditPlugin)
    {
      connect(imageEditPlugin->getObject(), SIGNAL(filesAltered(const QString)), collViewTab->collModel, SLOT(endOfReading()));
    }

  //create tab here so we catch the signal "currentChanged" and display the new tab
  if(type & Kbv::TypeAlbum)
    {
      i = this->addTab(collViewTab, QIcon(":/kbv/icons/di.png"), title);
    }
  if((type & Kbv::TypeCollection) || (type & Kbv::TypeCollectionRoot))
    {
      i = this->addTab(collViewTab, QIcon(":/kbv/icons/bookcase.png"), title);
    }
  
  //qDebug() << "KbvCollectionTabs::addCollectionTab tab" <<t.elapsed(); //====================
  
  //set tab active (and trigger signal currentChanged(int) which produces
  //signal currentTitle(QString) for the tabWidget, then read the database
  this->setCurrentIndex(i);
  emit dbReadBranch(title, branch);
}
/*************************************************************************//*!
 * SLOT: removes a collection/album tab when this tab is present.
 * Called by collection tree view when a collection has been removed.
 */
void    KbvCollectionTabs::removeCollectionTab(QString title)
{
  int   i, tabCount;

  //qDebug() << "KbvCollectionTabs::removeCollectionTab" << title; //###########
  //check if tab is existing and remove; tab 0=file view, tab 1=search view
  tabCount = this->count();
  for(i=2; i<tabCount; i++)
    {
      if (this->tabText(i) == title)
        {
          this->closeCollectionTab(i);
        }
    }
}
/*************************************************************************//*!
 * SLOT: removes a collection/album tab on double click. Called by tab bar.
 * All signals to/from this tab get disconnected by Qt, the tab content gets
 * deleted, the database closed and the tab removed.
 * Use deleteLater() to prevent crashes.
 * A signal is emitted for collectionTreeView to collapse the collection tree.
 */
void    KbvCollectionTabs::closeCollectionTab(int index)
{
  KbvCollectionStack  *tabWidget;
  
  emit tabClosed(this->tabText(index)); //collapse collection tree

  tabWidget = qobject_cast<KbvCollectionStack*>(this->widget(index));

  //qDebug() << "KbvCollectionTabs::closeCollectionTab" <<index; //###########
  tabWidget->deleteLater();
}
/*************************************************************************//*!
 * SLOT:Called by collection tree view.
 * Import collection or update the database. When the tab is found it becomes
 * active. When there is no tab the database get added.
 */
void    KbvCollectionTabs::readinOrUpdate(const QString &title, const QString &path, int type, const QString &location, const QString &root)
{
  int   i, tabCount;
  bool  found=false;
  KbvCollectionStack  *collTab = 0;

  //qDebug() << "KbvCollectionTabs::readinOrUpdate title type" <<title <<path <<type; //###########
  //check for already existing tab, ignore tab 0=file view, tab 1=search view
  tabCount = this->count();
  for(i=2; i<tabCount; i++)
    {
      if (this->tabText(i) == title)
        {
          found = true;
          break;
        }
    }
  if(found)
    {
      //set active and visible
      //qDebug() << "KbvCollectionTabs::readinOrUpdate index" <<i; //###########
      this->setCurrentIndex(i);
    }
  else
    {
      //add database tab
      //qDebug() << "KbvCollectionTabs::readinOrUpdate add tab" << title; //###########
      this->addCollectionTab(title, "", type, location, root);
    }
  collTab = qobject_cast<KbvCollectionStack*>(this->currentWidget());
  collTab->showAnimation(title, true);
  collTab->collModel->readinOrUpdateDB(path);
}

/*************************************************************************//*!
 * Called by drop on collection tree. Check if the database (the drop target)
 * is already open.
 */
bool    KbvCollectionTabs::isDBOpen(const QString name)
{
  int   i, tabCount;

  //check for already existing tab
  //ignore tab 0=file view, tab 1=search view
  tabCount = this->count();
  for(i=2; i<tabCount; i++)
    {
      if (this->tabText(i) == name)
        {
          return true;
        }
    }
  return false;
}
/*************************************************************************//*!
 * SLOT: renames an album tab (called by collection tree view) when this tab
 * is displayed. The related database has already been renamed.
 * The parameter oldnew contains old and new title separated by new line (\n).
 */
void    KbvCollectionTabs::renameCollectionTab(QString oldnew)
{
  int       i, tabCount, pos;
  QString   oldTitle, newTitle;

  pos = oldnew.indexOf("\n", 0);
  oldTitle = oldnew.left(pos);
  newTitle = oldnew.remove(0, pos+1);
  //check if tab is existing and rename; tab 0=file view, tab 1=search view
  tabCount = this->count();
  for(i=2; i<tabCount; i++)
    {
      if (this->tabText(i) == oldTitle)
        {
          this->setTabText(i, newTitle);
        }
    }
}
/*************************************************************************//*!
 * Check for SQLite3 data base. This can be verified reading the first 20
 * characters of the file and look for the string "SQLite format 3".
 * Needs the complete file path.
 */
bool    KbvCollectionTabs::checkForSQLiteDB(const QString &name)
{
  QFile         dbFile;
  QTextStream   in;
  QString       content;

  dbFile.setFileName(name);
  if (!dbFile.open(QIODevice::ReadOnly))
    {
      return false;
    }
  in.setDevice(&dbFile);
  content = in.read(20);  //only 15 chars needed.
  if (!content.startsWith("SQLite format 3"))
    {
      dbFile.close();
      return false;
    }
  dbFile.close();
  return true;
}
/****************************************************************************/
