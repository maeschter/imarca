/*****************************************************************************
 * kbvFileTab
 * This is the widget for the file tab
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2013.02.28
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvFileTab.h"

KbvFileTab::KbvFileTab(QWidget *parent) : KbvTabWidget(parent)
{
  //Create file view, model, file model thread
  sortModel = new KbvSortFilterModel();
  fileModel = new KbvFileModel();
  fileView = new KbvFileView();
  fileView->setModels(sortModel, fileModel);
  this->addWidget(fileView);

  //Pop up menu for file tab including signal connections
  popupMenu = new KbvCollectionPopUp(this, Kbv::TypeFile);
  connect(popupMenu, SIGNAL(copyPasteAction(QKeyEvent*)), fileView, SLOT(keyPressEvent(QKeyEvent*)));
  connect(popupMenu, SIGNAL(showEditor()),      this, SIGNAL(imageEdit()));
  connect(popupMenu, SIGNAL(showBatchEditor()), this, SIGNAL(imageBatchEdit()));
  connect(popupMenu, SIGNAL(showMetadata()),    this, SIGNAL(imageMetadata()));

  connect(this,  SIGNAL(viewModeChanged(int)),   fileView, SLOT(changeViewMode(int)));
  connect(this,  SIGNAL(sortButtonClicked(int)), sortModel, SLOT(sortButtonClicked(int)));
  connect(this,  SIGNAL(sortRoleChanged(int)),   sortModel, SLOT(sortRoleChanged(int)));

  connect(fileModel, SIGNAL(enableSorting(bool)), sortModel, SLOT(enableSorting(bool)));
  connect(fileModel, SIGNAL(infoText1(QString)), this, SLOT(setInfoText1(QString)));
  connect(fileModel, SIGNAL(infoText2(QString)), this, SLOT(setInfoText2(QString)));
  connect(fileModel, SIGNAL(infoText3(QString)), this, SLOT(setInfoText3(QString)));

  connect(fileView,  SIGNAL(infoText2(QString)), this, SLOT(setInfoText2(QString)));
  connect(fileView,  SIGNAL(infoText3(QString)), this, SLOT(setInfoText3(QString)));
}

KbvFileTab::~KbvFileTab()
{
  //qDebug() << "KbvFileTab::~KbvFileTab";//###########
  delete  fileModel;
  delete  sortModel;
}
/*************************************************************************//*!
 * Popup menu (reduced for file operations only).
 */
void    KbvFileTab::contextMenuEvent(QContextMenuEvent *event)
{
  if(this->fileView->selectionModel()->hasSelection())
    {
      this->popupMenu->exec(event->globalPos(), true);
    }
  else
    {
      this->popupMenu->exec(event->globalPos(), false);
    }
}

/****************************************************************************/
