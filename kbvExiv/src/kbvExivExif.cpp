/*****************************************************************************
 * KbvExivExif
 * Dyn. library interface to libExiv2, Exif functions
 * Derived from libkexiv2
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-05-05 17:35:37 +0200 (Fr, 05. Mai 2017) $
 * $Rev: 1042 $
 * Created: 2015.10.02
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <cmath>
#include "kbvExiv.h"

// Data types due to Exif Version 2.3
// BYTE      quint8    An 8-bit unsigned integer
// ASCII               An 8-bit byte containing one 7-bit ASCII code.
//                     The final byte is terminated with NULL.
// SHORT     quint16   A 16-bit (2-byte) unsigned integer
// LONG      quint32   A 32-bit (4-byte) unsigned integer
// RATIONAL            Two LONGs. The first is the numerator and the second is the denominator
// UNDEFINED           An 8-bit byte that can take any value depending on the field definition
// SLONG     qint32    A 32-bit (4-byte) signed integer
// SRATIONAL           Two SLONGs. The first is the numerator and the second is the denominator

// Group names:
// Image, Photo, GPSInfo, Iop, Thumbnail (IFD1 only)

/*************************************************************************//*!
 * Return 'true' when exif tags are present.
*/
bool KbvExiv::hasExif() const
{
    return !exifMetadata.empty();
}
/*************************************************************************//*!
 * Clear all exif tags in meta data container in memory.
*/
bool KbvExiv::clearExif()
{
  try
    {
      this->exifMetadata.clear();
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::clearExif exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::clearExif default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Remove exif tag indicated by exifTagName = familyName.groupName.tagName
*/
bool KbvExiv::removeExifTag(const QString exifTagName)
{
  try
    {
      Exiv2::ExifKey            exifKey(exifTagName.toStdString());
      Exiv2::ExifData::iterator it = this->exifMetadata.findKey(exifKey);

      if(it != this->exifMetadata.end())
        {
          this->exifMetadata.erase(it);
          return true;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::removeExifTag exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::removeExifTag default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Return the Exif tag title of 'exifTagName' or an empty string.
 */
QString KbvExiv::getExifTagTitle(const char* exifTagName)
{
  try
    {
      std::string exifkey(exifTagName);
      Exiv2::ExifKey ek(exifkey);
      return QString::fromLocal8Bit(ek.tagLabel().c_str());
    }
  catch (Exiv2::Error& e)
    {
      qDebug() <<"KbvExiv::getExifTagTitle exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagTitle default exception";
    }
  return QString();
}

/*************************************************************************//*!
 * Return the Exif tag description of 'exifTagName' or an empty string.
 */
QString KbvExiv::getExifTagDescription(const char* exifTagName)
{
  try
    {
      std::string exifkey(exifTagName);
      Exiv2::ExifKey ek(exifkey);
      return QString::fromLocal8Bit(ek.tagDesc().c_str());
    }
  catch (Exiv2::Error& e)
    {
      qDebug() <<"KbvExiv::getExifTagTagDescription exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagTagDescription default exception";
    }
  return QString();
}
/*************************************************************************//*!
 * Return a map of Exif tags name/value found in metadata sorted by exif keys.\n
 * The map only contains keys given by 'exifKeysFilter' (Groupnames like "Photo",
 * "Image", "Thumbnail", "Iop", "GPSInfo"). The filter can be inverted (exclude
 * all filter keys).\nThe map can be empty when nothing was found.
 */
KbvExiv::MetaDataMap KbvExiv::getExifTagsDataList(const QStringList& exifKeysFilter, bool invertSelection) const
{
  if(!this->hasExif())
    {
      return KbvExiv::MetaDataMap();
    }

  try
    {
      Exiv2::ExifData   exifData(this->exifMetadata);
      exifData.sortByKey();

      KbvExiv::MetaDataMap metaDataMap;

      for (Exiv2::ExifData::iterator md = exifData.begin(); md != exifData.end(); ++md)
        {
          QString key = QString::fromLatin1(md->key().c_str());

          // Decode the tag value with a user friendly output.
          QString tagValue;

          if (key == "Exif.Photo.UserComment")
            {
              tagValue = this->convertCommentValue(*md);
            }
          else if (key == "Exif.Image.0x935c")
            {
              tagValue = QString("Data of size %1").arg(md->value().size());
            }
          else
            {
              std::ostringstream os;
              os << *md;

              // Exif tag contents can be an i18n string, not only simple ascii.
              tagValue = QString::fromLocal8Bit(os.str().c_str());
            }

          tagValue.replace('\n', ' ');

          // We apply a filter to get only the Exif tags that we need.
          if(!exifKeysFilter.isEmpty())
            {
              if(!invertSelection)
                {
                  if(exifKeysFilter.contains(key.section('.', 1, 1)))
                    {
                      metaDataMap.insert(key, tagValue);
                    }
                }
              else
                {
                  if(!exifKeysFilter.contains(key.section('.', 1, 1)))
                    {
                      metaDataMap.insert(key, tagValue);
                    }
                }
            }
          else // else no filter at all.
            {
              metaDataMap.insert(key, tagValue);
            }
        }
      return metaDataMap;
    }
  catch (Exiv2::Error& e)
    {
      qDebug() <<"KbvExiv::getExifTagsDataList exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagsDataList default exception";
    }
  return KbvExiv::MetaDataMap();
}
/*************************************************************************//*!
 * Get exif tag data as byte array from exifTagName = familyName.groupName.tagName
*/
QByteArray KbvExiv::getExifTagData(const QString exifTagName) const
{
  try
    {
      Exiv2::ExifKey            exifKey(exifTagName.toStdString());
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          char* const s = new char[(*it).size()];
          (*it).copy((Exiv2::byte*)s, Exiv2::bigEndian);
          QByteArray data(s, (*it).size());
          delete[] s;
          return data;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << QString("KbvExivExif cannot find Exif key %1").arg(exifTagName) <<e.what(); //###########
    }
  return QByteArray();
}
/*************************************************************************//*!
 * Get exif tag data as string from exifTagName = familyName.groupName.tagName
*/
QString KbvExiv::getExifTagDataString(const QString exifTagName) const
{
  try
    {
      Exiv2::ExifKey            exifKey(exifTagName.toStdString());
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          std::string value = it->print(&exifData);
          return QString::fromLatin1(value.c_str());
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getExifTagDataString exception %1").arg(exifTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagDataString default exception";
    }
  return QString();
}

/*************************************************************************//*!
 * Get exif data as string from tag number and groupName.
 */
QString KbvExiv::getExifTagDataString(quint16 tag, const QString groupName) const
{
  try
    {
      Exiv2::ExifKey            exifKey(tag, groupName.toStdString());
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          std::string value = it->print(&exifData);
          return QString::fromLatin1(value.c_str());
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getExifTagDataString exception %1").arg(tag) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagDataString default exception";
    }
  return QString();
}
/*************************************************************************//*!
 * Get long (qint32) 'tagValue' of 'exifTagName'.\n
 * Returns the long tag data value of component '0'.
 */
bool  KbvExiv::getExifTagDataLong(const QString exifTagName, qint32 &tagValue) const
{
  return getExifTagDataLong(exifTagName, tagValue, 0);
}
/*************************************************************************//*!
 * Get long (qint32) 'tagValue' as nth 'component' of 'exifTagName'.
 */
bool  KbvExiv::getExifTagDataLong(const QString exifTagName, qint32 &tagValue, int component) const
{
  try
    {
      Exiv2::ExifKey            exifKey(exifTagName.toStdString());
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if((it != exifData.end()) && (it->count() > 0))
        {
          tagValue = it->toLong(component);
          return true;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifTagDataLong exception %1"<<exifTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagDataLong default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Get rational value of 'exifTagName' as nth 'component'.\n
 * 'num' and 'den' are the numerator and the denominator of the rational value.\n
 * Return true if Exif tag could be found.
 */
bool  KbvExiv::getExifTagDataRational(const QString exifTagName, qint32& num, qint32& den, int component=0) const
{
  try
    {
      Exiv2::ExifKey            exifKey(exifTagName.toStdString());
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if (it != exifData.end())
        {
          num = (*it).toRational(component).first;
          den = (*it).toRational(component).second;
          return true;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getExifTagDataRational exception") <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifTagDataRational default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Return comment (Photo.UserComment) as QString\n
 * UserComment: A tag for users to write keywords or text on the image
 * beside those in ImageDescription, and without the character code limitations
 * of the ImageDescription tag. The character code used in the UserComment tag
 * is identified based on an ID code in a fixed 8-byte area at the start of the
 * data area. The unused portion of the area shall be padded with NULL (0x00).\n
 * ASCII:   0x41, 0x53, 0x43, 0x49, 0x49, 0x00, 0x00, 0x00\n
 * JIS:     0x4A, 0x49, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00\n
 * Unicode: 0x55, 0x4E, 0x49, 0x43, 0x4F, 0x44, 0x45, 0x00\n
 */
QString KbvExiv::getExifComment() const
{
  QString exifComment;

  if(!this->hasExif())
    {
      return QString();
    }
  try
    {
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifKey            key("Exif.Photo.UserComment");
      Exiv2::ExifData::iterator iter = exifData.findKey(key);

      if(iter != exifData.end())
        {
           exifComment = this->convertCommentValue(*iter);

           // some cameras fill the UserComment with whitespace
           if (!exifComment.isEmpty() && !exifComment.trimmed().isEmpty())
             {
               return exifComment;
             }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifComment exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifComment default exception";
    }
  return QString();
}
/*************************************************************************//*!
 * Return description (Image.ImageDescription) as QString\n
 * ImageDescription: A character string giving the title of the image.\n
 * It is possible to add a comment such as "1988 company picnic" or the like.
 * Two-byte character codes cannot be used (ASCII only). When a 2-byte code is
 * necessary, the Exif private tag UserComment is to be used.
 */
QString KbvExiv::getExifDescription() const
{
  QString exifDescription;

  if(!this->hasExif())
    {
      return QString();
    }
  try
    {
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifKey            key("Exif.Image.ImageDescription");
      Exiv2::ExifData::iterator iter = exifData.findKey(key);
      if(iter != exifData.end())
        {
          exifDescription = this->convertCommentValue(*iter);

          // Some cameras fill in nonsense default values
           QStringList blackList;
           blackList << "SONY DSC"; // + whitespace
           blackList << "OLYMPUS DIGITAL CAMERA";
           blackList << "MINOLTA DIGITAL CAMERA";

           QString trimmed = exifDescription.trimmed();

           // some cameras fill the UserComment with whitespace
           if (!exifDescription.isEmpty() && !trimmed.isEmpty() && !blackList.contains(trimmed))
             {
               return exifDescription;
             }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifDescription exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifDescription default exception";
    }
  return QString();
}

/*************************************************************************//*!
 * Replace or add (if not present) tag 'exifTagName' data given as byte array.
*/
bool KbvExiv::setExifTagData(const QString exifTagName, const QByteArray& data)
{
  if(!data.isEmpty())
    {
      try
        {
          Exiv2::DataValue  val((Exiv2::byte*)data.data(), data.size());
          this->exifMetadata[exifTagName.toStdString()] = val;
          return true;
        }
      catch( Exiv2::Error& e )
        {
          qDebug() <<"KbvExiv::setExifTagData exception"<<e.what(); //###########
        }
      catch(...)
        {
          qDebug() <<"KbvExiv::setExifTagData default exception";
        }
    }

  return false;
}

/*************************************************************************//*!
 * Replace or add (if not present) tag 'exifTagName' data given as QString.
 */
bool KbvExiv::setExifTagDataString(const QString exifTagName, const QString& value)
{
  try
    {
      this->exifMetadata[exifTagName.toStdString()] = std::string(value.toLatin1().constData());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setExifTagDataString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifTagDataString default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Replace or add (if not present) tag 'exifTagName' data given as long value.
 */
bool KbvExiv::setExifTagDataLong(const QString exifTagName, qint32 value)
{
  try
    {
      this->exifMetadata[exifTagName.toStdString()] = static_cast<qint32>(value);
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setExifTagDataLong exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifTagDataLong default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Replace or add (if not present) rational value of tag 'exifTagName'.\n
 * 'num' and 'den' are the numerator and the denominator of the rational value.\n
 * Return true if Exif tag could be found.
 */
bool  KbvExiv::setExifTagDataRational(const QString exifTagName, qint32& num, qint32& den)
{
  try
    {
      this->exifMetadata[exifTagName.toStdString()] = Exiv2::Rational(num, den);
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setExifTagDataRational exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifTagDataRational default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Replace or add (if not present) UserComment or ImageDescription.\n
 *UserComment: A tag for Exif users to write keywords or comments on the image
 * besides those in ImageDescription, and without the character code limitations
 * of the ImageDescription tag. The character code used in the UserComment tag
 * is identified based on an ID code in a fixed 8-byte area at the start of the
 * data area. The unused portion of the area shall be padded with NULL (0x00).\n
 * ASCII:   0x41, 0x53, 0x43, 0x49, 0x49, 0x00, 0x00, 0x00\n
 * JIS:     0x4A, 0x49, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00\n
 * Unicode: 0x55, 0x4E, 0x49, 0x43, 0x4F, 0x44, 0x45, 0x00\n
 *ImageDescription: An ASCII character string giving the title of the image.
 */
bool KbvExiv::setExifCommentDescription(const QString& comment, const QString& description)
{
  try
    {
      removeExifTag("Exif.Image.ImageDescription"); //ASCII
      if(!description.isEmpty())
        {
          setExifTagDataString("Exif.Image.ImageDescription", description); //ASCII
        }

      removeExifTag("Exif.Photo.UserComment");
      if(!comment.isEmpty())
        {
          // Write as Unicode only when necessary.
          QTextCodec* latin1Codec = QTextCodec::codecForName("iso8859-1");
          if(latin1Codec->canEncode(comment))
            {
              // We know it's in the ISO-8859-1 8bit range.
              // Check if it's in the ASCII 7bit range
              if(KbvExiv::is7BitAscii(comment.toLatin1()))
                {
                  // write as ASCII
                  std::string exifComment("charset=\"Ascii\" ");
                  exifComment += comment.toLatin1().constData();
                  exifMetadata["Exif.Photo.UserComment"] = exifComment;
                  return true;
                }
            }
          // write as Unicode (UCS-2)
          std::string exifComment("charset=\"Unicode\" ");
          exifComment += comment.toUtf8().constData();
          exifMetadata["Exif.Photo.UserComment"] = exifComment;
        }
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setExifCommentDescription exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifCommentDescription default exception";
    }
  return false;
}
/*************************************************************************//*!
 * Replace or add (if not present) date and time the image was changed
 * (Exif.Image.DateTime) or manipulate date and time when the original image
 * data was generated (Exif.Image.DateTimeOriginal) or when the image was
 * stored as digital data (Exif.Photo.DateTimeDigitized).
 */
bool KbvExiv::setExifDateTime(const QString exifTagName, const QDateTime& dateTime)
{
  try
    {
      std::string exifdatetime(dateTime.toString(QString("yyyy:MM:dd hh:mm:ss")).toLatin1().constData());
      exifMetadata[exifTagName.toStdString()] = exifdatetime;
      return true;
    }
  catch( Exiv2::Error &e )
    {
      qDebug() <<"KbvExiv::setExifDateTime exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifDateTime default exception";
    }
  return false;

}
//Static functions*************************************************************
/*************************************************************************//*!
 * This method converts 'number' to a rational value, expressed in 'numerator'
 * and 'denominator' parameters. Set the precision using 'rounding' parameter.
 * The method is exact with rounding = 4 and more exact with rounding > 4.
 * Use this method if you want to retrieve a most exact rational for a number
 * without further properties, without any requirements to the denominator.
*/
void KbvExiv::convertToRational(const double number, long int* const numerator,
                                long int* const denominator, const int rounding)
{
  //The function converts the given decimal number to a rational (fractional) number.
  //Examples in comments use number = 25.12345, rounding = 4.
  // Split up the number.
  double whole      = trunc(number);
  double fractional = number - whole;

  //Calculate the "number" used for rounding.
  // This is 10^Digits - ie, 4 places gives us 10000.
  double rounder = pow(10.0, rounding);

  // Round the fractional part, and leave the number
  // as greater than 1.
  // To do this we: (for example)
  //  0.12345 * 10000 = 1234.5
  //  floor(1234.5) = 1234 - now bigger than 1 - ready...
  fractional = round(fractional * rounder);

  // Convert the whole thing to a fraction.
  // Fraction is:
  //     (25 * 10000) + 1234   251234
  //     ------------------- = ------ = 25.1234
  //           10000            10000
  double numTemp = (whole * rounder) + fractional;
  double denTemp = rounder;

  // Now we should reduce until we can reduce no more.

  // Try simple reduction...
  // if   Num
  //     ----- = integer out then....
  //      Den
  if (trunc(numTemp / denTemp) == (numTemp / denTemp))
    {
      // Divide both by Denominator.
      numTemp /= denTemp;
      denTemp /= denTemp;
    }

  // And, if that fails, brute force it.
  while(1)
    {
      // Jump out if we can't integer divide one.
      if ((numTemp / 2) != trunc(numTemp / 2)) break;
      if ((denTemp / 2) != trunc(denTemp / 2)) break;
      // Otherwise, divide away.
      numTemp /= 2;
      denTemp /= 2;
    }
  // Copy out the numbers.
  *numerator   = (int)numTemp;
  *denominator = (int)denTemp;
}

//*****************************************************************************
//Private methods *************************************************************

/*************************************************************************//*!
 * Return comment (Photo.UserComment and/or Image.ImageDescription) in
 * exifDatum as QString and perform the required conversion from other
 * character set.
 */
QString KbvExiv::convertCommentValue(const Exiv2::Exifdatum& exifDatum) const
{
  try
    {
      std::string comment;
      std::string charset;

      comment = exifDatum.toString();

      // libexiv2 will prepend "charset=\"SomeCharset\" " if charset is specified
      // Before conversion to QString, we must know the charset, so we stay with std::string for a while
      if (comment.length() > 8 && comment.substr(0, 8) == "charset=")
        {
          // the prepended charset specification is followed by a blank
          std::string::size_type pos = comment.find_first_of(' ');

          if (pos != std::string::npos)
            {
              // extract string between the = and the blank
              charset = comment.substr(8, pos-8);
              // get the rest of the string after the charset specification
              comment = comment.substr(pos+1);
            }
        }

      if (charset == "\"Unicode\"")
        {
          return QString::fromUtf8(comment.data());
        }
      else if (charset == "\"Jis\"")
        {
          QTextCodec* codec = QTextCodec::codecForName("JIS7");
          return codec->toUnicode(comment.c_str());
        }
      else if (charset == "\"Ascii\"")
        {
          return QString::fromLatin1(comment.c_str());
        }
      else
        {
          return detectEncodingAndDecode(comment);
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::convertCommentValue exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::convertCommentValue default exception";
    }
  return QString();
}

/*************************************************************************//*!
 * Return a map of standard exif tags supported by libexiv2 as map of
 * string (key) - stringlist (name title description)
 */
KbvExiv::TagsMap   KbvExiv::getExifStdTagsList() const
{
  try
    {
      QList<const Exiv2::TagInfo*> tags;
      KbvExiv::TagsMap          tagsMap;

      const Exiv2::GroupInfo* gi = Exiv2::ExifTags::groupList();

      while (gi->tagList_ != 0)
        {
          if (QString(gi->ifdName_) != QString("Makernote"))
            {
              Exiv2::TagListFct tl     = gi->tagList_;
              const Exiv2::TagInfo* ti = tl();

              while (ti->tag_ != 0xFFFF)
                {
                  tags << ti;
                  ++ti;
                }
            }
          ++gi;
        }

      for (QList<const Exiv2::TagInfo*>::iterator it = tags.begin(); it != tags.end(); ++it)
        {
          do
            {
              const Exiv2::TagInfo* const ti = *it;
              QString key                    = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
              QStringList values;
              values << ti->name_ << ti->title_ << ti->desc_;
              tagsMap.insert(key, values);
              ++(*it);
            }
          while((*it)->tag_ != 0xffff);
        }
      return tagsMap;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifStdTagsList exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifStdTagsList default exception";
    }
  return KbvExiv::TagsMap();
}
/*************************************************************************//*!
 * Return a map of all non-standard Exif tags (makernotes) supported by Exiv2.
 */
KbvExiv::TagsMap KbvExiv::getMakernoteTagsList() const
{
  try
    {
      QList<const Exiv2::TagInfo*> tags;
      KbvExiv::TagsMap          tagsMap;
      const Exiv2::GroupInfo*      gi = Exiv2::ExifTags::groupList();
      while(gi->tagList_ != 0)
        {
          if(QString(gi->ifdName_) == QString("Makernote"))
            {
              Exiv2::TagListFct     tl = gi->tagList_;
              const Exiv2::TagInfo* ti = tl();
              while (ti->tag_ != 0xFFFF)
                {
                  tags << ti;
                  ++ti;
                }
            }
          ++gi;
        }
      for(QList<const Exiv2::TagInfo*>::iterator it = tags.begin(); it != tags.end(); ++it)
        {
          do
            {
              const Exiv2::TagInfo* const ti = *it;
              QString               key = QLatin1String(Exiv2::ExifKey(*ti).key().c_str());
              QStringList values;
              values << ti->name_ << ti->title_ << ti->desc_;
              tagsMap.insert(key, values);
              ++(*it);
            }
          while((*it)->tag_ != 0xffff);
        }
      return tagsMap;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() <<"KbvExiv::getMakernoteTagsList exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getMakernoteTagsList default exception"; //###########
    }
  return KbvExiv::TagsMap();
}
/*************************************************************************//*!
 * Return a QImage copy of Exif thumbnail image or an empty image if thumbnail
 * cannot be found. The 'orientation' parameter will rotate automatically the
 * thumbnail if Exif orientation tags information are attached to thumbnail.
 */
QImage KbvExiv::getExifThumbnail(bool rotate)
{
  QImage thumbnail;

  if (this->exifMetadata.empty())
    {
      return thumbnail;
    }
  try
    {
      Exiv2::ExifThumbC   thumb(this->exifMetadata);
      Exiv2::DataBuf      const c1 = thumb.copy();
      thumbnail.loadFromData(c1.pData_, c1.size_);
      if(!thumbnail.isNull())
        {
          if(rotate)
            {
              Exiv2::ExifKey  key1("Exif.Thumbnail.Orientation");
              Exiv2::ExifKey  key2("Exif.Image.Orientation");
              Exiv2::ExifData exifData(this->exifMetadata);
              Exiv2::ExifData::iterator it = exifData.findKey(key1);
              if(it == exifData.end())
                {
                  it = exifData.findKey(key2);
                }
              if(it != exifData.end() && it->count())
                {
                  long orientation = it->toLong();
                  qDebug() <<"Exif Thumbnail Orientation: " << (int)orientation;
                  rotateExifQImage(thumbnail, (KbvExiv::ImageOrientation)orientation);
                }
              return thumbnail;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifThumbnail exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getExifThumbnail default exception"; //###########
    }
  return thumbnail;
}
/*************************************************************************//*!
 * Set the Exif Thumbnail image. The thumbnail image must have the right
 * dimensions before setting. See Exif specification for details.
 * Return true if thumbnail have been changed in metadata.
 */
bool  KbvExiv::setExifThumbnail(const QImage& thumbnail)
{
  if(thumbnail.isNull())
    {
      return removeExifThumbnail();
    }
  try
    {
      QByteArray data;
      QBuffer buffer(&data);
      buffer.open(QIODevice::WriteOnly);
      thumbnail.save(&buffer, "JPEG");
      Exiv2::ExifThumb thumb(this->exifMetadata);
      thumb.setJpegThumbnail((Exiv2::byte *)data.data(), data.size());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getExifThumbnail exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setExifThumbnail default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Remove the Exif Thumbnail from the metadata container in memory.
 * Return true in case of success.
 */
bool  KbvExiv::removeExifThumbnail()
{
  try
    {
      //Remove all IFD0 subimages.
      Exiv2::ExifThumb thumb(this->exifMetadata);
      thumb.erase();
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::removeExifThumbnail exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::removeExifThumbnail default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Rotate 'image' due to Exif orientation tag.
 * Return true if image has been rotated.
 */
bool  KbvExiv::rotateExifQImage(QImage& image, KbvExiv::ImageOrientation orientation)
{
  Q_UNUSED(image)
  Q_UNUSED(orientation)

  //TODO: rotate image
  /*
  QMatrix matrix = RotationMatrix::toMatrix(orientation);

  if((orientation != KbvExiv::ORIENTATION_NORMAL) && (orientation != KbvExiv::ORIENTATION_UNSPECIFIED))
    {
      image = image.transformed(matrix);
      return true;
    }
  */
  return false;
}

/****************************************************************************/
