/*****************************************************************************
 * kvb main window
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2008.11.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtGui>
#include <QtDebug>
#include <QPluginLoader>

#include "kbvConstants.h"
#include "kbvImageViewer.h"
#include "kbvMain.h"

//Globally provided methods
  KbvMainMenu               *mainMenu = nullptr;          //created in constructor
  KbvSetvalues              *settings = nullptr;
  KbvRenameDialog           *renameDialog = nullptr;
  KbvReplaceDialog          *replaceDialog = nullptr;
  KbvInformationDialog      *informationDialog = nullptr;
  KbvIptcExifXmpInterface   *metadataPlugin = nullptr;    //created in setupPlugin()
  KbvImageEditorInterface   *imageEditPlugin = nullptr;

  Q_IMPORT_PLUGIN(KbvIptcExifXmpPlugin)
  Q_IMPORT_PLUGIN(KbvImageEditorPlugin)

/*************************************************************************//*!
 * Create all widgets of main window, create rename dialog and options dialog
 * establish connections and read main window settings.
 */
KbvMain::KbvMain(QWidget *parent) : QMainWindow(parent=nullptr)
{
  KbvGeneral  generalFunc;

    //Valid Qt version?
    QString str1 = QString(qVersion());      //Installed version if any
    QString str2 = QString(QT_VERSION_STR);  //Version used for this release
    if (str1 < str2)
      {
        QString warnNoValidversion = QString(tr("Your Qt version is too old. "));
        warnNoValidversion.append(QString(tr("Installed version: %1\nRequired version: %2").arg(str1).arg(str2)));
        warnNoValidversion.append(tr("\nTo run the application please install at least Qt version %1 or later.").arg(str2));
        QMessageBox::critical(this, QString(appName) + " " + QString(appVersion),
                              warnNoValidversion, QMessageBox::NoButton, QMessageBox::Close);

        qDebug() << "Qt version too old"; //###########
      }
    qDebug() << "Qt version installed:" <<str1 <<"build:" <<str2; //###########

   // Attention: do not change order. !!!!!
    //This reads/stores the settings in ~/.config/imarca/imarca.conf (~/.config/organisation/application.conf)
    settings = new KbvSetvalues(QSettings::NativeFormat, QSettings::UserScope, "imarca", "imarca", this);

    //Main window
    QSizePolicy sizePolicy;
    sizePolicy.setHorizontalPolicy(QSizePolicy::Preferred);
    sizePolicy.setVerticalPolicy(QSizePolicy::Preferred);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setSizePolicy(sizePolicy);
    this->setMinimumSize(QSize(KbvConf::MainMimimumWidth, KbvConf::MainMimimumHeight));
    this->setWindowIcon(generalFunc.iconKbvCrown);
    this->setWindowTitle(QString(appName));
    this->setObjectName(QString(appMainWindow));

    //Dialogs
    renameDialog = new KbvRenameDialog(this);
    replaceDialog = new KbvReplaceDialog(this);
    informationDialog = new KbvInformationDialog(this);
    optionsDialog = new KbvOptions(this);
    helpDialog = new KbvHelpInfo(this);
    helpSystem = new KbvHelp(this);

    //Menus, tool bar, status bar
    mainMenuBar = new KbvMainMenu(this);
    this->setMenuBar(mainMenuBar);
    this->addToolBar(mainMenuBar->dbToolBar);
    this->addToolBar(mainMenuBar->fileToolBar);
    this->addToolBar(mainMenuBar->editToolBar);
    this->addToolBar(mainMenuBar->quitToolBar);
    mainMenu = mainMenuBar;

    //mainStatusBar = new QStatusBar(this);
    //this->setStatusBar(mainStatusBar);

    //Main window widgets - keep this order
    createCentralArea();

    //Apply settings
    readWindowSettings();

    //Set connections
    dirView->setFileModel(kbvTabsRight->fileViewTab->fileModel);

    QStringList   libInfo;
    globalFunc.getLibraryInfos(libInfo);
    helpDialog->setLibraryInfos(libInfo);


    //DirModel to FileModel and fileView and main window
    connect(dirModel, SIGNAL(dirActivated(const QString)),       kbvTabsRight->fileViewTab->fileModel, SLOT(startFillThread(const QString)));
    connect(dirModel, SIGNAL(expandedDirRenamed(const QString)), kbvTabsRight->fileViewTab->fileModel, SLOT(renamedDirUpdate(const QString)));

    //Collection tab -> enable collections menu
    connect(kbvTabsLeft,              SIGNAL(currentChanged(int)),      collTreeView, SLOT(enableMenu(int)));
    //Dir tab, collection tab, search tab -> bring view tab or result tab to front
    connect(kbvTabsLeft,              SIGNAL(currentChanged(int)),      kbvTabsRight, SLOT(activateTab(int)));
    //Clear selection in collTreeView whenever a collection tab is selected
    connect(kbvTabsRight->collTabBar, SIGNAL(keyOrMouseSelTab(int)),    collTreeView, SLOT(clearSelection()));
    connect(kbvTabsRight,             SIGNAL(tabClosed(const QString)), collTreeView, SLOT(collapseItem(const QString)));
    connect(kbvTabsRight->fileViewTab->fileView, SIGNAL(fileOpenColl(const QString)),  collTreeView, SLOT(databaseOpen(const QString)));

    //handle collection tabs and databases, kbvTabsRight gets an own pointer to the mainMenu
    connect(collTreeView,  SIGNAL(collItemActivated(const QString, const QString, int, const QString, const QString)),
            kbvTabsRight,  SLOT(addCollectionTab(const QString, const QString, int, const QString, const QString)));
    connect(collTreeView,  SIGNAL(collRemovedFromTree(const QString)),   kbvTabsRight, SLOT(removeCollectionTab(const QString)));
    connect(collTreeModel, SIGNAL(collRenamedInTree(const QString)),     kbvTabsRight, SLOT(renameCollectionTab(const QString)));
    connect(collTreeModel, SIGNAL(branchRenamedInTree(const QString)),   kbvTabsRight, SIGNAL(renameDBBranch(const QString)));
    connect(collTreeView,  SIGNAL(updateDB(const QString, const QString, int, const QString, const QString)),
            kbvTabsRight,  SLOT(readinOrUpdate(const QString, const QString, int, const QString, const QString)));
    //Application settings and options
    connect(settings,      SIGNAL(settingsChanged()), kbvTabsRight, SIGNAL(settingsChanged()));

    //Menu actions
    connect(mainMenuBar, SIGNAL(menuQuit()),        this, SLOT(close()));
    connect(mainMenuBar, SIGNAL(menuOptions()),     this, SLOT(editOptions()));
    connect(mainMenuBar, SIGNAL(menuHelpContent()), this, SLOT(helpContent()));
    connect(mainMenuBar, SIGNAL(menuHelpInfo()),    this, SLOT(helpInfo()));

    //Cmd line
    connect(this,  SIGNAL(cmdLineDatabase(QString)),   collTreeView, SLOT(databaseOpen(QString)));
    
    //Common key actions: the signals trigger signal "ctrlAKeyAction" in kbvTabsRight where
    //the visible one of fileView, searchView or collectionTabView selects all it's items 
    connect(this,               SIGNAL(keyCtrlAPressed(QKeyEvent*)), kbvTabsRight, SIGNAL(ctrlAKeyAction(QKeyEvent*)));
    connect(this->dirView,      SIGNAL(keyCtrlAPressed(QKeyEvent*)), kbvTabsRight, SIGNAL(ctrlAKeyAction(QKeyEvent*)));
    connect(this->collTreeView, SIGNAL(keyCtrlAPressed(QKeyEvent*)), kbvTabsRight, SIGNAL(ctrlAKeyAction(QKeyEvent*)));

    //install DBus interface
    dbusDeviceMonitor = new KbvDBusMonitor(nullptr);
    connect(dbusDeviceMonitor, SIGNAL(mountedDevice(QStringList)),    dirModel, SLOT(mountDevice(QStringList)));
    connect(dbusDeviceMonitor, SIGNAL(unmountedDevice(QStringList)),  dirModel, SLOT(unmountDevice(QStringList)));
    connect(dbusDeviceMonitor, SIGNAL(unmountedDevice(QStringList)),  collTreeView, SLOT(databaseUnmount(QStringList)));

    //get static and dynamic plugins in this order!
    //connect to signals since the target objects already exist (except collectionViewTab!)
    this->loadStaticPlugins();
    connect(imageEditPlugin->getObject(), SIGNAL(filesAltered(const QString)), kbvTabsRight->fileViewTab->fileModel, SLOT(startFileWatchThread(QString)));
    connect(imageEditPlugin->getObject(), SIGNAL(noBatchProcess(const bool)),  kbvTabsRight->fileViewTab->fileModel, SLOT(enableDirMonitor(bool)));
    connect(imageEditPlugin->getObject(), SIGNAL(filesAltered(const QString)), kbvTabsRight->searchViewTab->searchModel, SLOT(updateModel(QString)));

    this->loadDynamicPlugins();

    //expand directory view to last stored state (an empty stringlist forces reading of settings)
    this->dirModel->setTreeState(QStringList());

}
/*****************************************************************************
 * All objects of kbvMain get destroyed in the order of their creation after
 * the delete functions below. Dynamic plugins get destroyed at last.
*/
KbvMain::~KbvMain()
{
  //qDebug() << "KbvMain::~KbvMain"; //###########
  delete  dbusDeviceMonitor;    //parentless object
  delete  imageEditPlugin;      //static plugin
  delete  metadataPlugin;       //static plugin
}

