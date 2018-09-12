/*****************************************************************************
 * kbv data record dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-02-22 18:20:30 +0100 (Mi, 22. Feb 2017) $
 * $Rev: 1156 $
 * Created: 2013.12.03
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 * Description:
 * KbvRecordInfo displays the record values beginning with the first of the
 * selected items. KbvRecordInfo must be opened by context menu of the view.
 * When more than one items are selected they can be opened one after the other
 * in order of selection.
 * User date and comment as well as keywords are editable. When the 'save' or
 * 'addToAll' key was pressed user date and comment and the keywords are stored
 * in the related DB. The 'discard' key closes the dialog without storing.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvRecordInfo.h"

extern  KbvSetvalues    *settings;

KbvRecordInfo::KbvRecordInfo(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);
  this->setWindowTitle(QString(tr("Data record")));

  ui.checkUserDate->setChecked(false);
  ui.checkUserDate->setEnabled(false);
  ui.editUserDate->setEnabled(false);
  ui.editUserKeywords->setEnabled(false);
  ui.editUserComment->setEnabled(false);
  ui.lblRecCRC32->setText(QString(tr("CRC32 check sum")));
  ui.lblRecDateChanged->setText(QString(tr("Last modification")));
  ui.lblRecFileName->setText(QString(tr("File name")));
  ui.lblRecFilePath->setText(QString(tr("File path")));
  ui.lblRecFileSize->setText(QString(tr("File size")));
  ui.lblRecImageSize->setText(QString(tr("Image size")));
  ui.lblRecKeywords->setText(QString(tr("Import keywords")));
  ui.lblUserKeywords->setText(QString(tr("User keywords")));
  ui.lblUserComment->setText(QString(tr("User comment")));
  ui.lblUserDate->setText(QString(tr("User date")));
  ui.pushButtonToday->setText(QString(tr("Today")));
  ui.editUserDate->setToolTip(QString(tr("The nonsense date 3333-3-3 indicates an unset user date.\n"
                                            "Choose a date between 1752-09-14 and 3999-12-31")));
  this->setFocusPolicy(Qt::StrongFocus);
  
  connect(ui.checkUserDate,     SIGNAL(stateChanged(int)),    this, SLOT(enableUserDate(int)));
  connect(ui.editUserDate,      SIGNAL(dateChanged(QDate)),   this, SLOT(userDate(QDate)));
  connect(ui.editUserComment,   SIGNAL(textChanged()),        this, SLOT(userComment()));
  connect(ui.editUserKeywords,  SIGNAL(textChanged()),        this, SLOT(userKeywords()));

  connect(ui.pushButtonDiscard,  SIGNAL(clicked(bool)),  this, SLOT(discard(bool)));
  connect(ui.pushButtonSaveAdd,  SIGNAL(clicked(bool)),  this, SLOT(addToAllRecords(bool)));
  connect(ui.pushButtonSave,     SIGNAL(clicked(bool)),  this, SLOT(saveRecordData(bool)));
  connect(ui.pushButtonNext,     SIGNAL(clicked(bool)),  this, SLOT(next(bool)));
  connect(ui.pushButtonPrevious, SIGNAL(clicked(bool)),  this, SLOT(previous(bool)));
  connect(ui.pushButtonToday,    SIGNAL(clicked(bool)),  this, SLOT(today(bool)));
  
  ui.pushButtonDiscard->setToolTip(QString(tr("Close without saving")));
  ui.pushButtonSaveAdd->setToolTip(QString(tr("Add user keywords/date/comment to all selected records")));
  ui.pushButtonSave->setToolTip(QString(tr("Save user keywords/date/comment in record")));
  ui.pushButtonNext->setToolTip(QString(tr("Next record")));
  ui.pushButtonPrevious->setToolTip(QString(tr("Previous record")));
  
  nonsenseDate = QDate(3333, 3, 3);
}
KbvRecordInfo::~KbvRecordInfo()
{
  //qDebug() << "KbvRecordInfo::~KbvRecordInfo"; //###########
  //remove database connection
  infoDB = QSqlDatabase();                    //destroy db-object
  QSqlDatabase::removeDatabase(dbconnection);  //close connection
}

/*************************************************************************//*!
 * Execute modal dialog.
 * Each open collection owns one recordInfo. Once opened the dialog establishes
 * a database connection and uses this connection when closed and opened again.
 * On search there's only one recordInfo in searchTab. When an other collection
 * get searched we have to destroy the previous database and open a new one.
 * The last database get closed when recordInfo get destroyed.
 */
