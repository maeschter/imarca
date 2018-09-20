/*****************************************************************************
 * KbvExiv
 * Dyn. library interface to libExiv2, common functions
 * Derived from libkexiv2 0.1.5
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2015.10.02
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvExiv.h"

extern "C"
{
#include  <sys/stat.h>
#include  <utime.h>
}
//std constructor
KbvExiv::KbvExiv()
{
  QString dir;

  metadataWritingMode = KbvExiv::WRITETOIMAGEONLY;
  dir = QCoreApplication::applicationDirPath();
  if(!dir.startsWith(QString("/usr/")))
    {
      //qDebug() << "KbvExiv::KbvExiv app"<<dir; //###########
    }
}
//copy constructor
KbvExiv::KbvExiv(const KbvExiv &d)
{
  copyPrivateData(&d);
}
//load from data
KbvExiv::KbvExiv(const QByteArray& imageData)
{
  loadFromData(imageData);
  metadataWritingMode = KbvExiv::WRITETOIMAGEONLY;
}
//load from image file
KbvExiv::KbvExiv(const QString& imagePath)
{
  loadFromFile(imagePath);
  metadataWritingMode = KbvExiv::WRITETOIMAGEONLY;
}

KbvExiv::~KbvExiv()
{
  //qDebug() << "KbvExiv::~KbvExiv"; //###########
}

KbvExiv& KbvExiv::operator=(const KbvExiv &d)
{
  if(this != &d)
    {
      copyPrivateData(&d);
    }
  return *this;
}

//************************************************************************//*!
// Get version string from libexiv2
QString KbvExiv::getLibVersion()
{
  return QString::fromStdString(Exiv2::versionString());
}

/*************************************************************************//*!
 * Public load and save functions *******************************************/
QString KbvExiv::getMimeType()
{
  return this->mimeType;
}
QSize   KbvExiv::getImageSize()
{
  return  this->pixelSize;
}

