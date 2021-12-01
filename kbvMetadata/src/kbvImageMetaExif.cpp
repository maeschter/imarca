/*****************************************************************************
 * kvb image EXIF meta data
 * This file contains functions to handle the EXIF part of meta data gui.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2015.10.25
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <kbvConstants.h>
#include "kbvImageMetaData.h"


/*************************************************************************//*!
 * Set name and version of the software used to post-process the picture.
 */
void  KbvImageMetaData::setExifSoftware(const QString text)
{
  //qDebug() << "setExifSoftware"<<text; //###########
  metaData.setExifTagDataString("Exif.Image.Software", text);
}
/*************************************************************************//*!
 * SLOT: button EXIF data clear. Remove all data sets.
 */
void  KbvImageMetaData::exifClear(bool checked)
{
  Q_UNUSED(checked)

  this->exifClearGui();
}
/*************************************************************************//*!
 * Clear all EXIF gui data. Remove all items then remove rows.
 * Clear user comment text field.
 */
void  KbvImageMetaData::exifClearGui()
{
  uiExifData.exifTagTable->clearContents();
  while(uiExifData.exifTagTable->rowCount()>0)
    {
      uiExifData.exifTagTable->removeRow(0);
    }
  uiExifData.exifComment->clear();
}
/*************************************************************************//*!
 * Copy GUI to Exif meta data container.
 * When Exif gui is empty we clear the container.
 * Photo.UserComment which can hold text in UTF-8.
 */
void  KbvImageMetaData::exifGuiToExifData()
{
  QString   str;
  int       rows;

  rows = uiExifData.exifTagTable->rowCount();
  if(rows == 0)
    {
      metaData.clearExif();   //clear exif container
    }

  //set or clear user comment
  str = uiExifData.exifComment->toPlainText();
  metaData.setExifCommentDescription(str, "");

  //set program name and version
  str = QString(appName);
  str.append(" ");
  str.append(QString(appVersion));
  this->setExifSoftware(str);

  //set date and time
  QDateTime dt(QDate::currentDate(), QTime::currentTime(), Qt::LocalTime);
  metaData.setExifDateTime("Exif.Image.DateTime", dt);
  //qDebug() << "KbvImageMetaData::exifGuiToExifData"<<dt; //###########
}
/*************************************************************************//*!
 * Read EXIF data groups "photo" and "image" into GUI.
 */
bool  KbvImageMetaData::exifDataToGui()
{
  QStringList       strlist;
  QTableWidgetItem  *tag, *value;
  QMap<QString, QString>::iterator iter;
  KbvExiv::MetaDataMap exifMap;
  int   row;

  //Clear gui: remove all items and rows in exifTagTable
  this->exifClearGui();
  if(!metaData.hasExif())
    {
      //qDebug() << "KbvImageMetaData::exifDataToGui no Exif"; //###########
      return false;
    }

  //Prepare items for tag column and value column
  tag = new QTableWidgetItem(QTableWidgetItem::Type);
  tag->setFlags(Qt::ItemIsEnabled);
  value = new QTableWidgetItem(QTableWidgetItem::Type);
//  value->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
  value->setFlags(Qt::ItemIsEnabled);

  strlist.append("Image");
  exifMap = this->metaData.getExifTagsDataList(strlist);
  row = 0;
  for(iter=exifMap.begin(); iter != exifMap.end(); iter++)
    {
      if((iter.key() != "Exif.Image.ExifTag") && (iter.key() != "Exif.Image.GPSTag"))
        {
          tag->setText(iter.key().mid(5));
          value->setText(iter.value());
          uiExifData.exifTagTable->insertRow(row);
          uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
          uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
          row++;
        }
    }

  strlist.clear();
  strlist.append("Photo");
  exifMap.clear();
  exifMap = this->metaData.getExifTagsDataList(strlist);
  row = uiExifData.exifTagTable->rowCount();
  for(iter=exifMap.begin(); iter != exifMap.end(); iter++)
    {
      if(iter.key() == "Exif.Photo.UserComment")    //UserComment only is editable
        {
          uiExifData.exifComment->setPlainText(iter.value());
        }
      else
        {
          tag->setText(iter.key().mid(5));
          value->setText(iter.value());
          uiExifData.exifTagTable->insertRow(row);
          uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
          uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
        }
      row++;
    }

  delete tag;
  delete value;
  return true;
}
/*************************************************************************//*!
 * Read GPS data into GUI.
 */
