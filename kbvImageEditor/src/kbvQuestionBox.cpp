/*****************************************************************************
 * kvb question box
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2017.04.28
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <kbvConstants.h>
#include "kbvQuestionBox.h"

KbvQuestionBox::KbvQuestionBox(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  
  this->setWindowTitle(QString("Imarca"));
  
  connect(ui.leftButton,    SIGNAL(clicked(bool)), this, SLOT(leftButtonPressed(bool)));
  connect(ui.middleButton,  SIGNAL(clicked(bool)), this, SLOT(middleButtonPressed(bool)));
  connect(ui.rightButton,   SIGNAL(clicked(bool)), this, SLOT(rightButtonPressed(bool)));
}
KbvQuestionBox::~KbvQuestionBox()
{
  
}

/*************************************************************************//*!
 * SLOT: Show modal dialog.
 * When a button text is empty the related button will be hidden:
 * left button text is empty: show middle and right button
 * left & middle button text empty: only show right button
 * all button texts empty: only show right button with text "Close)
 */
int  KbvQuestionBox::exec(const QString caption, const QString text,
                          const QString leftButtonText, const QString middleButtonText, const QString rightButtonText)
{
  //default result for close (X) button
  mResult = Kbv::cancel;
  
  //qDebug() <<"KbvQuestionBox::exec";//###########
  ui.lblCaption->setText(caption);
  ui.lblText->setText(text);
  ui.leftButton->setText(leftButtonText);
  ui.middleButton->setText(middleButtonText);
  ui.rightButton->setText(rightButtonText);
  ui.leftButton->setVisible(true);
  ui.middleButton->setVisible(true);
  ui.rightButton->setVisible(true);
  if(leftButtonText.isEmpty())
    {
      ui.leftButton->setVisible(false);
    }
  if(leftButtonText.isEmpty() && middleButtonText.isEmpty())
    {
      ui.leftButton->setVisible(false);
      ui.middleButton->setVisible(false);
    }
  if(leftButtonText.isEmpty() && middleButtonText.isEmpty() && rightButtonText.isEmpty())
    {
      ui.leftButton->setVisible(false);
      ui.middleButton->setVisible(false);
      ui.rightButton->setText(QString(tr("Close")));
    }
  QDialog::exec();
  return  mResult;
}

/*************************************************************************//*!
 * SLOT: left button pressed (left button | middle button | right button).
 * The left button works as "discard or don't save" button.
 * If there are only two buttons, middle and right buttons are supported.
 * Triggers closeEvent()
 */
void  KbvQuestionBox::leftButtonPressed(bool checked)
{
  Q_UNUSED(checked)
  
  mResult = Kbv::discard;
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: right button pressed (left button | middle button | right button).
 * The right button works as "save" button.
 * If there are only two buttons, middle and right buttons are supported.
 * Triggers closeEvent()
 */
void  KbvQuestionBox::rightButtonPressed(bool checked)
{
  Q_UNUSED(checked)
  
  mResult = Kbv::save;
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: middle button pressed (left button | middle button | right button)).
 * The middle button works as "cancel" button.
 * If there are only two buttons, middle and right buttons are supported.
 * Triggers closeEvent()
 */
void  KbvQuestionBox::middleButtonPressed(bool checked)
{
  Q_UNUSED(checked)
  
  mResult = Kbv::cancel;
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: Quit widget on close (X) button.
 * This works as "cancel" button (by reason of preset "result = Kbv::cancel".
 */
void  KbvQuestionBox::closeEvent(QCloseEvent *event)
{
  event->accept();
}
/****************************************************************************/

