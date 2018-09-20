/*****************************************************************************
 * Main menu.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.19.22
 ****************************************************************************/
#ifndef KBVMAINMENU_H_
#define KBVMAINMENU_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class KbvMainMenu : public QMenuBar
{
  Q_OBJECT

public:
  KbvMainMenu(QWidget *parent = nullptr);
  virtual   ~KbvMainMenu();
  QMenu     *menuCollection, *menuImage, *menuHelp;
  QToolBar  *dbToolBar, *fileToolBar, *editToolBar, *quitToolBar;
  
  void  setActionMetadataVisible(bool visible);
  void  setActionImageEditVisible(bool visible);

signals:
void    menuRenameDir();    //file
void    menuRemoveDir();
void    menuInsertDir();
void    menuOpenColl(QString s);
void    menuImportColl();
void    menuExportColl();
void    menuQuit();
void    menuOptions();      //edit
void    menuCopyCutPaste(QKeyEvent *e);
void    menuRename();
void    menuFind();
void    menuFindClear();
void    menuShowSlides();   //view
void    menuDBInfo();       //collection
void    menuRecordInfo();
void    menuDBCreate();
void    menuDBRemove();
void    menuDBRead();
void    menuDBUpdate();
void    menuCsvCheck();
void    menuDBRename();
void    menuDBInsert();
void    menuDBDelete();
void    menuImageEdit();    //image
void    menuBatchEdit();
void    menuMetadata();
void    menuIptcTemplate();
void    menuHelpContent();  //help
void    menuHelpInfo();

private slots:
void    mOpenColl();
void    mCopy();
void    mCut();
void    mPaste();

private:
  QMenu     *menuFile, *menuEdit;                                //menu bar
  QAction   *actFile, *actEdit, *actColl, *actImage, *actHelp;
  QAction   *actRenameDir, *actInsertDir, *actRemoveDir, *actQuit;          //menu File
  QAction   *actImportColl, *actExportColl, *actOpenColl;
  QAction   *actOptions, *actRename, *actFind, *actFindClear;               //menu Edit
  QAction   *actCopy, *actCut, *actPaste;
  QAction   *actDBInfo, *actRecordInfo, *actDBCreate, *actDBRemove;         //menu Collection
  QAction   *actDBRead, *actDBUpdate, *actCsvCheck;
  QAction   *actDBRename, *actDBInsert, *actDBDelete;
  QAction   *actSlideshow, *actImageEdit, *actBatchEdit, *actMetadata, *actIptcTemplate;   //menu Image
  QAction   *actContent, *actInfo;                                          //menu Help
  QKeyEvent *eventCopy, *eventCut, *eventPaste;
};
#endif /* KBVMAINMENU_H_ */
/****************************************************************************/
