/*****************************************************************************
 * KbvExivGps
 * Dyn. library interface to libExiv2, GPS functions
 * Derived from libkexiv2
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-05-22 20:33:21 +0200 (Mo, 22. Mai 2017) $
 * $Rev: 1042 $
 * Created: 2015.11.07
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include <cmath>
#include "kbvExiv.h"

/*************************************************************************//*!
 * Get all GPS location information set in image.
 * Return true if all information can be found.
 */
bool KbvExiv::getGPSCoordinates(double& altitude, double& latitude, double& longitude) const
{
  // When latitude is zero we clear longitude too and vice versa
  // A valid GPS position can have zero altitude value.
  getGPSAltitude(&altitude);
  if(!getGPSLatitudeNumber(&latitude))
    {
      longitude = 0.0;
      return false;
    }
  if(!getGPSLongitudeNumber(&longitude))
    {
      latitude = 0.0;
      return false;
    }
  return true;
}
/*************************************************************************//*!
 * Get GPS latitude in degrees as a double floating point number. The sign
 * determines the position relative to the equator (North + / South -).
 * Returns true if the information is available.
*/
bool KbvExiv::getGPSLatitudeNumber(double* const latitude) const
{
  try
    {
      *latitude=0.0;
#ifdef _XMP_SUPPORT_
      // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.
      if(convertFromGPSCoordinateString(getXmpTagDataString("Xmp.exif.GPSLatitude"), latitude))
        {
          return true;
        }
#endif //_XMP_SUPPORT_

      //get the reference from Exif.
      const QByteArray latRef = getExifTagData("Exif.GPSInfo.GPSLatitudeRef");
      if(!latRef.isEmpty())
        {
          Exiv2::ExifData exifData(this->exifMetadata);
          Exiv2::ExifKey  exifKey("Exif.GPSInfo.GPSLatitude");
          Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

          if(it != exifData.end() && (*it).count() == 3)
            {
              // Latitude decoding from Exif.
              double num, den, min, sec;
              num = (double)((*it).toRational(0).first);
              den = (double)((*it).toRational(0).second);
              if(den == 0)
                {
                  return false;
                }

              *latitude = num/den;
              num = (double)((*it).toRational(1).first);
              den = (double)((*it).toRational(1).second);
              if(den == 0)
                {
                  return false;
                }

              min = num/den;
              if(min != -1.0)
                {
                  *latitude = *latitude + min/60.0;
                }

              num = (double)((*it).toRational(2).first);
              den = (double)((*it).toRational(2).second);
              if (den == 0)
                {
                  // be relaxed and accept 0/0 seconds.
                  if(num == 0)
                    {
                      den = 1;
                    }
                  else
                    {
                      return false;
                    }
                }

              sec = num/den;
              if(sec != -1.0)
                {
                  *latitude = *latitude + sec/3600.0;
                }
            }
          else
            {
              return false;
            }

          if(latRef[0] == 'S')
            {
              *latitude *= -1.0;
            }
          return true;
        }
    }
  catch(Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getGPSLatitudeNumber exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSLatitudeNumber default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get GPS longitude in degrees as a double floating point number. The sign
 * determines the position relative to Greenwich (West - / East +).
 * Returns true if the information is available.
*/
bool KbvExiv::getGPSLongitudeNumber(double* const longitude) const
{
  try
    {
      *longitude=0.0;
#ifdef _XMP_SUPPORT_
      // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.
      if(convertFromGPSCoordinateString(getXmpTagDataString("Xmp.exif.GPSLongitude"), longitude))
        {
          return true;
        }
#endif //_XMP_SUPPORT_

      //get the reference from Exif.
      const QByteArray lngRef = getExifTagData("Exif.GPSInfo.GPSLongitudeRef");
      if(!lngRef.isEmpty())
        {
          // Longitude decoding from Exif.
          Exiv2::ExifData exifData(this->exifMetadata);
          Exiv2::ExifKey  exifKey2("Exif.GPSInfo.GPSLongitude");
          Exiv2::ExifData::iterator it = exifData.findKey(exifKey2);

          if(it != exifData.end() && (*it).count() == 3)
            {
              double num, den;
              num = (double)((*it).toRational(0).first);
              den = (double)((*it).toRational(0).second);
              if(den == 0)
                {
                  return false;
                }

              *longitude = num/den;
              num = (double)((*it).toRational(1).first);
              den = (double)((*it).toRational(1).second);
              if(den == 0)
                {
                  return false;
                }

              const double min = num/den;
              if(min != -1.0)
                {
                  *longitude = *longitude + min/60.0;
                }

              num = (double)((*it).toRational(2).first);
              den = (double)((*it).toRational(2).second);
              if(den == 0)
                {
                  // be relaxed and accept 0/0 seconds
                  if(num == 0)
                    {
                      den = 1;
                    }
                  else
                    {
                      return false;
                    }
                }

              const double sec = num/den;
              if (sec != -1.0)
                {
                  *longitude = *longitude + sec/3600.0;
                }
            }
          else
            {
              return false;
            }

          if(lngRef[0] == 'W')
            {
              *longitude *= -1.0;
            }
          return true;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getGPSLongitudeNumber exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSLongitudeNumber default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get GPS altitude in meters, relative to sea level (above +, below -)
 */
bool KbvExiv::getGPSAltitude(double* const altitude) const
{
  try
    {
      double num, den;
      *altitude=0.0;
#ifdef _XMP_SUPPORT_
      // Try XMP first. Reason: XMP in sidecar may be more up-to-date than EXIF in original image.
      const QString altRefXmp = getXmpTagDataString("Xmp.exif.GPSAltitudeRef");
      if(!altRefXmp.isEmpty())
        {
          const QString altXmp = getXmpTagDataString("Xmp.exif.GPSAltitude");
          if(!altXmp.isEmpty())
            {
              num = altXmp.section('/', 0, 0).toDouble();
              den = altXmp.section('/', 1, 1).toDouble();
              if(den == 0)
                {
                  return false;
                }
              *altitude = num/den;
              if(altRefXmp == QString("1"))
                {
                  *altitude *= -1.0;
                }
              return true;
            }
        }
#endif //_XMP_SUPPORT_

      //Get the reference from Exif (above/below sea level)
      const QByteArray altRef = getExifTagData("Exif.GPSInfo.GPSAltitudeRef");
      if(!altRef.isEmpty())
        {
          // Altitude decoding from Exif.
          Exiv2::ExifData exifData(this->exifMetadata);
          Exiv2::ExifKey  exifKey3("Exif.GPSInfo.GPSAltitude");
          Exiv2::ExifData::iterator it = exifData.findKey(exifKey3);
          if(it != exifData.end() && (*it).count())
            {
              num = (double)((*it).toRational(0).first);
              den = (double)((*it).toRational(0).second);
              if(den == 0)
                {
                  return false;
                }
              *altitude = num/den;
            }
          else
            {
              return false;
            }

          if(altRef[0] == '1')
            {
              *altitude *= -1.0;
            }
          return true;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getGPSAltitude exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSAltitude default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get GPS location information, in the GPSCoordinate format as described in
 * the XMP specification: A Text value in the form “DDD,MM,SSk” or
 * “DDD,MM.mmk”, where:
 * DDD is a number of degrees, MM is a number of minutes, SS is a number of
 * seconds, mm is a fraction of minutes, k is a single character N, S, E, or
 * W indicating a direction. The DDD,MM.mmk form should be used when any of
 * the native Exif component rational values has a denominator other than 1.
 * There can be any number of fractional digits.
 * Returns a null string if the information cannot be found.
 */
QString KbvExiv::getGPSLatitudeString() const
{
  double latitude;

  if(getGPSLatitudeNumber(&latitude))
    {
      return convertToGPSCoordinateString(true, latitude);
    }
  return QString();
}
QString KbvExiv::getGPSLongitudeString() const
{
  double longitude;

  if(getGPSLongitudeNumber(&longitude))
    {
      return convertToGPSCoordinateString(false, longitude);
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS latitude in degrees minutes seconds as String with position
 * relative to the equator (North / South).
 * Returns an empty string if the information is available.
*/
QString KbvExiv::getGPSLatitudeDegMinSecString() const
{
  QString latitude;
  int     num, den;
  try
    {
      //get the reference from Exif.
      const QByteArray latRef = getExifTagData("Exif.GPSInfo.GPSLatitudeRef");
      if(!latRef.isEmpty())
        {
          Exiv2::ExifData           exifData(this->exifMetadata);
          Exiv2::ExifKey            exifKey("Exif.GPSInfo.GPSLatitude");
          Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

          if(it != exifData.end() && (*it).count() == 3)
            {
              //degrees
              num = (*it).toRational(0).first;
              den = (*it).toRational(0).second;
              if(den == 0)
                {
                  return QString();
                }
              latitude = QString::number(num/den);
              latitude.append(".");
              //minutes
              num = (*it).toRational(1).first;
              den = (*it).toRational(1).second;
              if(den == 0)
                {
                  return QString();
                }
              latitude.append(QString::number(num/den));
              latitude.append(".");
              //seconds
              num = (*it).toRational(2).first;
              den = (*it).toRational(2).second;
              if(den == 0)
                {
                  //accept 0/0 seconds.
                  den = 1;
                }
              latitude.append(QString::number(num/den));
            }
          else
            {
              return QString();
            }
          latitude.append(QString(latRef[0]));
          return latitude;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getGPSLatitudeDegMinSecString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSLatitudeDegMinSecString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS longitude in degrees minutes seconds as String with position
 * relative to Greenwich (West / East).
 * Returns an empty string if the information is available.
*/
QString KbvExiv::getGPSLongitudeDegMinSecString() const
{
  QString longitude;
  int     num, den;
  try
    {
      //get the reference from Exif.
      const QByteArray longRef = getExifTagData("Exif.GPSInfo.GPSLongitudeRef");
      if(!longRef.isEmpty())
        {
          Exiv2::ExifData           exifData(this->exifMetadata);
          Exiv2::ExifKey            exifKey("Exif.GPSInfo.GPSLongitude");
          Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

          if(it != exifData.end() && (*it).count() == 3)
            {
              //degrees
              num = (*it).toRational(0).first;
              den = (*it).toRational(0).second;
              if(den == 0)
                {
                  return QString();
                }
              longitude = QString::number(num/den);
              longitude.append(".");
              //minutes
              num = (*it).toRational(1).first;
              den = (*it).toRational(1).second;
              if(den == 0)
                {
                  return QString();
                }
              longitude.append(QString::number(num/den));
              longitude.append(".");
              //seconds
              num = (*it).toRational(2).first;
              den = (*it).toRational(2).second;
              if(den == 0)
                {
                  //accept 0/0 seconds.
                  den = 1;
                }
              longitude.append(QString::number(num/den));
            }
          else
            {
              return QString();
            }
          longitude.append(QString(longRef[0]));
          return longitude;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::getGPSLongitudeDegMinSecString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSLongitudeDegMinSecString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS date and time as String.
 * Returns an empty string if the information cannot be found.
 * GPS time stamp is expressed as three RATIONAL values (numerator, denominator)
 * for hour, minute, and second (atomic clock).
 * GPS date stamp is a character string recording time information relative to
 * UTC (Coordinated Universal Time). The format is "YYYY:MM:DD".
 */
QString KbvExiv::getGPSDateString() const
{
  //The format is YYYY:MM:DD coded in ASCII.
  try
    {
      Exiv2::ExifKey            exifKey("Exif.GPSInfo.GPSDateStamp");
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
      qDebug() << QString("KbvExiv::getGPSDateString exception")<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSDateString default exception"; //###########
    }
  return QString();
}
QString KbvExiv::getGPSTimeString() const
{
  //UTC time stamp expressed as three RATIONAL values (numerator, denominator)
  //giving the hour, minute, and second (atomic clock).
  qint32    num, den;
  QString   time;

  try
    {
      Exiv2::ExifKey            exifKey("Exif.GPSInfo.GPSTimeStamp");
      Exiv2::ExifData           exifData(this->exifMetadata);
      Exiv2::ExifData::iterator it = exifData.findKey(exifKey);

      if (it != exifData.end())
        {
          num = (*it).toRational(0).first;
          den = (*it).toRational(0).second;
          if(den <= 0)
            {
              return QString();
            }
          time.append(QString::number(num/den));
          time.append(":");
          num = (*it).toRational(1).first;
          den = (*it).toRational(1).second;
          if(den <= 0)
            {
              return QString();
            }
          time.append(QString::number(num/den));
          time.append(":");
          num = (*it).toRational(2).first;
          den = (*it).toRational(2).second;
          if(den <= 0)
            {
              return QString();
            }
          time.append(QString::number(num/den));
          time.append("  UTC");
          return time;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getGPSTimeString exception")<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSTimeString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS image direction as String.\n
 * The range of values is from 0.00 to 359.99.\n
 * Returns an empty string if the information cannot be found.
 */
QString KbvExiv::getGPSDirectionString() const
{
  qint32    num, den;
  QString   ref, dir;
  Exiv2::ExifData::iterator it;

  //Image direction reference
  //"T" denotes true direction and "M" is magnetic direction.
  try
    {
      Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSImgDirectionRef");
      Exiv2::ExifData  exifData(this->exifMetadata);
      it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          std::string value = it->print(&exifData);
          ref = QString::fromLatin1(value.c_str());

          Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSImgDirection");
          Exiv2::ExifData  exifData(this->exifMetadata);
          it = exifData.findKey(exifKey);
          if(it != exifData.end())
            {
              num = (*it).toRational(0).first;
              den = (*it).toRational(0).second;
              if(den > 0)
                {
                  dir = QString::number(double(num/den), 'f', 2);
                }
              if(ref == "M")  //magnetic
                {
                  dir.append(" magnetic");
                }
            }
        }
      return  dir;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getGPSDirectionString exception")<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSDirectionString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS camera movement direction as String.\n
 * The range of values is from 0.00 to 359.99.\n
 * Returns an empty string if the information cannot be found.
 */
QString KbvExiv::getGPSTrackString() const
{
  qint32    num, den;
  QString   ref, dir;
  Exiv2::ExifData::iterator it;

  //Image track direction reference
  //"T" denotes true direction and "M" is magnetic direction.
  try
    {
      Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSTrackRef");
      Exiv2::ExifData  exifData(this->exifMetadata);
      it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          std::string value = it->print(&exifData);
          ref = QString::fromLatin1(value.c_str());

          Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSTrack");
          Exiv2::ExifData  exifData(this->exifMetadata);
          it = exifData.findKey(exifKey);
          if(it != exifData.end())
            {
              num = (*it).toRational(0).first;
              den = (*it).toRational(0).second;
              if(den > 0)
                {
                  dir = QString::number(double(num/den), 'f', 2);
                }
              if(ref == "M")  //magnetic
                {
                  dir.append(" magnetic");
                }
            }
        }
      return  dir;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getGPSTrackString exception")<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSTrackString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Get GPS camera speed as String.\n
 * The range of values is knots, miles per hour, kilometer per hour.\n
 * Returns an empty string if the information cannot be found.
 */
QString KbvExiv::getGPSSpeedString() const
{
  qint32    num, den;
  QString   ref, dir;
  Exiv2::ExifData::iterator it;

  //Image track direction reference
  //"T" denotes true direction and "M" is magnetic direction.
  try
    {
      Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSSpeedRef");
      Exiv2::ExifData  exifData(this->exifMetadata);
      it = exifData.findKey(exifKey);

      if(it != exifData.end())
        {
          std::string value = it->print(&exifData);
          ref = QString::fromLatin1(value.c_str());

          Exiv2::ExifKey   exifKey("Exif.GPSInfo.GPSSpeed");
          Exiv2::ExifData  exifData(this->exifMetadata);
          it = exifData.findKey(exifKey);
          if(it != exifData.end())
            {
              num = (*it).toRational(0).first;
              den = (*it).toRational(0).second;
              if(den > 0)
                {
                  dir = QString::number(double(num/den), 'f', 2);
                }
              if(ref == "K")
                {
                  dir.append(" kmph");
                }
              if(ref == "M")
                {
                  dir.append(" mph");
                }
              if(ref == "N")
                {
                  dir.append(" knots");
                }
            }
        }
      return  dir;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getGPSSpeedString exception")<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getGPSSpeedString default exception"; //###########
    }
  return QString();
}

/*************************************************************************//*!
 * Set GPS location information altitude, latitude and longitude.
 * GPS tags get removed completely then and initialised newly by setting
 * GPSVersionID and GPSMapDatum before the location information is written.
 * Return true if the information can be set.
 */
bool KbvExiv::setGPSCoordinates(const double altitude, const double latitude, const double longitude)
{
    return setGPSTags(&altitude, latitude, longitude);
}

/*************************************************************************//*!
 * Set GPS location information altitude, latitude and longitude.
 * GPS tags get removed completely then and initialised newly by setting
 * GPSVersionID and GPSMapDatum before the location information is written.
 * Return true if the information can be set.
 */
bool KbvExiv::setGPSCoordinates(const double altitude, const QString& latitude, const QString& longitude)
{
    double longitudeValue, latitudeValue;
    
    if(!convertFromGPSCoordinateString(latitude, &latitudeValue))
      {
        return false;
      }
    if(!convertFromGPSCoordinateString(longitude, &longitudeValue))
      {
        return false;
      }
    return setGPSTags(&altitude, latitudeValue, longitudeValue);
}

/*************************************************************************//*!
 * Set GPS exif tags altitude, latitude and longitude.
 * GPS tags get removed completely then and initialised newly by setting
 * GPSVersionID and GPSMapDatum before the location information is written.
 * Return true if the information can be set.
 */
bool KbvExiv::setGPSTags(const double* const altitude, const double latitude, const double longitude)
{
  try
    {
      //clean up all existing GPS tags
      removeGPSTags();

      //initialize the GPS info:
      if(!initializeGPSTags())
        {
          return false;
        }
      char scratchBuf[100];
      long int nom, denom;
      long int deg, min;

      //ALTITUDE.
      if(altitude)
        {
          //Altitude reference: byte "00" means "above sea level", "01" means "below sea level".
          Exiv2::Value::AutoPtr value = Exiv2::Value::create(Exiv2::unsignedByte);

          if((*altitude) >= 0)
            {
              value->read("0");
            }
          else
            {
              value->read("1");
            }

          this->exifMetadata.add(Exiv2::ExifKey("Exif.GPSInfo.GPSAltitudeRef"), value.get());

          //Altitude, as absolute value.
          convertToRational(fabs(*altitude), &nom, &denom, 4);
          snprintf(scratchBuf, 100, "%ld/%ld", nom, denom);
          this->exifMetadata["Exif.GPSInfo.GPSAltitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_
         setXmpTagString("Xmp.exif.GPSAltitudeRef", ((*altitude) >= 0) ? QString("0") : QString("1"), false);
         setXmpTagString("Xmp.exif.GPSAltitude", QString(scratchBuf), false);
#endif // _XMP_SUPPORT_
        }

      //LATITUDE
      //Latitude reference: latitude < 0 : "S", latitude > 0 : "N"
      this->exifMetadata["Exif.GPSInfo.GPSLatitudeRef"] = (latitude < 0 ) ? "S" : "N";

      //Latitude as absolute value in three rationals.
      //Do it as:
      // dd/1 - degrees.
      // mmmm/100 - minutes
      // 0/1 - seconds
      //Exif standard allows minutes as mm/1 and seconds as ss/1, but its
      //(slightly) more accurate as mmmm/100 than to split it.
      //Further note: to translate from dd.dddddd to dd mm.mm we multiply
      //by 6000 - x60 to get minutes, x1000000 to get to mmmm/1000000.
      deg = (int)floor(fabs(latitude)); // Slice off after decimal.
      min = (int)floor((fabs(latitude) - floor(fabs(latitude))) * 60000000);
      snprintf(scratchBuf, 100, "%ld/1 %ld/1000000 0/1", deg, min);
      this->exifMetadata["Exif.GPSInfo.GPSLatitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_
      /** @todo The XMP spec does not mention Xmp.exif.GPSLatitudeRef,
       * because the reference is included in Xmp.exif.GPSLatitude.
       * Is there a historic reason for writing it anyway?
       */
      setXmpTagString("Xmp.exif.GPSLatitudeRef", (latitude < 0) ? QString("S") : QString("N"), false);
      setXmpTagString("Xmp.exif.GPSLatitude", convertToGPSCoordinateString(true, latitude), false);
#endif // _XMP_SUPPORT_

      //LONGITUDE
      //Longitude reference: longitude < 0 : "W", longitude > 0 : "E"
      this->exifMetadata["Exif.GPSInfo.GPSLongitudeRef"] = (longitude < 0 ) ? "W" : "E";

     //Longitude as absolute value in three rationals.
     //Do it as:
     // dd/1 - degrees.
     // mmmm/100 - minutes
     // 0/1 - seconds
     //Exif standard allows minutes as mm/1 and seconds as ss/1, but its
     //(slightly) more accurate as mmmm/100 than to split it.
     //Further note: to translate from dd.dddddd to dd mm.mm we multiply
     //by 6000 - x60 to get minutes, x1000000 to get to mmmm/1000000.
     deg = (int)floor(fabs(longitude)); // Slice off after decimal.
     min = (int)floor((fabs(longitude) - floor(fabs(longitude))) * 60000000);
     snprintf(scratchBuf, 100, "%ld/1 %ld/1000000 0/1", deg, min);
     this->exifMetadata["Exif.GPSInfo.GPSLongitude"] = scratchBuf;

#ifdef _XMP_SUPPORT_
     /** @todo The XMP spec does not mention Xmp.exif.GPSLongitudeRef,
      * because the reference is included in Xmp.exif.GPSLongitude.
      * Is there a historic reason for writing it anyway?
     */
        setXmpTagString("Xmp.exif.GPSLongitudeRef", (longitude < 0) ? QString("W") : QString("E"), false);
        setXmpTagString("Xmp.exif.GPSLongitude", convertToGPSCoordinateString(false, longitude), false);
#endif // _XMP_SUPPORT_

      return true;
    }
    catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setGPSTags exception" <<e.what(); //###########
    }

    return false;
}

/*************************************************************************//*!
 * Set/add GPS date given as "YYYY:MM:DD" and time given as "HH:MM:SS UTC" or
 * "HH:MM:S UTC". Returns false if the operation fails.
 * GPS time stamp is expressed as three RATIONAL values (numerator, denominator)
 * for hour, minute, and second.
 * GPS date stamp is a character string recording date information relative to
 * UTC (Coordinated Universal Time). The format is "YYYY:MM:DD".
 */
bool KbvExiv::setGPSDateTimeStamp(const QString& date, const QString& time)
{
  QString     s;
  QStringList sl;
  qint32      num, den=1;
  
  if(date.isEmpty() || time.isEmpty())
    {
      return false;
    }

  try
    {
      //add/replace gps date as is
      this->exifMetadata["Exif.GPSInfo.GPSDateStamp"] = std::string(date.toLatin1().constData());
    
      //convert time to three rationals, the denominators are set to den = 1
      s = time.simplified();
      s.remove(" UTC");
      sl = s.split(":");
      
      //create a variable of type Exiv2::URationalValue
      Exiv2::URationalValue rv;
      for(int i=0; i < sl.length(); i++)
        {
          num = sl.at(i).toUInt();
          //add the component as std::pair to the end of the variable 
          rv.value_.push_back(std::make_pair(num,den));
          
        }
      //add/replace the variable to the Exif key
      this->exifMetadata["Exif.GPSInfo.GPSTimeStamp"] = rv;
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::setGPSDateTimeStamp exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setGPSDateTimeStamp default exception";
    }
  return false;
}

/*************************************************************************//*!
 * Initialise non existent Exif GPS tags by setting the GPSVersionID.\n
 * Return true on success.
 */
bool KbvExiv::initializeGPSTags()
{
  try
    {
      // GPSVersionID tag: standard defines four bytes: 02 00 00 00, mandatory
      Exiv2::Value::AutoPtr value = Exiv2::Value::create(Exiv2::unsignedByte);
      value->read(gpsVersionID);
      this->exifMetadata.add(Exiv2::ExifKey("Exif.GPSInfo.GPSVersionID"), value.get());

      //Date: the date of the measured data. If not given, we insert WGS-84.
      this->exifMetadata["Exif.GPSInfo.GPSMapDatum"] = "WGS-84";

#ifdef _XMP_SUPPORT_
      setXmpTagString("Xmp.exif.GPSVersionID", QString("2.0.0.0"), false);
      setXmpTagString("Xmp.exif.GPSMapDatum", QString("WGS-84"), false);
#endif // _XMP_SUPPORT_

      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::initializeGPSTags exception" <<e.what(); //###########
    }
  return false;
}

/*************************************************************************//*!
 * Remove all Exif GPS tags in meta data container in memory.\n
 * Return true if all tags have been removed successfully.
 */
bool KbvExiv::removeGPSTags()
{
  try
    {
      QStringList gpsTagsKeys;
      for (Exiv2::ExifData::iterator it = this->exifMetadata.begin(); it != this->exifMetadata.end(); ++it)
        {
          QString key = QString::fromLocal8Bit(it->key().c_str());
          if(key.section('.', 1, 1) == QString("GPSInfo"))
            {
              gpsTagsKeys.append(key);
            }
        }

      for(QStringList::const_iterator it2 = gpsTagsKeys.constBegin(); it2 != gpsTagsKeys.constEnd(); ++it2)
        {
          Exiv2::ExifKey  gpsKey((*it2).toLatin1().constData());
          Exiv2::ExifData::iterator it3 = this->exifMetadata.findKey(gpsKey);
          if(it3 != this->exifMetadata.end())
            {
              this->exifMetadata.erase(it3);
            }
        }

#ifdef _XMP_SUPPORT_
    // The XMP spec does not mention Xmp.exif.GPSLongitudeRef,
    // and Xmp.exif.GPSLatitudeRef. But we could have been written then,
    // so remove them here.
      if(this->hasXmp())
        {
          removeXmpTag("Xmp.exif.GPSLatitudeRef");
          removeXmpTag("Xmp.exif.GPSLongitudeRef");
          removeXmpTag("Xmp.exif.GPSVersionID");
          removeXmpTag("Xmp.exif.GPSLatitude");
          removeXmpTag("Xmp.exif.GPSLongitude");
          removeXmpTag("Xmp.exif.GPSAltitudeRef");
          removeXmpTag("Xmp.exif.GPSAltitude");
          removeXmpTag("Xmp.exif.GPSTimeStamp");
          removeXmpTag("Xmp.exif.GPSSatellites");
          removeXmpTag("Xmp.exif.GPSStatus");
          removeXmpTag("Xmp.exif.GPSMeasureMode");
          removeXmpTag("Xmp.exif.GPSDOP");
          removeXmpTag("Xmp.exif.GPSSpeedRef");
          removeXmpTag("Xmp.exif.GPSSpeed");
          removeXmpTag("Xmp.exif.GPSTrackRef");
          removeXmpTag("Xmp.exif.GPSTrack");
          removeXmpTag("Xmp.exif.GPSImgDirectionRef");
          removeXmpTag("Xmp.exif.GPSImgDirection");
          removeXmpTag("Xmp.exif.GPSMapDatum");
          removeXmpTag("Xmp.exif.GPSDestLatitude");
          removeXmpTag("Xmp.exif.GPSDestLongitude");
          removeXmpTag("Xmp.exif.GPSDestBearingRef");
          removeXmpTag("Xmp.exif.GPSDestBearing");
          removeXmpTag("Xmp.exif.GPSDestDistanceRef");
          removeXmpTag("Xmp.exif.GPSDestDistance");
          removeXmpTag("Xmp.exif.GPSProcessingMethod");
          removeXmpTag("Xmp.exif.GPSAreaInformation");
          removeXmpTag("Xmp.exif.GPSDifferential");
        }
#endif // _XMP_SUPPORT_

      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() <<"KbvExiv::removeGPSTags exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::removeGPSTags default exception"; //###########
    }
  return false;
}

//*****************************************************************************
//Static functions *************************************************************

/*************************************************************************//*!
 * Convert a GPS position 'coordinate' as double floating point number of
 * degrees to the form described as GPSCoordinate in the XMP specification.
 * Select logitude or latidude by 'isLatitude'.
 */
QString KbvExiv::convertToGPSCoordinateString(const bool isLatitude, double coordinate)
{
  if(coordinate < -360.0 || coordinate > 360.0)
    {
      return QString();
    }

  QString coordinateString;
  char    directionReference;

  if(isLatitude)
    {
      if(coordinate < 0)
        {
          directionReference = 'S';
        }
      else
        {
          directionReference = 'N';
        }
    }
  else
    {
      if(coordinate < 0)
        {
          directionReference = 'W';
        }
      else
        {
          directionReference = 'E';
        }
    }

  // remove sign
  coordinate = fabs(coordinate);

  int degrees = (int)floor(coordinate);
  // get fractional part
  coordinate = coordinate - (double)(degrees);
  // To minutes
  double minutes = coordinate * 60.0;

  // use form DDD,MM.mmk
  coordinateString = "%1,%2%3";
  coordinateString = coordinateString.arg(degrees);
  coordinateString = coordinateString.arg(minutes, 0, 'f', 8).arg(directionReference);

  return coordinateString;
}
/*************************************************************************//*!
 * Convert a GPS position stored as rationals in Exif to the form described
 * as GPSCoordinate in the XMP specification ("256,45,34N" or "256,45.566667N").
 */
QString KbvExiv::convertToGPSCoordinateString(const long int numeratorDegrees, const long int denominatorDegrees,
                 const long int numeratorMinutes, const long int denominatorMinutes,
                 const long int numeratorSeconds, long int denominatorSeconds,
                 const char directionReference)
{
  //Precision:
  //A second at sea level measures 30m for our purposes, a minute 1800m.
  // (for more details, see http://en.wikipedia.org/wiki/Geographic_coordinate_system)
  // This means with a decimal precision of 8 for minutes we get +/-0,018mm.
  QString coordinate;

  // be relaxed with seconds of 0/0
  if(denominatorSeconds == 0 && numeratorSeconds == 0)
    {
      denominatorSeconds = 1;
    }

  if(denominatorDegrees == 1 && denominatorMinutes == 1 && denominatorSeconds == 1)
    {
      // use form DDD,MM,SSk
      coordinate = "%1,%2,%3%4";
      coordinate = coordinate.arg(numeratorDegrees).arg(numeratorMinutes).arg(numeratorSeconds).arg(directionReference);
    }
  else if(denominatorDegrees == 1 && denominatorMinutes == 100 && denominatorSeconds == 1)
    {
      // use form DDD,MM.mmk
      coordinate = "%1,%2%3";
      double minutes = (double)numeratorMinutes / (double)denominatorMinutes;
      minutes += (double)numeratorSeconds / 60.0;
      QString minutesString = QString::number(minutes, 'f', 8);

      while(minutesString.endsWith('0') && !minutesString.endsWith(".0"))
        {
          minutesString.chop(1);
        }
      coordinate = coordinate.arg(numeratorDegrees).arg(minutesString).arg(directionReference);
    }
  else if(denominatorDegrees == 0 || denominatorMinutes == 0 || denominatorSeconds == 0)
    {
      // Invalid. 1/0 is everything but 0. As is 0/0.
      return QString();
    }
  else
    {
      // use form DDD,MM.mmk
      coordinate = "%1,%2%3";
      double degrees = (double)numeratorDegrees / (double)denominatorDegrees;
      double wholeDegrees = trunc(degrees);
      double minutes = (double)numeratorMinutes / (double)denominatorMinutes;
      minutes += (degrees - wholeDegrees) * 60.0;
      minutes += ((double)numeratorSeconds / (double)denominatorSeconds) / 60.0;
      QString minutesString  = QString::number(minutes, 'f', 8);

      while(minutesString.endsWith('0') && !minutesString.endsWith(".0"))
        {
          minutesString.chop(1);
        }
      coordinate = coordinate.arg((int)wholeDegrees).arg(minutesString).arg(directionReference);
    }
  return coordinate;
}
/*************************************************************************//*!
 * Convert a GPSCoordinate string as defined by XMP to a double floating point
 * number in degrees where the sign determines the direction ref (North +,
 * South -, East +, West -).
 * Returns true if the conversion was successful.
*/
bool KbvExiv::convertFromGPSCoordinateString(const QString& gpsString, double* const degrees)
{
  if(gpsString.isEmpty())
    {
      return false;
    }

  char directionReference = gpsString.at(gpsString.length() - 1).toUpper().toLatin1();
  QString coordinate      = gpsString.left(gpsString.length() - 1);
  QStringList parts       = coordinate.split(',');

  if(parts.size() == 2)
    {
      // form DDD,MM.mmk
      *degrees =  parts[0].toLong();
      *degrees += parts[1].toDouble() / 60.0;

      if(directionReference == 'W' || directionReference == 'S')
        {
          *degrees *= -1.0;
        }
      return true;
    }
  else if(parts.size() == 3)
    {
      // use form DDD,MM,SSk
      *degrees =  parts[0].toLong();
      *degrees += parts[1].toLong() / 60.0;
      *degrees += parts[2].toLong() / 3600.0;
      if (directionReference == 'W' || directionReference == 'S')
        {
          *degrees *= -1.0;
        }
      return true;
    }
  else
    {
      return false;
    }
}
/*************************************************************************//*!
 * Convert a GPSCoordinate string as defined by XMP to three rationals and
 * the direction reference. Returns true if the conversion was successful.
 * If minutes are given in the fractional form, a denominator of 1000000 will
 * be used.
*/
bool KbvExiv::convertFromGPSCoordinateString(const QString& gpsString,
              long int* const numeratorDegrees, long int* const denominatorDegrees,
              long int* const numeratorMinutes, long int* const denominatorMinutes,
              long int* const numeratorSeconds, long int* const denominatorSeconds,
              char* const directionReference)
{
  if(gpsString.isEmpty())
    {
      return false;
    }

  *directionReference = gpsString.at(gpsString.length() - 1).toUpper().toLatin1();
  QString coordinate  = gpsString.left(gpsString.length() - 1);
  QStringList parts   = coordinate.split(',');

  if (parts.size() == 2)
    {
      // form DDD,MM.mmk
      *denominatorDegrees = 1;
      *denominatorMinutes = 1000000;
      *denominatorSeconds = 1;
      *numeratorDegrees   = parts[0].toLong();
      double minutes      = parts[1].toDouble();
      minutes            *= 1000000;
      *numeratorMinutes   = (long)round(minutes);
      *numeratorSeconds   = 0;
      return true;
    }
  else if(parts.size() == 3)
    {
      // use form DDD,MM,SSk
      *denominatorDegrees = 1;
      *denominatorMinutes = 1;
      *denominatorSeconds = 1;
      *numeratorDegrees   = parts[0].toLong();
      *numeratorMinutes   = parts[1].toLong();
      *numeratorSeconds   = parts[2].toLong();
      return true;
    }
  else
    {
      return false;
    }
}

/****************************************************************************/
