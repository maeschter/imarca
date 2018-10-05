/*****************************************************************************
 * kvb global
 * Globally used methods and information
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2008.08.31
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>

#include "kbvSetvalues.h"
#include "kbvGlobal.h"
#include "kbvImageViewer.h"
#include "kbvPluginInterfaces.h"
//#include "kbvSlideScene.h"  replaced by image viewer
//#include "kbvSlideView.h"  replaced by image viewer

extern  KbvSetvalues            *settings;
extern  KbvIptcExifXmpInterface *metadataPlugin;

KbvGlobal::KbvGlobal() : QObject()
{
  crc32BuildTab();
}

KbvGlobal::~KbvGlobal()
{
  //qDebug() << "KbvGlobal::~KbvGlobal"; //###########
}

/* Public functions *********************************************************/
/*************************************************************************//*!
 * Display images with imageViewer. Used in all views when F8 was pressed.
 * If nothing is selected, all items of the current view are displayed.
 * If a sole item is selected the show starts from this item.
 */
void    KbvGlobal::displayImages(QListView *view, QAbstractListModel *model, QSortFilterProxyModel *sortModel)
{
  QModelIndexList   selIndices;
  QModelIndex       index;
  QList<QPair<QString, QString> > filelist;
  QPair<QString, QString>         pathname;
  int               k;

  selIndices = view->selectionModel()->selectedIndexes();
  //nothing selected -> take all
  if(selIndices.count() == 0)
    {
      view->selectAll();
      selIndices = view->selectionModel()->selectedIndexes();
    }
  //only one item selected -> take all after the selected one
  if(selIndices.count() == 1)
    {
      index = selIndices[0];
      view->selectAll();
      selIndices = view->selectionModel()->selectedIndexes();
      k = selIndices.indexOf(index ,0);
      for (int i=0; i<k; i++)
        {
          selIndices.removeFirst();
        }
    }
  view->clearSelection();
  //map selection to source model and read path and name
  for(int i=0; i<selIndices.count(); ++i)
    {
      selIndices[i] = sortModel->mapToSource(selIndices[i]);
      pathname.first  = model->data(selIndices[i], Kbv::FilePathRole).toString();
      pathname.second = model->data(selIndices[i], Kbv::FileNameRole).toString();
      filelist.append(pathname);
    }
  //qDebug() << "KbvGlobal::displayImages" <<filelist.length(); //###########
  
  //Show images. Memory is released when the show finishes.
  KbvImageViewer *imageViewer = new KbvImageViewer();
  connect(imageViewer, SIGNAL(deleteFiles(QMap<QString, QString>)), model, SLOT(removeFiles(QMap<QString, QString>)));
  imageViewer->showImages(filelist);
}
/*************************************************************************//*!
 * Display slide show. Used in all views when F8 was pressed.
 * If nothing is selected, all items of the current view are displayed.
 * If a sole item is selected the slide show starts from this item.
 */
/* replaced by image viewer
void    KbvGlobal::displaySlideShow(QListView *view, QAbstractListModel *model, QSortFilterProxyModel *sortModel)
{
  QModelIndexList   selIndices;
  QModelIndex       index;
  QList<QPair<QString, QString> > filelist;
  QPair<QString, QString>         pathname;
  int               k;

  selIndices = view->selectionModel()->selectedIndexes();
  //nothing selected -> take all
  if(selIndices.count() == 0)
    {
      view->selectAll();
      selIndices = view->selectionModel()->selectedIndexes();
    }
  //only one item selected -> take all after the selected one
  if(selIndices.count() == 1)
    {
      index = selIndices[0];
      view->selectAll();
      selIndices = view->selectionModel()->selectedIndexes();
      k = selIndices.indexOf(index ,0);
      for (int i=0; i<k; i++)
        {
          selIndices.removeFirst();
        }
    }
  view->clearSelection();
  //map selection to source model and read path and name
  for(int i=0; i<selIndices.count(); ++i)
    {
      selIndices[i] = sortModel->mapToSource(selIndices[i]);
      pathname.first = model->data(selIndices[i], Kbv::FilePathRole).toString();
      pathname.second = model->data(selIndices[i], Kbv::FileNameRole).toString();
      filelist.append(pathname);
    }
  //qDebug() << "KbvGlobal::displaySlideShow" <<filelist.length(); //###########
  
  //Show images. Memory is released when show finishes.
  KbvSlideScene *slideScene = new KbvSlideScene();
  KbvSlideView *slideView = new KbvSlideView(slideScene, 0);
  slideView->show();
  slideView->activateWindow();

  connect(slideScene, SIGNAL(deleteFiles(QMap<QString, QString>)), model,SLOT(removeFiles(QMap<QString, QString>)));
  slideScene->startShow(filelist);
}
*/
/*************************************************************************//*!
 * Used by fileModel, collectionView und dndHandler.
 * Extract the list of file paths from mime data (urls or text) and return
 * them as list of absolute file paths.
 */
