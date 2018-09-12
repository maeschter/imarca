/*****************************************************************************
 * KbvExiv
 * Dyn. library interface to libExiv2 >= 0.25, main functions
 * Derived from libkexiv2 0.1.5, uses libexiv2
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-24 18:07:08 +0100 (Sa, 24. Feb 2018) $
 * $Rev: 981 $
 * Created: 2015.10.02
 * Exiv2: http://www.exiv2.org
 * Exif : http://www.exif.org/Exif2-2.PDF
 * Iptc : http://www.iptc.org/std/IIM IIMV4.2.pdf
 *        http://www.iptc.org/std/Iptc4xmpCore/ Iptc4xmpCore_1.0-spec-XMPSchema_8.pdf
 * Xmp  : http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
 * guide: http://www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
 *****************************************************************************/
#ifndef KBVEXIV_H
#define KBVEXIV_H

#include <QtCore>
#include <QImage>
#include <exiv2/exiv2.hpp>
#include "kbvExivGlobal.h"

class KBVEXIVSHARED_EXPORT KbvExiv
{
  
public:
  KbvExiv();                              //Standard constructor
  KbvExiv(const KbvExiv &d);              //Copy constructor
  KbvExiv(const QByteArray &imageData);   //Load from parsed data
  KbvExiv(const QString &imagePath);      //load from image file

virtual   ~KbvExiv();

  KbvExiv& operator=(const KbvExiv &d);

public:
  std::string       imageComments;
  Exiv2::ExifData   exifMetadata;
  Exiv2::IptcData   iptcMetadata;
  Exiv2::XmpData    xmpMetadata;

  //Types
  //A map used to store Tags Key and Tags Value.
  typedef QMap<QString, QString> MetaDataMap;

  /* A map used to store a list of Alternative Language values.
   * The map key is the language code following RFC3066 notation
   * (like "fr-FR" for French), and the map value the text.
   */
  typedef QMap<QString, QString> AltLangMap;

  /* A map used to store Tags Key and a list of Tags properties :
   * name, title, description.
   */
  typedef QMap<QString, QStringList> TagsMap;

  //IPTC data character set due to IPTC_NAA_v4_2014
  enum  IptcEncoding
    {
      IptcUTF8   = 196,   //UCS Transformation Format
      Iptc88591  = 100    //ISO 8859-1
    };
  //The image orientation values given by Exif metadata.
  enum ImageOrientation
    {
      ORIENTATION_UNSPECIFIED  = 0,
      ORIENTATION_NORMAL       = 1,
      ORIENTATION_HFLIP        = 2,
      ORIENTATION_ROT_180      = 3,
      ORIENTATION_VFLIP        = 4,
      ORIENTATION_ROT_90_HFLIP = 5,
      ORIENTATION_ROT_90       = 6,
      ORIENTATION_ROT_90_VFLIP = 7,
      ORIENTATION_ROT_270      = 8
    };
  //Xmp tag types, used by setXmpTag, only first three types are used
  enum XmpTagType
    {
      NormalTag    = 0,
      ArrayBagTag  = 1,
      StructureTag = 2,
      ArrayLangTag = 3,
      ArraySeqTag  = 4
    };
//************************************************************************//*!
/* Types: Exiv2::TypeId
 * unsignedByte     Exif BYTE type, 8-bit unsigned integer
 * asciiString      Exif ASCII type, 8-bit byte.
 * unsignedShort    Exif SHORT type, 16-bit (2-byte) unsigned integer.
 * unsignedLong     Exif LONG type, 32-bit (4-byte) unsigned integer.
 * unsignedRational Exif RATIONAL type, two LONGs: numerator and denumerator of a fraction.
 * signedByte       Exif SBYTE type, an 8-bit signed (twos-complement) integer.
 * signedShort      Exif SSHORT type, a 16-bit (2-byte) signed (twos-complement) integer.
 * signedLong       Exif SLONG type, a 32-bit (4-byte) signed (twos-complement) integer.
 * signedRational   Exif SRATIONAL type, two SLONGs: numerator and denumerator of a fraction.
 * tiffFloat        TIFF FLOAT type, single precision (4-byte) IEEE format.
 * tiffDouble       TIFF DOUBLE type, double precision (8-byte) IEEE format.
 * tiffIfd          TIFF IFD type, 32-bit (4-byte) unsigned integer.
 * undefined        Exif UNDEFINED type, an 8-bit byte that may contain anything.
*/
//************************************************************************//*!
// Get version string from libexiv2
  QString       getLibVersion();

//************************************************************************//*!
// Load metadata from image at filePath or from already loaded imgData.
  bool          loadFromFile(const QString &imagePath);
  bool          loadFromData(const QByteArray &imgData);

//************************************************************************//*!
// Write metadata into image at filePath (create new or replace existant).
  bool          writeToFile(const QString &imagePath);

//************************************************************************//*!
// Image comments are located in the COM marker segment of an jpeg image.
// Comment may be up to 65533 bytes of pure ASCII text.
  bool          hasComments() const;
  bool          clearComments();
  QByteArray    getComments() const;
  QString       getCommentsDecoded() const;
  bool          setComments(const QString &data);

