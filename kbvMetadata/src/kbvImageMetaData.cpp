/*****************************************************************************
 * kvb image meta data
 * kvbImageMetaData, kvbImageMetaDataIptc, kvbImageMetaDataExif, kvbImageMetaDataXMP
 * This is the common class of meta data gui.
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-24 17:30:29 +0100 (Sa, 24. Feb 2018) $
 * $Rev: 1455 $
 * Created: 2015.04.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <kbvConstants.h>
#include "kbvImageMetaData.h"

KbvImageMetaData::KbvImageMetaData(QWidget *parent) : QDialog(parent)
{
  QSizePolicy sizePol1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  sizePol1.setHorizontalStretch(0);
  sizePol1.setVerticalStretch(0);

  QSizePolicy sizePol2(QSizePolicy::Minimum, QSizePolicy::Fixed);
  sizePol2.setHorizontalStretch(0);
  sizePol2.setVerticalStretch(0);

  this->setWindowTitle(QString(tr("Image Meta Data")));
  this->setSizePolicy(sizePol1);
  this->setMinimumSize(QSize(770, 600));
  this->resize(770, 600);

  metaDataFile = new QFrame(this);
  uiMetaDataFile.setupUi(metaDataFile);
  iptcDataTab = new QWidget(this);
  uiIptcData.setupUi(iptcDataTab);
  exifDataTab = new QWidget(this);
  uiExifData.setupUi(exifDataTab);
  xmpDataTab = new QWidget(this);
  uiXmpData.setupUi(xmpDataTab);
  
  xmpDataTab->hide();

  uiExifData.exifTagTable->setToolTip(QString(tr("You only can change Photo.UserComment.")));
  uiXmpData.xmpTagTable->setToolTip(QString(tr("Changing and writing XMP metadata isn't supported yet.")));

  metaDataButt = new QWidget(this);
  uiMetaDataButt.setupUi(metaDataButt);

  iptcTemplateButt = new QWidget(this);
  uiTemplateButt.setupUi(iptcTemplateButt);
  
  iptcCountryBox = new KbvCountrySelector(this);
  iptcCountryBox->setSizePolicy(sizePol2);
  iptcCountryBox->setMinimumSize(128, 37);
  iptcCountryBox->setMaximumSize(128, 37);
  uiIptcData.iptcVLayout->insertWidget(0, iptcCountryBox);

  metaDataTab = new QTabWidget(this);
  metaDataTab->addTab(iptcDataTab, QString("IPTC Core"));
  metaDataTab->addTab(exifDataTab, QString("EXIF"));
//  metaDataTab->addTab(xmpDataTab,  QString("XMP"));
  
  metaDataVLayout = new QVBoxLayout(this);
  metaDataVLayout->setSpacing(1);
  metaDataVLayout->setContentsMargins(1, 1, 1, 1);
  metaDataVLayout->addWidget(metaDataFile);
  metaDataVLayout->addWidget(metaDataTab);
  metaDataVLayout->addWidget(metaDataButt);

  this->iptcPresetTable();  //create IPTC core table

  // uiMetaDataButt uses shortcuts to trigger push buttons by keyboard.
  // Alt + arrow right will read the next item (pushButtNext pressed).\n
  // Alt + arrow left key will read the previous item (pushButtPrevious pressed).\n
  // Alt + arrow up key will read the current item (pushButtRead pressed).\n
  // Ctrl-q will close the gui (pushButtClose pressed.\n
  // Ctrl-s will write the data into the image (pushButtWrite pressed).
  connect(uiMetaDataButt.pushButtWrite,     SIGNAL(clicked(bool)), this, SLOT(metaDataWrite(bool)));
  connect(uiMetaDataButt.pushButtRead,      SIGNAL(clicked(bool)), this, SLOT(metaDataRead(bool)));
  connect(uiMetaDataButt.pushButtNext,      SIGNAL(clicked(bool)), this, SLOT(metaDataReadNext(bool)));
  connect(uiMetaDataButt.pushButtPrevious,  SIGNAL(clicked(bool)), this, SLOT(metaDataReadPrevious(bool)));
  connect(uiMetaDataButt.pushButtClose,     SIGNAL(clicked(bool)), this, SLOT(closeMetaDataGui(bool)));

  connect(uiTemplateButt.pushButtActivate,  SIGNAL(clicked(bool)), this, SLOT(iptcTemplateActivate(bool)));
  connect(uiTemplateButt.pushButtSaveAs,    SIGNAL(clicked(bool)), this, SLOT(iptcTemplateSaveAs(bool)));
  connect(uiTemplateButt.pushButtSave,      SIGNAL(clicked(bool)), this, SLOT(iptcTemplateSave(bool)));
  connect(uiTemplateButt.pushButtOpen,      SIGNAL(clicked(bool)), this, SLOT(iptcTemplateOpen(bool)));
  connect(uiTemplateButt.pushButtClose,     SIGNAL(clicked(bool)), this, SLOT(closeMetaDataGui(bool)));

  connect(iptcCountryBox,                   SIGNAL(currentIndexChanged(int)), this, SLOT(iptcCountry(int)));
  connect(uiIptcData.iptcButtonDate,        SIGNAL(clicked(bool)), this, SLOT(iptcDate(bool)));
  connect(uiIptcData.iptcButtonGenre,       SIGNAL(clicked(bool)), this, SLOT(iptcGenre(bool)));
  connect(uiIptcData.iptcButtonScene,       SIGNAL(clicked(bool)), this, SLOT(iptcScene(bool)));
  connect(uiIptcData.iptcButtonSubject,     SIGNAL(clicked(bool)), this, SLOT(iptcSubject(bool)));
  connect(uiIptcData.iptcButtonClear,       SIGNAL(clicked(bool)), this, SLOT(iptcClear(bool)));
  connect(uiIptcData.iptcButtonCopyrightSign, SIGNAL(clicked(bool)), this, SLOT(iptcCopyrrightSign(bool)));
  connect(uiIptcData.iptcButtonSet,         SIGNAL(clicked(bool)), this, SLOT(iptcSetFromTemplate(bool)));
  connect(uiIptcData.iptcTagTable,          SIGNAL(cellChanged(int, int)), this, SLOT(iptcDataChanged(int, int)));
  connect(uiIptcData.iptcCaption,           SIGNAL(textChanged()), this, SLOT(iptcDataChanged()));

  connect(uiExifData.exifButtonClear,       SIGNAL(clicked(bool)), this, SLOT(exifClear(bool)));
  connect(uiExifData.gpsGoogleMaps,         SIGNAL(clicked(bool)), this, SLOT(gpsGoogleMaps(bool)));
  connect(uiExifData.gpsOpenStreetMap,      SIGNAL(clicked(bool)), this, SLOT(gpsOpenStreetMap(bool)));

  connect(uiXmpData.xmpButtonClear,         SIGNAL(clicked(bool)), this, SLOT(xmpClear(bool)));
  connect(uiXmpData.xmpTagTable,            SIGNAL(cellChanged(int, int)), this, SLOT(xmpDataChanged(int, int)));
  connect(uiXmpData.xmpComment,             SIGNAL(textChanged()), this, SLOT(xmpDataChanged()));

  xmlFile = new QFile();
  templateActive = "";
  errorMsg = QString(tr("File error: %1"));
  helpMsg = QString(tr("Possible reason:\nfile corrupted or no image"));
}
/*************************************************************************//*!
 * Delete all table items and xml-file;
 */
