/*****************************************************************************
 * kbv options dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.02.18
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvOptions.h"

extern  KbvSetvalues            *settings;
extern  KbvInformationDialog    *informationDialog;

KbvOptions::KbvOptions(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  ui.kbvOptionTabs->setCurrentIndex(0);
  this->setWindowTitle(QString(tr("Options")));

  //Group ids for slide change
  ui.optGroupSlideChange->setId(ui.optSlideChange0, KbvConf::slideReplace);
  ui.optGroupSlideChange->setId(ui.optSlideChange1, KbvConf::slideFadeIn);
  ui.optGroupSlideChange->setId(ui.optSlideChange2, KbvConf::slideInsertLeft);
  ui.optGroupSlideChange->setId(ui.optSlideChange3, KbvConf::slideInsertRight);

  ui.kbvOptionTabs->setTabText(ui.kbvOptionTabs->indexOf(ui.optTab0), QString(tr("&General")));
  ui.kbvOptionTabs->setTabText(ui.kbvOptionTabs->indexOf(ui.optTab1), QString(tr("&View")));
  ui.kbvOptionTabs->setTabText(ui.kbvOptionTabs->indexOf(ui.optTab2), QString(tr("&Slide Show")));

  ui.optCheckSaveDirTreeState->setToolTip(QString(tr("Save state of directory tree when program finishes.")));
  ui.optCheckSaveDirTreeState->setText(QString(tr("Save dir tree state on exit")));
  ui.optCheckHiddenFiles->setText(QString(tr("Show hidden files")));
  ui.optGroup01->setTitle(QString(tr("Directory for data bases")));
  ui.optGenButtonDatabase->setText(QString(tr("&Open")));
  ui.optGroup12->setTitle(QString(tr("Directory watcher")));
  ui.optViewLblDirRefresh->setToolTip(QString(tr("Monitoring cycle for directory being shown.")));
  ui.optViewLblDirRefresh->setText(QString(tr("Refresh cycle (s)")));
  ui.optGroup13->setTitle(QString(tr("Show file info")));
  ui.optViewFileInfoName->setText(QString(tr("File name")));
  ui.optViewFileInfoSize->setText(QString(tr("File size")));
  ui.optViewFileInfoDim->setText(QString(tr("Image dimensions")));
  ui.optGroup11->setTitle(QString(tr("Icon size")));
  ui.optGroup22->setTitle(QString(tr("Slide change")));
  ui.optSlideChange0->setText(QString(tr("Replace")));
  ui.optSlideChange1->setText(QString(tr("Fade-in")));
  ui.optSlideChange2->setText(QString(tr("Insert left")));
  ui.optSlideChange3->setText(QString(tr("Insert right")));
  ui.optGroup21->setTitle(QString(tr("Slide control")));
  ui.opDiaLblDelay->setText(QString(tr("Delay (s)")));
  ui.optDiaCheckManual->setText(QString(tr("Slide change manually")));
  ui.optDiaCheckFullScreen->setText(QString(tr("Use full screen")));
  ui.optDiaCheckStretch->setText(QString(tr("Stretch images to screen")));
  ui.optDiaStopAtDirEnd->setText(QString(tr("Stop at end of directory")));
  ui.optSlideLblBackColourText->setText(QString(tr("Background colour")));

  ui.optSlideChange1->setEnabled(false);    //slide show in graphic scene removed
  ui.optSlideChange2->setEnabled(false);
  ui.optSlideChange3->setEnabled(false);

  connect(ui.optGenButtonDatabase, SIGNAL(clicked()),    this, SLOT(dialogDatabase()));
  connect(ui.optButtonBox, SIGNAL(accepted()),           this, SLOT(accept()));
  connect(ui.optButtonBox, SIGNAL(rejected()),           this, SLOT(reject()));
  connect(ui.optViewSizeSlider,SIGNAL(sliderMoved(int)), this, SLOT(slider(int)));

  databaseHome = settings->dataBaseDir;
  halfstep = ui.optViewSizeSlider->singleStep()/2;

  presetElements();
}

KbvOptions::~KbvOptions()
{
  //qDebug() << "KbvOptions::~KbvOptions"; //###########
}
/*************************************************************************//*!
 * Slider steps by mouse move are 25.
 */
void    KbvOptions::slider(int pos)
{
  if(pos<=75+halfstep)    {ui.optViewSizeSlider->setValue(75);}
  if(pos>75+halfstep  && pos<=100+halfstep) {ui.optViewSizeSlider->setSliderPosition(100);}
  if(pos>100+halfstep && pos<=125+halfstep) {ui.optViewSizeSlider->setSliderPosition(125);}
  if(pos>125+halfstep && pos<=150+halfstep) {ui.optViewSizeSlider->setSliderPosition(150);}
  if(pos>150+halfstep && pos<=175+halfstep) {ui.optViewSizeSlider->setSliderPosition(175);}
  if(pos>175+halfstep && pos<=200+halfstep) {ui.optViewSizeSlider->setSliderPosition(200);}
  if(pos>200+halfstep)    {ui.optViewSizeSlider->setSliderPosition(225);}
}
/*************************************************************************//*!
 * Ok button clicked. Save all elements, inform other objects about changed
 * options and hide dialog.
 */
void    KbvOptions::accept()
{
  this->setResult(QDialog::Accepted);
  hide();
  saveElements();
}
/*************************************************************************//*!
 * Cancel button clicked. Load all elements again to discard changes and hide
 * dialog.
 */
void    KbvOptions::reject()
{
  this->setResult(QDialog::Rejected);
  hide();
  presetElements();
}
/*************************************************************************//*!
 * File dialog to determine the disk location for databases.
 * When the directory does not exist it will be created.
 */