/*************************************************************************//*!
 * Command line arguments.
 * The first one is the program name ("Imarca") including path. All following
 * are interpreted as dirs/files. Each argument must contain the full path.
 * The first path (argument 2) will expand the dir view to this directory
 * so its content gets displayed in file view.
 * The paths of args > 1 are collected in a list for slide view.
 */
void    KbvMain::cmdLineArgs()
{
  QList<QPair<QString, QString> > filelist;
  QPair<QString, QString>         pathname;
  QStringList   paths, args;
  QFileInfo     info;
  QDir          dir;
  int           n;


  //Command line parameters from application
  args = QApplication::arguments();

  //At least we need 2 arguments since the first one is the app name "Imarca"
  //qDebug() <<"Imarca app" <<args[0]; //###########
  if(args.length() > 1)
    {
      if(args[1].endsWith(QString(dbNameExt)))
        {
          //database 
          //open second cmd line arg as database and activate collection tab
          emit cmdLineDatabase(args[1]);
          kbvTabsLeft->setCurrentIndex(KbvConf::CollTreeTab); 
        }
      else
        {
          //no database
          //directory of second argument will be expanded in file view
          //following dirs are not considered
          info.setFile(args[1]);
          if(info.isFile())
            {
              //requires [path/]name.ext or [path/]* or [path/]*.*
              paths.append(args[1].left(args[1].lastIndexOf("/", -1, Qt::CaseInsensitive)));
            }
          else if(info.isDir())
            {
              //requires /home/user/[path] or ~/[path]
              if(args[1].endsWith("/"))
                {
                  args[1].remove(args[1].length()-1, 1);
                }
              paths.append(args[1]);
            }
          else
            {
              return;
            }
          //qDebug() << "Imarca expand" <<paths; //###########
          //expand directory tree to file path
          dirModel->setTreeState(paths);
    
          //open image viewer for slide show of files contained in cmd line args
          //or read the entire directory
          for(int i = 1; i < args.length(); ++i)
            {
              info.setFile(args[i]);
              if(info.isFile())
                {
                  n = args[i].lastIndexOf("/", -1, Qt::CaseInsensitive);
                  pathname.first  = args[i].left(n+1);
                  pathname.second = args[i].right(args[i].length()-n-1);
                  filelist.append(pathname);
                  //qDebug() << "KbvMain::cmdLineArgs" <<pathname.first+pathname.second; //###########
                }
              if(info.isDir())
                {
                  dir.setPath(args[i]);
                  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
                  dir.setSorting(QDir::Name | QDir::LocaleAware);
                  paths = dir.entryList();
                  for(int k=0; k<paths.length(); ++k)
                    {
                      pathname.first = args[i];
                      if(!args[i].endsWith("/"))
                        {
                          pathname.first.append("/");
                        }
                      pathname.second = paths.at(k);
                      filelist.append(pathname);
                    }
                }
              }
            }
          //qDebug() << "Imarca filelist" <<filelist; //###########
          //Show images. Memory is released when the show finishes.
          if(!filelist.isEmpty())
            {
              KbvImageViewer *imageViewer = new KbvImageViewer();
              connect(imageViewer, SIGNAL(deleteFiles(QMap<QString, QString>)),
                      kbvTabsRight->fileViewTab->fileModel, SLOT(removeFiles(QMap<QString, QString>)));
              imageViewer->showImages(filelist);
            }
    }
}