QStringList KbvGlobal::dropExtractMimeData(const QMimeData *mime)
{
  QStringList   pathlist;
  QList<QUrl>   urllist;

  //qDebug() << "KbvGlobal::dropExtractMimeData urls" << mime->urls(); //###########
  //qDebug() << "KbvGlobal::dropExtractMimeData text" << mime->text(); //###########

  //Extract the list of file paths from mime data (urls or text)
  pathlist.clear();
  if(mime->hasUrls())
    {
      urllist = mime->urls();
      for(int i=0; i<urllist.length();i++)
        {
          pathlist.append(urllist[i].toLocalFile());
        }
    }
  else
    {
      if(mime->hasText())
        {
          //qDebug() << "KbvGlobal::dropExtractMimeData text" << mime->text(); //###########
          //split text into list and remove "file://" and line separator "\n"
          pathlist = mime->text().split("\n", QString::SkipEmptyParts, Qt::CaseInsensitive);
          for(int i=0; i<pathlist.length();i++)
            {
              pathlist[i].remove("file://", Qt::CaseInsensitive);
              pathlist[i] = pathlist[i].left(pathlist[i].size()-1);
            }
        }
    }
  //qDebug() << "KbvGlobal::dropExtractMimeData paths" <<pathlist; //###########
  return pathlist;
}
/*************************************************************************//*!
 * Model data for display role.
 * Compose the string to be shown for display role below the icon. The string
 * depends on values defined in settings. The item at least must contain
 * values for Kbv::FileSizeRole, Kbv::FileNameRole and Kbv::ImageDimRole.
 */
QVariant KbvGlobal::displayRoleData(const Kbv::kbvItem *item, int viewMode) const
{
  QString       displayrole, str1, str2, str3;
  QSize         imageDim;
  double        filesize;
  bool          moreInfos = false;

  //Show info on crc errors
  if(item->value(Kbv::FileNameRole).toString() == "error")
    {
      displayrole = ("!");
      return QVariant(displayrole);
    }

  str1 = item->value(Kbv::FileNameRole).toString();
  imageDim = item->value(Kbv::ImageDimRole).toSize();
  str2 = QString("%L1x%L2").arg(imageDim.width()).arg(imageDim.height());
  filesize = item->value(Kbv::FileSizeRole).toDouble();
  str3 = QString::number((filesize/1024.0), 'f', 1) + " KB";

  //List mode
  if(viewMode == QListView::ListMode)
    {
      displayrole = QString("%1 %2 %3").arg(str1, -20, ' ').arg(str2, 16, ' ').arg(str3, 12, ' ');
      return QVariant(displayrole);
    }

  //Icon mode
  if (settings->showImageSize && item->value(Kbv::ImageDimRole).isValid())
    {
      displayrole = str2;
      moreInfos = true;
    }
  if (settings->showFileSize && item->value(Kbv::FileSizeRole).isValid())
    {
      if (moreInfos)
        {
          displayrole += "\n";
        }
      displayrole += str3;
      moreInfos = true;
    }
  if (settings->showFileName && item->value(Kbv::FileNameRole).isValid())
    {
      if (moreInfos)
        {
          displayrole += "\n";
        }
      displayrole += str1;
    }
  if(displayrole.isEmpty())
    {
      return QVariant();
    }
  return QVariant(displayrole);
}
/*************************************************************************//*!
 * This function composes tooltips which are requested from (file-, collection,
 * search-) view by TooTipRole.
 * The information contains (in order of appearance):
 * file path and name, size, crc32 in bytes and image height and width.
 */
