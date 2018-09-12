/*****************************************************************************
 * Main window.
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-01-19 15:49:55 +0100 (Fr, 19. Jan 2018) $
 * $Rev: 1387 $
 * Created: 2008.11.06
 ****************************************************************************/
#ifndef KBV_H_
#define KBV_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "kbvMainMenu.h"
#include "kbvSetvalues.h"
#include "kbvGlobal.h"
#include "kbvGeneral.h"
#include "kbvRenameDialog.h"
#include "kbvReplaceDialog.h"
#include "kbvInformationDialog.h"
#include "kbvDirModel.h"
#include "kbvDirView.h"
#include "kbvCollectionTreeModel.h"
#include "kbvCollectionTreeView.h"
#include "kbvCollectionTabs.h"
#include "kbvOptions.h"
#include "kbvHelpInfo.h"
#include "kbvHelp.h"
#include "kbvDBusMonitor.h"
#include "kbvPluginInterfaces.h"

extern  KbvMainMenu             *mainMenu;
extern  KbvSetvalues            *settings;
extern  KbvRenameDialog         *renameDialog;
extern  KbvReplaceDialog        *replaceDialog;
extern  KbvInformationDialog    *informationDialog;
extern  KbvIptcExifXmpInterface *metadataPlugin;
extern  KbvImageEditorInterface *imageEditPlugin;


class KbvMain : public QMainWindow
{
    Q_OBJECT

public:
  KbvMain(QWidget *parent=nullptr);
  virtual ~KbvMain();

  QStringList getPluginAttributes();
  
signals:
  void    newpath(QString);       // New path selected in dir tree
  void    keyCtrlAPressed(QKeyEvent *event);
  void    cmdLineDatabase(QString database);

public slots:
  void    cmdLineArgs();

private slots:
  void    fileOpen();
  void    editOptions();
  void    helpContent();
  void    helpInfo();

private:
  void    keyPressEvent(QKeyEvent *event);
  void    closeEvent(QCloseEvent *event);
  void    createCentralArea();
  void    writeWindowSettings();
  void    readWindowSettings();
  void    loadDynamicPlugins();
  void    loadStaticPlugins();
  
  //QStatusBar                *mainStatusBar;
  KbvGeneral                generalFunc;
  KbvGlobal                 globalFunc;
  KbvMainMenu               *mainMenuBar;
  KbvDirModel               *dirModel;
  KbvDirView                *dirView;
  KbvCollectionTreeModel    *collTreeModel;
  KbvCollectionTreeView     *collTreeView;
  KbvCollectionTabs         *kbvTabsRight;
  QTabWidget                *kbvTabsLeft;
  QSplitter                 *kbvSplitter;
  KbvHelpInfo               *helpDialog;
  KbvHelp                   *helpSystem;
  KbvOptions                *optionsDialog;

  QApplication              *app;
  KbvDBusMonitor            *dbusDeviceMonitor;
  
  QStringList               pluginAttributes;
  QStringList               args;                   //command line
};
#endif // KBV_H
/****************************************************************************/
