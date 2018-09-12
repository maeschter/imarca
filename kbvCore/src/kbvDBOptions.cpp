/*****************************************************************************
 * kbv dboptions dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-02-17 20:55:59 +0100 (Fr, 17. Feb 2017) $
 * $Rev: 1151 $
 * Created: 2009.02.18
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvDBOptions.h"
#include "kbvConstants.h"
#include "kbvInformationDialog.h"

extern  KbvInformationDialog    *informationDialog;

KbvDBOptions::KbvDBOptions(QWidget *parent) : QDialog(parent)
{
  this->setWindowTitle(QString(tr("Database options")));
  ui.setupUi(this);

  ui.dbNameLbl->setText(QString(tr("Name")));
  ui.dbCommentLbl->setText(QString(tr("Description")));
  ui.dbSliderLbl->setText(QString(tr("Icon size")));
  ui.dbKeyCheck1->setText(QString(tr("From words and numbers in file name")));
  ui.dbKeyCheck2->setText(QString(tr("From words in file name (ignore numbers)")));
  ui.dbKeyCheck3->setText(QString(tr("Include file type")));
  ui.dbKeyWordLbl->setText(QString(tr("Key words")));
  ui.dbTypeButt2->setText(QString(tr("Collection")));
  ui.dbTypeButt1->setText(QString(tr("Album")));
  ui.dbTypeLbl->setText(QString(tr("Type")));
  ui.dbRootLbl->setText(QString(tr("Collection root dir")));
  ui.dbButtonRootDir->setText(QString(tr("Open")));

  connect(ui.dbButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(ui.dbButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(ui.dbSizeSlider,SIGNAL(sliderMoved(int)), this, SLOT(slider(int)));
  connect(ui.dbTypeButt1, SIGNAL(toggled(bool)), this, SLOT(typeButton(bool)));
  connect(ui.dbTypeButt2, SIGNAL(toggled(bool)), this, SLOT(typeButton(bool)));
  connect(ui.dbKeyCheck1, SIGNAL(stateChanged(int)), this, SLOT(keywordCheck(int)));
  connect(ui.dbKeyCheck2, SIGNAL(stateChanged(int)), this, SLOT(keywordCheck(int)));
  connect(ui.dbKeyCheck3, SIGNAL(stateChanged(int)), this, SLOT(keywordCheck(int)));
  connect(ui.dbButtonRootDir, SIGNAL(clicked()), this, SLOT(dialogRootDir()));

  QDir  dir;
  userHome = dir.homePath();

  halfstep = ui.dbSizeSlider->singleStep()/2;
  dbtype = Kbv::TypeAlbum;
  dbkeywordtype = Kbv::keywordNone;
  dbname = "";
  dbcomment = "";
  collrootdir = "";
  ui.dbKeyCheck2->setCheckState(Qt::Unchecked);
  ui.dbKeyCheck2->setCheckState(Qt::Checked);   //trigger for correct reading
  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);

  warntext1 = QString(tr("No valid directory!\n"));
  warntext2 = QString(tr("A collection must have a root directory."));
}
KbvDBOptions::~KbvDBOptions()
{
  //qDebug() << "KbvDBOptions::~KbvDBOptions"; //###########
}
/*************************************************************************//*!
 * Slider steps by mouse move are 25.
 */
void    KbvDBOptions::slider(int pos)
{
  if(pos<=75+halfstep)    {ui.dbSizeSlider->setValue(75);}
  if(pos>75+halfstep  && pos<=100+halfstep) {ui.dbSizeSlider->setSliderPosition(100);}
  if(pos>100+halfstep && pos<=125+halfstep) {ui.dbSizeSlider->setSliderPosition(125);}
  if(pos>125+halfstep && pos<=150+halfstep) {ui.dbSizeSlider->setSliderPosition(150);}
  if(pos>150+halfstep && pos<=175+halfstep) {ui.dbSizeSlider->setSliderPosition(175);}
  if(pos>175+halfstep && pos<=200+halfstep) {ui.dbSizeSlider->setSliderPosition(200);}
  if(pos>200+halfstep)    {ui.dbSizeSlider->setSliderPosition(225);}
}
/*************************************************************************//*!
 * Database type: album or collection.
 */