int     KbvRecordInfo::perform(QList<QVariant> pkl, QString db, QString dbPath)
{
  //Create a new database connection or use the existing again.
  if(dbName != db)
    {
      //close existing db
      //qDebug() << "KbvRecordInfo::setRecordInfo close db" <<dbName; //###########
      //qDebug() << "KbvRecordInfo::setRecordInfo   new db" <<db; //###########
      infoDB = QSqlDatabase();                    //destroy previous db-object
      QSqlDatabase::removeDatabase(dbconnection);  //close connection

      //create new connection
      dbName = db;
      dbconnection = "info"+dbName;
      infoDB = QSqlDatabase::addDatabase("QSQLITE", dbconnection);
      infoDB.setHostName("host");
      infoDB.setDatabaseName(dbPath+dbName+QString(dbNameExt));
      infoDB.open();
    }
  pkList = pkl;
  indexNo = 0;

  ui.editUserKeywords->setEnabled(true);
  ui.editUserComment->setEnabled(true);
  ui.checkUserDate->setEnabled(true);
  ui.checkUserDate->setChecked(false);
  ui.pushButtonNext->setEnabled(true);
  ui.pushButtonPrevious->setEnabled(false);

  readRecord(pkList.at(indexNo));
  updateRecordInfo();

  return QDialog::exec();
}

/*************************************************************************//*!
 * The map always contains record data where user comment or user date may be
 * empty. The parameter 'firstlast' indicates that the first or last item has
 * been met (values: Kbv::first or Kbv::last).
 * When the user date (as Julian day) is empty, a nonsense date is displayed to
 * indicate this (see tooltip). When the results gets fetched, an invalid date
 * gets returned when the nonsense date was found.
 */
void    KbvRecordInfo::updateRecordInfo()
{
  QByteArray    ba;
  QDateTime     datetime;
  int           mod, h, m, s;
  bool          ok;
  
  if(!dataSet.isEmpty())
    {
      ba = dataSet.value("icon").toByteArray();
      recIcon.loadFromData(ba, 0, Qt::AutoColor);
    
      //record from database
      //qDebug() << "KbvRecordInfo::setRecordInfo data found"; //###########
      recName = dataSet.value("name").toString();
      recPath = dataSet.value("path").toString();
      recImageSize = QString("%1 x ").arg(dataSet.value("imageW").toInt(), 0, 10);//width
      recImageSize += QString("%1").arg(dataSet.value("imageH").toInt(), 0, 10);  //height
      recFileSize = QString("%L1").arg(dataSet.value("filesize").toUInt(), 0, 10) + QString(" bytes");
      recCRC32 =    QString("%1").arg(dataSet.value("crc32").toUInt(), 0, 16);
      recUserComment = dataSet.value("usercomment").toString();


      if(0 == dataSet.value("userdate").toInt(&ok))
        {
          recUserdate = nonsenseDate;
        }
      else
        {
          recUserdate = QDate::fromJulianDay(dataSet.value("userdate").toInt(&ok));
        }
      //qDebug() << "KbvRecordInfo::updateRecordInfo recUserdate" << recUserdate; //###########
    
      //replace the 'keywordBoundary' by spaces then reduce to one space
      recKeywords = dataSet.value("keywords").toString();
      recKeywords.replace(QString(keywordBoundary), " ", Qt::CaseInsensitive);
      recKeywords.replace(QRegExp("\\s+"), " ");

      //replace the 'keywordBoundary' by spaces then reduce to one space
      recUserKeywords = dataSet.value("userkeywords").toString();
      recUserKeywords.replace(QString(keywordBoundary), " ", Qt::CaseInsensitive);
      recUserKeywords.replace(QRegExp("\\s+"), " ");

    
      datetime.setDate(QDate::fromJulianDay(dataSet.value("datechanged").toInt()));
      mod = dataSet.value("timechanged").toInt();

      h = mod / 3600;
      mod = mod % 3600;
      m = mod / 60;
      s = mod % 60;
      datetime.setTime(QTime(h, m, s));
      recDateChanged = datetime.toString("yyyy-MM-dd  hh:mm:ss");
      
      ui.showRecFileName->setText(recName);
      ui.showRecFilePath->setText(recPath);
      ui.showRecImageSize->setText(recImageSize);
      ui.showRecDateChanged->setText(recDateChanged);
      ui.showRecFileSize->setText(recFileSize);
      ui.showRecCRC32->setText(recCRC32);
      ui.showRecIcon->setPixmap(recIcon);
      ui.showRecKeywords->setText(recKeywords);
      ui.editUserKeywords->setPlainText(recUserKeywords);
      ui.editUserComment->setPlainText(recUserComment);
      ui.editUserDate->setDate(recUserdate);
      
      ui.showRecFilePath->setToolTip(QString("%1").arg(recPath));
    }
}
/*************************************************************************//*!
 * Read record from database and store all values in the map.
 */