bool  KbvImageMetaData::gpsDataToGui()
{
  QTableWidgetItem  *tag, *value;
  QString   str;
  double    altitude;
  int       row;

  if(!this->metaData.getGPSCoordinates(altitude, latitude, longitude))
    {
      uiExifData.gpsGoogleMaps->setEnabled(false);
      uiExifData.gpsOpenStreetMap->setEnabled(false);
      return false;
    }
  uiExifData.gpsGoogleMaps->setEnabled(true);
  uiExifData.gpsOpenStreetMap->setEnabled(true);

  //Prepare items for tag column and value column
  tag = new QTableWidgetItem(QTableWidgetItem::Type);
  tag->setFlags(Qt::ItemIsEnabled);
  value = new QTableWidgetItem(QTableWidgetItem::Type);
  value->setFlags(Qt::ItemIsEnabled);

  str = this->metaData.getGPSDateString();
  if(!str.isEmpty())
    {
      tag->setText("GPSDateStamp");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }
  str = this->metaData.getGPSTimeString();
  if(!str.isEmpty())
    {
      tag->setText("GPSTimeStamp");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  str = this->metaData.getGPSLatitudeDegMinSecString();
  if(!str.isEmpty())
    {
      tag->setText("GPSLatitude");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  str = this->metaData.getGPSLongitudeDegMinSecString();
  if(!str.isEmpty())
    {
      tag->setText("GPSLongitude");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  if(this->metaData.getGPSAltitude(&altitude))
    {
      tag->setText("GPSAltitude");
      value->setText(QString::number(altitude, 'f', 2) + " m");
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  str = this->metaData.getGPSDirectionString();
  if(!str.isEmpty())
    {
      tag->setText("GPSImgageDirection");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  str = this->metaData.getGPSTrackString();
  if(!str.isEmpty())
    {
      tag->setText("GPSTrack");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  str = this->metaData.getGPSSpeedString();
  if(!str.isEmpty())
    {
      tag->setText("GPSSpeed");
      value->setText(str);
      row = uiExifData.exifTagTable->rowCount();
      uiExifData.exifTagTable->insertRow(row);
      uiExifData.exifTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiExifData.exifTagTable->setItem(row, KbvMeta::valueCol, value->clone());
    }

  delete tag;
  delete value;
  return true;
}
/*************************************************************************//*!
 * SLOT: button GPS GoogleMaps.
 */
void  KbvImageMetaData::gpsGoogleMaps(bool checked)
{
  Q_UNUSED(checked)
  QStringList       paramsList;
  QString           lat, lon;

  if(longitude > 180.0 || longitude < -180.0)
    {
      return;
    }
  if(latitude > 90.0 || latitude < -90.0)
    {
      return;
    }
  lon = QString::number(longitude, 'f', 8);
  lat = QString::number(latitude, 'f', 8);
  //qDebug() << "KbvImageMetaData::gpsMap lat lon"<<lat <<lon; //###########
  //coordinates in the form +-DDD.dddd, DDD degrees, ddd is a fraction of degrees
  paramsList.append(QString("http://maps.google.com/?q=%1,%2").arg(lat).arg(lon));
  openApplication("application/xhtml+xml", paramsList);
}
/*************************************************************************//*!
 * SLOT: button GPS OpenStreetMap.
 */
void  KbvImageMetaData::gpsOpenStreetMap(bool checked)
{
  Q_UNUSED(checked)
  QStringList       paramsList;
  QString           lat, lon;

  if(longitude > 180.0 || longitude < -180.0)
    {
      return;
    }
  if(latitude > 90.0 || latitude < -90.0)
    {
      return;
    }
  lon = QString::number(longitude, 'f', 8);
  lat = QString::number(latitude, 'f', 8);
  //coordinates in the form +-DDD.dddd, DDD degrees, ddd is a fraction of degrees
  paramsList.append(QString("http://www.openstreetmap.org/?mlat=%1&mlon=%2#map=14/%1/%2").arg(lat).arg(lon));
//  paramsList.append(QString("http://www.openstreetmap.org/#map=14/%1/%2").arg(lat).arg(lon));
  openApplication("application/xhtml+xml", paramsList);
}

/****************************************************************************/
