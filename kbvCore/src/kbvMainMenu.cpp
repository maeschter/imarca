/*****************************************************************************
 * kvb main menu
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.09.22
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvMainMenu.h"

KbvMainMenu::KbvMainMenu(QWidget *parent) : QMenuBar(parent)
{
  //Events for copy-cut-paste
  eventCopy  = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier, QString("c"), false, 1);
  eventCut   = new QKeyEvent(QEvent::KeyPress, Qt::Key_X, Qt::ControlModifier, QString("x"), false, 1);
  eventPaste = new QKeyEvent(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier, QString("v"), false, 1);
  
  //Menu File
  actRenameDir = new QAction(tr("Rename Directory"), this);
  actRenameDir->setToolTip(tr("Rename the selected directory"));
  actInsertDir = new QAction(tr("Insert Directory"), this);
  actInsertDir->setToolTip(tr("Create a new subdirectory of the selected directory"));
  actRemoveDir = new QAction(tr("Remove Directory"), this);
  actRemoveDir->setToolTip(tr("Warning: the directory and all contents get deleted from file system"));
  actQuit = new QAction(QIcon(":/kbv/icons/app-exit.png"), tr("Quit"), this);
  actQuit->setShortcut(QKeySequence(tr("Ctrl+Q")));
  actQuit->setToolTip("Quit Imarca");
  actQuit->setIconVisibleInMenu(true);

  menuFile = new QMenu(tr("&File"), nullptr);
  menuFile->setToolTipsVisible(true);
  menuFile->addAction(actRenameDir);
  menuFile->addAction(actInsertDir);
  menuFile->addAction(actRemoveDir);
  menuFile->addSeparator();
  menuFile->addAction(actQuit);

  //Menu Edit
  actRename = new QAction(QIcon(":/kbv/icons/file-rename.png"), tr("Rename"), this);
  actRename->setShortcut(QKeySequence("F2"));
  actRename->setIconVisibleInMenu(true);
  actFind = new QAction(QIcon(":/kbv/icons/kbv_find_24x24.png"), tr("Find"), this);
  actFind->setShortcut(QKeySequence(tr("Ctrl+F")));
  actFind->setIconVisibleInMenu(true);
  actFindClear = new QAction(QIcon(":/kbv/icons/edit-clear.png"), tr("Clear search result"), this);
  actFindClear->setShortcut(QKeySequence(tr("Ctrl+Shift+F")));
  actFindClear->setIconVisibleInMenu(true);
  actCopy = new QAction(QIcon(":/kbv/icons/edit-copy.png"), tr("copy"), this);
  actCopy->setShortcut(QKeySequence(tr("Ctrl+C")));
  actCopy->setIconVisibleInMenu(true);
  actCut = new QAction(QIcon(":/kbv/icons/edit-cut.png"), tr("cut"), this);
  actCut->setShortcut(QKeySequence(tr("Ctrl+X")));
  actCut->setIconVisibleInMenu(true);
  actPaste = new QAction(QIcon(":/kbv/icons/edit-paste.png"), tr("paste"), this);
  actPaste->setShortcut(QKeySequence(tr("Ctrl+V")));
  actPaste->setIconVisibleInMenu(true);
  actOptions = new QAction(QIcon(":/kbv/icons/app-system.png"), tr("&Options"), this);
  actOptions->setIconVisibleInMenu(true);

  menuEdit = new QMenu(tr("&Edit"), nullptr);
  menuEdit->addAction(actCopy);
  menuEdit->addAction(actCut);
  menuEdit->addAction(actPaste);
  menuEdit->addAction(actRename);
  menuEdit->addSeparator();
  menuEdit->addAction(actFind);
  menuEdit->addAction(actFindClear);
  menuEdit->addSeparator();
  menuEdit->addAction(actOptions);

  //Menu Collection
  actDBInfo = new QAction(tr("Database info"), this);
  actDBInfo->setToolTip(tr("Show settings of the database."));
  actRecordInfo = new QAction(tr("Data set content"), this);
  actRecordInfo->setToolTip(tr("Show content of the data set."));
  actDBCreate = new QAction(tr("New database"), this);
  actDBCreate->setToolTip(tr("Create a new empty database."));
  actDBRemove = new QAction(tr("Remove collection from tree"), this);
  actDBRemove->setToolTip(tr("Removes the selected album or collection from tree.\nThe database file and collection files will not be deleted."));

  actOpenColl = new QAction(QIcon(":/kbv/icons/database-add.png"), tr("Open Collection"), this);
  actOpenColl->setToolTip(tr("Open collection on an arbitrary device"));
  actImportColl = new QAction(QIcon(":/kbv/icons/database-import.png"), tr("Import Collection"), this);
  actImportColl->setToolTip(tr("Import a collection to the local file system from an arbitrary device"));
  actExportColl = new QAction(QIcon(":/kbv/icons/database-export.png"), tr("Export Collection"), this);
  actExportColl->setToolTip(tr("Export a collection to an arbitrary device"));
  
  actDBRead = new QAction(tr("Read"), this);
  actDBRead->setToolTip(tr("Read a collection from file system into database."));
  actDBUpdate = new QAction(tr("Update"), this);
  actDBUpdate->setToolTip(tr("The collection or branch will be scanned and the database updated."));
  actCsvCheck = new QAction(tr("CSV check"), this);
  actCsvCheck->setToolTip(tr("The collection files will be checked by a CSV file."));
  actDBRename = new QAction(tr("Rename"), this);
  actDBRename->setToolTip(tr("Rename the selected collection or branch."));
  actDBInsert = new QAction(tr("Insert branch"), this);
  actDBInsert->setToolTip(tr("Insert a new empty collection branch below the selected item."));
  actDBDelete = new QAction(tr("Delete branch"), this);
  actDBDelete->setToolTip(tr("Delete the selected collection branch. The branch must be empty."));

  menuCollection = new QMenu(tr("&Collection"), nullptr);
  menuCollection->setToolTipsVisible(true);
  menuCollection->addAction(actOpenColl);
  menuCollection->addAction(actImportColl);
  menuCollection->addAction(actExportColl);
  menuCollection->addSeparator();
  menuCollection->addAction(actDBInfo);
  menuCollection->addAction(actRecordInfo);
  menuCollection->addAction(actDBCreate);
  menuCollection->addAction(actDBRemove);
  menuCollection->addSeparator();
  menuCollection->addAction(actDBRead);
  menuCollection->addAction(actDBUpdate);
  menuCollection->addSeparator();
  menuCollection->addAction(actCsvCheck);
  menuCollection->addSeparator();
  menuCollection->addAction(actDBRename);
  menuCollection->addAction(actDBInsert);
  menuCollection->addSeparator();
  menuCollection->addAction(actDBDelete);
  menuCollection->setEnabled(false);

  //Menu Image
  actImageEdit = new QAction(tr("Edit"), this);
  actImageEdit->setToolTip(tr("Open the image editor."));
  actImageEdit->setShortcut(QKeySequence("F7"));
  actImageEdit->setVisible(false);
  actBatchEdit = new QAction(tr("Batch process"), this);
  actBatchEdit->setToolTip(tr("Batch processing of multiple images."));
  actBatchEdit->setShortcut(QKeySequence("F10"));
  actBatchEdit->setVisible(false);
  actSlideshow = new QAction(tr("Show"), this);
  actSlideshow->setToolTip(tr("Open slide show"));
  actSlideshow->setShortcut(QKeySequence("F8"));
  actMetadata  = new QAction(tr("&Meta data"), this);
  actMetadata->setToolTip(tr("Open the editor for image meta data."));
  actMetadata->setShortcut(QKeySequence("F6"));
  actMetadata->setVisible(false);
  actIptcTemplate = new QAction(tr("IPTC &template"), this);
  actIptcTemplate->setToolTip(tr("Create a new or edit an existing template for IPTC meta data."));
  actIptcTemplate->setVisible(false);

  menuImage = new QMenu(tr("&Image"), nullptr);
  menuImage->setToolTipsVisible(true);
  menuImage->addAction(actImageEdit);
  menuImage->addAction(actBatchEdit);
  menuImage->addAction(actSlideshow);
  menuImage->addSeparator();
  menuImage->addAction(actMetadata);
  menuImage->addAction(actIptcTemplate);

  //Menu Help
  actContent = new QAction(QIcon(":/kbv/icons/help-contents.png"), tr("Content"), this);
  actContent->setShortcut(QKeySequence("F1"));
  actContent->setIconVisibleInMenu(true);
  actInfo = new QAction(QIcon(":/kbv/icons/help-about.png"), tr("&Info"), this);
  actInfo->setIconVisibleInMenu(true);

  menuHelp = new QMenu(tr("Help"), nullptr);
  menuHelp->addAction(actContent);
  menuHelp->addAction(actInfo);

  //Add to menu bar and store the menu pointers
  actFile  = this->addMenu(menuFile);
  actEdit  = this->addMenu(menuEdit);
  actColl  = this->addMenu(menuCollection);
  actImage = this->addMenu(menuImage);
  actHelp  = this->addMenu(menuHelp);

  //Signals
  connect(actRenameDir,  SIGNAL(triggered()), this, SIGNAL(menuRenameDir()));
  connect(actRemoveDir,  SIGNAL(triggered()), this, SIGNAL(menuRemoveDir()));
  connect(actInsertDir,  SIGNAL(triggered()), this, SIGNAL(menuInsertDir()));
  connect(actOpenColl,   SIGNAL(triggered()), this, SLOT  (mOpenColl()));
  connect(actImportColl, SIGNAL(triggered()), this, SIGNAL(menuImportColl()));
  connect(actExportColl, SIGNAL(triggered()), this, SIGNAL(menuExportColl()));
  connect(actQuit,       SIGNAL(triggered()), this, SIGNAL(menuQuit()));
  connect(actOptions,    SIGNAL(triggered()), this, SIGNAL(menuOptions()));
  connect(actCopy,       SIGNAL(triggered()), this, SLOT  (mCopy()));
  connect(actCut,        SIGNAL(triggered()), this, SLOT  (mCut()));
  connect(actPaste,      SIGNAL(triggered()), this, SLOT  (mPaste()));
  connect(actRename,     SIGNAL(triggered()), this, SIGNAL(menuRename()));
  connect(actFind,       SIGNAL(triggered()), this, SIGNAL(menuFind()));
  connect(actFindClear,  SIGNAL(triggered()), this, SIGNAL(menuFindClear()));
  connect(actDBInfo,     SIGNAL(triggered()), this, SIGNAL(menuDBInfo()));
  connect(actRecordInfo, SIGNAL(triggered()), this, SIGNAL(menuRecordInfo()));
  connect(actDBCreate,   SIGNAL(triggered()), this, SIGNAL(menuDBCreate()));
  connect(actDBRemove,   SIGNAL(triggered()), this, SIGNAL(menuDBRemove()));
  connect(actDBRead,     SIGNAL(triggered()), this, SIGNAL(menuDBRead()));
  connect(actDBUpdate,   SIGNAL(triggered()), this, SIGNAL(menuDBUpdate()));
  connect(actCsvCheck,   SIGNAL(triggered()), this, SIGNAL(menuCsvCheck()));
  connect(actDBRename,   SIGNAL(triggered()), this, SIGNAL(menuDBRename()));
  connect(actDBInsert,   SIGNAL(triggered()), this, SIGNAL(menuDBInsert()));
  connect(actDBDelete,   SIGNAL(triggered()), this, SIGNAL(menuDBDelete()));
  
  connect(actImageEdit,  SIGNAL(triggered()),  this, SIGNAL(menuImageEdit()));
  connect(actBatchEdit,  SIGNAL(triggered()),  this, SIGNAL(menuBatchEdit()));
  connect(actSlideshow,  SIGNAL(triggered()),  this, SIGNAL(menuShowSlides()));
  connect(actMetadata,   SIGNAL(triggered()),  this, SIGNAL(menuMetadata()));
  connect(actIptcTemplate, SIGNAL(triggered()),this, SIGNAL(menuIptcTemplate()));

  connect(actContent,    SIGNAL(triggered()),  this, SIGNAL(menuHelpContent()));
  connect(actInfo,       SIGNAL(triggered()),  this, SIGNAL(menuHelpInfo()));

  //The tool bars
  dbToolBar = new QToolBar("Database", nullptr);
  dbToolBar->addAction(actOpenColl);
  dbToolBar->addAction(actImportColl);
  dbToolBar->addAction(actExportColl);
  dbToolBar->setVisible(true);
  dbToolBar->setEnabled(false);
  
  fileToolBar = new QToolBar("File", nullptr);
  fileToolBar->addAction(actRename);
  fileToolBar->setVisible(false);

  editToolBar = new QToolBar("Edit", nullptr);
  editToolBar->addAction(actRename);
  editToolBar->addAction(actCopy);
  editToolBar->addAction(actCut);
  editToolBar->addAction(actPaste);
  editToolBar->addAction(actFind);
  editToolBar->addAction(actOptions);
  editToolBar->setVisible(true);

  quitToolBar = new QToolBar("Exit", nullptr);
  quitToolBar->addAction(actQuit);
  quitToolBar->setVisible(true);
}

KbvMainMenu::~KbvMainMenu()
{
  //qDebug() << "KbvMainMenu::~KbvMainMenu"; //###########
  delete  eventCopy;
  delete  eventCut;
  delete  eventPaste;
}
void  KbvMainMenu::setActionMetadataVisible(bool visible)
{
  actMetadata->setVisible(visible);
  actIptcTemplate->setVisible(visible);
}
void  KbvMainMenu::setActionImageEditVisible(bool visible)
{
  actImageEdit->setVisible(visible);
  actBatchEdit->setVisible(visible);
}

/*************************************************************************//*!
 * Change signal formats only.
 */
void  KbvMainMenu::mOpenColl()
{
  emit  menuOpenColl(QString(""));
}
void  KbvMainMenu::mCopy()
{
  emit  menuCopyCutPaste(eventCopy);
}
void  KbvMainMenu::mCut()
{
  emit  menuCopyCutPaste(eventCut);
}
void  KbvMainMenu::mPaste()
{
  emit  menuCopyCutPaste(eventPaste);
}

/****************************************************************************/
