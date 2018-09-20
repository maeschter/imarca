/*****************************************************************************
 * kbvCollectionPopUp
 * This is the pop up menu for search view and collection view
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.10.01
 ******************************************************************************/
#ifndef KBVCOLLECTIONPOPUP_H_
#define KBVCOLLECTIONPOPUP_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvConstants.h"

/* Description *//*!
  The class KbvCollectionPopUp provides a menu on right mouse click with the items:
  DB Attributes, Record Content, Search Collection, Search Clear and Update Record.
  On collection view the item Search Clear gets disabled when the parameter
  menuType = Kbv::TypeCollection. On search view the items Update and Update Record
  get disabled when the parameter menuType = Kbv::TypeSearch.
*/
class KbvCollectionPopUp : public QMenu
{
  Q_OBJECT

public:
  KbvCollectionPopUp(QWidget *parent=nullptr, int menuType=Kbv::TypeNone);
  virtual
  ~KbvCollectionPopUp();

  QAction* exec(const QPoint & p, bool selection);

signals:
  void  copyPasteAction(QKeyEvent *keyEvent);
  void  search();
  void  searchClear();
  void  showDBAttributes();
  void  showRecordContent();
  void  showMetadata();
  void  showEditor();
  void  showBatchEditor();

private slots:
  void  copyTriggered();
  void  cutTriggered();
  void  pasteTriggered();

private:
  QAction     *actKeyCtrlC, *actKeyCtrlX, *actKeyCtrlV,
              *actDBAttributes, *actRecordContent,
              *actSearchCollection, *actSearchClear,
              *actShowMetadata, *actShowEditor, *actShowBatchEditor;
  QKeyEvent   *keyEvent;
  QClipboard  *clipboard;
};

#endif /* KBVCOLLECTIONPOPUP_H_ */
/****************************************************************************/