/*************************************************************************//*!
 * Read init values for kbvWindow.
 */
void    KbvMain::readWindowSettings()
{
  settings->beginGroup("App");
  settings->beginGroup("MainWindow");
  resize(settings->value("size", QSize(KbvConf::MainMimimumWidth, KbvConf::MainMimimumHeight)).toSize());
  move(settings->value("pos", QPoint(KbvConf::MainPosX, KbvConf::MainPosY)).toPoint());
  kbvSplitter->restoreState(settings->value("splittersize").toByteArray());
  settings->endGroup();
  settings->endGroup(); //group App
}
/*************************************************************************//*!
 * Write init values of kbvWindow.
 */
void    KbvMain::writeWindowSettings()
{
  settings->beginGroup("App");
  settings->beginGroup("MainWindow");
  settings->setValue("size", this->size());
  settings->setValue("pos", this->pos());
  settings->setValue("splittersize", this->kbvSplitter->saveState());
  settings->endGroup();
  settings->endGroup(); //group App
  settings->sync();
}
/*************************************************************************//*!
 * SLOT: Key press event
 * Ctrl A key pressed: inform the visible view to select all items
 */
void    KbvMain::keyPressEvent(QKeyEvent *event)
  {
    if((event->key() == Qt::Key_A) && (event->modifiers() == Qt::ControlModifier))
      {
        //qDebug() << "kbvMain::keyPressEvent ctrl a"; //###########
        emit  keyCtrlAPressed(event);
      }
    else
      {
        //Delegate key event to base class: tab key, cursor key navigation etc.
        QMainWindow::keyPressEvent(event);
      }
  }

