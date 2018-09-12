/*****************************************************************************
 * KbvExivIptc
 * Dyn. library interface to libExiv2, IPTC functions
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
#include "kbvExiv.h"

/*************************************************************************//*!
 * Return 'true' when Iptc tags are present.
*/
bool KbvExiv::hasIptc() const
{
    return !iptcMetadata.empty();
}
/*************************************************************************//*!
 * Clear all Iptc tags in meta data container in memory.
*/
bool  KbvExiv::clearIptc()
{
  try
    {
      this->iptcMetadata.clear();
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::clearIptc exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::clearIptc default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Remove Iptc tag indicated by iptcTagName = familyName.groupName.tagName
*/
bool  KbvExiv::removeIptcTag(const QString iptcTagName)
{
  QString   key;
  try
    {
      Exiv2::IptcData::iterator it = this->iptcMetadata.begin();
      int i = 0;

      while(it != this->iptcMetadata.end())
        {
          key = QString::fromLatin1(it->key().c_str());
          if(key == iptcTagName)
            {
              it = this->iptcMetadata.erase(it);
              ++i;
            }
          else
            {
              ++it;
            }
        };
      return (i > 0);
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::removeIptcTag exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::removeIptcTag default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Return the Iptc Tag title or an empty string.
 */
QString KbvExiv::getIptcTagTitle(const QString iptcTagName)
{
  try
    {
      std::string     iptckey(iptcTagName.toStdString());
      Exiv2::IptcKey  key(iptckey);
      return QString::fromLocal8Bit(Exiv2::IptcDataSets::dataSetTitle(key.tag(), key.record()) );
    }
  catch (Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcTagTitle exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcTagTitle default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Return the Iptc Tag description or an empty string.
 */
QString KbvExiv::getIptcTagDescription(const QString iptcTagName)
{
  try
    {
      std::string     iptckey(iptcTagName.toStdString());
      Exiv2::IptcKey  key(iptckey);
      return QString::fromLocal8Bit(Exiv2::IptcDataSets::dataSetDesc(key.tag(), key.record()) );
    }
  catch (Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcTagDescription exception"<<e.what(); //###########
    }
  catch(...)
    {
    qDebug() <<"KbvExiv::getIptcTagDescription default exception"; //###########
    }
  return QString();
}

/*************************************************************************//*!
 * Read character set from Iptc tag 1:90 Coded Character Set.
*/
int KbvExiv::getIptcCharacterSet() const
{
  QByteArray    ba;
  int           charSet;

  //IIM Envelope Record
  //1:90 Coded Character Set (Optinal)
  ba = getIptcTagData("Iptc.Envelope.CharacterSet").toHex();
  if(ba.isEmpty())
    {
      charSet = KbvExiv::IptcUTF8; //TODO: use Exiv2 IptcData detectCharset() ????
    }
  if(ba.startsWith(QByteArray("1b2547")))
    {
      charSet = KbvExiv::IptcUTF8;
    }
  if(ba.startsWith(QByteArray("1b2e41")))
    {
      charSet = KbvExiv::Iptc88591;
    }
  //qDebug() << "KbvImageMetaData 1:90 character set"<<charSet; //###########
  return  charSet;
}
/*************************************************************************//*!
 * Set character set in Iptc tag 1:90 Coded Character Set.
*/
bool KbvExiv::setIptcCharacterSet(int charSet)
{
  QString value;

  //IIM Envelope Record, 1:90 Coded Character Set (Optinal)
  if(charSet == KbvExiv::IptcUTF8)
    {
      value = "\33%G";
    }
  if(charSet == KbvExiv::Iptc88591)
    {
      value = "\33.A";
    }
  if(!setIptcTagDataString("Iptc.Envelope.CharacterSet", value))
    {
      qDebug() << "KbvExivIptc cannot set Iptc char set UTF-8"; //###########
      return false;
    }
  return true;
}
/*************************************************************************//*!
 * Get Iptc tag data as byte array from iptcTagName = familyName.groupName.tagName
*/
QByteArray  KbvExiv::getIptcTagData(const QString iptcTagName) const
{
  try
    {
      Exiv2::IptcKey    iptcKey(iptcTagName.toStdString());
      Exiv2::IptcData   iptcData(this->iptcMetadata);
      Exiv2::IptcData::iterator it = iptcData.findKey(iptcKey);

      if(it != iptcData.end())
        {
          char* const s = new char[(*it).size()];
          (*it).copy((Exiv2::byte*)s, Exiv2::bigEndian);
          QByteArray data(s, (*it).size());
          delete [] s;
          return data;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcTagData exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcTagData default exception"; //###########
    }
  return QByteArray();
}
/*************************************************************************//*!
 * Replace or add (if not present) non empty tag data as QByteArray in
 * iptcTagName = familyName.groupName.tagName
*/
bool  KbvExiv::setIptcTagData(QString iptcTagName, const QByteArray& data)
{
  if(data.isEmpty())
    {
      return false;
    }
  try
    {
      Exiv2::DataValue val((Exiv2::byte *)data.data(), data.size());
      this->iptcMetadata[iptcTagName.toStdString()] = val;
      return true;
    }
    catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::setIptcTagData exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setIptcTagData default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get Iptc tag data as QString from iptcTagName = familyName.groupName.tagName
*/
QString KbvExiv::getIptcTagDataString(const QString iptcTagName, int charSet) const
{
  QString   value;

  try
    {
      Exiv2::IptcKey    iptcKey(iptcTagName.toStdString());
      Exiv2::IptcData   iptcData(this->iptcMetadata);
      Exiv2::IptcData::iterator it = iptcData.findKey(iptcKey);

      if(it != iptcData.end())
        {
          std::ostringstream os;
          os << *it;
          if(charSet == KbvExiv::IptcUTF8)
            {
              value = QString::fromUtf8(os.str().c_str());
            }
          if(charSet == KbvExiv::Iptc88591)
            {
              value = QString::fromLatin1(os.str().c_str());
            }
          return value;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcTagDataString exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcTagDataString default exception"; //###########
    }
  return QString();
}
/*************************************************************************//*!
 * Replace or add (if not present) non empty tag data as QString in
 * iptcTagName = familyName.groupName.tagName
*/
bool  KbvExiv::setIptcTagDataString(QString iptcTagName, const QString& value)
{
  if(value.isEmpty())
    {
      return false;
    }
  try
    {
      this->iptcMetadata[iptcTagName.toStdString()] = std::string(value.toUtf8().constData());
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::setIptcTagDataString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setIptcTagDataString default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get a list of data from repeatable Iptc tag (familyName.groupName.tagName)
 * Returns an empty list if no tag is found.\n
 * Some Iptc tags can appear as multiple tags with the same name, e.g. keywords.\n
 * If the 'escapeCR' parameter is true, the CR characters will be removed.\n
 * The list content is coded in character set 'charSet' (UTF-8, 8859-1 or local).
 */
QStringList KbvExiv::getIptcTagStringList(const QString iptcTagName, bool escapeCR, int charSet) const
{
  QStringList   valuelist;
  QString       key, value;

  try
    {
      if(!this->iptcMetadata.empty())
        {
          Exiv2::IptcData iptcData(this->iptcMetadata);
          for(Exiv2::IptcData::iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
              key = QString::fromLatin1(it->key().c_str());
              if(key == iptcTagName)
                {
                  if(charSet == KbvExiv::IptcUTF8)
                    {
                      value = QString::fromUtf8(it->toString().c_str());
                    }
                  if(charSet == KbvExiv::Iptc88591)
                    {
                      value = QString::fromLatin1(it->toString().c_str());
                    }
                  if(escapeCR)
                    {
                      value.replace('\n', ' ');
                    }
                  valuelist.append(value);
                }
            }
          return valuelist;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcTagsStringList exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcTagsStringList default exception"; //###########
    }
  return QStringList();
}
/*************************************************************************//*!
 * Set repeatable Iptc tag 'iptcTagName' with data of list 'newValues'. When
 * 'oldValues' allready contain a member of 'newValues' nothing will be added.
 * 'maxSize' is the max characters size of one entry.\n
 * Returns true if all tags have been set successfully.
*/
bool  KbvExiv::setIptcTagStringList(const QString iptcTagName, int maxSize,
                                   const QStringList& oldValues, const QStringList& newValues)
{
  QString   key, value;
  try
    {
      QStringList oldvals = oldValues;
      QStringList newvals = newValues;

      // Remove all old values.
      Exiv2::IptcData iptcData(this->iptcMetadata);
      Exiv2::IptcData::iterator it = iptcData.begin();
      while(it != iptcData.end())
        {
          key = QString::fromLocal8Bit(it->key().c_str());
          value = QString::fromUtf8(it->toString().c_str());

          // Also remove new values to avoid duplicates. They will be added again below.
          if(key == QString(iptcTagName) && (oldvals.contains(value) || newvals.contains(value)))
            {
              it = iptcData.erase(it);
            }
          else
            {
              ++it;
            }
        };

      // Add new values.
      Exiv2::IptcKey iptcTag(iptcTagName.toStdString());

      for(QStringList::iterator it = newvals.begin(); it != newvals.end(); ++it)
        {
          key = *it;
          key.truncate(maxSize);

          Exiv2::Value::AutoPtr rawval = Exiv2::Value::create(Exiv2::string);
          rawval->read(key.toUtf8().constData());
          iptcData.add(iptcTag, rawval.get());
        }
      this->iptcMetadata = iptcData;
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::setIptcTagsStringList default exception"<<iptcTagName <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setIptcTagsStringList default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get a list of Iptc keyword data. The list content is coded in character set
 * 'charSet' (UTF-8, 8859-1 or local).\n
 * Returns an empty list if no tag is found.\n
 */
QStringList KbvExiv::getIptcKeywords(const int charSet) const
{
  QStringList   keywords;
  QString       key, value;

  try
    {
      if(!this->iptcMetadata.empty())
        {
          Exiv2::IptcData iptcData(this->iptcMetadata);
          for (Exiv2::IptcData::iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
              key = QString::fromLocal8Bit(it->key().c_str());
              if(key == QString("Iptc.Application2.Keywords"))
                {
                  if(charSet == KbvExiv::IptcUTF8)
                    {
                      value = QString::fromUtf8(it->toString().c_str());
                    }
                  if(charSet == KbvExiv::Iptc88591)
                    {
                      value = QString::fromLatin1(it->toString().c_str());
                    }
                  value.replace('\n', ' ');  //remove CR
                  keywords.append(value);
                }
            }
          //qDebug() <<"KbvExiv::getIptcKeywords" <<keywords;
          return keywords;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcKeywords exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcKeywords default exception"; //###########
    }
  return QStringList();
}
/*************************************************************************//*!
 * Set repeatable Iptc tag keywords with data of list 'keywords'.
 * Existing keywords are removed then the data of list 'keywords' are added.
 * Returns true if all tags have been set successfully.
*/
bool  KbvExiv::setIptcKeywords(const QStringList& keywords)
{
  QString     key;
  QStringList values = keywords;

  try
    {
      // Remove all existent keywords.
      Exiv2::IptcData iptcData(this->iptcMetadata);
      Exiv2::IptcData::iterator it = iptcData.begin();
      while(it != iptcData.end())
        {
          key = QString::fromLatin1(it->key().c_str());
          if(key.endsWith("Application2.Keywords"))
            {
              it = iptcData.erase(it);
            }
          else
            {
              ++it;
            }
        };

      // Add new keywords. Each IPTC keyword tag is limited to 64 characters but
      //there can be multiple keyword tags.
      Exiv2::IptcKey iptcTag("Iptc.Application2.Keywords");
      for(QStringList::iterator it = values.begin(); it != values.end(); ++it)
        {
          key = *it;
          key.truncate(64);

          Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
          val->read(key.toUtf8().constData());
          iptcData.add(iptcTag, val.get());
        }
      this->iptcMetadata = iptcData;
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::setIptcKeywords exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setIptcKeywords default exception"; //###########
    }
  return false;
}
/*************************************************************************//*!
 * Get a list of Iptc subjects. Returns an empty list if no tag is found.
 */
QStringList KbvExiv::getIptcSubjects() const
{
  QStringList   subjects;
  QString       key, value;

  try
    {
      if(!this->iptcMetadata.empty())
        {
          Exiv2::IptcData iptcData(this->iptcMetadata);
          for(Exiv2::IptcData::iterator it = iptcData.begin(); it != iptcData.end(); ++it)
            {
              key = QString::fromLocal8Bit(it->key().c_str());
              if(key == QString("Iptc.Application2.Subject"))
                {
                  value = QString::fromUtf8(it->toString().c_str());
                  subjects.append(value);
                }
            }
          return subjects;
        }
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getIptcSubjects exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getIptcSubjects default exception"; //###########
    }
  return QStringList();
}
/*************************************************************************//*!
 * Set repeatable Iptc tag subjects with data of list 'subjects'.
 * Existing subjects are removed then the data of 'subjects' are added.
 * Returns true if all tags have been set successfully.
*/
bool  KbvExiv::setIptcSubjects(const QStringList& subjects)
{
  QString   key;
  QStringList values = subjects;
  try
    {
      // Remove all existing subjects.
      Exiv2::IptcData iptcData(this->iptcMetadata);
      Exiv2::IptcData::iterator it = iptcData.begin();
      while(it != iptcData.end())
        {
          key = QString::fromLatin1(it->key().c_str());
          if(key.endsWith("Application2.Subject"))
            {
              it = iptcData.erase(it);
            }
          else
            {
              ++it;
            }
        };

      // Add new subjects. Each IPTC subject tag is limited to 236 characters but
      //there can be multiple subject tags.
      Exiv2::IptcKey iptcTag("Iptc.Application2.Subject");
      for(QStringList::iterator it = values.begin(); it != values.end(); ++it)
        {
          key = *it;
          key.truncate(236);

          Exiv2::Value::AutoPtr val = Exiv2::Value::create(Exiv2::string);
          val->read(key.toUtf8().constData());
          iptcData.add(iptcTag, val.get());
        }
      this->iptcMetadata = iptcData;
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::setIptcSubjects default exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setIptcSubjects default exception"; //###########
    }
  return false;
}

/****************************************************************************/
