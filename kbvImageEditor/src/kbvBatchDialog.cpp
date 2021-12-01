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
#include "kbvBatchDialog.h"

KbvBatchDialog::KbvBatchDialog(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  this->setWindowTitle(QString("Imarca"));

  mPages = ui.stackedWidget->count();
  
  ui.stackedWidget->setCurrentIndex(0);
  ui.leftButton->setEnabled(false);

  ui.dialRotate->setValue(0);
  ui.dialRotate->setEnabled(true);
  ui.doubleSpinRotate->setValue(0.0);
  ui.doubleSpinRotate->setEnabled(true);
  ui.buttonFlipHor->setChecked(false);
  ui.buttonFlipVer->setChecked(false);
  ui.buttonFlipHor->setEnabled(true);
  ui.buttonFlipVer->setEnabled(true);

  ui.spinWidthPixels->setValue(0);
  ui.spinWidthPixels->setEnabled(true);
  ui.spinHeightPixels->setValue(0);
  ui.spinHeightPixels->setEnabled(true);
  ui.spinPercent->setValue(0.0);
  ui.spinPercent->setEnabled(true);

  //File formats and parameters
  //We need the last three characters as format information
  QStringList sl;
  sl << tr("JPEG image  *.jpeg *.jpg");
  sl << tr("PNG image  *.png");
  sl << tr("TIFF image  *.tiff *.tif");
  sl << tr("Windows bitmap  *.bmp");
  ui.comboFileFormat->clear();
  ui.comboFileFormat->addItems(sl);

  connect(ui.leftButton,        SIGNAL(clicked(bool)), this, SLOT(leftButtonPressed(bool)));
  connect(ui.middleButton,      SIGNAL(clicked(bool)), this, SLOT(middleButtonPressed(bool)));
  connect(ui.rightButton,       SIGNAL(clicked(bool)), this, SLOT(rightButtonPressed(bool)));

  connect(ui.buttonFlipHor,     SIGNAL(clicked(bool)), this, SLOT(buttonFlipHorizontal(bool)));
  connect(ui.buttonFlipVer,     SIGNAL(clicked(bool)), this, SLOT(buttonFlipVertical(bool)));
  connect(ui.buttonRotateExif,  SIGNAL(clicked(bool)), this, SLOT(buttonRotateExif(bool)));
  connect(ui.dialRotate,        SIGNAL(valueChanged(int)), this, SLOT(dialRotate(int)));
  connect(ui.doubleSpinRotate,  SIGNAL(valueChanged(double)), this, SLOT(doubleSpinRotate(double)));

  connect(ui.spinHeightPixels,  SIGNAL(valueChanged(int)), this, SLOT(spinHeightPixels(int)));
  connect(ui.spinWidthPixels,   SIGNAL(valueChanged(int)), this, SLOT(spinWidthPixels(int)));
  connect(ui.spinPercent,       SIGNAL(valueChanged(double)), this, SLOT(spinPercent(double)));

  connect(ui.comboFileFormat,   SIGNAL(activated(int)), this, SLOT(comboFileFormat(int)));

  connect(ui.buttonSelect,      SIGNAL(clicked(bool)), this, SLOT(buttonSelectPressed(bool)));
  connect(ui.buttonStart,       SIGNAL(clicked(bool)), this, SLOT(buttonStartPressed(bool)));

  mResult = Kbv::cancel;      //default result for close (X) button
  this->comboFileFormat(0);   //initialise
}
KbvBatchDialog::~KbvBatchDialog()
{
}

/*************************************************************************//*!
 * SLOT: Show modal dialog.
 */
int  KbvBatchDialog::exec(Kbv::kbvBatchParam &params)
{
  //qDebug() <<"KbvBatchDialog::exec";//###########

  QDialog::exec();

  //page Rotate
  params.flipHor = ui.buttonFlipHor->isChecked();
  params.flipVer = ui.buttonFlipVer->isChecked();
  params.rotExif = ui.buttonRotateExif->isChecked();
  params.rotAngle = ui.doubleSpinRotate->value();
  //page Edge Trimming
  params.trimTop = ui.spinTrimtop->value();
  params.trimLeft = ui.spinTrimLeft->value();
  params.trimBottom = ui.spinTrimBottom->value();
  params.trimRight = ui.spinTrimRight->value();
  //page Resize
  params.resizeHeight = ui.spinHeightPixels->value();
  params.resizeWidth = ui.spinWidthPixels->value();
  params.resizePercent = ui.spinPercent->value();
  //page Save
  params.saveFormat = ui.comboFileFormat->currentText().right(3); //format = last three characters jpg, png, etc.
  params.saveParam = ui.editFormatParam->text().toInt();
  params.targetDir = ui.editTargetDir->text();
  params.overwrite = ui.checkOverwrite->isChecked();

  return  mResult;
}