QString KbvGlobal::tooltip(const Kbv::kbvItem *item) const
{
  QString   tip, path, name, str;

  path = item->value(Kbv::FilePathRole).toString();
  name = item->value(Kbv::FileNameRole).toString();

  tip.append(QString(tr("Path:")));
  tip.append("   " + path + "\n");
  tip.append(QString(tr("File:")));
  tip.append("   " + name + "\n");
  tip.append(QString(tr("Size:")));
  tip.append(QString("   %1").arg(QString("%L1").arg(item->value(Kbv::FileSizeRole).toLongLong(), 0, 10)));
  tip.append(tr(" bytes"));
  tip.append("\n");
  tip.append(QString(tr("CRC32:")));
  str.append(QString("%L1").arg(item->value(Kbv::FileCRCRole).toUInt(), 0, 10));
  if(str == "0")   { str = QString("---"); }
  tip.append("  " + str);
  tip.append("\n");
  tip.append(QString(tr("Width:")));
  tip.append(QString("   %1").arg(item->value(Kbv::ImageDimRole).toSize().width(), 0, 10));
  tip.append("\n");
  tip.append(QString(tr("Height:")));
  tip.append(QString("   %1").arg(item->value(Kbv::ImageDimRole).toSize().height(), 0, 10));
  tip.append("\n");
  //qDebug() << "KbvGlobal::tooltip"<<path <<name <<metadataPlugin; //##########
  if(metadataPlugin)
    {
      tip.append(metadataPlugin->getIptcInfo(path+name));
      tip.append(metadataPlugin->getExifInfo(path+name));
    }
  return tip;
}
/*************************************************************************//*!
 * Build the item of 'path' and 'name' for the fileModel (no database content).
 * Returns true when the item could be created successfully.
 * Created item:
 * QString   - Qt::DisplayRole\n
 * icon      - Qt::decorationRole\n
 * QString   - Kbv::FileNameRole\n
 * QString   - Kbv::FilePathRole\n
 * qint64    - Kbv::FileSizeRole\n
 * QDateTime - Kbv::FileDateRole\n
 * QSize     - Kbv::ImageDimRole\n
 * quint64   - Kbv::ImageSizeRole\n
 */
Kbv::kbvItem*   KbvGlobal::itemFromFile(const QString path, const QString name, const QSize iconsize)
{
  Kbv::kbvItem *item;
  QFileInfo     file;
  QDateTime     lastmod;
  QImageReader  reader;
  QImage        fileimage;
  QSize         imgDim;
  quint64       imgSize;
  int           w=1, h=1;
  float         ar;
  
/* TODO: Obviously the image reader does not read hidden image files (.image.png)
 * simply ignores, no message
 */
  
  reader.setAutoDetectImageFormat(true);
  reader.setScaledSize(iconsize);
  //The jpeghandler only shrinks very fast to 1/2, 1/4, 1/8 when quality < 50!
  reader.setQuality(49);

  file = QFileInfo(path + name);
  //create icon, insert -no_support- on unknown files
  reader.setFileName(path + name);
  //scale down only - do not scale up, keep aspect ratio
  imgDim = reader.size();
  imgSize = imgDim.width() * imgDim.height();
  lastmod = file.lastModified();

  if((imgDim.width() <= iconsize.width()) && (imgDim.height() <= iconsize.height()))
    {
      //image of same size or smaller than icon
      w = imgDim.width();
      h = imgDim.height();
    }
  else
    {
      ar = (float) imgDim.height() / imgDim.width();
      if(ar >= 1)
        {
          //higher than wide
          w = (int)(iconsize.width()/ar);
          h = iconsize.height();
        }
      else
        {
          //wider than high
          w = iconsize.width();
          h = (int)(ar * iconsize.height());
        }
    }
  reader.setScaledSize(QSize(w, h));

  //create new item and return it
  item = new Kbv::kbvItem;
  item->insert(Qt::DisplayRole,     QVariant(name));        //QString
  item->insert(Kbv::FileNameRole,   QVariant(name));        //QString
  item->insert(Kbv::FilePathRole,   QVariant(path));        //QString
  item->insert(Kbv::FileSizeRole,   QVariant(file.size())); //qint64
  item->insert(Kbv::FileDateRole,   QVariant(lastmod));     //QDateTime
  item->insert(Kbv::ImageSizeRole,  QVariant(imgSize));     //quint64
  item->insert(Kbv::ImageDimRole,   QVariant(imgDim));      //QSize
  //qDebug() << "KbvGlobal::itemFromFile" << name; //##########
  if(reader.read(&fileimage))
    {
      item->insert(Qt::DecorationRole,  QVariant(fileimage));   //QImage
    }
  else
    {
      //qDebug() << "KbvGlobal::itemFromFile" <<name <<reader.errorString(); //##########
      reader.setScaledSize(iconsize);
      if(name.endsWith(QString(dbNameExt)))
        {
          reader.setFileName(":/kbv/icons/imarca.png");
        }
      else
        {
          reader.setFileName(":/kbv/icons/kbv_no_support.png");
        }
      reader.read(&fileimage);
      item->insert(Qt::DecorationRole,  QVariant(fileimage));   //QImage
    }
  return item;
}
/*************************************************************************//*!
 * Build the item from query data.
 * Mandatory item data are: file name, file size, image dimensions and primary
 * key, since those can be selected by options dialog or are needed to get
 * fast access to the database records. The rest are convient data.
 * Created item:
 * QString   - Kbv::FileNameRole
 * QString   - Kbv::FilePathRole
 * qint64    - Kbv::FileSizeRole
 * QDateTime - Kbv::FileDateRole <- from dateChanged and timeChanged (integer)
 * QSize     - Kbv::ImageDimRole
 * quint64   - Kbv::ImageSizeRole
 * Integer   - Kbv::PrimaryKeyRole\n
 * quint32   - Kbv::FileCRCRole\n
 * icon      - Qt::decorationRole\n
 * The query statement:
 * "SELECT filePath,fileName,icon,imageW,imageH,fileSize,crc32,dateChanged,timeChanged,userDate,pk FROM album "
 * The file date (last modified) must be compiled from: dateChanged = Julian Day
 * and timeChanged = h*3600 + min*60 + sec*60. Both as integers.
 * Note:
 * Databases for albums contain an empty root dir but absolute file paths.
 * Databases for collections contain an absolute root dir but relative file paths.
 * A model item always contains an absolute file path.
 */
