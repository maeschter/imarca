/*****************************************************************************
 * kbv db import/export progress dialog
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2017.01.31
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvDBImExportProgress.h"


KbvDBImExportProgress::KbvDBImExportProgress(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);

  ui.dbImExTextFiles->setText(QString(tr("Files:")));
  ui.dbImExFileLbl->setText("");
  ui.dbImExTextSize->setText(QString(tr("Size:")));
  ui.dbImExSizeLbl->setText("");
  ui.dbImExTextInfo->setText(QString(""));
  ui.dbImExProgressBar->setValue(0);
  ui.dbImExButtonCancel->setText(QString(tr("Cancel")));

  connect(ui.dbImExButtonCancel, SIGNAL(clicked()), this, SLOT(buttonClicked()));

  //default values
  finished = false;
  action = Kbv::none;
  textCancel = QString(tr("Cancel"));
  textClose  = QString(tr("Close"));
}
KbvDBImExportProgress::~KbvDBImExportProgress()
{
  //qDebug() << "KbvDBImExportProgress::~KbvDBImExportProgress"; //###########
}
/*************************************************************************//*!
 * Close dialog requested.
 * Don't close the dialog but handle it like buttonCklicked().
 */
void  KbvDBImExportProgress::closeEvent(QCloseEvent *event)
{
  //qDebug() << "KbvDBImExportProgress::closeEvent"; //###########
  this->buttonClicked();
  event->ignore();
}
/*************************************************************************//*!
 * Cancel/close button clicked.
 * When the action is still in progress a cancel signal will be sent and the
 * button text changes from "cancel" to "close".
 */
void  KbvDBImExportProgress::buttonClicked()
{
  if(finished)
    {
      //reset all display elements and hide dialog
      finished = false;
      ui.dbImExButtonCancel->setText(textCancel);
      ui.dbImExProgressBar->setValue(0);
      ui.dbImExFileLbl->clear();
      ui.dbImExSizeLbl->clear();
      ui.dbImExTextInfo->clear();
      this->hide();
    }
  else
    {
      //cancel operation
      emit  cancel();
      ui.dbImExButtonCancel->setText(textClose);
      finished = true;
    }
}
/*************************************************************************//*!
 * Display title and set action type export/import.
 */
void  KbvDBImExportProgress::setTitle(QString text, int type)
{
  this->setWindowTitle(text);
  this->action = type;
}
/*************************************************************************//*!
 * Display information text.
 */
void  KbvDBImExportProgress::setInfotext(QString text)
{
  ui.dbImExTextInfo->setText(text);
}
/*************************************************************************//*!
 * Display number of files.
 */
void  KbvDBImExportProgress::setFilecount(int count)
{
  QLocale loc = QLocale::system();
  ui.dbImExFileLbl->setText(QString("%1").arg(loc.toString(count)));
}
/*************************************************************************//*!
 * Display number of files.
 */
void  KbvDBImExportProgress::setDiskSize(quint64 size)
{
  QLocale loc = QLocale::system();
  ui.dbImExSizeLbl->setText(QString("%1 kB").arg(loc.toString(size/1000)));
}
/*************************************************************************//*!
 * Set progress value, range: 0 - 100.
 * progress<=1 sets finished=false, progress>=99 sets finished=true.
 */
void  KbvDBImExportProgress::setProgress(int progress)
{
  ui.dbImExProgressBar->setValue(progress);
  if(progress <= 1)
    {
      finished = false;
      //qDebug() << "KbvDBImExportProgress::setProgress" <<progress <<finished; //###########
    }
  if(progress >= 99)
    {
      finished = true;
      ui.dbImExButtonCancel->setText(textClose);
      //qDebug() << "KbvDBImExportProgress::setProgress" <<progress <<finished; //###########
    }
}
/****************************************************************************/