/*************************************************************************//*!
 * SLOT: button "mirror horizontal" changed.
 */
void  KbvBatchDialog::buttonFlipHorizontal(bool checked)
{
  if(checked)
    {
      ui.buttonFlipVer->setChecked(false);
      ui.buttonRotateExif->setChecked(false);
      ui.dialRotate->setValue(0);
      ui.dialRotate->setEnabled(false);
      ui.doubleSpinRotate->setValue(0.0);
      ui.doubleSpinRotate->setEnabled(false);
    }
  else
    {
      if(!ui.buttonFlipVer->isChecked() && !ui.buttonRotateExif->isChecked())
        {
          ui.dialRotate->setEnabled(true);
          ui.doubleSpinRotate->setEnabled(true);
        }
    }
}
/*************************************************************************//*!
 * SLOT: button "mirror vertical" changed.
 */
void  KbvBatchDialog::buttonFlipVertical(bool checked)
{
  if(checked)
    {
      ui.buttonFlipHor->setChecked(false);
      ui.buttonRotateExif->setChecked(false);
      ui.dialRotate->setValue(0);
      ui.dialRotate->setEnabled(false);
      ui.doubleSpinRotate->setValue(0.0);
      ui.doubleSpinRotate->setEnabled(false);
    }
  else
    {
      if(!ui.buttonFlipHor->isChecked() && !ui.buttonRotateExif->isChecked())
        {
          ui.dialRotate->setEnabled(true);
          ui.doubleSpinRotate->setEnabled(true);
        }
    }
}
/*************************************************************************//*!
 * SLOT: button "rotate due to exif orientation tag".
 */
void  KbvBatchDialog::buttonRotateExif(bool checked)
{
  if(checked)
    {
      ui.buttonFlipHor->setChecked(false);
      ui.buttonFlipVer->setChecked(false);
      ui.dialRotate->setValue(0);
      ui.dialRotate->setEnabled(false);
      ui.doubleSpinRotate->setValue(0.0);
      ui.doubleSpinRotate->setEnabled(false);
    }
  else
    {
      if(!ui.buttonFlipHor->isChecked() && !ui.buttonFlipVer->isChecked())
        {
          ui.dialRotate->setEnabled(true);
          ui.doubleSpinRotate->setEnabled(true);
        }
    }
}
/*************************************************************************//*!
 * SLOT: dial rotate angle value changed.
 * The value comes as integer where 180 equels 180°
 */
void  KbvBatchDialog::dialRotate(int val)
{
  //enabling of flip buttons is done in doubleSpinRotate()
  ui.doubleSpinRotate->setValue(double(val));
}
/*************************************************************************//*!
 * SLOT: spin box rotate angle value changed.
 * The value comes as double with one decimal (e.g. 35.4°)
 */
void  KbvBatchDialog::doubleSpinRotate(double val)
{
  ui.dialRotate->setValue(int(val));
  if(val != 0.0)
    {
      ui.buttonFlipHor->setChecked(false);
      ui.buttonFlipHor->setEnabled(false);
      ui.buttonFlipVer->setChecked(false);
      ui.buttonFlipVer->setEnabled(false);
      ui.buttonRotateExif->setChecked(false);
      ui.buttonRotateExif->setEnabled(false);
    }
  else
    {
      ui.buttonFlipHor->setEnabled(true);
      ui.buttonFlipVer->setEnabled(true);
      ui.buttonRotateExif->setEnabled(true);
    }
}

/*************************************************************************//*!
 * SLOT: spin boxes height or width or percent value changed.
 * The values come as integer width or height in pixels or as double percent
 * with one decimal  (e.g. 85.7%).
 */