KbvImageMetaData::~KbvImageMetaData()
{
  //qDebug() << "KbvImageMetaData::~KbvImageMetaData"; //###########
  while(uiIptcData.iptcTagTable->rowCount() > 0)
    {
      uiIptcData.iptcTagTable->removeRow(0);
    }
  while(uiExifData.exifTagTable->rowCount() > 0)
    {
      uiExifData.exifTagTable->removeRow(0);
    }
  delete  xmlFile;
}
/*************************************************************************//*!
 * SLOT: close meta data gui. Don't delete the gui!
 */
void  KbvImageMetaData::closeMetaDataGui(bool checked)
{
  Q_UNUSED(checked)
  this->close();
}
/*************************************************************************//*!
 * Open editor for IPTC, EXIF and XMP meta data.
 */
/*
void  KbvImageMetaData::openMetaDataGui(QListView *ptr, int view)
{
  QString           str1, str2;
  QModelIndexList   selIndices;
  QPair<QString, QString> file;

  if(ptr == 0)
    {
      return;
    }
  //qDebug() << "KbvImageMetaData::openMetaDataEditor pointer type"<<ptr <<view; //###########
  fileList.clear();
  viewType = view;
  if(viewType & Kbv::TypeFile)
    {
      pFileView = qobject_cast<KbvFileView*>(ptr);
      selIndices = pFileView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              str1 = pFileView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              str2 = pFileView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              file.first = str1;
              file.second = str2;
              fileList.append(file);
            }
        }
    }
  else if(viewType & Kbv::TypeSearch)
    {
      pSearchView = qobject_cast<KbvSearchView*>(ptr);
      selIndices = pSearchView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              str1 = pSearchView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              str2 = pSearchView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              file.first = str1;
              file.second = str2;
              fileList.append(file);
            }
        }
    }
  else if(viewType & Kbv::TypeCollection)
    {
      pCollView = qobject_cast<KbvCollectionView*>(ptr);
      selIndices = pCollView->getSelectedItems();
      if(!selIndices.isEmpty())
        {
          for(int k=0; k<selIndices.length(); k++)
            {
              str1 = pCollView->getItemData(selIndices[k], Kbv::FilePathRole).toString();
              str2 = pCollView->getItemData(selIndices[k], Kbv::FileNameRole).toString();
              file.first = str1;
              file.second = str2;
              fileList.append(file);
            }
        }
    }
  else
    {
      return;
    }

  if(fileList.isEmpty())
    {
      return;
    }
  currentIndex = 0;
  str1 = fileList.at(currentIndex).first;
  str2 = fileList.at(currentIndex).second;
  uiMetaDataFile.fileIcon->setPixmap(QPixmap(str1+str2).scaled(Kbv::Icon75, Kbv::Icon75, Qt::KeepAspectRatio));
  uiMetaDataFile.fileName->setText(str2);
  currentFile = str1+str2;

  //qDebug() << "KbvImageMetaData::openMetaDataGui current"<<currentFile; //###########
  //Finalise dialog window:
  // Enable tabs: iptc, exif, xmp, comment.
  // Remove template buttons and add meta data buttons
  // Populate all tabs and display the gui.
  for(int i=1; i<metaDataTab->count(); i++)
    {
      this->metaDataTab->setTabEnabled(i, true);
    }
  this->metaDataTab->setTabText(0, QString("IPTC Core"));
  this->metaDataVLayout->removeWidget(iptcTemplateButt);
  iptcTemplateButt->hide();
  this->metaDataVLayout->addWidget(metaDataButt);
  metaDataButt->show();

  this->populateGui();
  this->show();
}
*/
/*************************************************************************//*!
 * Open editor for IPTC, EXIF and XMP meta data.
 */