//*****************************************************************************
bool  KbvExiv::loadFromFile(const QString &filePath)
{
  if (filePath.isEmpty())
    {
      return false;
    }
  bool  hasLoaded = false;
  try
    {
      Exiv2::Image::AutoPtr image;
      image = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(filePath)));
      image->readMetadata();
      this->filePath = filePath;

      // Size and mimetype ---------------------------------
      this->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
      this->mimeType = QString::fromStdString(image->mimeType().c_str());

      // Image comments ---------------------------------
      this->imageComments = image->comment();
      //qDebug() << "KbvExiv::loadFromFile comment"; //###########

      // Exif metadata ----------------------------------
      this->exifMetadata = image->exifData();
      //qDebug() << "KbvExiv::loadFromFile exif"; //###########

      // Iptc metadata ----------------------------------
      this->iptcMetadata = image->iptcData();
      //qDebug() << "KbvExiv::loadFromFile iptc"; //###########

      // Xmp metadata -----------------------------------
      this->xmpMetadata = image->xmpData();
      //qDebug() << "KbvExiv::loadFromFile xmp"; //###########

      hasLoaded = true;
    }
  catch( Exiv2::Error& e )
    {
      //qDebug() << "KbvExiv::loadFromFile Cannot load metadata"<<e.what() <<e.code(); //###########
    }

  try
    {
      if(this->useXMPSidecar4Reading)
        {
          QString xmpSidecarPath = sidecarFilePathForFile(filePath);
          QFileInfo xmpSidecarFileInfo(xmpSidecarPath);

          Exiv2::Image::AutoPtr xmpsidecar;
          if(xmpSidecarFileInfo.exists() && xmpSidecarFileInfo.isReadable())
            {
              // Read sidecar data
              xmpsidecar = Exiv2::ImageFactory::open((const char*)QFile::encodeName(xmpSidecarPath));
              xmpsidecar->readMetadata();

              // Merge
              this->loadSidecarData(xmpsidecar);
              hasLoaded = true;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      //qDebug() << "KbvExiv::loadFromFile cannot load metadata XMP sidecar"<<e.what() <<e.code(); //###########
    }
  return hasLoaded;
}
//*****************************************************************************
bool  KbvExiv::loadFromData(const QByteArray& imgData)
{
  if(imgData.isEmpty())
    {
      return false;
    }

  bool  hasLoaded = false;
  try
    {
      Exiv2::Image::AutoPtr image;
      image = Exiv2::ImageFactory::open((Exiv2::byte*)imgData.data(), imgData.size());
      image->readMetadata();

      // Size and mimetype ---------------------------------
      this->pixelSize = QSize(image->pixelWidth(), image->pixelHeight());
      this->mimeType  = QString::fromStdString(image->mimeType().c_str());

      // Image comments ---------------------------------
      this->imageComments = image->comment();

      // Exif metadata ----------------------------------
      this->exifMetadata = image->exifData();

      // Iptc metadata ----------------------------------
      this->iptcMetadata = image->iptcData();

      // Xmp metadata -----------------------------------
      this->xmpMetadata = image->xmpData();

      hasLoaded = true;
    }
  catch( Exiv2::Error& e )
    {
      //qDebug() <<"KbvExiv::loadFromData cannot load metadata"<<e.what() <<e.code(); //###########
    }
  return hasLoaded;
}
/*************************************************************************//*!
 * Write metadata into image at filePath (create new or replace existant).
 */
bool  KbvExiv::writeToFile(const QString& imagePath)
{
  QFileInfo   givenFileInfo, finfo, dinfo;
  QString     regularFilePath;

/* If our image is really a symlink, we should follow the symlink so that
   when we delete the file and rewrite it, we are honoring the symlink
  (rather than just deleting it and putting a file there).
  However, this may be surprising to the user when they are writing sidecar
  files.  They might expect them to show up where the symlink is.  So, we
  shouldn't follow the link when figuring out what the filename for the
  sidecar should be.  metadataWritingMode = KbvExiv::WRITETOIMAGEONLY;

  Note, we are not yet handling the case where the sidecar itself is a symlink.
*/
  regularFilePath = imagePath;               // imageFilePath might be a
                                             // symlink.  Below we will change
                                             // regularFile to the pointed to
                                             // file if so.
  givenFileInfo = QFileInfo(imagePath);
  if(givenFileInfo.isSymLink())
    {
      //qDebug() <<"KbvExiv::save symlink" <<imagePath; //###########
      regularFilePath = givenFileInfo.canonicalPath();// Walk all the symlinks
    }

  //never touch the file if is read only.
  finfo = QFileInfo(regularFilePath);
  dinfo = QFileInfo(finfo.path());
  if(!dinfo.isWritable())
    {
      //qDebug() <<"KbvExiv::writeToFile readonly" <<imagePath; //###########
      return false;
    }

  //qDebug() <<"KbvExiv::save metadataWritingMode" << this->metadataWritingMode;
  bool writeToFile                     = false;
  bool writeToSidecar                  = false;
  bool writeToSidecarIfFileNotPossible = false;
  bool writtenToFile                   = false;
  bool writtenToSidecar                = false;
  switch(this->metadataWritingMode)     //in constructor set to: WRITETOIMAGEONLY
    {
      case KbvExiv::WRITETOSIDECARONLY:
            writeToSidecar = true;
            break;
      case KbvExiv::WRITETOIMAGEONLY:
            writeToFile    = true;
            break;
      case KbvExiv::WRITETOSIDECARANDIMAGE:
            writeToFile    = true;
            writeToSidecar = true;
            break;
      case KbvExiv::WRITETOSIDECARONLY4READONLYFILES:
            writeToFile = true;
            writeToSidecarIfFileNotPossible = true;
            break;
    }

  if (writeToFile)
    {
      writtenToFile = saveToFile(finfo);
    }
  if(writeToSidecar || (writeToSidecarIfFileNotPossible && !writtenToFile))
    {
      writtenToSidecar = saveToXMPSidecar(imagePath);
      //qDebug() <<"KbvExiv::save written to XMP sidecar"<<writtenToSidecar;
    }
  return (writtenToFile || writtenToSidecar);
}
/*************************************************************************//*!
 * Public information functions *********************************************/
/*************************************************************************//*!
 * Extract a subset of iptc meta data from image at 'imagePath' and return
 * them as string (due to core schema 1.2 spec oct. 2014):
 * 2:05 Title, 2:105 Headline, 2:80 Creator, 2:55 Date Created, 2:60 Time Created,
 * 2:101 Country, 2:95 Province/State, 2:90 City, 2:92 Sublocation, 2:115 Source,
 * 2:120 Caption/Abstract
 */
QString KbvExiv::getIptcInfo(const QString &filePath)
{
  KbvExiv     metadata;
  QString     str, info;
  int         charSet;

//read metadata from image file
  metadata = KbvExiv(filePath);
  if(!metadata.hasIptc())
    {
      return QString();
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
 * Extract a subset of exif meta data from file at 'imagePath' and return them
 * as string: Image.Make, Image.Model, Image.Artist, Image.DateTime, ExposureTime,
 * Photo.FNumber, Photo.Flash, Photo.PixelXDimension, Photo.PixelYDimension,
 * Image.ImageDescription, Image.Software
 */
QString KbvExiv::getExifInfo(const QString &filePath)
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
      return QString();
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
/************************************************************************//*!
 * Extract exif image alignment from file and return it as byte array ( 2 bytes)
 */
QByteArray KbvExiv::getPhotoAlignmentFromFile(const QString &imagePath)
{
  KbvExiv  metadata = KbvExiv(imagePath);
  return metadata.getExifTagData(QString("Exif.Image.Orientation"));
}

/*************************************************************************//*!
 * Public comment functions *************************************************
 * This is neither IPTC comment nor Exif user comment. Image comments are
 * located in the COM marker segment of an jpeg image. Comment may be up to
 * 65533 bytes of pure ASCII text.
 */

bool KbvExiv::hasComments() const
{
    return !this->imageComments.empty();
}
//*****************************************************************************
bool KbvExiv::clearComments()
{
    return setComments(QByteArray());
}
//*****************************************************************************
QByteArray KbvExiv::getComments() const
{
    return QByteArray(this->imageComments.data(), this->imageComments.size());
}
//*****************************************************************************
QString KbvExiv::getCommentsDecoded() const
{
    return this->detectEncodingAndDecode(this->imageComments);
}
//*****************************************************************************
bool KbvExiv::setComments(const QString& data)
{
    this->imageComments = data.toStdString();
    return true;
}

/*************************************************************************//*!
 * Private functions ********************************************************/
void KbvExiv::copyPrivateData(const KbvExiv* const other)
{
  imageComments         = other->imageComments;
  exifMetadata          = other->exifMetadata;
  iptcMetadata          = other->iptcMetadata;
  xmpMetadata           = other->xmpMetadata;

  filePath              = other->filePath;
  mimeType              = other->mimeType;
  pixelSize             = other->pixelSize;
  writeRawFiles         = other->writeRawFiles;
  keepFileTimeStamp     = other->keepFileTimeStamp;
  useXMPSidecar4Reading = other->useXMPSidecar4Reading;
  metadataWritingMode   = other->metadataWritingMode;
}
//*****************************************************************************
bool  KbvExiv::saveToFile(const QFileInfo &finfo) const
{
  QStringList rawTiffBasedSupported, rawTiffBasedNotSupported;
  QString   ext;

  if(!finfo.isWritable())
    {
      //File is readonly
      return false;
    }

  // Raw files supported by Exiv2 0.25
  rawTiffBasedSupported <<"cr2" <<"dng" << "nef" << "pef" << "orf" <<"pgf" << "srw";

  // Raw files not supported by Exiv2 0.25
  rawTiffBasedNotSupported  << "3fr" << "arw" << "dcr" << "erf" << "k25" << "kdc"
                            << "mrw" << "mos" << "raw" << "sr2" << "srf" << "rw2";

  ext = finfo.suffix().toLower();
  if(!writeRawFiles && (rawTiffBasedSupported.contains(ext) || rawTiffBasedNotSupported.contains(ext)) )
    {
      return false;
    }

  try
    {
      Exiv2::Image::AutoPtr image;
      image = Exiv2::ImageFactory::open((const char*)(QFile::encodeName(finfo.filePath())));
      return saveOperations(finfo, image);
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::saveToFile cannot save metadata to file"<<e.what() <<e.code(); //###########
      return false;
    }
}
//*****************************************************************************
bool  KbvExiv::saveOperations(const QFileInfo& finfo, Exiv2::Image::AutoPtr image) const
{
  Exiv2::AccessMode   mode;
  bool                wroteComment=false, wroteEXIF=false, wroteIPTC=false, wroteXMP=false;
  QStringList         untouchedTags;

  try
    {
      // We need to load target file metadata to merge with new one. It's mandatory
      //with TIFF format: like all tiff file structure is based on Exif.
      image->readMetadata();

      // Image Comments ---------------------------------
      mode = image->checkMode(Exiv2::mdComment);
      if((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
          image->setComment(this->imageComments);
          wroteComment = true;
        }
      //qDebug() << "KbvExiv::saveOperations wroteComment"<<wroteComment; //###########

      mode = image->checkMode(Exiv2::mdExif);
      if((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
          if(image->mimeType() == "image/tiff")
            {
              Exiv2::ExifData orgExif = image->exifData();
              Exiv2::ExifData newExif;
              // With tiff image we cannot overwrite whole Exif data as well, because
              // image data are stored in Exif container. We need to take a care about
              // to not loose image data.
              untouchedTags << "Exif.Image.ImageWidth";
              untouchedTags << "Exif.Image.ImageLength";
              untouchedTags << "Exif.Image.BitsPerSample";
              untouchedTags << "Exif.Image.Compression";
              untouchedTags << "Exif.Image.PhotometricInterpretation";
              untouchedTags << "Exif.Image.FillOrder";
              untouchedTags << "Exif.Image.SamplesPerPixel";
              untouchedTags << "Exif.Image.StripOffsets";
              untouchedTags << "Exif.Image.RowsPerStrip";
              untouchedTags << "Exif.Image.StripByteCounts";
              untouchedTags << "Exif.Image.XResolution";
              untouchedTags << "Exif.Image.YResolution";
              untouchedTags << "Exif.Image.PlanarConfiguration";
              untouchedTags << "Exif.Image.ResolutionUnit";

              for(Exiv2::ExifData::iterator it = orgExif.begin(); it != orgExif.end(); ++it)
                {
                  if(untouchedTags.contains(it->key().c_str()))
                    {
                      newExif[it->key().c_str()] = orgExif[it->key().c_str()];
                    }
                }

              Exiv2::ExifData readExif = this->exifMetadata;
              for(Exiv2::ExifData::iterator it = readExif.begin(); it != readExif.end(); ++it)
                {
                    if (!untouchedTags.contains(it->key().c_str()))
                    {
                        newExif[it->key().c_str()] = readExif[it->key().c_str()];
                    }
                }
              image->setExifData(newExif);
            }
          else
            {
              image->setExifData(this->exifMetadata);
            }
          wroteEXIF = true;
        }
      //qDebug() << "KbvExiv::saveOperations wroteExif"<<wroteEXIF; //###########

      // Iptc metadata ----------------------------------
      mode = image->checkMode(Exiv2::mdIptc);
      if((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
          image->setIptcData(this->iptcMetadata);
          wroteIPTC = true;
        }
      //qDebug() << "KbvExiv::saveOperations wroteIPTC"<<wroteIPTC; //###########

      // Xmp metadata -----------------------------------
      mode = image->checkMode(Exiv2::mdXmp);
      if((mode == Exiv2::amWrite) || (mode == Exiv2::amReadWrite))
        {
          image->setXmpData(this->xmpMetadata);
          wroteXMP = true;
        }
      //qDebug() << "KbvExiv::saveOperations wroteXMP"<<wroteXMP; //###########

      if(!wroteComment && !wroteEXIF && !wroteIPTC && !wroteXMP)
        {
          //qDebug() << "KbvExiv::saveOperations writing not supported"<<finfo.fileName(); //###########
          return false;
        }
      else if(!wroteEXIF || !wroteIPTC || !wroteXMP)
        {
          qDebug() << "KbvExiv::saveOperations writing is limitted"<<finfo.fileName(); //###########
        }

      if(keepFileTimeStamp)
        {
          // Don't touch access and modification timestamp of file.
          //qDebug() << "KbvExiv::saveOperations restore time stamp"; //###########
          struct stat    st;
          struct utimbuf ut;

          int ret = ::stat(QFile::encodeName(filePath), &st);     //filePath from last loadFromFile() <<<<<<<<<<<<<
          if(ret == 0)
            {
              ut.modtime = st.st_mtime;
              ut.actime  = st.st_atime;
            }

          image->writeMetadata();

          if(ret == 0)
            {
              ::utime(QFile::encodeName(filePath), &ut);
            }
        }
      else
        {
          //qDebug() << "KbvExiv::saveOperations don't restore time stamp"; //###########
          image->writeMetadata();
        }
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::saveOperations cannot save metadata to image"<<e.what() <<e.code(); //###########
    }
  return false;
}
//*****************************************************************************
QString KbvExiv::detectEncodingAndDecode(const std::string& value) const
{
  // For charset autodetection, we could use sophisticated code
  // (Mozilla chardet, KHTML's autodetection, QTextCodec::codecForContent),
  // but that is probably too much.
  // We check for UTF8, Local encoding and ASCII.

  if(value.empty())
    {
      return QString();
    }

  if(detectUtf8(value.c_str()))
    {
      return QString::fromUtf8(value.c_str());
    }

  // Utf8 has a pretty unique byte pattern. Thats not true for ASCII, it is
  //not possible to reliably autodetect different ISO-8859 charsets.
  //So we use either local encoding or latin1.

  return QString::fromLocal8Bit(value.c_str());
  //return QString::fromLatin1(value.c_str());
}
//****************************************************************************
//from: kstringhandler.cpp (ugly code, uses goto)
//but better than: exiv2 iptc.cpp detectCharset()
bool KbvExiv::detectUtf8(const char *buf ) const
{
  int i, n;
  unsigned char c;
  bool gotone = false;

  if(!buf)
    {
      return true; // whatever, just don't crash
    }

#define F 0   /* character never appears in text */
#define T 1   /* character appears in plain ASCII text */
#define I 2   /* character appears in ISO-8859 text */
#define X 3   /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

  static const unsigned char text_chars[256] = {
        /*                  BEL BS HT LF    FF CR    */
        F, F, F, F, F, F, F, T, T, T, T, F, T, T, F, F,  /* 0x0X */
        /*                              ESC          */
        F, F, F, F, F, F, F, F, F, F, F, T, F, F, F, F,  /* 0x1X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x2X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x3X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x4X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x5X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T,  /* 0x6X */
        T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F,  /* 0x7X */
        /*            NEL                            */
        X, X, X, X, X, T, X, X, X, X, X, X, X, X, X, X,  /* 0x8X */
        X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,  /* 0x9X */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xaX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xbX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xcX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xdX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I,  /* 0xeX */
        I, I, I, I, I, I, I, I, I, I, I, I, I, I, I, I   /* 0xfX */
  };

  /* *ulen = 0; */
  for (i = 0; (c = buf[i]); ++i)
    {
      if ((c & 0x80) == 0)        /* 0xxxxxxx is plain ASCII */
        {
          /*
          * Even if the whole file is valid UTF-8 sequences,
          * still reject it if it uses weird control characters.
          */
          if (text_chars[c] != T)
            {
              return false;
            }
        }
      else if ((c & 0x40) == 0) /* 10xxxxxx never 1st byte */
        {
          return false;
        }
      else
        {                           /* 11xxxxxx begins UTF-8 */
          int following;
          if ((c & 0x20) == 0)             /* 110xxxxx */
            {
              following = 1;
            }
          else if ((c & 0x10) == 0)      /* 1110xxxx */
            {
              following = 2;
            }
          else if ((c & 0x08) == 0)      /* 11110xxx */
            {
              following = 3;
            }
          else if ((c & 0x04) == 0)      /* 111110xx */
            {
              following = 4;
            }
          else if ((c & 0x02) == 0)      /* 1111110x */
            {
              following = 5;
            }
          else
            {
              return false;
            }

          for(n = 0; n < following; ++n)
            {
              i++;
              if(!(c = buf[i]))
                {
                  return false;
                }
              if((c & 0x80) == 0 || (c & 0x40))
                {
                  return false;
                }
            }
          gotone = true;
        }
    }
  return gotone;   /* don't claim it's UTF-8 if it's all 7-bit */
}

#undef F
#undef T
#undef I
#undef X

/*************************************************************************//*!
 * Static function: check for ascii (7 bit)
 */
bool  KbvExiv::is7BitAscii(const QByteArray& s)
{
  const int size = s.size();

  for(int i=0; i<size; i++)
    {
      if(!isascii(s[i]))
        {
          return false;
        }
    }
  return true;
}
/*****************************************************************************/