void    KbvOptions::dialogDatabase()
{
  QString       path, str;
  QFileInfo     info;

  //ask for directory and create it if required
  path = dataBaseDialog.getExistingDirectory(this, tr("Open directory for databases"),
                        databaseHome, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  ui.optGenLineEditDatabase->setText(path);
  //check for directory
  info = QFileInfo(path);
  if(!info.exists())
    {
      str = QString(tr("The directory does not exist or couldn't be created."));
      informationDialog->perform(str,QString(),1);
    }
}
/*************************************************************************//*!
 * Open colour dialog by mouse click on label. This only sets the colour label
 * with choosen colour.
 */
void    KbvOptions::mousePressEvent(QMouseEvent *me)
{
  Q_UNUSED(me)
  //qDebug() << "KbvOptions::mousePressEvent under mouse" << me->pos(); //###########
  if (ui.optSlideLblBackColour->underMouse())
    {
      QColor colour = colorDialog.getColor(QColor(Qt::white), this);
      if (colour.isValid())
        {
          QPalette palette = ui.optSlideLblBackColour->palette();
          palette.setColor(QPalette::Window, colour);
          ui.optSlideLblBackColour->setPalette(palette);
        }
    }
}

/*************************************************************************//*!
 * Store all values in configuration file and inform kbvSetting about changes.
 */
void    KbvOptions::saveElements()
{
  QPalette  palette;
  QColor    colour;

  //General
  settings->beginGroup("App");
  settings->beginGroup("Options-General");
  settings->setValue("dataBaseDir", ui.optGenLineEditDatabase->text());
  settings->endGroup();

  //View
  settings->beginGroup("Options-View");
  settings->setValue("iconSize",        ui.optViewSizeSlider->value());
  settings->setValue("dirWatchCycle",   ui.optViewDirWatchCycle->value());
  settings->setValue("showHiddenFiles", ui.optCheckHiddenFiles->isChecked());
  settings->setValue("saveDirTreeState",ui.optCheckSaveDirTreeState->isChecked());
  settings->setValue("showFileName",    ui.optViewFileInfoName->isChecked());
  settings->setValue("showFileSize",    ui.optViewFileInfoSize->isChecked());
  settings->setValue("showImageSize",   ui.optViewFileInfoDim->isChecked());
  settings->endGroup();

  //Slide Show
  settings->beginGroup("Options-Slide");
  settings->setValue("checkManual",     ui.optDiaCheckManual->isChecked());
  settings->setValue("checkFullScreen", ui.optDiaCheckFullScreen->isChecked());
  settings->setValue("checkStretch",     ui.optDiaCheckStretch->isChecked());
  settings->setValue("checkStopAtDirEnd", ui.optDiaStopAtDirEnd->isChecked());
  settings->setValue("slideDelay",      ui.optDiaValueDelay->value());
  settings->setValue("slideChangeId",   ui.optGroupSlideChange->checkedId());

  palette = ui.optSlideLblBackColour->palette();
  colour = palette.color(QPalette::Window);
  settings->setValue("slideBackColour", colour);
  settings->endGroup();
  settings->endGroup(); //group App

  //qDebug() << "KbvOptions::saveElements" << colour; //###########

  settings->sync();
  settings->setvaluesChanged();
}

/*************************************************************************//*!
 * Presets dialog elements with stored values.
 * Note: the default values only are used when the key is missing in config-file.
 * When the config-file doesn't exist, settings contains default values.
 */
void  KbvOptions::presetElements()
{
  int       id;
  QString   str;
  QPalette  palette;
  QColor    colour;

  //Tab General
  settings->beginGroup("App");
  settings->beginGroup("Options-General");
  str = settings->value("dataBaseDir", databaseHome).toString();

  ui.optGenLineEditDatabase->setText(str);
  settings->endGroup();

  //Tab View
  settings->beginGroup("Options-View");
  ui.optViewSizeSlider->setValue(settings->value("iconSize", KbvConf::stdIconSize).toInt());
  ui.optViewDirWatchCycle->setValue(settings->value("dirWatchCycle", 5).toInt());
  ui.optCheckHiddenFiles->setChecked(settings->value("showHiddenFiles", false).toBool());
  ui.optCheckSaveDirTreeState->setChecked(settings->value("saveDirTreeState", false).toBool());

  ui.optViewFileInfoName->setChecked(settings->value("showFileName", true).toBool());
  ui.optViewFileInfoSize->setChecked(settings->value("showFileSize", false).toBool());
  ui.optViewFileInfoDim->setChecked(settings->value("showImageSize", false).toBool());
  settings->endGroup();

  //Tab Slide Show
  settings->beginGroup("Options-Slide");
  ui.optDiaCheckManual->setChecked(settings->value("checkManual", true).toBool());
  ui.optDiaCheckFullScreen->setChecked(settings->value("checkFullScreen", false).toBool());
  ui.optDiaCheckStretch->setChecked(settings->value("checkStretch", false).toBool());
  ui.optDiaStopAtDirEnd->setChecked(settings->value("checkStopAtDirEnd", true).toBool());
  ui.optDiaValueDelay->setValue(settings->value("slideDelay", 2).toInt());

  id = (settings->value("slideChangeId", KbvConf::slideReplace).toInt());
  ui.optGroupSlideChange->button(id)->setChecked(true);

  colour = settings->value("slideBackColour", QColor(Qt::black)).value<QColor>();
  palette = ui.optSlideLblBackColour->palette();
  palette.setColor(QPalette::Window, colour);
  ui.optSlideLblBackColour->setPalette(palette);
  settings->endGroup();
  settings->endGroup(); //group App
  //qDebug() << "KbvOptions::presetElements" << colour; //###########
}
/****************************************************************************/