void  KbvImageMetaData::openMetaDataGui(const QList<QPair<QString, QString> > &files)
{
  QString           str1, str2;

  //clear list of files, then append new files
  if(!fileList.isEmpty())
    {
      fileList.clear();
    }
  fileList.append(files);
  
  if(!fileList.isEmpty())
    {
      currentIndex = 0;
      str1 = fileList.at(currentIndex).first;
      str2 = fileList.at(currentIndex).second;
      uiMetaDataFile.fileIcon->setPixmap(QPixmap(str1+str2).scaled(Kbv::Icon75, Kbv::Icon75, Qt::KeepAspectRatio));
      uiMetaDataFile.fileName->setText(str2);
      currentFile = str1+str2;
    
      //qDebug() << "KbvImageMetaData::openMetaDataGui current"<<currentFile; //###########
      //Finalise dialog window:
      // Enable tabs: iptc, exif, xmp, comment.
      // Remove template buttons and add meta data buttons
      // Populate all tabs and display the gui.
      for(int i=1; i<metaDataTab->count(); i++)
        {
          this->metaDataTab->setTabEnabled(i, true);
        }
      this->metaDataTab->setTabText(0, QString("IPTC Core"));
      this->metaDataVLayout->removeWidget(iptcTemplateButt);
      iptcTemplateButt->hide();
      this->metaDataVLayout->addWidget(metaDataButt);
      metaDataButt->show();
      uiIptcData.iptcButtonSet->setEnabled(true);   //may have been disabled by template

      this->populateGui();
      //show dialog and bring it to front
      this->show();
      this->raise();
      this->activateWindow();
    }
}