void    KbvRecordInfo::readRecord(QVariant pk)
{
  QSqlQuery   query;
  QString     stmt;
  bool        ok;

  stmt = QString("SELECT fileName,filePath,icon,imageW,imageH,fileSize,crc32,"
                 "dateChanged,timeChanged,userDate,userComment,keywords,"
                 "userKeywords FROM album WHERE pk = %1").arg(pk.toLongLong(&ok));
  infoDB.transaction();
  query.setForwardOnly(true);
  query = infoDB.exec(stmt);
  dataSet.clear();
  if(query.first())
    {
      //path and name alredy are in the map
      dataSet.insert("name",        query.value(0));   //text
      dataSet.insert("path",        query.value(1));   //text
      dataSet.insert("icon",        query.value(2));   //BLOB
      dataSet.insert("imageW",      query.value(3));   //integer
      dataSet.insert("imageH",      query.value(4));   //integer
      dataSet.insert("filesize",    query.value(5));   //integer
      dataSet.insert("crc32",       query.value(6));   //integer
      dataSet.insert("datechanged", query.value(7));   //integer
      dataSet.insert("timechanged", query.value(8));   //integer
      dataSet.insert("userdate",    query.value(9));   //integer
      dataSet.insert("usercomment", query.value(10));  //text
      dataSet.insert("keywords",    query.value(11));  //text
      dataSet.insert("userkeywords",query.value(12));  //text
      //qDebug() << "KbvRecordInfo::readRecord userdate" << QDate::fromJulianDay(data.value("userdate").toInt(&ok)); //###########
    }
  else
    {
      dataSet.insert("icon", QVariant(generalFunc.iconKbvNoSupport));
    }
  infoDB.commit();
}
/*************************************************************************//*!
 * Update record with user inputs.
 * TABLE "album" ("userKeyWords" TEXT,"userComment" TEXT,"userDate" INTEGER)
 */
void    KbvRecordInfo::updateRecord(QVariant pk)
{
  QSqlQuery     query;
  QString       stmt1;
  bool          ok;

  infoDB.transaction();
  query = QSqlQuery(infoDB);
  stmt1 = QString("UPDATE OR REPLACE album SET userKeywords = :userKeywords,"
                  "userComment = :userComment, userDate = :userDate "
                  "WHERE pk = %1").arg(pk.toLongLong(&ok));

  //Update record on valid primary key. Empty fields get cleared!
  if(ok)
    {
      query.prepare(stmt1);
      query.bindValue(":userKeywords",dataSet.value("userkeywords"));
      query.bindValue(":userComment", dataSet.value("usercomment"));
      query.bindValue(":userDate",    dataSet.value("userdate"));
      query.exec();
      //qDebug() << "KbvRecordInfo::updateRecord " << query.lastError(); //###########
    }
  infoDB.commit();
}
/*************************************************************************//*!
 * Slots: enable setting of user date and handle users actions.
 */
void    KbvRecordInfo::enableUserDate(int state)
{
  if(state == Qt::Checked)
    {
      ui.editUserDate->setEnabled(true);
    }
  else
    {
      ui.editUserDate->setEnabled(false);
    }
}
/*************************************************************************//*!
 * Slot: User keywords have been changed
 */
void    KbvRecordInfo::userKeywords()
{
  this->recUserKeywords = ui.editUserKeywords->toPlainText();
}
/*************************************************************************//*!
 * Slot: User date has been changed
 */
void    KbvRecordInfo::userDate(QDate date)
{
  this->recUserdate = date;
}
/*************************************************************************//*!
 * Slot: User comment has been changed
 */
void    KbvRecordInfo::userComment()
{
  this->recUserComment = ui.editUserComment->toPlainText();
}
/*************************************************************************//*!
 * TODAY button clicked.
 */
void    KbvRecordInfo::today(bool checked)
{
  Q_UNUSED(checked)

  if(ui.checkUserDate->isChecked())
    {
      recUserdate = QDate::currentDate();
      ui.editUserDate->setDate(recUserdate);  
    }
}
/*************************************************************************//*!
 * DISCARD button clicked. Close the dialog
 */
void    KbvRecordInfo::discard(bool checked)
{
  Q_UNUSED(checked)
  setResult(QDialog::Rejected);
  close();
}
/*************************************************************************//*!
 * Close dialog. This hides the widget, but does not delete it.
 */
void    KbvRecordInfo::closeEvent(QCloseEvent *event)
{
  //qDebug() << "KbvRecordInfo::closeEvent"; //###########
  event->accept();
}

/*************************************************************************//*!
 * SAVE-ADD button clicked.
 * Add actual user related keywords, date and comment to all selected records
 * and keep the previous values.
 * The user date only gets considered when valid. When invalid an empty date
 * is used which will not change the value in the database.
 */
