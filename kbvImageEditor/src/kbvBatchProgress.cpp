/*****************************************************************************
 * kvb question box
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.04.28
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <kbvConstants.h>
#include "kbvBatchProgress.h"

/*************************************************************************//*!
 * Create a non blocking window modal dialog and return immediately.
 * When the dialog get closed, the signal accepted() will be emitted.
 */
KbvBatchProgress::KbvBatchProgress(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  
  this->setWindowTitle(QString("Imarca"));
  ui.buttonClose->setVisible(true);
  ui.buttonClose->setEnabled(false);
  ui.buttonCancel->setVisible(true);
  ui.buttonCancel->setEnabled(true);
  ui.buttonReport->setVisible(true);
  ui.buttonReport->setEnabled(false);

  connect(ui.buttonClose,  SIGNAL(clicked(bool)), this, SLOT(buttonClosePressed(bool)));
  connect(ui.buttonCancel, SIGNAL(clicked(bool)), this, SLOT(buttonCancelPressed(bool)));
  connect(ui.buttonReport, SIGNAL(clicked(bool)), this, SLOT(buttonReportPressed(bool)));
}
KbvBatchProgress::~KbvBatchProgress()
{
}

/*************************************************************************//*!
 * SLOT: Show window modal dialog and return immediately.
 */
void  KbvBatchProgress::open(const QString file)
{
  //qDebug() <<"KbvBatchProgress::exec";//###########
  ui.lblFilename->setText(file);
  ui.buttonCancel->setEnabled(true);
  ui.buttonClose->setEnabled(false);
  ui.buttonReport->setEnabled(false);

  QDialog::open();
}
/*************************************************************************//*!
 * SLOT: Interaction with thread. Set progress information (text and progress
 * value 0-100). Control close and report button.
 */
void    KbvBatchProgress::setProgressText(QString text)
{
  ui.lblFilename->setText(text);
}
void  KbvBatchProgress::setProgressValue(int value)
{
  ui.progressBar->setValue(value);
}
void  KbvBatchProgress::finished(bool report)
{
  ui.buttonClose->setEnabled(true);
  ui.buttonCancel->setEnabled(false);
  ui.buttonReport->setEnabled(report);
}

/*************************************************************************//*!
 * SLOT: Button "Report" opens the report file but does not close the dialog.
 */
void  KbvBatchProgress::buttonReportPressed(bool checked)
{
  Q_UNUSED(checked)

  emit  showReport();
}
/*************************************************************************//*!
 * SLOT: Button "cancel" sends signal "rejected" to stop the thread but does
 * not close the dialog.
 */
void  KbvBatchProgress::buttonCancelPressed(bool checked)
{
  Q_UNUSED(checked)

  ui.buttonClose->setEnabled(true);
  emit  rejected();
}/*************************************************************************//*!
 * SLOT: Button "close" triggers closeEvent() and emits the signal "accepted()".
 */
void  KbvBatchProgress::buttonClosePressed(bool checked)
{
  Q_UNUSED(checked)
  
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: Quit widget on close (X) button and emit the signal "accepted()".
 */
void  KbvBatchProgress::closeEvent(QCloseEvent *event)
{
  Q_UNUSED(event)
  //event->accept();
  this->accept();
}
/****************************************************************************/