void    KbvDBOptions::typeButton(bool checked)
{
  Q_UNUSED(checked)
  if(ui.dbTypeButt1->isChecked())
    {
      dbtype = Kbv::TypeAlbum;
      ui.dbRootLbl->setEnabled(false);
      ui.dbLineEdit2->setEnabled(false);
      ui.dbButtonRootDir->setEnabled(false);
    }
  if(ui.dbTypeButt2->isChecked())
    {
      dbtype = Kbv::TypeCollection;
      ui.dbRootLbl->setEnabled(true);
      ui.dbLineEdit2->setEnabled(true);
      ui.dbButtonRootDir->setEnabled(true);
    }
}
/*************************************************************************//*!
 * Keyword rule.
 */
void    KbvDBOptions::keywordCheck(int state)
{
  Q_UNUSED(state)

  if(ui.dbKeyCheck1->isChecked())
    {
      dbkeywordtype = Kbv::keywordWordsNumbers;
    }
  if(ui.dbKeyCheck2->isChecked())
    {
      dbkeywordtype = Kbv::keywordWordsOnly;
    }
  if(ui.dbKeyCheck3->isChecked())
    {
      dbkeywordtype = dbkeywordtype | Kbv::keywordFiletype;
    }
  //qDebug() << "KbvDBOptions::keywordCheck" << dbkeywordtype; //###########

}
/*************************************************************************//*!
 * File dialog to determine the root directory of a collection.
 */
void    KbvDBOptions::dialogRootDir()
{
  QString   str;
  int       n;

  //ask for directory
  str = rootDirDialog.getExistingDirectory(this, tr("Open root directory of collection"),
                        userHome, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  //qDebug() << "KbvDBOptions::dialogRootDir" << str; //###########
  ui.dbLineEdit2->setText(str);

  //a valid dir at least contains one "/"
  n = str.lastIndexOf("/", -1, Qt::CaseInsensitive);
  if(n <= 0)
    {
      informationDialog->perform(warntext1,warntext2,1);
    }
}
/*************************************************************************//*!
 * Ok button clicked.
 */
void    KbvDBOptions::accept()
{
  collrootdir = ui.dbLineEdit2->text();
  //qDebug() << "KbvDBOptions::accept" << collrootdir; //###########

  if((dbtype & Kbv::TypeCollection) && collrootdir.isEmpty())
    {
      informationDialog->perform(warntext1,warntext2,1);
      return;
    }
  dbname = ui.dbLineEdit1->text();
  dbcomment = ui.dbTextEdit->toPlainText();
  dbicon = ui.dbSizeSlider->value();
  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);
  this->setResult(QDialog::Accepted);
  hide();
}
/*************************************************************************//*!
 * Cancel button clicked.
 */
void    KbvDBOptions::reject()
{
  dbname = "";
  dbcomment = "";
  collrootdir = "";
  ui.dbLineEdit1->setText(dbname);
  ui.dbLineEdit2->setText(collrootdir);
  ui.dbTextEdit->setPlainText(dbcomment);
  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);
  this->setResult(QDialog::Rejected);
  hide();
}
/*************************************************************************//*!
 * Dialog result: icon dimension.
 */
QString KbvDBOptions::databaseName()
{
  return dbname;
}
/*************************************************************************//*!
 * Dialog result: icon dimension.
 */
int    KbvDBOptions::iconsize()
{
  return dbicon;
}
/*************************************************************************//*!
 * Dialog result: icon dimension.
 */
int    KbvDBOptions::databaseType()
{
  return dbtype;
}
/*************************************************************************//*!
 * Dialog result: description.
 */
QString    KbvDBOptions::description()
{
  return dbcomment;
}
/*************************************************************************//*!
 * Dialog result: keyword types.
 */
int    KbvDBOptions::keywordType()
{
  return dbkeywordtype;
}
/*************************************************************************//*!
 * Dialog result: collection root dir.
 */
QString KbvDBOptions::rootDir()
{
  return collrootdir;
}/****************************************************************************/