Kbv::kbvItem*   KbvGlobal::itemFromRecord(const QSqlQuery &query, const QString &rootDir)
{
  Kbv::kbvItem  *item;
  QString       path;
  QSize         imgDim;
  QImage        img;
  QDate         date;
  QTime         time;
  quint64       imgSize;
  int           mod, h, m, s;

  img = QImage::fromData(query.value(2).toByteArray());
  imgDim = QSize(query.value(3).toInt(), query.value(4).toInt());
  imgSize = query.value(3).toInt() * query.value(4).toInt();

  date = QDate::fromJulianDay(query.value(7).toInt());
  mod = query.value(8).toInt();
  h = mod / 3600;
  mod = mod % 3600;
  m = mod / 60;
  s = mod % 60;
  time = QTime(h, m, s);

  path = rootDir + query.value(0).toString();
  //qDebug() << "KbvGlobal::itemFromRecord " <<path <<query.value(1).toString(); //###########

  item = new  Kbv::kbvItem;
  item->insert(Kbv::FilePathRole,   QVariant(path));
  item->insert(Kbv::FileNameRole,   query.value(1));
  item->insert(Kbv::FileSizeRole,   query.value(5));
  item->insert(Kbv::FileDateRole,   QVariant(QDateTime(date, time, Qt::LocalTime)));
  item->insert(Kbv::ImageSizeRole,  QVariant(imgSize));
  item->insert(Kbv::ImageDimRole,   QVariant(imgDim));
  item->insert(Kbv::UserDateRole,   query.value(9));
  item->insert(Qt::DecorationRole,  QVariant(img));
  item->insert(Kbv::PrimaryKeyRole, query.value(10));
  item->insert(Kbv::FileCRCRole,    query.value(6));

  return item;
}
/*************************************************************************//*!
 * Build the record data from an image file only. Do not touch user date or
 * keywords. Fill a map with values for the keys:
 * icon, imageW, imageH, fileSize, dateChanged, timeChanged, keywords.
 * dateChanged and timeChanged are integer so we convert:
 * dateChanged = filedate ToJulianDay, timeChanged = h*3600 + min*60 + sec*60
 * Returns true when record could be created successfully.
 */
