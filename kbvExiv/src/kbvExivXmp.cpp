/*****************************************************************************
 * KbvExivCommon
 * Dyn. library interface to libExiv2, XMP functions
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
 * Return 'true' when XMP tags are present.
*/
bool KbvExiv::hasXmp() const
{
#ifdef _XMP_SUPPORT_
    return !this->xmpMetadata.empty();
#endif // _XMP_SUPPORT_
    return false;
}
/*************************************************************************//*!
 * Clear all XMP tags in meta data container in memory.
*/
bool KbvExiv::clearXmp()
{
#ifdef _XMP_SUPPORT_
  try
    {
      this->xmpMetadata.clear();
      return true;
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::clearXmp exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::clearXmp default exception"; //###########
    }
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Remove the Xmp tag 'xmpTagName' from Xmp metadata.
 * Return true if tag is removed successfully or if no tag was present.
*/
bool KbvExiv::removeXmpTag(const char* xmpTagName)
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpKey             xmpKey(xmpTagName);
      Exiv2::XmpData::iterator  it = this->xmpMetadata.findKey(xmpKey);
      if(it != this->xmpMetadata.end())
        {
          this->xmpMetadata.erase(it);
          return true;
        }
    }
  catch( Exiv2::Error& e )
    {
     qDebug() << "KbvExiv::removeXmpTag exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::removeXmpTag default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
#endif // _XMP_SUPPORT_
  return false;
}

/*************************************************************************//*!
 * Return a QByteArray copy of XMP meta data container read from current image.
 * Return an empty QByteArray if there is no Xmp meta data available.
 */