  QString       getMimeType();
  QSize         getImageSize();

//************************************************************************//*!
// Extract a subset of iptc meta data from image at 'imagePath' and return
// them as string (due to core schema 1.2 spec oct. 2014):
// 2:05 Title, 2:105 Headline, 2:80 Creator, 2:55 Date Created, 2:60 Time Created,
// 2:101 Country, 2:95 Province/State, 2:90 City, 2:92 Sublocation, 2:115 Source,
// 2:120 Caption/Abstract
  QString       getIptcInfo(const QString &imagePath);

//************************************************************************//*!
// Extract a subset of exif meta data from file at 'imagePath' and return them
// as string: Image.Make, Image.Model, Image.Artist, Image.DateTime, ExposureTime,
// Photo.FNumber, Photo.Flash, Photo.PixelXDimension, Photo.PixelYDimension,
// Image.ImageDescription, Image.Software
  QString       getExifInfo(const QString &imagePath);

//************************************************************************//*!
// Get/set exif image alignment as byte array (two bytes, short value)
  QByteArray    getPhotoAlignmentFromFile(const QString &imagePath);

//************************************************************************//*!
// common functions
  static bool     is7BitAscii(const QByteArray& s);
  static QString  convertToGPSCoordinateString(const bool isLatitude, double coordinate);
  static QString  convertToGPSCoordinateString(const long int numeratorDegrees, const long int denominatorDegrees,
                                               const long int numeratorMinutes, const long int denominatorMinutes,
                                               const long int numeratorSeconds, long int denominatorSeconds,
                                               const char directionReference);
  static bool     convertFromGPSCoordinateString(const QString& gpsString, double* const degrees);
  static bool     convertFromGPSCoordinateString(const QString& gpsString,
                                                 long int* numeratorDegrees, long int* denominatorDegrees,
                                                 long int* numeratorMinutes, long int* denominatorMinutes,
                                                 long int* numeratorSeconds, long int* denominatorSeconds,
                                                 char* directionReference);
  static void     convertToRational(const double number, long int* const numerator,
                                    long int* const denominator, const int rounding);
  static QString  detectLanguageAlt(const QString& value, QString& lang);
  static bool     registerXmpNameSpace(const QString& uri, const QString& prefix);
  static bool     unregisterXmpNameSpace(const QString& uri);
  

// Public IPTC functions
  bool          hasIptc() const;
  bool          clearIptc();
  bool          removeIptcTag(const QString iptcTagName);
  QString       getIptcTagTitle(const QString iptcTagName);
  QString       getIptcTagDescription(const QString iptcTagName);
  int           getIptcCharacterSet() const;
  QByteArray    getIptcTagData(const QString iptcTagName) const;
  QString       getIptcTagDataString(const QString iptcTagName, int charSet) const;
  QStringList   getIptcTagStringList(const QString iptcTagName, bool escapeCR, int charSet) const;
  QStringList   getIptcKeywords(const int charSet) const;
  QStringList   getIptcSubjects() const;
  bool          setIptcCharacterSet(int charSet);
  bool          setIptcTagData(const QString iptcTagName, const QByteArray &data);
  bool          setIptcTagDataString(const QString iptcTagName, const QString &value);
  bool          setIptcTagStringList(const QString iptcTagName, int maxSize,
                                     const QStringList& oldValues, const QStringList& newValues);
  bool          setIptcKeywords(const QStringList &keywords);
  bool          setIptcSubjects(const QStringList &subjects);

// Public Exif functions
  bool          hasExif() const;
  bool          clearExif();
  bool          removeExifTag(const QString exifTagName);
  QString       getExifTagTitle(const char* exifTagName);
  QString       getExifTagDescription(const char* exifTagName);
  QByteArray    getExifTagData(const QString exifTagName) const;
  QString       getExifTagDataString(const QString exifTagName) const;
  QString       getExifTagDataString(quint16 tag, const QString groupName) const;
  bool          getExifTagDataLong(const QString exifTagName, qint32 &tagValue) const;
  bool          getExifTagDataLong(const QString exifTagName, qint32 &tagValue, int component) const;
  bool          getExifTagDataRational(const QString exifTagName, qint32& num, qint32& den, int component) const;
  QString       getExifComment() const;
  QString       getExifDescription() const;
  MetaDataMap   getExifTagsDataList(const QStringList& exifKeysFilter=QStringList(), bool invertSelection=false) const;
  TagsMap       getExifStdTagsList() const;
  TagsMap       getMakernoteTagsList() const;
  QImage        getExifThumbnail(bool rotate);
  bool          setExifTagData(const QString exifTagName, const QByteArray& data);
  bool          setExifTagDataString(const QString exifTagName, const QString& value);
  bool          setExifTagDataLong(const QString exifTagName, qint32 value);
  bool          setExifTagDataRational(const QString exifTagName, qint32& num, qint32& den);
  bool          setExifCommentDescription(const QString& comment, const QString& description);
  bool          setExifDateTime(const QString exifTagName, const QDateTime& dateTime);
  bool          setExifThumbnail(const QImage& thumbnail);
  bool          removeExifThumbnail();
  bool          rotateExifQImage(QImage& image, KbvExiv::ImageOrientation orientation);

// Public Exif GPS functions
  bool          getGPSCoordinates(double& altitude, double& latitude, double& longitude) const;
  bool          getGPSLatitudeNumber(double* const latitude) const;
  bool          getGPSLongitudeNumber(double* const longitude) const;
  bool          getGPSAltitude(double* const altitude) const;
  QString       getGPSLatitudeString() const;
  QString       getGPSLatitudeDegMinSecString() const;
  QString       getGPSLongitudeString() const;
  QString       getGPSLongitudeDegMinSecString() const;
  QString       getGPSDateString() const;
  QString       getGPSTimeString() const;
  QString       getGPSDirectionString() const;
  QString       getGPSTrackString() const;
  QString       getGPSSpeedString() const;
  bool          setGPSCoordinates(const double altitude, const double latitude, const double longitude);
  bool          setGPSCoordinates(const double altitude, const QString& latitude, const QString& longitude);
  bool          setGPSDateTimeStamp(const QString& date, const QString& time);
  bool          removeGPSTags();

// Public XMP functions
  bool          hasXmp() const;
  bool          clearXmp();
  bool          removeXmpTag(const char* xmpTagName);
  QByteArray    getXmp() const;
  bool          setXmp(const QByteArray& data);
  QString       getXmpTagTitle(const char* xmpTagName);
  QString       getXmpTagDescription(const char* xmpTagName);
  QString       getXmpTagDataString(const char* xmpTagName, bool escapeCR=true) const;
  QString       getXmpTagStringLangAlt(const char* xmpTagName, const QString& langAlt, bool escapeCR) const;
  QStringList   getXmpTagStringSeq(const char* xmpTagName, bool escapeCR=true) const;
  QStringList   getXmpTagStringBag(const char* xmpTagName, bool escapeCR) const;
  QVariant      getXmpTagVariant(const char* xmpTagName, bool rationalAsListOfInts=true, bool stringEscapeCR=true) const;
  QStringList   getXmpKeywords() const;
  QStringList   getXmpSubjects() const;
  QStringList   getXmpSubCategories() const;
  AltLangMap    getXmpTagStringListLangAlt(const char* xmpTagName, bool escapeCR=true) const;
  MetaDataMap   getXmpTagsDataList(const QStringList& xmpKeysFilter=QStringList(), bool invertSelection=false) const;
  TagsMap       getXmpTagsList();
  bool          setXmpTagDataString(const char* xmpTagName, const QString& value);
  bool          setXmpTagDataString(const char* xmpTagName, const QString& value, KbvExiv::XmpTagType type);
  bool          setXmpTagStringListLangAlt(const char* xmpTagName, const KbvExiv::AltLangMap& values);
  bool          setXmpTagStringLangAlt(const char* xmpTagName, const QString& value, const QString& langAlt);
  bool          setXmpTagStringSeq(const char* xmpTagName, const QStringList& seq);
  bool          setXmpTagStringBag(const char* xmpTagName, const QStringList& bag);
  bool          addToXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToAdd);
  bool          removeFromXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToRemove);
  bool          setXmpKeywords(const QStringList& newKeywords);
  bool          removeXmpKeywords(const QStringList& keywordsToRemove);
  bool          setXmpSubjects(const QStringList& newSubjects);
  bool          removeXmpSubjects(const QStringList& subjectsToRemove);
  bool          setXmpSubCategories(const QStringList& newSubCategories);
  bool          removeXmpSubCategories(const QStringList& categoriesToRemove);


private:
#include "kbvExivLocal.h"
  
};

#endif // KBVEXIV_H
/****************************************************************************/
