/*****************************************************************************
 * kbv database attributes dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-04-23 17:58:29 +0200 (So, 23. Apr 2017) $
 * $Rev: 1294 $
 * Created: 2011.11.25
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 * Description:
 * kbvdbinfo displays the parameters of the database.
 * Immediately after creating the object the database parameters must be set
 * since collection and search views, models and threads need these values.
 * Except of db description these parameters are constant.
 * When the dialog is closed with the "save" button the db description is
 * to DB in table description.
 *****************************************************************************/
#include <QtDebug>
#include <QtSql>
#include "kbvConstants.h"
#include "kbvDBInfo.h"

KbvDBInfo::KbvDBInfo(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  this->setWindowTitle(QString(tr("Database attributes")));

  ui.lblDBVersion->setText(QString(tr("Version")));
  ui.lblDBType->setText(QString(tr("Type")));
  ui.lblDBIconSize->setText(QString(tr("Icon size")));
  ui.lblDBKeywordType->setText(QString(tr("Keyword type")));
  ui.lblDBName->setText(QString(tr("Name")));
  ui.lblDBRecords->setText(QString(tr("Records")));
  ui.lblDBRoot->setText(QString(tr("Root directory")));
  ui.lblDBDescription->setText(QString(tr("Description")));
  ui.editDBDescription->setEnabled(true);

  connect(ui.finishButtons,    SIGNAL(clicked(QAbstractButton*)), this, SLOT(finish(QAbstractButton*)));
  connect(ui.editDBDescription,SIGNAL(textChanged()),             this, SLOT(description()));
}
KbvDBInfo::~KbvDBInfo()
{
  //qDebug() << "KbvDBInfo::~KbvDBInfo"; //###########
}
void    KbvDBInfo::setDBInfo(int type, int iconsize, int keywordtype, QString version,
                          QString name, QString description, QString location, QString collRoot)
{
  QString       str, connection;
  QSqlDatabase  db;
  QSqlQuery     query;
  int           n=0;

  this->dbType = type;
  this->dbIconSize = iconsize;
  this->dbKeywordType = keywordtype;
  this->dbVersion = version;
  this->dbName = name;
  this->dbDescription = description;
  this->dbLocation = location;
  str.setNum(dbIconSize);
  this->dbIconDim = QString(str + " x " + str);
  
  //type of collection
  if(dbType & Kbv::TypeAlbum)
    {
      this->dbTypeCollection = QString(tr("Photo album"));
      this->collectionRootDir = QString("");
    }
  else
    {
      this->dbTypeCollection = QString(tr("Collection"));
      this->collectionRootDir = collRoot;
    }
  
  //keyword rule, same order as in kbvdboptions!
  this->dbTypeKeywords = QString(tr("no keywords"));
  if((dbKeywordType & Kbv::keywordWordsNumbers))
    {
      this->dbTypeKeywords = QString(tr("from words and numbers"));
    }
  if((dbKeywordType & Kbv::keywordWordsOnly))
    {
      this->dbTypeKeywords = QString(tr("from words only (no numbers)"));
    }
  if((dbKeywordType & Kbv::keywordFiletype))
    {
      this->dbTypeKeywords.append(QString(tr(", file type)")));
    }
  
  ui.showDBVersion->setText(dbVersion);
  ui.showDBName->setText(dbName);
  ui.showDBType->setText(dbTypeCollection);
  ui.showDBRoot->setText(collectionRootDir);
  ui.showDBKeywordType->setText(dbTypeKeywords);
  ui.showDBIconSize->setText(dbIconDim);
  ui.editDBDescription->setPlainText(dbDescription);
  //tool tip to show the complete (long) path
  ui.showDBRoot->setToolTip(QString("%1").arg(collectionRootDir));

  //create db connection and calculate the amount of records
  connection = "info"+dbName;
  db = QSqlDatabase::addDatabase("QSQLITE", connection);
  db.setHostName("host");
  db.setDatabaseName(dbLocation+dbName+QString(dbNameExt));
  
  if(db.open())
    {
      db.transaction();
      query = db.exec(QString("SELECT count(*) FROM album"));
      if(query.first())
        {
          n = query.value(0).toInt();
        }
      db.commit();
    }
  ui.showDBRecords->setNum(n);

  db = QSqlDatabase();                        //destroy db-object
  QSqlDatabase::removeDatabase(connection);   //close connection
}
/*************************************************************************//*!
 * SLOT: Database description has been altered.
 */
void    KbvDBInfo::description()
{
  this->dbDescription = ui.editDBDescription->toPlainText();
}
/*************************************************************************//*!
 * Return DB parameters. Description may be altered.
 */
int     KbvDBInfo::getType()
{
  return dbType;
}
int     KbvDBInfo::getIconSize()
{
  return dbIconSize;
}
int     KbvDBInfo::getKeyWordType()
{
  return dbKeywordType;
}
QString KbvDBInfo::getVersion()
{
  return dbVersion;
}
QString KbvDBInfo::getName()
{
  return dbName;
}
QString KbvDBInfo::getLocation()
{
  return dbLocation;
}
QString KbvDBInfo::getRootDir()
{
  return collectionRootDir;
}
QString KbvDBInfo::getDescription()
{
  return dbDescription;
}
/*************************************************************************//*!
 * SLOT: SAVE or DISCARD button clicked.
 */
void    KbvDBInfo::finish(QAbstractButton *button)
{
  if(ui.finishButtons->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
      setResult(QDialog::Accepted);
    }
  else
    {
      setResult(QDialog::Rejected);
    }
  hide();
}
/****************************************************************************/