/*************************************************************************//*!
 * Clear gui read meta data from current file and populate all tabs.
 */
void  KbvImageMetaData::populateGui()
{
  this->iptcClearGui();

  //qDebug() << "KbvImageMetaData::populateGui"<<currentFile; //###########
  if(this->metaDataFromFile(currentFile))
    {
      this->iptcDataToGui();
      this->exifDataToGui();
      this->gpsDataToGui();
      this->xmpDataToGui();
      this->commentToGui();
    }
  iptcChanged = false;
  xmpChanged = false;
}

/*************************************************************************//*!
 * SLOT: read meta data of current file.
 */
void  KbvImageMetaData::metaDataRead(bool checked)
{
  Q_UNUSED(checked)
  QString str1, str2;

  if(currentIndex>=0 && currentIndex<fileList.length())
    {
      str1 = fileList.at(currentIndex).first;
      str2 = fileList.at(currentIndex).second;
      currentFile = str1+str2;
      uiMetaDataFile.fileIcon->setPixmap(QPixmap(currentFile).scaled(Kbv::Icon75, Kbv::Icon75, Qt::KeepAspectRatio));
      uiMetaDataFile.fileName->setText(str2);
      //qDebug() << "KbvImageMetaData::readData"<<currentIndex <<str2; //###########
      this->populateGui();
    }
}
/*************************************************************************//*!
 * SLOT: read meta data of next file.
 */
void  KbvImageMetaData::metaDataReadNext(bool checked)
{
  Q_UNUSED(checked)
  QString str1, str2;

  if(currentIndex + 1 < fileList.length())
    {
      currentIndex++;
      str1 = fileList.at(currentIndex).first;
      str2 = fileList.at(currentIndex).second;
      currentFile = str1+str2;
      uiMetaDataFile.fileIcon->setPixmap(QPixmap(currentFile).scaled(Kbv::Icon75, Kbv::Icon75, Qt::KeepAspectRatio));
      uiMetaDataFile.fileName->setText(str2);
      //qDebug() << "KbvImageMetaData::readNextData"<<currentIndex <<str2; //###########
      this->populateGui();
    }
}
/*************************************************************************//*!
 * SLOT: read meta data of previous file.
 */
void  KbvImageMetaData::metaDataReadPrevious(bool checked)
{
  Q_UNUSED(checked)
  QString str1, str2;

  if(currentIndex - 1 >= 0)
    {
      currentIndex--;
      str1 = fileList.at(currentIndex).first;
      str2 = fileList.at(currentIndex).second;
      currentFile = str1+str2;
      uiMetaDataFile.fileIcon->setPixmap(QPixmap(currentFile).scaled(Kbv::Icon75, Kbv::Icon75, Qt::KeepAspectRatio));
      uiMetaDataFile.fileName->setText(str2);
      //qDebug() << "KbvImageMetaData::readPreviousData"<<currentIndex <<str2; //###########
      this->populateGui();
    }
}
/*************************************************************************//*!
 * SLOT: write meta data into current file. Trigger view to update it's
 * content (new file size) and related database if required.
 * Writing metadata must follow the guide:
 * http://www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
 * Creator: Write Core & Extension or convert from IPTC-IIM
 *          If IPTC-IIM and XMP are both written, create the checksum
 * Changer: If IPTC-IIM is already in the file, write back XMP and IPTC-IIM
 *          otherwise only XMP SHOULD be written.
 *          Use Coded Character Set (1:90) as UTF-8
 *          If IPTC-IIM and XMP are both present, create or update the checksum
 *          If no metadata are present, act as creator
 * The checksum over the entire IPTC-IIM block must be stored in the Photoshop
 * Image Resource 1061 (photoshop files) as 16 byte MD5 hash.
 * This app may act as creator or as changer but doesn't support a checksum
 * and doesn't convert from IPTC-IIM to IPTC core/extended.
 * For detection of changes a md5-hash is taken after reading and before writing.
 * Writing IPTC and/or XMP get handled as follows:
 * If neither IPTC nor XMP data have been changed: write back unaltered.
 * If IPTC data have changed and are present, update XMP data (if present)
 * then write back both.
 * If IPTC data are not present, write only XMP (if present).
 * In Exif data only photo.user.comment is changeable. All other data are
 * written back unaltered.
 */
