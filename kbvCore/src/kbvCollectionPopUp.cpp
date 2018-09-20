/*****************************************************************************
 * kbvCollectionPopUp
 * This is the pop up menu for search view and collection view
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.10.01
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvCollectionPopUp.h"

KbvCollectionPopUp::KbvCollectionPopUp(QWidget *parent, int menuType) : QMenu(parent)
{
  actKeyCtrlC = new QAction(tr("Copy"), this);
  actKeyCtrlC->setShortcut(tr("Ctrl+C"));
  actKeyCtrlX = new QAction(tr("Cut"), this);
  actKeyCtrlX->setShortcut(tr("Ctrl+X"));
  actKeyCtrlV = new QAction(tr("Paste"), this);
  actKeyCtrlV->setShortcut(tr("Ctrl+V"));
  actSearchCollection = new QAction(tr("Find in collection"), this);
  actSearchCollection->setShortcut(tr("Ctrl+F"));
  actDBAttributes = new QAction(tr("Database info"), this);
  actRecordContent = new QAction(tr("Data set content"), this);
  actSearchClear = new QAction(tr("Clear search result"), this);
  actSearchClear->setShortcut(tr("Ctrl+Shift+F"));
  actShowEditor = new QAction(tr("Edit"), this);
  actShowEditor->setShortcut(tr("F7"));
  actShowBatchEditor = new QAction(tr("Batch edit"), this);
  actShowBatchEditor->setShortcut(tr("F10"));
  actShowMetadata = new QAction(tr("Meta data"), this);
  actShowMetadata->setShortcut(tr("F6"));

  this->addAction(actKeyCtrlV);
  this->addSeparator();
  this->addAction(actKeyCtrlC);
  this->addAction(actKeyCtrlX);
  this->addSeparator();
  this->addAction(actSearchClear);
  this->addAction(actSearchCollection);
  this->addAction(actDBAttributes);
  this->addAction(actRecordContent);
  this->addSeparator();
  this->addAction(actShowEditor);
  this->addAction(actShowBatchEditor);
  this->addAction(actShowMetadata);

  connect(actKeyCtrlC,          SIGNAL(triggered()), this, SLOT(copyTriggered()));
  connect(actKeyCtrlX,          SIGNAL(triggered()), this, SLOT(cutTriggered()));
  connect(actKeyCtrlV,          SIGNAL(triggered()), this, SLOT(pasteTriggered()));
  connect(actSearchCollection,  SIGNAL(triggered()), this, SIGNAL(search()));
  connect(actSearchClear,       SIGNAL(triggered()), this, SIGNAL(searchClear()));
  connect(actDBAttributes,      SIGNAL(triggered()), this, SIGNAL(showDBAttributes()));
  connect(actRecordContent,     SIGNAL(triggered()), this, SIGNAL(showRecordContent()));
  connect(actShowEditor,        SIGNAL(triggered()), this, SIGNAL(showEditor()));
  connect(actShowBatchEditor,   SIGNAL(triggered()), this, SIGNAL(showBatchEditor()));
  connect(actShowMetadata,      SIGNAL(triggered()), this, SIGNAL(showMetadata()));

  if(menuType & Kbv::TypeCollection)
    {
      this->actSearchClear->setEnabled(false);
      this->actSearchClear->setVisible(false);
    }
  if(menuType & Kbv::TypeSearch)
    {
      this->actSearchCollection->setEnabled(false);
      this->actKeyCtrlV->setEnabled(false);
    }
  if(menuType & Kbv::TypeFile)
    {
      this->actSearchCollection->setVisible(false);
      this->actSearchClear->setVisible(false);
      this->actDBAttributes->setVisible(false);
      this->actRecordContent->setVisible(false);
    }
  //default
  clipboard = QApplication::clipboard();
  keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Pause, Qt::ControlModifier);
}

KbvCollectionPopUp::~KbvCollectionPopUp()
{
  //qDebug() << "KbvCollectionPopUp::~KbvCollectionPopUp";//###########
  delete keyEvent;
}
void  KbvCollectionPopUp::copyTriggered()
{
  delete keyEvent;
  keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
  emit copyPasteAction(keyEvent);
}
void  KbvCollectionPopUp::cutTriggered()
{
  delete keyEvent;
  keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_X, Qt::ControlModifier);
  emit copyPasteAction(keyEvent);
}
void  KbvCollectionPopUp::pasteTriggered()
{
  delete keyEvent;
  keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier);
  emit copyPasteAction(keyEvent);
}

QAction* KbvCollectionPopUp::exec(const QPoint & p, bool selection)
{
  if(selection)
    {
      actKeyCtrlC->setEnabled(true);
      actKeyCtrlX->setEnabled(true);
    }
  else
    {
      actKeyCtrlC->setEnabled(false);
      actKeyCtrlX->setEnabled(false);
    }
  actKeyCtrlV->setEnabled(false);
  QMimeData const *mimeData = clipboard->mimeData();
  if(mimeData->hasFormat("x-special/imarca-copied-files") ||
     mimeData->hasFormat("x-special/gnome-copied-files"))
    {
      actKeyCtrlV->setEnabled(true);
    }
  return QMenu::exec(p, 0);
}

/****************************************************************************/