bool    KbvGlobal::createRecordData(QString filepath, QString filename,
                          QMap<QString, QVariant> &data, int iconSize, int keyWordType)
{
  QSize         imgDim;
  QImage        fileimage;
  QImageReader  reader;
  QFileInfo     file;
  int           w=1, h=1, md, mt;
  float         ar;
  QByteArray    ba;
  QBuffer       buffer(&ba);


  file = QFileInfo(filepath+filename);
  reader.setFileName(filepath+filename);

  //get image size, scale down only when imagesize > iconsize, keep aspect ratio
  imgDim = reader.size();
  if((imgDim.width() <= iconSize) && (imgDim.height() <= iconSize))
    {
      reader.setScaledSize(imgDim);
    }
  else if((imgDim.width() > iconSize) || (imgDim.height() > iconSize))
    {
      ar = (float) imgDim.height() / imgDim.width();
      if(ar >= 1)
        {
          w = (int)(iconSize/ar);
          h = iconSize;
        }
      else
        {
          w = iconSize;
          h = (int)(ar * iconSize);
        }
      reader.setScaledSize(QSize(w, h));
    }

  md = file.lastModified().date().toJulianDay();
  mt = file.lastModified().time().hour()*3600 + file.lastModified().time().minute()*60 + file.lastModified().time().second();
  buffer.open(QIODevice::WriteOnly);

  data.insert("imageW",       QVariant(imgDim.width()));
  data.insert("imageH",       QVariant(imgDim.height()));
  data.insert("fileSize",     QVariant(file.size()));
  data.insert("dateChanged",  QVariant(md));
  data.insert("timeChanged",  QVariant(mt));
  data.insert("keywords",     QVariant(this->extractKeywords(filename, keyWordType)));

  if(reader.read(&fileimage))
    {
      fileimage.save(&buffer, "JPG");
      data.insert("icon",         QVariant(ba));
    }
  else
    {
      if(filename.endsWith(QString(dbNameExt)))
        {
          reader.setFileName(":/kbv/icons/kbv_crown_40x64.png");
        }
      else
        {
          reader.setFileName(":/kbv/icons/kbv_no_support.png");
        }
      reader.read(&fileimage);
      fileimage.save(&buffer, "PNG");
      data.insert("icon",         QVariant(ba));
    }
  return true;
}
/*************************************************************************//*!
 * Extract keywords from file name with Regular Expression due to database
 * settings. All found keywords are kept in one string.
 * For text search sqlite3 only offers a % symbol in the LIKE operator to match
 * a sequence of zero or more characters in a string.
 * Hence, for inaccurate search it's sufficient to enclose the search term with
 * % symbols and find the enclosed string at arbitrary position in the 'keyword'
 * part of a database record.
 * For accurate search the search term must match exactly a keyword. To achieve
 * this we need regular expressions which are not supported in sqlite3 or an
 * other way to solve the problem. The other way does this: surround the keywords
 * in the database with a special character serving as word boundary (defined
 * as 'keywordBoundary'). So an accurate search simply has to enclose the search
 * term in word boundaries and has to perform an inaccurate search as described
 * above. The result is an exact match of search term and keyword.
 */
QString    KbvGlobal::extractKeywords(QString filename, int mode)
{
  QStringList   kwList;
  QString   str, keywords, kwb;
  int       n;

  //qDebug() << "KbvGlobal::extractKeywords" << filename << mode; //###########

  kwb = QString(keywordBoundary);
  keywords = "";
  n = filename.lastIndexOf(".", -1, Qt::CaseInsensitive);
  str = filename.left(n);
  //file type due to file extension
  if(mode & Kbv::keywordFiletype)
    {
      keywords.append(kwb + filename.right(filename.size()-n-1) + kwb);
    }
  //words and numbers:
  //replace non word characters and "_" with " "
  //then replace multiple white spaces by one
  if(mode & Kbv::keywordWordsNumbers)
    {
      str.replace(QRegExp("\\W+|_+"), " ");
      str.replace(QRegExp("\\s+"), " ");  //replace multiple white spaces
      //qDebug() << "KeywordWordsNumbers" << filename << str; //###########
    }
  //words only:
  //replace non word characters and "_" with " "
  //remove all digits after a word boundary
  //remove all digits before a word boundary
  //then replace multiple white spaces by one
  if(mode & Kbv::keywordWordsOnly)
    {
      str.replace(QRegExp("\\W+|_+"), " ");
      str.replace(QRegExp("\\b\\d+"), "");
      str.replace(QRegExp("\\d+\\b"), "");
      str.replace(QRegExp("\\s+"), " ");
      //qDebug() << "KeywordWordsOnly" << filename << str; //###########
    }
  //Get a list of single keywords and add a boundaries around each
  kwList = str.split(" ",QString::SkipEmptyParts,Qt::CaseInsensitive);
  for(int i=0; i<kwList.size();i++)
    {
      keywords.append(kwb + kwList.at(i) + kwb);
    }
  //qDebug() << "extractKeywords" << filename << keywords; //###########
  //Test file: Fische-1234Fisch Fische02__&_%08Pesces-11_22 1(33)___.jpg
  return keywords;
}
/*************************************************************************//*!
 * Reads the version info of used shared libraries:
 * jpeglib (dirty c-code), libpng, libexiv2
 */
