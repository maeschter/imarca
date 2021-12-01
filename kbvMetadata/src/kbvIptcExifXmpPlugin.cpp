/*****************************************************************************
 * kbv plugin meta data.
 * Plugin to edit image meta data IPTC, EXIF and XMP. The plugin is a static
 * wrapper for libExiv2.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.03.17
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvIptcExifXmpPlugin.h"

KbvIptcExifXmpPlugin::~KbvIptcExifXmpPlugin()
{
  //qDebug() << "KbvIptcExifXmpPlugin::~KbvIptcExifXmpPlugin";//###########
  cleanupPluginResource();
  
  delete  imageMetadata;
}
/*************************************************************************//*!
 * Initialise ressources, setup gui etc.
 */
void   KbvIptcExifXmpPlugin::initialisePlugin()
{
  initPluginResource();
  
  imageMetadata = new KbvImageMetaData(nullptr);
  //qDebug() << "KbvIptcExifXmpPlugin::initialisePlugin done"; //###########
}
/*************************************************************************//*!
 * Open the gui to Iptc, Exif and XMP meta data
 */
void   KbvIptcExifXmpPlugin::openEditor(const QList<QPair<QString, QString> > &files)
{
  imageMetadata->openMetaDataGui(files);
}
/*************************************************************************//*!
 * Open the template for Iptc meta data
 */
void   KbvIptcExifXmpPlugin::openIptcTemplate()
{
  imageMetadata->openIptcTemplate();
}
/*************************************************************************//*!
 * IPTC info for tooltips
 */
QString   KbvIptcExifXmpPlugin::getIptcInfo(QString path) const
{
  //qDebug() << "KbvIptcExifXmpPlugin::getIptcInfo";//###########
  return imageMetadata->getIptcInfo(path);
}
/*************************************************************************//*!
 * EXIF info for tooltips
 */
QString   KbvIptcExifXmpPlugin::getExifInfo(QString path) const
{
  //qDebug() << "KbvIptcExifXmpPlugin::getExifInfo";//###########
  return  imageMetadata->getExifInfo(path);
}

/*************************************************************************//*!
 * Return the plugin info
 */
QString   KbvIptcExifXmpPlugin::pluginInfo() const
{
  return  QString("libImageMetadata: manipulate meta data iptc, exif, xmp");
}
/*************************************************************************//*!
 * Show menu items "Metadata" and "IptcTemplate"
 */
bool   KbvIptcExifXmpPlugin::showMenuItems() const
{
  return true;
}


/****************************************************************************/