void  KbvBatchDialog::spinHeightPixels(int val)
{
  if(val != 0)
    {
      ui.spinWidthPixels->setValue(0);
      ui.spinWidthPixels->setEnabled(false);
      ui.spinPercent->setValue(0.0);
      ui.spinPercent->setEnabled(false);
    }
  else
    {
      ui.spinWidthPixels->setEnabled(true);
      ui.spinPercent->setEnabled(true);
    }
}
void  KbvBatchDialog::spinWidthPixels(int val)
{
  if(val != 0)
    {
      ui.spinHeightPixels->setValue(0);
      ui.spinHeightPixels->setEnabled(false);
      ui.spinPercent->setValue(0.0);
      ui.spinPercent->setEnabled(false);
    }
  else
    {
      ui.spinHeightPixels->setEnabled(true);
      ui.spinPercent->setEnabled(true);
    }
}
void  KbvBatchDialog::spinPercent(double val)
{
  if(val != 0.0)
    {
      ui.spinWidthPixels->setValue(0);
      ui.spinWidthPixels->setEnabled(false);
      ui.spinHeightPixels->setValue(0);
      ui.spinHeightPixels->setEnabled(false);
    }
  else
    {
      ui.spinHeightPixels->setEnabled(true);
      ui.spinWidthPixels->setEnabled(true);
    }
}
/*************************************************************************//*!
 * SLOT: file format selected. Index:
 * 0 = JPEG image, 1 = PNG image, 2 = TIFF image, 3 = Windows bitmap;
 */

void  KbvBatchDialog::comboFileFormat(int index)
{
  switch(index)
    {
    case 0: { ui.lblFormatParam->setText(QString(tr("JPEG quality")));
              ui.editFormatParam->setEnabled(true);
              ui.editFormatParam->setText("90"); }
      break;
    case 1: { ui.lblFormatParam->setText(QString(tr("PNG compression 1-9")));
              ui.editFormatParam->setEnabled(true);
              ui.editFormatParam->setText("6"); }
      break;
    case 2: { ui.lblFormatParam->setText(QString(tr("")));
              ui.editFormatParam->setEnabled(false);
              ui.editFormatParam->setText(""); }
      break;
    case 3: { ui.lblFormatParam->setText(QString(""));
              ui.editFormatParam->setEnabled(false);
              ui.editFormatParam->setText(""); }
      break;
    default: { ui.lblFormatParam->setText(QString(""));
               ui.editFormatParam->setEnabled(false);
               ui.editFormatParam->setText(""); }
    }
}
/*************************************************************************//*!
 * SLOT: select button pressed. Open file dialog for dirs only.
 */
void  KbvBatchDialog::buttonSelectPressed(bool checked)
{
  Q_UNUSED(checked)
  QString dir;

  dir = QFileDialog::getExistingDirectory(this, QString(tr("Select directory. If left empty the source directory will be used,")));
  ui.editTargetDir->setText(dir);
}

/*************************************************************************//*!
 * SLOT: start button pressed. Apply all changes to selected files.
 * Triggers closeEvent()
 */
void  KbvBatchDialog::buttonStartPressed(bool checked)
{
  Q_UNUSED(checked)

  mResult = Kbv::apply;
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: middle button pressed (left button | middle button | right button)).
 * The middle button works as "cancel" button.
 * Triggers closeEvent()
 */
void  KbvBatchDialog::middleButtonPressed(bool checked)
{
  Q_UNUSED(checked)

  mResult = Kbv::cancel;
  QWidget::close();
}
/*************************************************************************//*!
 * SLOT: left button pressed (left button | middle button | right button).
 * The left button works as "previous sheet" button.
 * Triggers closeEvent()
 */
void  KbvBatchDialog::leftButtonPressed(bool checked)
{
  Q_UNUSED(checked)
  int index;

  index = ui.stackedWidget->currentIndex();
  index -= 1;
  ui.rightButton->setEnabled(true);
  if(index > 0)
    {
      ui.stackedWidget->setCurrentIndex(index);
      ui.leftButton->setEnabled(true);
    }
  else
    {
      ui.stackedWidget->setCurrentIndex(index);
      ui.leftButton->setEnabled(false);
    }
}
/*************************************************************************//*!
 * SLOT: right button pressed (left button | middle button | right button).
 * The right button works as "next sheet" button.
 * Triggers closeEvent()
 */
void  KbvBatchDialog::rightButtonPressed(bool checked)
{
  Q_UNUSED(checked)
  int index;

  index = ui.stackedWidget->currentIndex();
  index += 1;
  ui.leftButton->setEnabled(true);
  if(index < mPages-1)
    {
      ui.stackedWidget->setCurrentIndex(index);
      ui.rightButton->setEnabled(true);
    }
  else
    {
      ui.stackedWidget->setCurrentIndex(index);
      ui.rightButton->setEnabled(false);
    }
}
/*************************************************************************//*!
 * SLOT: Quit widget on close (X) button.
 * This works as "cancel" button (by reason of preset "result = Kbv::cancel".
 */
void  KbvBatchDialog::closeEvent(QCloseEvent *event)
{
  event->accept();
}
/****************************************************************************/

