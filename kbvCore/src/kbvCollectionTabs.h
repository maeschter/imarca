/*****************************************************************************
 * kbvCollectionTabs
 * This is the tab widget for file, search, album and collection tabs
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.10.11
 ****************************************************************************/
#ifndef KBVCOLLECTIONTABS_H_
#define KBVCOLLECTIONTABS_H_
#include <QtCore>
#include <QtGui>

#include "kbvCollectionDragDrop.h"
#include "kbvTabBar.h"
#include "kbvTabWidget.h"
#include "kbvCollectionStack.h"
#include "kbvFileTab.h"
#include "kbvSearchTab.h"
#include "kbvDBInfo.h"
#include "kbvBusyAnimation.h"

class KbvCollectionTabs : public QTabWidget
{
  Q_OBJECT

public:
  KbvCollectionTabs(QWidget *parent = nullptr);
  virtual
  ~KbvCollectionTabs();

  KbvTabWidget          *searchWidget, *collWidget;

  KbvTabBar             *collTabBar;
  KbvCollectionStack    *collViewTab;
  KbvFileTab            *fileViewTab;
  KbvSearchTab          *searchViewTab;
  KbvCollectionDragDrop *dndHandler;

void    dropDataScheduler(const QMimeData *mimeData, Qt::DropAction action,
                       QString branch, KbvDBInfo *info, bool visible);

signals:
void    settingsChanged();
void    currentTitle(QString title);
void    tabClosed(const QString title);
void    dbReadBranch(const QString &title, const QString &branch);
void    renameDBBranch(const QString &names);
void    ctrlAKeyAction(QKeyEvent *event);
void    showAnimation(QString title, bool show);        //kbvCollectionStack

public slots:
void    addCollectionTab(QString title, QString branch, int type, QString location, QString root);
void    removeCollectionTab(QString title);
void    closeCollectionTab(int index);
void    renameCollectionTab(QString oldnew);
void    searchCollection();
void    checkCollection();
void    activateTab(int index);
void    currentTabTitle(int index);
void    readinOrUpdate(const QString &title, const QString &path, int type, const QString &location, const QString &root);
bool    isDBOpen(const QString name);
void    openIptcTemplate();
void    openMetaDataEditor();
void    openImageEditor();
void    openBatchEditor();

private:
bool    checkForSQLiteDB(const QString &name);
void    selectedFilesList(QList<QPair<QString, QString> > &files);

QString messageNoConnect, messageNoConnectReason;

};
#endif /* KBVCOLLECTIONTABS_H_ */
/****************************************************************************/
