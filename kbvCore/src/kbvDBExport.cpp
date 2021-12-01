/*****************************************************************************
 * kbv dbexport dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2016.12.08
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvSetvalues.h"
#include "kbvInformationDialog.h"
#include "kbvConstants.h"
#include "kbvDBExport.h"

extern  KbvSetvalues          *settings;
extern  KbvInformationDialog  *informationDialog;

KbvDBExport::KbvDBExport(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);

  QString title = QString(tr("Export collection"));
  informationDialog->setWindowTitle(title);
  this->setWindowTitle(title);

  ui.dbNameLbl->setText(QString(tr("Database name")));
  ui.dbTargetLbl->setText(QString(tr("Export destination")));
  ui.dbButtonTarget->setText(QString(tr("Select")));
  ui.dbButtonApply->setText(QString(tr("Apply")));
  ui.dbButtonCancel->setText(QString(tr("Cancel")));

  ui.dbComboBox->setFocus(Qt::ActiveWindowFocusReason);

  connect(ui.dbButtonApply,   SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui.dbButtonCancel,  SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui.dbButtonTarget,  SIGNAL(clicked()), this, SLOT(dialogDestination()));
  connect(ui.dbComboBox,      SIGNAL(activated(int)), this, SLOT(comboDBActivated(int)));

  //default values
  targetDir = QDir::homePath() + "/";
}
KbvDBExport::~KbvDBExport()
{
  //qDebug() << "KbvDBExport::~KbvDBExport"; //###########
}

/*************************************************************************//*!
 * Get a collection database from combobox.
 */
void    KbvDBExport::comboDBActivated(int index)
{
  dbName = ui.dbComboBox->itemText(index);
  ui.dbEditTarget->setText(targetDir + dbName);
}
/*************************************************************************//*!
 * Select button clicked.
 * Open file dialog to determine the destination directory for export.
 */
void    KbvDBExport::dialogDestination()
{
  QString   str;

  //ask for target directory
  str = fileDialog.getExistingDirectory(this, tr("Select destination directory"),
                        QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if(!str.endsWith("/") && !str.isEmpty())
    {
      str.append("/");
    }
  targetDir = str;

  str.append(dbName);
  ui.dbEditTarget->setText(str);

  //the path of a valid dir at least contains one "/"
  if(!str.contains("/"))
    {
      informationDialog->perform(QString(tr("Destination directory is invalid.")), "", 1);
      targetDir = QDir::homePath();
    }
}
/*************************************************************************//*!
 * SLOT: Show modal dialog.
 */
int    KbvDBExport::perform(const QStringList dblist)
{
  ui.dbComboBox->addItems(dblist);
  dbName = dblist.at(0);                        //default
  ui.dbEditTarget->setText(targetDir + dbName); //default

  return QDialog::exec();
}

/*************************************************************************//*!
 * Apply button clicked.
 * Destination dir and database name have been set.
 */
void    KbvDBExport::accept()
{
  this->setResult(QDialog::Accepted);
  //qDebug() << "KbvDBExport::accept" <<dbName <<targetDir <<exportCD; //###########
  hide();
}
/*************************************************************************//*!
 * Cancel button clicked.
 */
void    KbvDBExport::reject()
{
  this->setResult(QDialog::Rejected);
  //qDebug() << "KbvDBExport::reject" <<dbName <<targetDir <<exportCD; //###########
  hide();
}
/*************************************************************************//*!
 * Dialog result: destination.
 */
QString    KbvDBExport::destination()
{
  //read target dir from lineEdit to get manual changes too
  return ui.dbEditTarget->text();
}
/*************************************************************************//*!
 * Dialog result: database name without extension.
 */
QString KbvDBExport::databaseName()
{
  return dbName;
}
/****************************************************************************/