void    KbvRecordInfo::addToAllRecords(bool checked)
{
  Q_UNUSED(checked)
  QString     kwds, uc, uk;
  QStringList kwdslist;

  //reduce spaces to one space between keywords
  kwds = recUserKeywords.replace(QRegExp("[,;.:|]"), " ");
  kwds = kwds.replace(QRegExp("\\s+"), " ");
  kwdslist = kwds.split(" ", QString::SkipEmptyParts);
  kwds.clear();
  for(int i=0; i<kwdslist.size();i++)
    {
      kwds.append(QString(keywordBoundary) + kwdslist.at(i) + QString(keywordBoundary));
    }

  //process all records of selection
  for(int i=0; i<pkList.length(); i++)
    {
      dataSet.clear();
      readRecord(pkList.at(i));

      uk = dataSet.value("userkeywords").toString() + kwds;
      uc = dataSet.value("usercomment").toString() + recUserComment;

      dataSet.insert("usercomment", QVariant(uc));
      dataSet.insert("userkeywords", QVariant(uk));
      if(recUserdate != nonsenseDate)
        {
          dataSet.insert("userdate",  QVariant(recUserdate.toJulianDay()));
        }
      else
        {
          dataSet.insert("userdate",  QVariant());
        }
      //qDebug() << "KbvRecordInfo::saveToAllRecords" <<uk <<userdate <<uc; //###########
      this->updateRecord(pkList.at(i));
    }
}
/*************************************************************************//*!
 * SAVE button clicked.
 * Store actual record data in database. User date and comment as well as
 * the keywords may have been changed.
 * The user date only gets considered when valid. When invalid an empty date
 * is used which will not change the value in the database.
 */
void    KbvRecordInfo::saveRecordData(bool checked)
{
  Q_UNUSED(checked)
  QString     kwds;
  QStringList kwdslist;

  //reduce spaces to one space between keywords
  kwds = recUserKeywords.replace(QRegExp("[,;.:|]"), " ");
  kwds = kwds.replace(QRegExp("\\s+"), " ");
  kwdslist = kwds.split(" ", QString::SkipEmptyParts);
  kwds.clear();
  for(int i=0; i<kwdslist.size();i++)
    {
      kwds.append(QString(keywordBoundary) + kwdslist.at(i) + QString(keywordBoundary));
    }

  dataSet.clear();
  dataSet.insert("usercomment",  QVariant(recUserComment));
  dataSet.insert("userkeywords", QVariant(kwds));
  if(recUserdate != nonsenseDate)
    {
      dataSet.insert("userdate",  QVariant(recUserdate.toJulianDay()));
    }
  else
    {
      dataSet.insert("userdate",  QVariant());
    }
  //qDebug() << "KbvRecordInfo::saveRecordData" <<recUserKeywords <<recUserdate <<recUsercomment; //###########
  updateRecord(pkList.at(indexNo));
}
/*************************************************************************//*!
 * NEXT button clicked.
 * When the last record was met while stepping forward, the 'next' button gets
 * disabled and the index number is kept on last index. This allows stepping
 * in the opposite direction without additional mouse clicks.
 */
void    KbvRecordInfo::next(bool checked)
{
  Q_UNUSED(checked)

  indexNo++;
  ui.pushButtonPrevious->setEnabled(true);
  //qDebug() << "KbvRecordInfo::next"; //###########
  if(indexNo < pkList.size())
    {
      readRecord(pkList.at(indexNo));
      updateRecordInfo();
      if(indexNo == (pkList.size()-1))
        {
          //last position
          ui.pushButtonNext->setEnabled(false);
        }
    }
  else
    {
      //keep index at last position and keep info data
      indexNo = pkList.size()-1;
    }
}
/*************************************************************************//*!
 * PREVIOUS button clicked.
 * When the first record was met while stepping backward, the 'previous' button
 * gets disabled and the index number is kept on first index. This allows
 * stepping in the opposite direction without additional mouse clicks.
 */
void    KbvRecordInfo::previous(bool checked)
{
  Q_UNUSED(checked)

  indexNo--;
  ui.pushButtonNext->setEnabled(true);
  //qDebug() << "KbvRecordInfo::previous"; //###########
  if(indexNo >= 0)
    {
      readRecord(pkList.at(indexNo));
      updateRecordInfo();
      if(indexNo == 0)
        {
          //first position
          ui.pushButtonPrevious->setEnabled(false);
        }
    }
  else
    {
      //keep index at first position and keep info data
      indexNo = 0;
    }
}
/*************************************************************************//*!
 * Key press event. Evaluated are arrow left/right keys as keyboard inputs
 * for NEXT/PREVIOUS selection.
 */
void    KbvRecordInfo::keyPressEvent(QKeyEvent *event)
{
  //qDebug() << "KbvRecordInfo::keyPressEvent"; //###########
  
  if(event->key() == Qt::Key_Left)
    {
      this->previous(true);
    }
  if(event->key() == Qt::Key_Right)
    {
      this->next(true);
    }
  QDialog::keyPressEvent(event);
}
/****************************************************************************/
