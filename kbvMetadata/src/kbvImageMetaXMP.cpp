/*****************************************************************************
 * kvb image XMP meta data
 * This file contains functions to handle the XMP part of meta data gui.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2015.04.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvImageMetaData.h"

/*************************************************************************//*!
 * SLOT: button XMP data clear. Remove all data sets.
 */
void  KbvImageMetaData::xmpClear(bool checked)
{
  Q_UNUSED(checked)

  this->xmpClearGui();
}
/*************************************************************************//*!
 * Clear all XMP gui data. Remove all items then remove rows.
 * Clear comment and description text fields.
 */
void  KbvImageMetaData::xmpClearGui()
{

  uiXmpData.xmpTagTable->clearContents();
  while(uiXmpData.xmpTagTable->rowCount()>0)
    {
      uiXmpData.xmpTagTable->removeRow(0);
    }
}
/*************************************************************************//*!
 * Read XMP data into GUI.
 * Find tags for iptc iptcExt exif dc xmp xmpRights xmpMM xmpBJ xmpTPg xmpDM
 * Note: Exiv2 uses iptc/iptcExt instead of Iptc4xmpCore/Iptc4xmpExt
 */
bool  KbvImageMetaData::xmpDataToGui()
{
  QStringList       prefixList;
  QTableWidgetItem  *tag, *value;
  KbvExiv::MetaDataMap::iterator iter;
  KbvExiv::MetaDataMap xmpMap;
  int   row;

  //Clear gui: remove all items and rows in exifTagTable
  this->xmpClearGui();
  if(!metaData.hasXmp())
    {
      //qDebug() << "KbvImageMetaData::xmpDataToGui no XMP"; //###########
      return false;
    }

  //Prepare items for tag column and value column
  tag = new QTableWidgetItem(QTableWidgetItem::Type);
  tag->setFlags(Qt::ItemIsEnabled);
  value = new QTableWidgetItem(QTableWidgetItem::Type);
  value->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

  prefixList  <<"dc" <<"iptc" <<"iptcExt" <<"exif" <<"xmp" <<"xmpRights";
  xmpMap = this->metaData.getXmpTagsDataList(prefixList, false);
  row = 0;
  for(iter=xmpMap.begin(); iter != xmpMap.end(); iter++)
    {
      tag->setText(iter.key().mid(4));
      value->setText(iter.value());
      uiXmpData.xmpTagTable->insertRow(row);
      uiXmpData.xmpTagTable->setItem(row, KbvMeta::tagCol, tag->clone());
      uiXmpData.xmpTagTable->setItem(row, KbvMeta::valueCol, value->clone());
      row++;
    }
  //qDebug() <<"KbvImageMetaData::xmpDataToGui map size:"<<xmpMap.size() <<"- rows:"<<row; //###########


  delete tag;
  delete value;
  return true;
}
/*************************************************************************//*!
 * Copy GUI to XMP meta data container.
 * When XMP gui is empty we clear the container.
 */
void  KbvImageMetaData::xmpGuiToXmpData()
{
  //TODO: XMP gui to data

}

/*************************************************************************//*!
 * SLOT: button XMP GPS GoogleMaps.
 */
void  KbvImageMetaData::xmpGoogleMaps(bool checked)
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
 * SLOT: button XMP GPS OpenStreetMap.
 */
void  KbvImageMetaData::xmpOpenStreetMap(bool checked)
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