/*************************************************************************//*!
 * Quit application but first save settings.
 */
void    KbvMain::closeEvent(QCloseEvent *event)
{
  this->dirModel->storeTreeState();
  this->collTreeModel->storeCollTreeState();
  this->writeWindowSettings();
  event->accept();        //action_quit
}

/*************************************************************************//*!
 *SLOT: menubar item File - Open <<<<<<<<<<<<<<< not used yet
 */
void    KbvMain::fileOpen()
  {
    qDebug() << "KbvMain::fileOpen"; //###########
  }

/*************************************************************************//*!
 *SLOT: menubar item Edit - Options
 */
void    KbvMain::editOptions()
  {
    optionsDialog->show();
  }

/*************************************************************************//*!
 *SLOT: menubar item Help - Content
 */
void    KbvMain::helpContent()
  {
    helpSystem->showHelpContent();
  }

/*************************************************************************//*!
 *SLOT: menubar item Help - Info
 */
void    KbvMain::helpInfo()
  {
    helpDialog->show();
  }
/*************************************************************************//*!
 *Attributes of loaded plugins
 */
QStringList   KbvMain::getPluginAttributes()
{
  return  pluginAttributes;
}

/*************************************************************************//*!
 * Create splitter pane and populate with models and views.
 */
void   KbvMain::createCentralArea()
{
  //splitter pane
  kbvSplitter = new QSplitter(Qt::Horizontal, this);
  kbvSplitter->setObjectName("kbvSplitter");
  kbvSplitter->setChildrenCollapsible(false);
  kbvSplitter->setHandleWidth (6);
  kbvSplitter->setStretchFactor(0, KbvConf::HorStretchLeft);
  kbvSplitter->setStretchFactor(1, KbvConf::HorStretchRight);
  setCentralWidget (kbvSplitter);

  //dir model/view and collection model/view 
  dirModel = new KbvDirModel(this);
  dirView = new  KbvDirView(this);
  dirView->setModelAndConnect(dirModel);

  collTreeModel = new KbvCollectionTreeModel(this);
  collTreeView = new KbvCollectionTreeView(this);
  collTreeView->setModelAndConnect(collTreeModel);

  //Splitter left side: tab widget with 2 tabs for file tree and collection tree
  QSizePolicy sizepol;
  kbvTabsLeft = new QTabWidget(this);
  kbvTabsLeft->setObjectName("kbvTabsLeft");
  kbvTabsLeft->setIconSize(QSize (32, 22));
  kbvTabsLeft->setMinimumSize(QSize(KbvConf::TabMinimumWidth, KbvConf::TabMinimumHeight));
  sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
  sizepol.setVerticalPolicy(QSizePolicy::Expanding);
  sizepol.setHorizontalStretch(KbvConf::HorStretchLeft);
  kbvTabsLeft->setSizePolicy(sizepol);

  kbvTabsLeft->insertTab(KbvConf::DirTreeTab, dirView, generalFunc.iconKbvFolderTree, "");
  kbvTabsLeft->insertTab(KbvConf::CollTreeTab, collTreeView, generalFunc.iconKbvAlbums, "");
  
  //Splitter right side: tab widget with 2 tabs for file view and search results
  //all collection/album models and views are added on demand
  kbvTabsRight = new KbvCollectionTabs(this);
  kbvTabsRight->setObjectName("kbvTabsRight");
  kbvTabsRight->setUsesScrollButtons(true);
  kbvTabsRight->setIconSize(QSize (32, 22));
  kbvTabsRight->setMinimumSize(QSize(KbvConf::TabMinimumWidth, KbvConf::TabMinimumHeight));
  sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
  sizepol.setVerticalPolicy(QSizePolicy::Expanding);
  sizepol.setHorizontalStretch(KbvConf::HorStretchRight);
  kbvTabsRight->setSizePolicy(sizepol);

  //populate the splitter panes
  kbvSplitter->insertWidget(0, kbvTabsLeft);  //left side
  kbvSplitter->insertWidget(1, kbvTabsRight); //right side
}
/*************************************************************************//*!
 * Directory structures:
 * Development:
 *  ~imarca/kbvCore/src       - source files (*.cpp, *.h)
 *         /kbvCore/prd       - application,help and translation files
 *         /test              - help and translation files, dyn. plugins
 * Installed:
 *  /usr/bin                     - application (appInstallDir)
 *  /var/lib/imarca"             - help & translation files
 *  /usr/lib/imarca/myPlugin"    - dyn. plugin named "myPlugin"
 * Note: paths must be provided in kbv.pro and be exported with LD_LIBRARY_PATH
 * when running imarca from qt-creator (otherwise they must be found in
 * standard search paths)
 */