QByteArray KbvExiv::getXmp() const
{
#ifdef _XMP_SUPPORT_
  try
    {
      if(!this->xmpMetadata.empty())
        {
          std::string xmpPacket;
          Exiv2::XmpParser::encode(xmpPacket, this->xmpMetadata);
          QByteArray data(xmpPacket.data(), xmpPacket.size());
          return data;
        }
    }
  catch(Exiv2::Error& e)
    {
      if(!this->filePath.isEmpty())
        {
          qDebug() << "KbvExiv::getXmp exception"<<e.what(); //###########
        }
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmp default exception"; //###########
    }
#endif // _XMP_SUPPORT_
  return QByteArray();
}
/*************************************************************************//*!
 * Set the XMP data using a QByteArray. Return true if XMP metadata have been
 * changed in the container.
*/
bool KbvExiv::setXmp(const QByteArray& data)
{
#ifdef _XMP_SUPPORT_
  try
    {
      if(!data.isEmpty())
        {
          std::string xmpPacket;
          xmpPacket.assign(data.data(), data.size());
          if(Exiv2::XmpParser::decode(this->xmpMetadata, xmpPacket) != 0)
            {
              return false;
            }
          else
            {
              return true;
            }
        }
    }
  catch(Exiv2::Error& e)
    {
      if(!this->filePath.isEmpty())
        {
          qDebug() << "KbvExiv::setXmp exception"<<e.what(); //###########
        }
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmp default exception"; //###########
    }
#else
Q_UNUSED(data);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Return the XMP Tag title or a null string.
*/
QString KbvExiv::getXmpTagTitle(const char* xmpTagName)
{
#ifdef _XMP_SUPPORT_
  try
    {
      std::string     xmpkey(xmpTagName);
      Exiv2::XmpKey   xk(xmpkey);
      return QString::fromLocal8Bit( Exiv2::XmpProperties::propertyTitle(xk) );
    }
  catch (Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getXmpTagTitle exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagTitle default exception"; //###########
    }
#else
Q_UNUSED(xmpTagName);
#endif // _XMP_SUPPORT_
  return QString();
}
/*************************************************************************//*!
 * Return the XMP Tag description or a null string.
*/
QString KbvExiv::getXmpTagDescription(const char* xmpTagName)
{
#ifdef _XMP_SUPPORT_
  try
    {
      std::string xmpkey(xmpTagName);
      Exiv2::XmpKey xk(xmpkey);
      return QString::fromLocal8Bit(Exiv2::XmpProperties::propertyDesc(xk));
    }
  catch(Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getXmpTagDescription exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagDescription default exception"; //###########
    }
#else
Q_UNUSED(xmpTagName);
#endif // _XMP_SUPPORT_
  return QString();
}
/*************************************************************************//*!
 * Return a map of XMP name/value found in metadata sorted by Xmp keys given
 * in 'xmpKeysFilter'. 'xmpKeysFilter' is a QStringList of Xmp keys.
 * For example, if you use the string list given below:
 *   "dc"           // Dubling Core schema.
 *   "xmp"          // Standard Xmp schema.
 * The returned list will contain all XMP tags including "dc" or "xmp" in the
 * Xmp tag keys. If 'inverSelection' is true the list will not contain tags
 * including "dc" or "xmp".
 * Currently supported by Exiv2-0.25: dc, digiKam, xmp, xmpRights, xmpMM, xmpBJ,
 * xmpTPg, xmpDM, pdf, photoshop, crs, tiff, exif, aux, kipi,
 * plus, mgw-rs, mgw-kc, dwc, Iptc4xmpCore, Iptc4xmpExt, dcterms,
 * GPano, lr, MP, acadsee, mediapro, expressionmedia, MicrosoftPhoto
 * Note: Exiv2 keys are
 * Xmp.iptc.<Property> instead of Xmp.Iptc4xmpCore.<Property> and
 * Xmp.iptcExt.<Property> instead of Xmp.Iptc4xmpExt.<Property>
 * An empty list will not filter (returns all tags).
*/
KbvExiv::MetaDataMap KbvExiv::getXmpTagsDataList(const QStringList& xmpKeysFilter, bool invertSelection) const
{
#ifdef _XMP_SUPPORT_
  if(xmpMetadata.empty())
    {
      return KbvExiv::MetaDataMap();
    }

  try
    {
      Exiv2::XmpData xmpData = xmpMetadata;
      xmpData.sortByKey();

      KbvExiv::MetaDataMap       metaDataMap;
      for(Exiv2::XmpData::iterator  md = xmpData.begin(); md != xmpData.end(); ++md)
        {
          QString key = QString::fromLatin1(md->key().c_str());
          // Decode the tag value with a user friendly output.
          std::ostringstream os;
          os << *md;
          QString value = QString::fromUtf8(os.str().c_str());

          // If the tag is a language alternative type, parse content to detect language.
          if(md->typeId() == Exiv2::langAlt)
            {
              QString lang;
              value = detectLanguageAlt(value, lang);
            }
          else
            {
              value = QString::fromUtf8(os.str().c_str());
            }
            // To make a string just on one line.
            value.replace('\n', ' ');
            // Some XMP key are redundant. Check if already one exists.
            KbvExiv::MetaDataMap::iterator it = metaDataMap.find(key);

           // We apply a filter to get only the XMP tags that we need.
          if(!xmpKeysFilter.isEmpty())
            {
              if(!invertSelection)
                {
                  if(xmpKeysFilter.contains(key.section('.', 1, 1)))
                    {
                      if (it == metaDataMap.end())
                        {
                          metaDataMap.insert(key, value);
                        }
                      else
                        {
                          QString v = *it;
                          v.append(", ");
                          v.append(value);
                          metaDataMap.insert(key, v);
                        }
                    }
                }
              else
                {
                  if(!xmpKeysFilter.contains(key.section('.', 1, 1)))
                    {
                      if(it == metaDataMap.end())
                        {
                          metaDataMap.insert(key, value);
                        }
                      else
                        {
                          QString v = *it;
                          v.append(", ");
                          v.append(value);
                          metaDataMap.insert(key, v);
                        }
                    }
                }
            }
          else // else no filter at all.
            {
              if(it == metaDataMap.end())
                {
                  metaDataMap.insert(key, value);
                }
              else
                {
                  QString v = *it;
                  v.append(", ");
                  v.append(value);
                  metaDataMap.insert(key, v);
                }
            }
        }
      return metaDataMap;
    }
  catch (Exiv2::Error& e)
    {
      qDebug() << "KbvExiv::getXmpTagsDataList exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagsDataList default exception"; //###########
    }
#else
  Q_UNUSED(xmpKeysFilter);
  Q_UNUSED(invertSelection);
#endif // _XMP_SUPPORT_
  return KbvExiv::MetaDataMap();
}
/*************************************************************************//*!
 * Get a Xmp tag content like a string. If 'escapeCR' parameter is true, the CR characters
 * will be removed. If Xmp tag cannot be found a null string is returned.
*/
QString KbvExiv::getXmpTagDataString(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData            xmpData(this->xmpMetadata);
      Exiv2::XmpKey             key(xmpTagName);
      Exiv2::XmpData::iterator  it = xmpData.findKey(key);
      if(it != xmpData.end())
        {
          std::ostringstream os;
          os << *it;
          QString tagValue = QString::fromUtf8(os.str().c_str());
          if(escapeCR)
            {
              tagValue.replace('\n', ' ');
            }
          return tagValue;
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagDataString exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagDataString default exception"; //###########
    }

#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(escapeCR);
#endif // _XMP_SUPPORT_
  return QString();
}
/*************************************************************************//*!
 * Set data of XMP tag 'xmpTagName' from QString 'value'.
 * Return true if tag is set successfully.
*/
bool KbvExiv::setXmpTagDataString(const char* xmpTagName, const QString& value)
{
#ifdef _XMP_SUPPORT_
  try
    {
      const std::string &txt(value.toUtf8().constData());
      Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::xmpText);
      xmpTxtVal->read(txt);
      this->xmpMetadata[xmpTagName].setValue(xmpTxtVal.get());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagDataString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagDataString default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(value);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Set data of XMP tag 'xmpTagName' from QString 'value' as XMP type 'type'.
 * This method only accepts NormalTag, ArrayBagTag and StructureTag. Other
 * XmpTagTypes do nothing. Return true if tag is set successfully.
*/
bool KbvExiv::setXmpTagDataString(const char* xmpTagName, const QString& value,
                                  KbvExiv::XmpTagType type)
{
#ifdef _XMP_SUPPORT_
  try
    {
      const std::string &txt(value.toUtf8().constData());
      Exiv2::XmpTextValue xmpTxtVal("");

      if(type == KbvExiv::NormalTag) // normal type
        {
          xmpTxtVal.read(txt);
          this->xmpMetadata.add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
          return true;
        }
      if(type == KbvExiv::ArrayBagTag) // xmp type = bag
        {
          xmpTxtVal.setXmpArrayType(Exiv2::XmpValue::xaBag);
          xmpTxtVal.read("");
          this->xmpMetadata.add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
        }
      if(type == KbvExiv::StructureTag) // xmp type = struct
        {
          xmpTxtVal.setXmpStruct();
          this->xmpMetadata.add(Exiv2::XmpKey(xmpTagName),&xmpTxtVal);
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagDataString exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagDataString default exception"; //###########
    }

#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(value);
  Q_UNUSED(type);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Get all redundant Alternative Language XMP tags content as map.
 * See AltLangMap class description for details.
 * If 'escapeCR' parameter is true, the CR characters will be removed from strings.
 * If Xmp tag cannot be found a null string list is returned.
*/
KbvExiv::AltLangMap KbvExiv::getXmpTagStringListLangAlt(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData xmpData = this->xmpMetadata;
      for(Exiv2::XmpData::iterator it = xmpData.begin(); it != xmpData.end(); ++it)
        {
          if(it->key() == xmpTagName && it->typeId() == Exiv2::langAlt)
            {
              KbvExiv::AltLangMap map;
              const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());
              for(Exiv2::LangAltValue::ValueType::const_iterator it2 = value.value_.begin(); it2 != value.value_.end(); ++it2)
                {
                  QString lang = QString::fromUtf8(it2->first.c_str());
                  QString text = QString::fromUtf8(it2->second.c_str());
                  if (escapeCR)
                    {
                      text.replace('\n', ' ');
                    }
                  map.insert(lang, text);
                }
              return map;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagStringListLangAlt exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagStringListLangAlt default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(escapeCR);
#endif // _XMP_SUPPORT_
  return KbvExiv::AltLangMap();
}
/*************************************************************************//*!
 * Set an Alternative Language XMP tag contents from a map.
 * See AltLangMap class description for details.
 * If tag already exist, it will be removed before.
 * Return true if tag is set successfully.
*/
bool KbvExiv::setXmpTagStringListLangAlt(const char* xmpTagName, const KbvExiv::AltLangMap& values)
{
#ifdef _XMP_SUPPORT_
  try
    {
      // Remove old XMP alternative Language tag.
      removeXmpTag(xmpTagName);
      if(!values.isEmpty())
        {
          Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);
          for(KbvExiv::AltLangMap::const_iterator it = values.constBegin(); it != values.constEnd(); ++it)
            {
              QString lang = it.key();
              QString text = it.value();
              QString txtLangAlt = QString("lang=%1 %2").arg(lang).arg(text);
              const std::string &txt(txtLangAlt.toUtf8().constData());
              xmpTxtVal->read(txt);
            }
          // ...and add the new one instead.
          this->xmpMetadata.add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());
        }
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagStringListLangAlt exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagStringListLangAlt default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(values);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Get Xmp tag content as string containing an alternative language header
 * 'langAlt' (like "fr-FR" for French - RFC3066 notation).
 * If 'escapeCR' parameter is true, the CR characters will be removed.
 * If Xmp tag cannot be found a null string is returned.
*/
QString KbvExiv::getXmpTagStringLangAlt(const char* xmpTagName, const QString& langAlt, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData                xmpData(this->xmpMetadata);
      Exiv2::XmpKey                 key(xmpTagName);
      for(Exiv2::XmpData::iterator  it = xmpData.begin(); it != xmpData.end(); ++it)
        {
          if(it->key() == xmpTagName && it->typeId() == Exiv2::langAlt)
            {
              for(int i = 0; i < it->count(); i++)
                {
                  std::ostringstream os;
                  os << it->toString(i);
                  QString lang;
                  QString tagValue = QString::fromUtf8(os.str().c_str());
                  tagValue = detectLanguageAlt(tagValue, lang);
                  if(langAlt == lang)
                    {
                      if (escapeCR)
                        {
                          tagValue.replace('\n', ' ');
                        }
                      return tagValue;
                    }
                }
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagStringLangAlt exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagStringLangAlt default exception"; //###########
    }

#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(langAlt);
  Q_UNUSED(escapeCR);
#endif // _XMP_SUPPORT_
  return QString();
}
/*************************************************************************//*!
 * Set a Xmp tag content using a string with an alternative language header
 * 'langAlt' containing the alternative language information (like "fr-FR" for
 * French - RFC3066 notation) or containing null to set default settings
 * ("x-default"). Return true if tag is set successfully.
*/
bool KbvExiv::setXmpTagStringLangAlt(const char* xmpTagName, const QString& value, const QString& langAlt)
{
#ifdef _XMP_SUPPORT_
  try
    {
      QString language("x-default"); // default alternative language.
      if(!langAlt.isEmpty())
        {
          language = langAlt;
        }
      QString txtLangAlt = QString("lang=%1 %2").arg(language).arg(value);
      const std::string &txt(txtLangAlt.toUtf8().constData());
      Exiv2::Value::AutoPtr xmpTxtVal = Exiv2::Value::create(Exiv2::langAlt);

      // Search if an Xmp tag already exist.
      KbvExiv::AltLangMap map = getXmpTagStringListLangAlt(xmpTagName, false);
      if(!map.isEmpty())
        {
          for(KbvExiv::AltLangMap::iterator it = map.begin(); it != map.end(); ++it)
            {
              if(it.key() != langAlt)
                {
                  const std::string &val((*it).toUtf8().constData());
                  xmpTxtVal->read(val);
                  //kDebug() << *it;
                }
            }
        }
      xmpTxtVal->read(txt);
      removeXmpTag(xmpTagName);
      this->xmpMetadata.add(Exiv2::XmpKey(xmpTagName), xmpTxtVal.get());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagStringLangAlt exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagStringLangAlt default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(value);
  Q_UNUSED(langAlt);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Get a XMP tag content as sequence of strings.
 * If 'escapeCR' parameter is true, the CR characters will be removed from
 * strings. If Xmp tag cannot be found a null string list is returned.
*/
QStringList KbvExiv::getXmpTagStringSeq(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData            xmpData(this->xmpMetadata);
      Exiv2::XmpKey             key(xmpTagName);
      Exiv2::XmpData::iterator  it = xmpData.findKey(key);
      if(it != xmpData.end())
        {
          if(it->typeId() == Exiv2::xmpSeq)
            {
              QStringList seq;
              for(int i = 0; i < it->count(); i++)
                {
                  std::ostringstream os;
                  os << it->toString(i);
                  QString seqValue = QString::fromUtf8(os.str().c_str());
                  if (escapeCR)
                    {
                      seqValue.replace('\n', ' ');
                    }
                  seq.append(seqValue);
                }
              return seq;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagStringSeq exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagStringSeq default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(escapeCR);
#endif // _XMP_SUPPORT_
  return QStringList();
}
/*************************************************************************//*!
 * Set a XMP tag content using the sequence of strings 'seq'.
 * Return true if tag is set successfully.
*/
bool KbvExiv::setXmpTagStringSeq(const char* xmpTagName, const QStringList& seq)
{
#ifdef _XMP_SUPPORT_
  try
    {
      if(seq.isEmpty())
        {
          removeXmpTag(xmpTagName);
        }
      else
        {
          const QStringList               list = seq;
          Exiv2::Value::AutoPtr           xmpTxtSeq = Exiv2::Value::create(Exiv2::xmpSeq);
          for(QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
            {
              const std::string &txt((*it).toUtf8().constData());
              xmpTxtSeq->read(txt);
            }
          this->xmpMetadata[xmpTagName].setValue(xmpTxtSeq.get());
        }
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagStringSeq exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagStringSeq default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(seq);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Get a Xmp tag content as bag of strings. If 'escapeCR' parameter is true,
 * the CR characters will be removed from strings.
 * If Xmp tag cannot be found a null string list is returned.
*/
QStringList KbvExiv::getXmpTagStringBag(const char* xmpTagName, bool escapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData            xmpData(this->xmpMetadata);
      Exiv2::XmpKey             key(xmpTagName);
      Exiv2::XmpData::iterator  it = xmpData.findKey(key);
      if(it != xmpData.end())
        {
          if(it->typeId() == Exiv2::xmpBag)
            {
              QStringList bag;
              for(int i = 0; i < it->count(); i++)
                {
                  std::ostringstream os;
                  os << it->toString(i);
                  QString bagValue = QString::fromUtf8(os.str().c_str());
                  if (escapeCR)
                    {
                      bagValue.replace('\n', ' ');
                    }
                  bag.append(bagValue);
                }
              return bag;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagStringBag exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagStringBag default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(escapeCR);
#endif // _XMP_SUPPORT_
  return QStringList();
}
/*************************************************************************//*!
 * Set a Xmp tag content as bag from stringlist 'bag'.
 * Return true if tag is set successfully.
*/
bool  KbvExiv::setXmpTagStringBag(const char* xmpTagName, const QStringList& bag)
{
#ifdef _XMP_SUPPORT_
  try
    {
      if(bag.isEmpty())
        {
            removeXmpTag(xmpTagName);
        }
      else
        {
          QStringList           list = bag;
          Exiv2::Value::AutoPtr xmpTxtBag = Exiv2::Value::create(Exiv2::xmpBag);
          for(QStringList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it )
            {
              const std::string &txt((*it).toUtf8().constData());
              xmpTxtBag->read(txt);
            }
          this->xmpMetadata[xmpTagName].setValue(xmpTxtBag.get());
        }
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::setXmpTagStringBag exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::setXmpTagStringBag default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(bag);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Set an Xmp tag content as bag from a list of strings 'entriesToAdd'.
 * The existing entries are preserved. The method compares the new entries with
 * already existing entries to prevent duplicates.
 * Return true if the entries have been added to metadata.
*/
bool KbvExiv::addToXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToAdd)
{
  QStringList oldEntries = getXmpTagStringBag(xmpTagName, false);
  QStringList newEntries = entriesToAdd;
  // Create a list of keywords including old one which already exists.
  for(QStringList::const_iterator it = oldEntries.constBegin(); it != oldEntries.constEnd(); ++it )
    {
      if(!newEntries.contains(*it))
        {
          newEntries.append(*it);
        }
    }
  if(setXmpTagStringBag(xmpTagName, newEntries))
    {
      return true;
    }
  return false;
}
/*************************************************************************//*!
 * Remove those XMP tag entries that are listed in entriesToRemove from the
 * entries in metadata. Return true if tag entries are no longer contained in
 * metadata. All other entries are preserved.
*/
bool  KbvExiv::removeFromXmpTagStringBag(const char* xmpTagName, const QStringList& entriesToRemove)
{
  QStringList currentEntries = getXmpTagStringBag(xmpTagName, false);
  QStringList newEntries;
  // Create a list of current keywords except those that shall be removed
  for(QStringList::const_iterator it = currentEntries.constBegin(); it != currentEntries.constEnd(); ++it )
    {
      if(!entriesToRemove.contains(*it))
        {
          newEntries.append(*it);
        }
    }
  if(setXmpTagStringBag(xmpTagName, newEntries))
    {
      return true;
    }
  return false;
}
/*************************************************************************//*!
 * Get an Xmp tag content as a QVariant. Returns a null QVariant if the Xmp
 * tag cannot be found.
 * For string and integer values the matching QVariant types will be used,
 * for date and time values QVariant::DateTime.
 * Rationals will be returned as QVariant::List with two integer QVariants
 * (numerator, denominator) if rationalAsListOfInts is true, as double if
 * rationalAsListOfInts is false.
 * Arrays (ordered, unordered, alternative) are returned as type StringList.
 * LangAlt values will have type Map (QMap<QString, QVariant>) with the language
 * code as key and the contents as value, of type String.
*/
QVariant KbvExiv::getXmpTagVariant(const char* xmpTagName, bool rationalAsListOfInts,
                                   bool stringEscapeCR) const
{
#ifdef _XMP_SUPPORT_
  try
    {
      Exiv2::XmpData            xmpData(this->xmpMetadata);
      Exiv2::XmpKey             key(xmpTagName);
      Exiv2::XmpData::iterator  it = xmpData.findKey(key);
      if(it != xmpData.end())
        {
          switch(it->typeId())
            {
              case Exiv2::unsignedByte:
              case Exiv2::unsignedShort:
              case Exiv2::unsignedLong:
              case Exiv2::signedShort:
              case Exiv2::signedLong:
                    return QVariant((int)it->toLong());
              case Exiv2::unsignedRational:
              case Exiv2::signedRational:
                if(rationalAsListOfInts)
                  {
                    QList<QVariant> list;
                    list << (*it).toRational().first;
                    list << (*it).toRational().second;
                    return QVariant(list);
                  }
                else
                  {
                    // prefer double precision
                    double num = (*it).toRational().first;
                    double den = (*it).toRational().second;
                    if(den == 0.0)
                      {
                        return QVariant(QVariant::Double);
                      }
                    return QVariant(num / den);
                  }
              case Exiv2::date:
              case Exiv2::time:
                {
                  QDateTime dateTime = QDateTime::fromString(QString(it->toString().c_str()), Qt::ISODate);
                  return QVariant(dateTime);
                }
              case Exiv2::asciiString:
              case Exiv2::comment:
              case Exiv2::string:
                {
                  std::ostringstream os;
                  os << *it;
                  QString tagValue = QString::fromLocal8Bit(os.str().c_str());
                  if (stringEscapeCR)
                    {
                      tagValue.replace('\n', ' ');
                    }
                  return QVariant(tagValue);
                }
              case Exiv2::xmpText:
                {
                  std::ostringstream os;
                  os << *it;
                  QString tagValue = QString::fromUtf8(os.str().c_str());
                  if (stringEscapeCR)
                    {
                      tagValue.replace('\n', ' ');
                    }
                  return tagValue;
                }
              case Exiv2::xmpBag:
              case Exiv2::xmpSeq:
              case Exiv2::xmpAlt:
                {
                  QStringList list;
                  for(int i=0; i < it->count(); i++)
                    {
                      list << QString::fromUtf8(it->toString(i).c_str());
                    }
                  return list;
                }
              case Exiv2::langAlt:
                {
                  // access the value directly
                  const Exiv2::LangAltValue &value = static_cast<const Exiv2::LangAltValue &>(it->value());
                  QMap<QString, QVariant> map;
                  // access the ValueType std::map< std::string, std::string>
                  Exiv2::LangAltValue::ValueType::const_iterator i;
                  for(i = value.value_.begin(); i != value.value_.end(); ++i)
                    {
                      map[QString::fromUtf8(i->first.c_str())] = QString::fromUtf8(i->second.c_str());
                    }
                  return map;
                }
              default:
              break;
            }
        }
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << QString("KbvExiv::getXmpTagVariant exception %1").arg(xmpTagName) <<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::getXmpTagVariant default exception"; //###########
    }
#else
  Q_UNUSED(xmpTagName);
  Q_UNUSED(rationalAsListOfInts);
  Q_UNUSED(stringEscapeCR);
#endif // _XMP_SUPPORT_
  return QVariant();
}
/*************************************************************************//*!
* Return a string list of XMP keywords
* Return an empty list if no keyword are set.
*/
QStringList KbvExiv::getXmpKeywords() const
{
  return getXmpTagStringBag("Xmp.dc.subject", false);
}
/*************************************************************************//*!
 * Set XMP keywords from a list of strings 'newKeywords'.
 * The existing keywords from image are preserved. The method compares all new
 * keywords with already existing keywords to prevent duplicate entries.
 * Return true if keywords have been changed in metadata.
*/
bool KbvExiv::setXmpKeywords(const QStringList& newKeywords)
{
  return addToXmpTagStringBag("Xmp.dc.subject", newKeywords);
}
/*************************************************************************//*!
 * Remove those Xmp keywords that are listed in 'keywordsToRemove' from the
 * keywords in metadata.
 * Return true if keywords are no longer contained in metadata.
*/
bool KbvExiv::removeXmpKeywords(const QStringList& keywordsToRemove)
{
  return removeFromXmpTagStringBag("Xmp.dc.subject", keywordsToRemove);
}
/*************************************************************************//*!
 * Return a string list of XMP subjects.
 * Return an empty list if no subject are set.
*/
QStringList KbvExiv::getXmpSubjects() const
{
  return (getXmpTagStringBag("Xmp.iptc.SubjectCode", false));
}
/*************************************************************************//*!
 * Set XMP subjects from list 'newSubjects'.
 * The existing subjects from image are preserved. The method compares all new
 * subjects with already existing keywords to prevent duplicate entries.
 * Return true if subjects have been changed in metadata.
*/
bool KbvExiv::setXmpSubjects(const QStringList& newSubjects)
{
  return addToXmpTagStringBag("Xmp.iptc.SubjectCode", newSubjects);
}
/*************************************************************************//*!
 * Remove those XMP subjects in list 'subjectsToRemove' from metadata.
 * Return true if subjects are no longer contained in metadata.
*/
bool KbvExiv::removeXmpSubjects(const QStringList& subjectsToRemove)
{
  return removeFromXmpTagStringBag("Xmp.iptc.SubjectCode", subjectsToRemove);
}
/*************************************************************************//*!
 * Return a string list of XMP sub-categories.
 * Return an empty list if no sub-category is set.
 */
QStringList KbvExiv::getXmpSubCategories() const
{
  return getXmpTagStringBag("Xmp.photoshop.SupplementalCategories", false);
}
/*************************************************************************//*!
 * Set XMP sub-categories from list 'newSubCategories'.
 * The existing sub-categories from image are preserved. The method compares
 * all new sub-categories with already existing ones to prevent duplicates.
 * Return true if sub-categories have been changed in metadata.
*/
bool KbvExiv::setXmpSubCategories(const QStringList& newSubCategories)
{
  return addToXmpTagStringBag("Xmp.photoshop.SupplementalCategories", newSubCategories);
}
/*************************************************************************//*!
 * Remove those XMP sub-categories in list 'categoriesToRemove' from
 * metadata. Return true if subjects are no longer contained in metadata.
*/
bool KbvExiv::removeXmpSubCategories(const QStringList& subCategoriesToRemove)
{
  return removeFromXmpTagStringBag("Xmp.photoshop.SupplementalCategories", subCategoriesToRemove);
}
/*************************************************************************//*!
 * Return a map of all standard XMP tags supported by Exiv2.
*/
KbvExiv::TagsMap KbvExiv::getXmpTagsList()
{
  QStringList prefixList;
  prefixList  <<"dc" <<"digiKam" <<"xmp" <<"xmpRights" <<"xmpMM" <<"xmpBJ" <<"xmpTPg"
              <<"xmpDM" <<"MicrosoftPhoto" <<"pdf" <<"photoshop" <<"crs" <<"tiff" <<"exif"
              <<"aux" <<"iptc" <<"iptcExt" <<"plus" <<"mgw-rs" <<"dwc";

  KbvExiv::TagsMap tagsMap;
  for(int i=0; i<prefixList.size(); i++)
    {
      try
        {
          getXMPTagsListFromPrefix(prefixList.at(i), tagsMap);
        }
      catch(Exiv2::Error& e)
        {
          qDebug() <<"KbvExiv::getXmpTagsList exception"<<e.what(); //###########
        }
      catch(...)
        {
          qDebug() <<"KbvExiv::getXmpTagsList default exception"; //###########
        }
    }
  return tagsMap;
}
/*************************************************************************//*!
 * Insert into map 'tagsMap' all tags for a given prefix 'pf'.
*/
int KbvExiv::getXMPTagsListFromPrefix(const QString& pf, KbvExiv::TagsMap& tagsMap) const
{
    QList<const Exiv2::XmpPropertyInfo*> tags;
    tags << Exiv2::XmpProperties::propertyList(pf.toLatin1().data());
    int i = 0;

    for (QList<const Exiv2::XmpPropertyInfo*>::iterator it = tags.begin(); it != tags.end(); ++it)
      {
        while((*it) && !QString((*it)->name_).isNull())
          {
            QString     key = QLatin1String( Exiv2::XmpKey( pf.toLatin1().data(), (*it)->name_ ).key().c_str() );
            QStringList values;
            values << (*it)->name_ << (*it)->title_ << (*it)->desc_;
            tagsMap.insert(key, values);
            ++(*it);
            i++;
          }
      }
    return i;
}

// Static functions *********************************************************
/*************************************************************************//*!
 * Static function: Register a namespace which Exiv2 doesn't know yet.
 * This is only needed when new Xmp properties are added manually. 'uri' is
 * the namespace url and prefix the string used to construct new XMP key
 * (e.g. "XMP.digiKam.tagList").
 * Note: If the XMP metadata is read from an image, namespaces are decoded and
 * registered by Exiv2 at the same time.
*/
bool KbvExiv::registerXmpNameSpace(const QString& uri, const QString& prefix)
{
#ifdef _XMP_SUPPORT_
  try
    {
      QString ns = uri;
      if(!uri.endsWith('/'))
        {
          ns.append('/');
        }
      Exiv2::XmpProperties::registerNs(ns.toLatin1().constData(), prefix.toLatin1().constData());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::registerXmpNameSpace exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::registerXmpNameSpace default exception"; //###########
    }
#else
  Q_UNUSED(uri);
  Q_UNUSED(prefix);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Static function: Unregister a previously registered custom namespace.
*/
bool KbvExiv::unregisterXmpNameSpace(const QString& uri)
{
#ifdef _XMP_SUPPORT_
  try
    {
      QString ns = uri;
      if(!uri.endsWith('/'))
        {
          ns.append('/');
        }
      Exiv2::XmpProperties::unregisterNs(ns.toLatin1().constData());
      return true;
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::unregisterXmpNameSpace exception"<<e.what(); //###########
    }
  catch(...)
    {
      qDebug() <<"KbvExiv::unregisterXmpNameSpace default exception"; //###########
    }
#else
  Q_UNUSED(uri);
#endif // _XMP_SUPPORT_
  return false;
}
/*************************************************************************//*!
 * Static function: detect alternative language
 * Return a QString without language alternative header. Header is saved into
 * 'lang'. If no language alternative is found, value is returned as well and
 * 'lang' is set empty.
*/
QString KbvExiv::detectLanguageAlt(const QString& value, QString& lang)
{
  //E.g.  Xmp.tiff.copyright: "lang="x-default"
  if (value.size() > 6 && value.startsWith(QString("lang=\"")))
    {
      int pos = value.indexOf(QString("\""), 6);
      if (pos != -1)
        {
          lang = value.mid(6, pos-6);
          return (value.mid(pos+2));
        }
    }
  lang.clear();
  return value;
}


// Sidecar stuff **************************************************************
//*****************************************************************************
QString KbvExiv::sidecarFilePathForFile(const QString& path)
{
  QString ret;
  if (!path.isEmpty())
    {
      ret = path + QString(".xmp");
    }
  return ret;
}
//*****************************************************************************
void KbvExiv::loadSidecarData(Exiv2::Image::AutoPtr xmpsidecar)
{
  // Having a sidecar is a special situation.
  // The sidecar data often "dominates", see in particular bug 309058 for important aspects:
  // If a field is removed from the sidecar, we must ignore (older) data for this field in the file.
  // First: Ignore file XMP, only use sidecar XMP
  this->xmpMetadata = xmpsidecar->xmpData();
  loadedFromSidecar = true;

  // EXIF
  // Four groups of properties are mapped between EXIF and XMP:
  // Date/Time, Description, Copyright, Creator
  // A few more tags are defined "writeback" tags in the XMP specification, the sidecar value therefore overrides the Exif value.
  // The rest is kept side-by-side.
  // (to understand, remember that the xmpsidecar's Exif data is actually XMP data mapped back to Exif)

  //>>>>>>>> kexiv2_p.h -> KbvExiv
//TODO: correct sidcar IPTC, Exif
  // Description, Copyright and Creator is dominated by the sidecar: Remove file Exif fields, if field not in XMP.
  ExifMergeHelper exifDominatedHelper;
  exifDominatedHelper << QLatin1String("Exif.Image.ImageDescription")
                      << QLatin1String("Exif.Photo.UserComment")
                      << QLatin1String("Exif.Image.Copyright")
                      << QLatin1String("Exif.Image.Artist");
  exifDominatedHelper.exclusiveMerge(xmpsidecar->exifData(), this->exifMetadata);
  // Date/Time and "the few more" from the XMP spec are handled as writeback
  // Note that Date/Time mapping is slightly contradictory in latest specs.
  ExifMergeHelper exifWritebackHelper;
  exifWritebackHelper << QLatin1String("Exif.Image.DateTime")
                      << QLatin1String("Exif.Image.DateTime")
                      << QLatin1String("Exif.Photo.DateTimeOriginal")
                      << QLatin1String("Exif.Photo.DateTimeDigitized")
                      << QLatin1String("Exif.Image.Orientation")
                      << QLatin1String("Exif.Image.XResolution")
                      << QLatin1String("Exif.Image.YResolution")
                      << QLatin1String("Exif.Image.ResolutionUnit")
                      << QLatin1String("Exif.Image.Software")
                      << QLatin1String("Exif.Photo.RelatedSoundFile");
  exifWritebackHelper.mergeFields(xmpsidecar->exifData(), this->exifMetadata);

  // IPTC
  // These fields cover almost all relevant IPTC data and are defined in the XMP specification for reconciliation.
  IptcMergeHelper iptcDominatedHelper;
  iptcDominatedHelper << QLatin1String("Iptc.Application2.ObjectName")
                      << QLatin1String("Iptc.Application2.Urgency")
                      << QLatin1String("Iptc.Application2.Category")
                      << QLatin1String("Iptc.Application2.SuppCategory")
                      << QLatin1String("Iptc.Application2.Keywords")
                      << QLatin1String("Iptc.Application2.SubLocation")
                      << QLatin1String("Iptc.Application2.SpecialInstructions")
                      << QLatin1String("Iptc.Application2.Byline")
                      << QLatin1String("Iptc.Application2.BylineTitle")
                      << QLatin1String("Iptc.Application2.City")
                      << QLatin1String("Iptc.Application2.ProvinceState")
                      << QLatin1String("Iptc.Application2.CountryCode")
                      << QLatin1String("Iptc.Application2.CountryName")
                      << QLatin1String("Iptc.Application2.TransmissionReference")
                      << QLatin1String("Iptc.Application2.Headline")
                      << QLatin1String("Iptc.Application2.Credit")
                      << QLatin1String("Iptc.Application2.Source")
                      << QLatin1String("Iptc.Application2.Copyright")
                      << QLatin1String("Iptc.Application2.Caption")
                      << QLatin1String("Iptc.Application2.Writer");
  iptcDominatedHelper.exclusiveMerge(xmpsidecar->iptcData(), this->iptcMetadata);

  IptcMergeHelper iptcWritebackHelper;
  iptcWritebackHelper << QLatin1String("Iptc.Application2.DateCreated")
                      << QLatin1String("Iptc.Application2.TimeCreated")
                      << QLatin1String("Iptc.Application2.DigitizationDate")
                      << QLatin1String("Iptc.Application2.DigitizationTime");
  iptcWritebackHelper.mergeFields(xmpsidecar->iptcData(), this->iptcMetadata);

/*
  TODO: Exiv2 (referring to 0.23) does not correctly synchronize all time values as given below.
  Time values and their synchronization:
  Original Date/Time – Creation date of the intellectual content (e.g. the photograph),
  rather than the creation date of the content being shown
  Exif DateTimeOriginal (36867, 0x9003) and SubSecTimeOriginal (37521, 0x9291)
  IPTC DateCreated (IIM 2:55, 0x0237) and TimeCreated (IIM 2:60, 0x023C)
  XMP (photoshop:DateCreated)
  Digitized Date/Time – Creation date of the digital representation
  Exif DateTimeDigitized (36868, 0x9004) and SubSecTimeDigitized (37522, 0x9292)
  IPTC DigitalCreationDate (IIM 2:62, 0x023E) and DigitalCreationTime (IIM 2:63, 0x023F)
  XMP (xmp:CreateDate)
  Modification Date/Time – Modification date of the digital image file
  Exif DateTime (306, 0x132) and SubSecTime (37520, 0x9290)
  XMP (xmp:ModifyDate)
*/
}
//*****************************************************************************
bool  KbvExiv::saveToXMPSidecar(const QFileInfo& finfo)
{
  QString filePath = this->sidecarFilePathForFile(finfo.filePath());

  if(filePath.isEmpty())
    {
      return false;
    }
  try
    {
      Exiv2::Image::AutoPtr image;
      image = Exiv2::ImageFactory::create(Exiv2::ImageType::xmp, (const char*)(QFile::encodeName(filePath)));
      return saveOperations(finfo, image);
    }
  catch( Exiv2::Error& e )
    {
      qDebug() << "KbvExiv::saveToXMPSidecar exception"<<e.what(); //###########
      return false;
    }
}

/****************************************************************************/
