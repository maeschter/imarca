/*****************************************************************************
 * kbv search dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2012.08.16
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvDBSearchDialog.h"

KbvDBSearchDialog::KbvDBSearchDialog(QWidget *parent) : QDialog(parent)
{
  date = QDate(0,0,0); //invalid date = 0

  this->setWindowTitle(QString(tr("Search")));
  ui.setupUi(this);
  ui.searchTabs->setTabText(ui.searchTabs->indexOf(ui.tabKeyword), QString(tr("Terms")));
  ui.searchTabs->setTabText(ui.searchTabs->indexOf(ui.tabDate), QString(tr("Date")));

  ui.lblKeywords->setText(QString(tr("Search terms")));
  ui.lblKeywords->setToolTip(QString(tr("Type in key words separated by spaces.")));
  ui.buttonOR->setText(QString(tr("OR")));
  ui.buttonAND->setText(QString(tr("AND")));
  ui.buttonAccurate->setText(QString(tr("ACCURATE")));
  ui.buttonInaccurate->setText(QString(tr("INACCURATE")));

  ui.lblPaths->setText(QString(tr("Search paths")));
  ui.lblKeywords->setToolTip(QString(tr("Type in complete or incomplete paths separated by spaces.")));


  ui.lblDateInfo->setText(QString(tr("Search between two dates. FROM must be equal to / earlier than TO. Format: Year-Month-Day")));
  ui.lblDateFrom->setText(QString(tr("From")));
  ui.lblDateTo->setText(QString(tr("To")));
  ui.checkDateSearch->setText(QString(tr("Enable date search")));
  ui.dateFrom->setDate(date);
  ui.dateUntil->setDate(date);
  
  ui.pushButtonAbort->setText(QString(tr("Abort")));
  ui.pushButtonApply->setText(QString(tr("Apply")));
  
  ui.searchKeywords->setFocus();

  connect(ui.pushButtonAbort, SIGNAL(clicked(bool)),              this, SLOT(finishAbort(bool)));
  connect(ui.pushButtonApply, SIGNAL(clicked(bool)),              this, SLOT(finishApply(bool)));
  connect(ui.checkDateSearch, SIGNAL(stateChanged(int)),          this, SLOT(enableDate(int)));
  connect(ui.searchKeywords,  SIGNAL(textChanged(const QString)), this, SLOT(enableTermsPaths(const QString)));
  connect(ui.searchPaths,     SIGNAL(textChanged(const QString)), this, SLOT(enableTermsPaths(const QString)));

}
KbvDBSearchDialog::~KbvDBSearchDialog()
{
  //qDebug() << "KbvDBSearchDialog::~KbvDBSearchDialog"; //###########
}
/*************************************************************************//*!
 * CLEAR button clicked.
 */
void    KbvDBSearchDialog::enableDate(int state)
{
  if(state == Qt::Checked)
    {
      ui.dateFrom->setEnabled(true);
      ui.dateUntil->setEnabled(true);
    }
  else
    {
      ui.dateFrom->setDate(date);
      ui.dateUntil->setDate(date);
      ui.dateFrom->setEnabled(false);
      ui.dateUntil->setEnabled(false);
    }
}
/*************************************************************************//*!
 * Text changed in lineEdits for keywords or paths.
 */
void    KbvDBSearchDialog::enableTermsPaths(const QString text)
{
  Q_UNUSED(text);

  if(ui.searchKeywords->text().isEmpty() && ui.searchPaths->text().isEmpty())
    {
      ui.searchKeywords->setEnabled(true);
      ui.searchPaths->setEnabled(true);
      this->searchKeywords.clear();
      this->searchPaths.clear();
    }
  else if(!ui.searchKeywords->text().isEmpty())
    {
      ui.searchPaths->setEnabled(false);
      this->searchPaths.clear();
    }
  else if(!ui.searchPaths->text().isEmpty())
    {
      ui.searchKeywords->setEnabled(false);
      this->searchKeywords.clear();
    }
}
/*************************************************************************//*!
 * SLOT: Show modal dialog.
 */
int    KbvDBSearchDialog::perform(const QString name)
{
  this->setWindowTitle(QString(tr("Find in"))+" "+name);
  ui.searchKeywords->setFocus();
  return QDialog::exec();
}
/*************************************************************************//*!
 * Return list of serach keywords.
 */
QStringList    KbvDBSearchDialog::getKeywordList()
{
  return searchKeywords;
}
/*************************************************************************//*!
 * Return list of search paths.
 */
QStringList    KbvDBSearchDialog::getPathsList()
{
  return searchPaths;
}
/*************************************************************************//*!
 * Return true if the logical combination of keywords is OR.
 */
int    KbvDBSearchDialog::getKeywordLogic()
{
  int   logic = Kbv::none;

  if(ui.buttonOR->isChecked())
    {
      logic = Kbv::logicOR;
    }
  if(ui.buttonAND->isChecked())
    {
      logic = Kbv::logicAND;
    }
  return logic;
}
/*************************************************************************//*!
 * Return true if the logical combination of keywords is OR.
 */
int    KbvDBSearchDialog::getSearchLogic()
{
  int   logic = Kbv::none;

  if(ui.buttonAccurate->isChecked())
    {
      logic = Kbv::accurate;
    }
  if(ui.buttonInaccurate->isChecked())
    {
      logic = Kbv::inaccurate;
    }
  return logic;
}
/*************************************************************************//*!
 * Return date and time for begin (index=0) and end (index=1) of search time
 * span only when date search is enabled. Otherwise return invalid dates.
 */
QDate   KbvDBSearchDialog::getDate(int index)
{
  if(ui.checkDateSearch->isChecked())
    {
      if(index == 0)
        {
          return ui.dateFrom->date();
        }
      if(index == 1)
        {
          return ui.dateUntil->date();
        }
    }
  return date;
}
/*************************************************************************//*!
 * Extract keywords as lists of strings from content of lineEdit searchKeywords.
 * Extract path from content of lineEdit searchPaths.
 * This is called when the dialog is closed with search button.
 */
void  KbvDBSearchDialog::extractParameter()
{
  QString   text;

  text = ui.searchKeywords->text();
  if(!text.isEmpty())
    {
      text.replace(QRegExp("[;.:-\\s]"), ","); //only allow comma
      searchKeywords = text.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
    }

  text = ui.searchPaths->text();
  if(!text.isEmpty())
    {
      text.replace(QRegExp("[;.:-\\s]"), ","); //only allow comma
      searchPaths = text.split(",", QString::SkipEmptyParts, Qt::CaseInsensitive);
    }
  //qDebug() << "KbvDBSearchDialog::extractParam" << searchKeywords <<searchPaths; //###########
}
/*************************************************************************//*!
 * APPLY button clicked.
 */
void  KbvDBSearchDialog::finishApply(bool checked)
{
  Q_UNUSED(checked)

  extractParameter();
  setResult(QDialog::Accepted);
  hide();
}
/*************************************************************************//*!
 * ABORT button clicked.
 */
void  KbvDBSearchDialog::finishAbort(bool checked)
{
  Q_UNUSED(checked)

  extractParameter();
  setResult(QDialog::Rejected);
  hide();
}
/****************************************************************************/