void   KbvMain::loadDynamicPlugins()
{
  KbvGeneral    generalFunc;
  QString       appdir, locale, name, dirPath, dirName, fileName, nameSpace;
  QStringList   registeredHelp, staticHelp;
  QDir          pluginsDir;
  KbvDynamicPluginInterface::PLUGIN_INFO   pluginInfo;

  //get already registered help name spaces and locale
  registeredHelp = this->helpSystem->helpEngine->registeredDocumentations();
  locale = QLocale::system().name();
  pluginAttributes.clear();

  //dynamic plugins must be loaded from libInstallDir/pluginName
  //for developement plugins are loaded from ~/workspace/test/pluginName
  appdir = QCoreApplication::applicationDirPath();
  if(appdir.startsWith(QString(appInstallDir)))
    {
      //install dir for dyn. plugins, their translations and help files
      pluginsDir.setPath(QString(libInstallDir));
    }
  else
    {
      //app in development environment /kbv/prd, search in /test
      pluginsDir.setPath(QString(appdir));
      pluginsDir.cdUp();
      pluginsDir.cdUp();
      pluginsDir.cd("test");
    }
  //qDebug() << "KbvMain::loadDynamicPlugins appdir:"<<appdir <<" pluginsdir:"<<pluginsDir.path(); //###########

  //load dynamic plugin from subdirectory of same name
  //the subdir contains the plugin (provided as shared lib *.so) as well as help and translation files
  foreach(dirName, pluginsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
      dirPath = pluginsDir.absoluteFilePath(dirName);
      fileName = dirPath + "/" + dirName;      //without extension
      QPluginLoader loader(fileName + ".so");
      QObject *dynPlugin = loader.instance();
      //qDebug() << "KbvMain::loadDynamicPlugins load"<<fileName + ".so"; //###########
      KbvDynamicPluginInterface *pluginIface = qobject_cast<KbvDynamicPluginInterface *>(dynPlugin);
      if(pluginIface)
        {
          //qDebug() << "KbvMain::loadDynamicPlugins loaded" <<dirName; //###########
          //try to install translation to initilise the plugin with the right language
          name = generalFunc.getFileForLocale(dirPath+"/", dirName, locale, QString(".qm"));
          if(!name.isEmpty())
            {
              QTranslator *translator = new QTranslator(nullptr);
              if(translator->load(name, dirPath))
                {
                  //TODO: installing a translator at this point will crash the application
                  //qApp->installTranslator(translator);
                }
            }
          //register plugin help file *.qch for given locale if required
          name = generalFunc.getFileForLocale(dirPath+"/", dirName, locale, QString(".qch"));
          nameSpace = this->helpSystem->helpEngine->namespaceName(dirPath +"/" + name);
          if(!registeredHelp.contains(nameSpace))
            {
              this->helpSystem->helpEngine->registerDocumentation(dirPath +"/" + name);
            }
          else
            {
              //remove already registered namespace from list
              registeredHelp.removeOne(nameSpace);
            }

          //initialise plugin with installed language
          pluginIface->initialisePlugin();

          mainMenuBar->addMenu(pluginIface->getPluginMenu());

          //collect attributes of plugin
          pluginInfo = pluginIface->getPluginInfo();
          pluginAttributes.append(QString(pluginInfo.name));
          pluginAttributes.append(QString(pluginInfo.version));
          pluginAttributes.append(QString(pluginInfo.author));
          pluginAttributes.append(QString(pluginInfo.abstract));
        }
      else
        {
          //qDebug() << "KbvMain::loadDynamicPlugins not loaded" <<dirName; //###########
          pluginAttributes.append(dirName);
          pluginAttributes.append(QString("?"));
          pluginAttributes.append(QString(tr("error")));
          pluginAttributes.append(QString("?"));
        }
    }
  //all available plugins checked, clear registered help from orphand plugins
  //get namespaces of static plugins and remove them from registered namespaces
  staticHelp = helpSystem->staticNameSpaces();
  foreach(nameSpace, staticHelp)
    {
      registeredHelp.removeOne(nameSpace);
    }
  //qDebug() << "KbvMain::loadDynamicPlugins orphaned" <<registeredHelp; //###########
  //now the list only contains orphand items
  foreach(nameSpace, registeredHelp)
    {
      this->helpSystem->helpEngine->unregisterDocumentation(nameSpace);
    }

  //set plugin attributes for help info
  helpDialog->setPluginsList(pluginAttributes);

 // qDebug() << "KbvMain::loadDynamicPlugins registered help" << this->helpSystem->helpEngine->registeredDocumentations(); //###########
}
/*************************************************************************//*!
 * Load static plugins, initialise and supplement main menu with the plugin menu
 * Note: each static plugin is compiled into the app and needs the macro
 * Q_IMPORT_PLUGIN(plugin class name).
 */
