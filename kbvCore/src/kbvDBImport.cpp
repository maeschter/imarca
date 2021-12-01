/*****************************************************************************
 * kbv dbimport dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2016.12.08
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvDBImport.h"
#include "kbvConstants.h"
#include "kbvInformationDialog.h"

extern  KbvInformationDialog    *informationDialog;

KbvDBImport::KbvDBImport(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);

  QString title = QString(tr("Import collection"));
  informationDialog->setWindowTitle(title);
  this->setWindowTitle(title);

  ui.dbNameLbl->setText(QString(tr("Database name")));
  ui.dbTargetLbl->setText(QString(tr("Destination directory")));
  ui.dbButtonName->setText(QString(tr("Select")));
  ui.dbButtonTarget->setText(QString(tr("Select")));
  ui.dbButtonOk->setText(QString(tr("Ok")));
  ui.dbButtonCancel->setText(QString(tr("Cancel")));

  connect(ui.dbButtonOk,      SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui.dbButtonCancel,  SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui.dbButtonTarget,  SIGNAL(clicked()), this, SLOT(dialogDestination()));
  connect(ui.dbButtonName,    SIGNAL(clicked()), this, SLOT(dialogName()));

  homeDir = QDir::homePath();
  dbName = "";

  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);
  ui.dbLineEdit1->setReadOnly(true);
  ui.dbLineEdit2->setText(homeDir);
}
KbvDBImport::~KbvDBImport()
{
  //qDebug() << "KbvDBImport::~KbvDBImport"; //###########
}
/*************************************************************************//*!
 * File dialog to select a collection database file for export.
 * The destination dir gets extended with the database name.
 */
void    KbvDBImport::dialogName()
{
  QString   str;
  int       n;

  //ask database file
  str = fileDialog.getOpenFileName(this, tr("Select collection database file"),
                        homeDir, QString("*") + QString(dbNameExt));
  ui.dbLineEdit1->setText(str);

  //the path of a valid database name at least contains one "/"
  n = str.lastIndexOf("/");
  if(n<0)
    {
      informationDialog->perform(QString(tr("Database name is invalid.")), "", 1);
    }
  else
    {
      //extend destination dir
      //separate name
      dbName = str.right(str.length()-n-1);
      dbName.remove(QString(dbNameExt));
      str = ui.dbLineEdit2->text();
      if(!str.endsWith("/"))
        {
          str.append("/");
        }
      str.append(dbName);
      ui.dbLineEdit2->setText(str);
    }
}
/*************************************************************************//*!
 * File dialog to determine the destination directory for import.
 */
void    KbvDBImport::dialogDestination()
{
  QString   str;

  //ask for directory
  str = fileDialog.getExistingDirectory(this, tr("Select destination directory"),
                        homeDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  ui.dbLineEdit2->setText(str);

  //the path of a valid dir at least contains one "/"
  if(!str.contains("/"))
    {
      informationDialog->perform(QString(tr("Destination directory is invalid.")), "", 1);
    }
  else
    {
      if(!str.endsWith("/"))
        {
          str.append("/");
        }
      str.append(dbName);
      ui.dbLineEdit2->setText(str);
    }
}
/*************************************************************************//*!
 * Ok button clicked.
 */
void    KbvDBImport::accept()
{
  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);
  this->setResult(QDialog::Accepted);
  hide();
}
/*************************************************************************//*!
 * Cancel button clicked.
 */
void    KbvDBImport::reject()
{
  dbName="";
  ui.dbLineEdit1->setText("");
  ui.dbLineEdit2->setText(homeDir);
  ui.dbLineEdit1->setFocus(Qt::ActiveWindowFocusReason);
  this->setResult(QDialog::Rejected);
  hide();
}
/*************************************************************************//*!
 * Dialog result: description.
 */
QString    KbvDBImport::destination()
{
  return ui.dbLineEdit2->text();
}
/*************************************************************************//*!
 * Dialog result: database name.
 */
QString KbvDBImport::databaseName()
{
  return ui.dbLineEdit1->text();;
}
/****************************************************************************/