void  KbvImageMetaData::metaDataWrite(bool checked)
{
  Q_UNUSED(checked)
  //qDebug() << "KbvImageMetaData::writeData"; //###########

  //TODO: changing XMP data and writing to XMP (IPTC, EXIF) is not supported
  this->iptcGuiToIptcData();
  this->exifGuiToExifData();
  //this->xmpGuiToXmpData();
  //TODO: 4th tab for jpeg comment still missing
  //this->commentGuiToData();

  //Handle IPTC and XMP
  if(metaData.writeToFile(currentFile))
    {
      //qDebug() << "KbvImageMetaData::written to"<<currentFile; //###########
      //TODO: update related view and database
    }
  else
    {
      //Display warning on file error
      QMessageBox::warning(this, errorMsg.arg(currentFile), helpMsg);
    }
}
/*************************************************************************//*!
 * Read meta data from file 'filePath' and store iptc, exif, xmp, comments
 * in 'metaData'. For IPTC and XMP crc32-hashes are calculated.
 * Returns true when file could be read.
 */
bool  KbvImageMetaData::metaDataFromFile(const QString &filePath)
{
  //qDebug() << "KbvImageMetaData::metaDataFromFile"<<filePath; //###########

  //Load IPTC, Exif, XMP meta data and jfif comment from image file
  if(!metaData.loadFromFile(filePath))
    {
      //Display warning on file error
      QMessageBox::warning(this, errorMsg.arg(filePath), helpMsg);
      return false;
    }
  return true;
}

/*************************************************************************//*!
 * Read image comment into GUI.\n
 * Image comments are located in the COM marker segment of an jpeg image.
 * Jpeg comment may be up to 65533 bytes of pure ASCII text.
 */
void  KbvImageMetaData::commentToGui()
{
  //TODO: reading jpeg comment is not supported ('comment' tab still missing)
  QString           str;

  if(metaData.hasComments())
    {
      str = metaData.getCommentsDecoded();
      //uiContactData.jpgComment->setPlainText(str);
    }
}
/*************************************************************************//*!
 * Copy content of image comment field meta data container.
 */
void  KbvImageMetaData::commentGuiToData()
{
  //TODO: writing jpeg comment is not supported ('comment' tab still missing)
  QString           str;

    //str = uiContactData.jpgComment->toPlainText();
    metaData.setComments(str);
}
/*************************************************************************//*!
 * SLOT: Triggered when item data in iptc table or the caption (2:120) have
 * been changed. Note: changing the table delivers the affected row and column
 * but changing the caption delivers default values row=-1 and column=-1.
 */
void  KbvImageMetaData::iptcDataChanged(int row, int column)
{
  Q_UNUSED(row)
  Q_UNUSED(column)
  //qDebug() << "KbvImageMetaData::iptcDataChanged"<<row <<column <<iptcChanged; //###########
  iptcChanged = true;
}
/*************************************************************************//*!
 * SLOT: Triggered when item data in xmp table or comment have been changed.
 * Note: changing the table delivers the affected row and column but changing
 * the comment delivers default values row=-1 and column=-1.
 */