void    KbvGlobal::getLibraryInfos(QStringList &info)
{
  QString     verstr;
  int         vernum;
  cv::String  ocvVer;

  info.clear();

  //libpng version: from 1.0.1 it's like xxyyzz, where x=major, y=minor, z=release
  vernum = png_access_version_number();
  verstr = QString::number(vernum);
  verstr.insert(verstr.length()-4,".");
  verstr.insert(verstr.length()-2,".");
  //qDebug() << "version libpng:" <<vernum <<verstr; //###########
  info.append((QString("PNG lib")));
  info.append(verstr);
  info.append(QString("libpng.org"));
  info.append("Portable Network Graphics");

  // Need to construct a decompress struct and give it an error handler
  // by passing an invalid version number we always trigger an error
  // the error returns the linked version number as the first integer parameter
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr error_mgr;
  error_mgr.error_exit = &JPEGVersionError;
  cinfo.err = &error_mgr;
  jpeg_CreateDecompress(&cinfo, -1 /*version*/, sizeof(cinfo)); // Pass -1 to force an error
  vernum = cinfo.err->msg_parm.i[0];
  jpeg_destroy_decompress(&cinfo);
  verstr = QString::number(vernum);
  //qDebug() << "version libjpeg:" <<verstr; //###########;
  info.append((QString("JPEG lib")));
  info.append(verstr);
  info.append(QString("JPEG.org"));
  info.append("Joint Photographic Experts Group");

  KbvExiv exiv;
  verstr = exiv.getLibVersion();
  //qDebug() << "version libexiv2.so:" <<verstr; //###########;
  info.append((QString("Metadata")));
  info.append(verstr);
  info.append(QString("exiv2.org"));
  info.append("Exif, IPTC and XMP metadata");

  //TODO: wait for openCV >= 3.4.2
//  ocvVer = cv::getVersionString();    // >= 3.4.2
//  verstr = QString::fromStdString(ocvVer.operator std::string());
  verstr = QString("3.2.0");            // < 3.4.2
  info.append((QString("OpenCV")));
  info.append(verstr);
  info.append(QString("opencv.org"));
  info.append("Open Source Computer Vision");
}
void  JPEGVersionError(j_common_ptr cinfo)
{
  Q_UNUSED(cinfo);

  //int jpegver = cinfo->err->msg_parm.i[0];
  //qDebug() << "version libjpeg:" <<jpegver; //###########;
}

/*************************************************************************//*!
 * Build the crc32 checksum of a file. Return crc value or zero on faults.
 * The file is stepwise read into a bloc of bytes (unsigned char) of given
 * length then on this bloc the crc sum is calculated. This is repeated
 * until the file end is reached.
 */
quint32    KbvGlobal::crc32FromFile(QString pathName)
{
  QFile         crcFile;
  QByteArray    crcBloc;
  quint32       crc;
  int           i;


  crc = 0xFFFFFFFF;
  crcBloc = QByteArray(16384, '0');
  crcFile.setFileName(pathName);
  if(crcFile.open(QIODevice::ReadOnly))
    {
      while(!crcFile.atEnd())
        {
          crcBloc = crcFile.read(16384);

          for (i = 0; i < crcBloc.length(); i++)
            {
              crc = ((crc >> 8) & 0x00FFFFFF) ^ crcTable[(crc ^ crcBloc[i]) & 0xFF];
            }
        }

      crc = crc ^ 0xFFFFFFFF;
    }
  else
    {
      crc = 0;
    }
  return crc;
}

/* Private functions ********************************************************/
/*************************************************************************//*!
 * Build the table for crc32 checksums due to polynom 0xEDB88320L.
 */
void    KbvGlobal::crc32BuildTab()
{
  quint32   crc, poly;
  int       i, j;

  poly = 0xEDB88320L;
  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
        {
          if (crc & 1)
            {
              crc = (crc >> 1) ^ poly;
            }
          else
            {
              crc >>= 1;
            }
        }
      crcTable[i] = crc;
    }
}

/****************************************************************************/
