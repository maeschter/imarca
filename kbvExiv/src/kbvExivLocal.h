/*****************************************************************************
 * KbvExivLocal
 * This is the private part of kbvExiv.h
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 09:38:50 +0100 (Di, 27. Feb 2018) $
 * $Rev: 981 $
 * Created: 2017.05.21
 *****************************************************************************/
#ifndef KBVEXIVLOCAL_H
#define KBVEXIVLOCAL_H

//definitions
#define gpsVersionID    "2 0 0 0"

// Private common functions
  void      copyPrivateData(const KbvExiv* const other);
  bool      saveToFile(const QFileInfo &finfo) const;
  bool      saveOperations(const QFileInfo& finfo, Exiv2::Image::AutoPtr image) const;
  bool      detectUtf8(const char *buf) const;

// Private EXIF functions
  QString   convertCommentValue(const Exiv2::Exifdatum& exifDatum) const;
  QString   detectEncodingAndDecode(const std::string& value) const;
  bool      initializeGPSTags();
  bool      setGPSTags(const double* const altitude, const double latitude, const double longitude);

// Private XMP functions
  int       getXMPTagsListFromPrefix(const QString& pf, KbvExiv::TagsMap& tagsMap) const;
  void      loadSidecarData(Exiv2::Image::AutoPtr xmpsidecar);
  QString   sidecarFilePathForFile(const QString& path);
  bool      saveToXMPSidecar(const QFileInfo& finfo);

//Private  members
  bool      writeRawFiles;
  bool      keepFileTimeStamp = false;
  bool      useXMPSidecar4Reading;
  int       metadataWritingMode;
  bool      loadedFromSidecar;
  QString   filePath, mimeType;
  QSize     pixelSize;

enum MetadataWritingMode
  {
    // Write metadata to image file only.
    WRITETOIMAGEONLY = 0,
    // Write metadata to sidecar file only.
    WRITETOSIDECARONLY = 1,
    // Write metadata to image and sidecar files.
    WRITETOSIDECARANDIMAGE = 2,
    // Write metadata to sidecar file only for read only images such as RAW files for example.
    WRITETOSIDECARONLY4READONLYFILES = 3
  };

// The image color workspace values given by Exif metadata.
enum ImageColorWorkSpace
  {
    WORKSPACE_UNSPECIFIED  = 0,
    WORKSPACE_SRGB         = 1,
    WORKSPACE_ADOBERGB     = 2,
    WORKSPACE_UNCALIBRATED = 65535
  };

//*****************************************************************************
template <class Data, class Key, class KeyString, class KeyStringList = QList<KeyString> >
class MergeHelper
{
public:
  KeyStringList   keys;

  MergeHelper& operator<<(const KeyString& key)
    {
      keys << key;
      return *this;
    }

/* Merge two (Exif,IPTC,Xmp)Data packages, where the result is stored in dest
 * and fields from src take precedence over existing data from dest.
 */
  void mergeAll(const Data& src, Data& dest)
    {
      for (typename Data::const_iterator it = src.begin(); it != src.end(); ++it)
        {
          typename Data::iterator destIt = dest.findKey(Key(it->key()));

          if (destIt == dest.end())
            {
              dest.add(*it);
            }
          else
            {
              *destIt = *it;
            }
        }
    }

/* Merge two (Exif,IPTC,Xmp)Data packages, the result is stored in dest.
 * Only keys in keys are considered for merging.
 * Fields from src take precedence over existing data from dest.
 */
  void mergeFields(const Data& src, Data& dest)
    {
      foreach (const KeyString& keyString, keys)
        {
          Key key(keyString.latin1());
          typename Data::const_iterator it = src.findKey(key);
          if(it == src.end())
            {
              continue;
            }
          typename Data::iterator destIt = dest.findKey(key);
          if(destIt == dest.end())
            {
              dest.add(*it);
            }
          else
            {
              *destIt = *it;
            }
        }
    }

/* Merge two (Exif,IPTC,Xmp)Data packages, the result is stored in dest.
 * The following steps apply only to keys in "keys":
 * The result is determined by src.
 * Keys must exist in src to kept in dest.
 * Fields from src take precedence over existing data from dest.
 */
  void exclusiveMerge(const Data& src, Data& dest)
    {
      foreach (const KeyString& keyString, keys)
        {
          Key key(keyString.latin1());
          typename Data::const_iterator it = src.findKey(key);
          typename Data::iterator destIt = dest.findKey(key);

          if(destIt == dest.end())
            {
              if(it != src.end())
                {
                  dest.add(*it);
                }
            }
          else
            {
              if(it == src.end())
                {
                  dest.erase(destIt);
                }
              else
                {
                  *destIt = *it;
                }
            }
        }
    }
};

class ExifMergeHelper : public MergeHelper<Exiv2::ExifData, Exiv2::ExifKey, QLatin1String>
{
};

class IptcMergeHelper : public MergeHelper<Exiv2::IptcData, Exiv2::IptcKey, QLatin1String>
{
};

class XmpMergeHelper : public MergeHelper<Exiv2::XmpData, Exiv2::XmpKey, QLatin1String>
{
};

#endif // KBVEXIVLOCAL_H
/****************************************************************************/