void  KbvImageMetaData::xmpDataChanged(int row, int column)
{
  Q_UNUSED(row)
  Q_UNUSED(column)
  //qDebug() << "KbvImageMetaData::xmpDataChanged"<<row <<column <<xmpChanged; //###########
  xmpChanged = true;
}
/*************************************************************************//*!
 * Static function:
 * Extract the iptc meta data of 'filePath' and return them in 'info'.
 * Get IPTC meta data due to core schema 1.2 spec oct. 2014 and display a subset
 * of IIM Application2 records in this order:
 * 2:05 Title, 2:105 Headline, 2:80 Creator, 2:55 Date Created, 2:60 Time Created,
 * 2:101 Country, 2:95 Province/State, 2:90 City, 2:92 Sublocation, 2:115 Source,
 * 2:120 Caption/Abstract
 */
QString  KbvImageMetaData::getIptcInfo(const QString &filePath)
{
  KbvExiv     metadata;
  QString     str, info;
  int         charSet;

//read metadata from image file
  metadata = KbvExiv(filePath);
  if(!metadata.hasIptc())
    {
      return  QString();
    }
  info.append(QString("\nType:  %1").arg(metadata.getMimeType()));
  info.append(QString("\n\nIPTC:"));

  charSet = metadata.getIptcCharacterSet();
  //2:00 Record Version
  str = QString::fromLatin1(metadata.getIptcTagData("Iptc.Application2.RecordVersion").toHex());
  if(!str.isEmpty())
    {
      info.append("\nVersion:   " + str);
    }
  //2:05 Title (Object name)
  str = metadata.getIptcTagDataString("Iptc.Application2.ObjectName", charSet);
  if(!str.isEmpty())
    {
      info.append("\nTitle:   " + str);
    }
  //2:105 Headline
  str = metadata.getIptcTagDataString("Iptc.Application2.Headline", charSet);
  if(!str.isEmpty())
    {
      info.append("\nHeadline:   " + str);
    }
  //2:80 Creator (By-line)
  str = metadata.getIptcTagDataString("Iptc.Application2.Byline", charSet);
  if(!str.isEmpty())
    {
      info.append("\nCreator:   " + str);
    }
  //2:55 Date Created ISO8601
  str = QString::fromLatin1(metadata.getIptcTagData("Iptc.Application2.DateCreated"));
  if(8 == str.length())
    {
      str.insert(4,"-");
      str.insert(7, "-");
      info.append("\nDate:   " + str);
    }
  //2:60 Time Created ISO8601
  str = QString::fromLatin1(metadata.getIptcTagData("Iptc.Application2.TimeCreated"));
  if(6 == str.length())
    {
      str.insert(2,":");
      str.insert(5, ":");
      info.append("\nTime:   " + str);
    }
  if(11 == str.length())
    {
      str.insert(2,":");
      str.insert(5, ":");
      str.insert(11, ":");
      info.append("\nTime:   " + str);
    }
  //2:101 Country  Exiv2::IptcData::iterator iptciter;
  str = metadata.getIptcTagDataString("Iptc.Application2.CountryName", charSet);
  if(!str.isEmpty())
    {
      info.append("\nCountry:   " + str);
    }
  //2:95 Province/State
  str = metadata.getIptcTagDataString("Iptc.Application2.ProvinceState", charSet);
  if(!str.isEmpty())
    {
      info.append("\nProvince/State:   " + str);
    }
  //2:90 City
  str = metadata.getIptcTagDataString("Iptc.Application2.City", charSet);
  if(!str.isEmpty())
    {
      info.append("\nCity:   " + str);
    }
  //2:92 Sublocation
  str = metadata.getIptcTagDataString("Iptc.Application2.SubLocation", charSet);
  if(!str.isEmpty())
    {
      info.append("\nSublocation:   " + str);
    }
  //2:115 Source
  str = metadata.getIptcTagDataString("Iptc.Application2.Source", charSet);
  if(!str.isEmpty())
    {
      info.append("\nSource:   " + str);
    }
  //2:120 Caption
  str = metadata.getIptcTagDataString("Iptc.Application2.Caption", charSet);
  if(!str.isEmpty())
    {
      info.append("\nDescription:   " + str);
    }
  //2:65 Originating Program
  str = metadata.getIptcTagDataString("Iptc.Application2.Program", charSet);
  if(!str.isEmpty())
    {
      info.append("\nProgram:   " + str);
    }
  return  info;
}
/*************************************************************************//*!
 * Static function:
 * Extract the exif meta data of 'filePath' and return them in 'info'.
 */
