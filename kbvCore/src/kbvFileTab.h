/*****************************************************************************
 * KbvFileTab
 * This is the widget for file tab
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2013.02.28
 ****************************************************************************/
#ifndef KBVFILETAB_H_
#define KBVFILETAB_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvTabWidget.h"
#include "kbvFileView.h"
#include "kbvFileModel.h"
#include "kbvSortFilterModel.h"
#include "kbvCollectionPopUp.h"

class KbvFileTab : public KbvTabWidget
{
  Q_OBJECT

public:
  KbvFileTab(QWidget *parent = nullptr);
  virtual
  ~KbvFileTab();

  KbvFileView         *fileView;
  KbvFileModel        *fileModel;
  KbvSortFilterModel  *sortModel;
  KbvCollectionPopUp  *popupMenu;

signals:
  void	imageEdit();
  void	imageBatchEdit();
  void	imageMetadata();

private slots:
  void    contextMenuEvent(QContextMenuEvent *event);

};

#endif /* KBVFILETAB_H_ */
/****************************************************************************/