void   KbvMain::loadStaticPlugins()
{
  foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
      //Static plugin libImageMetadata
      KbvIptcExifXmpInterface *metadata = qobject_cast<KbvIptcExifXmpInterface *>(plugin);
      //qDebug() << "KbvMain::loadStaticPlugins *metadata *plugin" <<metadata <<plugin; //###########
      if(metadata)
        {
          metadataPlugin = metadata;
          //qDebug() << "KbvMain::loadStaticPlugins" <<metadataPlugin->pluginInfo(); //###########
          metadataPlugin->initialisePlugin();
          if(metadataPlugin->showMenuItems())
            {
              mainMenu->setActionMetadataVisible(true);
            }
        }

      //Static plugin libKbvImageEditor
      KbvImageEditorInterface *imageEdit = qobject_cast<KbvImageEditorInterface *>(plugin);
      //qDebug() << "KbvMain::loadStaticPlugins imageEdit" <<imageEditPlugin; //###########
      if(imageEdit)
        {
          imageEditPlugin = imageEdit;
          //qDebug() << "KbvMain::loadStaticPlugins" <<imageEditPlugin->pluginInfo(); //###########
          imageEditPlugin->initialisePlugin();
          if(imageEditPlugin->showMenuItems())
            {
              mainMenu->setActionImageEditVisible(true);
            }
        }
    }
}

/****************************************************************************/