QString  KbvImageMetaData::getExifInfo(const QString &filePath)
{
  QMap<QString, QString>::iterator iter;
  KbvExiv::MetaDataMap exifMap;
  KbvExiv     metadata;
  QStringList strlist;
  QString     str, info;

  //read metadata from image file
  metadata = KbvExiv(filePath);

  if(!metadata.hasExif())
    {
      return  QString();
    }

  strlist << QString("Image") << QString("Photo");
  exifMap = metadata.getExifTagsDataList(strlist);

  info.append(QString("\n\nEXIF:"));
  for(iter=exifMap.begin(); iter != exifMap.end(); iter++)
    {
      if(iter.key() == "Exif.Image.Make")
        {
          info.append(QString("\nCamera Make: " + iter.value()));
        }
      else if(iter.key() == "Exif.Image.Model")
        {
          info.append(QString("\nCamera Model: " + iter.value()));
        }
      else if(iter.key() == "Exif.Image.Artist")
        {
          info.append(QString("\nArtist: " + iter.value()));
        }
      else if(iter.key() == "Exif.Image.DateTime")
        {
          info.append(QString("\nDate: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.ExposureTime")
        {
          info.append(QString("\nExposure Time: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.FNumber")
        {
          info.append(QString("\nFNumber: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.Flash")
        {
          info.append(QString("\nFlash: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.PixelXDimension")
        {
          info.append(QString("\nX Dimension: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.PixelYDimension")
        {
          info.append(QString("\nY Dimension: " + iter.value()));
        }
      else if(iter.key() == "Exif.Image.ImageDescription")
        {
          info.append(QString("\nDescription: " + iter.value()));
        }
      else if(iter.key() == "Exif.Image.Software")
        {
          info.append(QString("\nSoftware: " + iter.value()));
        }
      else if(iter.key() == "Exif.Photo.UserComment")
        {
          info.append(QString("\n" + iter.value()));
        }
    }

  str = metadata.getGPSLongitudeDegMinSecString();
  if(!str.isEmpty())
    {
      info.append(QString("\nGPS logitude: " + str));
    }
  str = metadata.getGPSLatitudeDegMinSecString();
  if(!str.isEmpty())
    {
      info.append(QString("\nGPS latitude: " + str));
    }
  return  info;
}

/*************************************************************************//*!
 * Find the application for the given mime type and open it in an own process.
 * The parameter string 'params' contains the command line parameters.
 */
void  KbvImageMetaData::openApplication(const QString mimetype, const QStringList params)
{
  QString       prog;
  QProcess      *app;
  
  prog = this->getMimeApplication(mimetype);
  
  if(!prog.isEmpty())
    {
      app = new QProcess(this);
      app->start(prog, params);
      //qDebug() << "kbvGlobal::openApplication" <<prog <<params; //###########
    }
}
/*************************************************************************//*!
 * Find the application which should open a file associated with a mime type.
 * The algorithm follows the recommandations of freedesktop.org "Association
 * between MIME types and applications".
 * Considered are the environment variables:
 * $XDG_CONFIG_HOME/$desktop-mimeapps.list  user overrides, desktop-specific
 * $XDG_CONFIG_HOME/mimeapps.list           user overrides (recommended location for user configuration GUIs)
 * $XDG_CONFIG_DIRS/$desktop-mimeapps.list  sysadmin and ISV overrides, desktop-specific
 * $XDG_CONFIG_DIRS/mimeapps.list           sysadmin and ISV overrides
 * $XDG_DATA_DIRS/applications/$desktop-mimeapps.list   distribution-provided defaults, desktop-specific
 * $XDG_DATA_DIRS/applications/mimeapps.list            distribution-provided defaults
 * The *.list files of a directory are examined and the first found application
 * for matching the mime type get used.
 */
QString  KbvImageMetaData::getMimeApplication(const QString mimetype)
{
  QString       app, env;
  QStringList   strList;


  app.clear();
  env = QString(qgetenv("XDG_CONFIG_HOME"));
  //qDebug() << "kbvGlobal::getMimeApplication $XDG_CONFIG_HOME" <<env; //###########
  if(!env.isEmpty())
    { 
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "");
    }
  
  env = QString(qgetenv("XDG_CONFIG_DIRS"));
  //qDebug() << "kbvGlobal::getMimeApplication $XDG_CONFIG_DIRS" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    { 
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "");
    }

  env = QString(qgetenv("XDG_DATA_HOME"));
  //qDebug() << "kbvGlobal::getMimeApplication $XDG_DATA_HOME" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    { 
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "applications/");
    }

  env = QString(qgetenv("XDG_DATA_DIRS"));
  //qDebug() << "kbvGlobal::getMimeApplication $XDG_DATA_DIRS" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    { 
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "applications/");
    }

  return app;
}
/*************************************************************************//*!
 * Helper function: Search the application associated to a mime type.
 * Evaluate the content of an environment variable given in parameter
 * 'environment'. Return an empty string when no app could be found.
 */
QString  KbvImageMetaData::mimeAssociation(const QStringList dirList, const QString mimetype, const QString subPath)
{
  QDir          xdgDir;
  QDir::Filters filter;
  QFile         associations;
  QString       path, line, appDesktop;
  QStringList   fileList, nameFilter;
  QTextStream   stream;
  int           i, k, n;
  bool          found=false;
  
  filter = QDir::Files;
  nameFilter << "*.list";
  
  //qDebug() << "kbvGlobal::mimeApplication" <<dirList <<mimetype <<subPath; //###########
  //Outer loop: check all dirs for presence of a *.list file
  appDesktop.clear();
  i = 0;
  while((i < dirList.size()) && !found)
    {
      path = dirList.at(i);
      if(!path.endsWith("/"))
        {
          path.append("/");
        }
      path.append(subPath);
      xdgDir.setPath(path);
      //qDebug() << "kbvGlobal::mimeApplication path" <<path; //###########
      fileList = xdgDir.entryList(nameFilter, filter, QDir::NoSort);
      //Inner loop: check all found files for mime type
      k = 0;
      while((k < fileList.size()) && !found)
        {
          //qDebug() << "kbvGlobal::mimeApplication file" <<fileList.at(k); //###########
          associations.setFileName(path + fileList.at(k));
          if(associations.open(QIODevice::ReadOnly | QIODevice::Text))
            {
              stream.setDevice(&associations);
              while(!stream.atEnd())
                {
                  line = stream.readLine();
                  if(line.startsWith(mimetype))
                    {
                      found = true;
                      n = line.indexOf("=", 0, Qt::CaseInsensitive);
                      appDesktop = line.remove(0, n+1);
                      break;      //break loop through the file
                    }
                }
              associations.close();
            }
          k++;
        }
      i++;
   }
  //Find executable in desktop file (app.desktop)
  //Search line: Exec=/usr/bin/app or Exec=app
  line.clear();
  if(found)
    {
      associations.setFileName("/usr/share/applications/" + appDesktop);
      if(associations.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           stream.setDevice(&associations);
           while (!stream.atEnd())
             {
                line = stream.readLine();
                if(line.startsWith("Exec="))
                  {
                     //qDebug() << "kbvGlobal::mimeApplication application found" <<line; //###########
                     n = line.indexOf("=", 0, Qt::CaseInsensitive);
                     line.remove(0, n+1);
                     n = line.indexOf(" ", 0, Qt::CaseInsensitive);
                     if(n > 0)
                       {
                         line = line.left(n);
                       }
                     //qDebug() << "kbvGlobal::mimeApplication application" <<line; //###########
                     break;      //break loop through file
                  }
              }
            associations.close();
         }
     }
  return line;
}

/****************************************************************************/
