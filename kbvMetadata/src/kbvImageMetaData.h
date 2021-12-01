/*****************************************************************************
 * kvb image meta data
 * This is the common header for all functions of meta data gui.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2015.04.27
 * Exiv2: http://www.exiv2.org
 * Exif : http://www.exif.org/Exif2-2.PDF
 * Iptc : http://www.iptc.org/std/IIM IIMV4.2.pdf
 *        http://www.iptc.org/std/Iptc4xmpCore/ Iptc4xmpCore_1.0-spec-XMPSchema_8.pdf
 * Xmp  : http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
 * guide: http://www.metadataworkinggroup.com/pdf/mwg_guidance.pdf
 *****************************************************************************/
#ifndef KBVIMAGEMETADATA_H
#define KBVIMAGEMETADATA_H
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <kbvExiv.h>            //libKbvExiv.so
#include "kbvCountrySelector.h"
#include "ui_kbvmetadatafile.h"
#include "ui_kbvmetadatabuttons.h"
#include "ui_kbviptctemplatebuttons.h"
#include "ui_kbviptcdata.h"
#include "ui_kbvexifdata.h"
#include "ui_kbvxmpdata.h"

class Ui_kbvMetaDataFileClass;
class Ui_kbvMetaDataButtonClass;
class Ui_kbvIptcTemplateButtonClass;
class Ui_kbvIptcDataClass;
class Ui_kbvExifDataClass;
class Ui_kbvXmpDataClass;


class KbvImageMetaData : public QDialog
{
  Q_OBJECT

public:
  explicit KbvImageMetaData(QWidget *parent = 0);
  virtual  ~KbvImageMetaData();

  QString getIptcInfo(const QString &imagePath);
  QString getExifInfo(const QString &imagePath);

public slots:
  //void  openMetaDataGui(QListView *ptr, int view);
  void  openMetaDataGui(const QList<QPair<QString, QString> > &files);
  void  openIptcTemplate();
  void  setExifSoftware(const QString text);

private slots:
  void  metaDataRead(bool checked);
  void  metaDataReadNext(bool checked);
  void  metaDataReadPrevious(bool checked);
  void  metaDataWrite(bool checked);
  void  closeMetaDataGui(bool checked);
//KbvImageMetaIptc ------------------------------------------
  void  iptcDate(bool checked);
  void  iptcCountry(int index);
  void  iptcGenre(bool checked);
  void  iptcScene(bool checked);
  void  iptcSubject(bool checked);
  void  iptcCopyrrightSign(bool checked);
  void  iptcClear(bool checked);
  void  iptcSetFromTemplate(bool checked);
  void  iptcTemplateOpen(bool checked);
  void  iptcTemplateSave(bool checked);
  void  iptcTemplateSaveAs(bool checked);
  void  iptcTemplateActivate(bool checked);
  void  iptcDataChanged(int row=-1, int column=-1);
//KbvImageMetaExif ------------------------------------------
  void  exifClear(bool checked);
  void  gpsGoogleMaps(bool checked);
  void  gpsOpenStreetMap(bool checked);
//KbvImageMetaXMP -------------------------------------------
  void  xmpClear(bool checked);
  void  xmpGoogleMaps(bool checked);
  void  xmpOpenStreetMap(bool checked);
  void  xmpDataChanged(int row=-1, int column=-1);

private:
  void  populateGui();
  bool  metaDataFromFile(const QString &filePath);
//KbvImageMetaIptc ------------------------------------------
  void  iptcPresetTable();
  void  iptcClearGui();
  void  iptcGuiToIptcData();
  bool  iptcDataToGui();
  void  iptcReadTemplate(QIODevice *device);
  void  iptcWriteTemplate(QIODevice *device);
//KbvImageMetaExif ------------------------------------------
  void  exifClearGui();
  bool  exifDataToGui();
  void  exifGuiToExifData();
  bool  gpsDataToGui();
//KbvImageMetaXMP -------------------------------------------
  void  xmpClearGui();
  bool  xmpDataToGui();
  void  xmpGuiToXmpData();

  void  commentToGui();
  void  commentGuiToData();
  void  openApplication(const QString mimetype, const QStringList params);
  QString getMimeApplication(const QString mimetype);
  QString mimeAssociation(const QStringList dirList, const QString mimetype, const QString subPath);
  

  Ui::kbvMetaDataFileClass        uiMetaDataFile;
  Ui::kbvMetaDataButtonClass      uiMetaDataButt;
  Ui::kbvIptcDataClass            uiIptcData;
  Ui::kbvExifDataClass            uiExifData;
  Ui::kbvXmpDataClass             uiXmpData;
  Ui::kbvIptcTemplateButtonClass  uiTemplateButt;

  KbvCountrySelector    *iptcCountryBox;
  QVBoxLayout           *metaDataVLayout;
  QTabWidget            *metaDataTab;
  QFrame                *metaDataFile;
  QWidget               *metaDataButt, *iptcTemplateButt;
  QWidget               *iptcDataTab, *exifDataTab, *xmpDataTab;
  QFile                 *xmlFile;
  QXmlStreamWriter      xmlwriter;
  QXmlStreamReader      xmlreader;
  QString               currentFile, templateFile, templateActive, errorMsg, helpMsg;
  int                   currentIndex,viewType;
  QString               encoding, iptcSceneCode, rightsUsageTerms;
  KbvExiv               metaData;   //libKbvExiv.so
  QList<QPair<QString, QString> >  fileList;
  double                altitude, latitude, longitude;
  bool                  iptcChanged, xmpChanged;
};

/*****************************************************************************
 * Metadata constants
 */
namespace KbvMeta
{
//iptc tag table.
enum iptctable
{
  Byline = 0,     //Creator
  BylineTitle,    //Creator's Jobtitle
  DateTime,       //Date / Time created
  Genre,          //Intellectual genre
  Scene,          //Scene Code
  Sublocation,    //
  City,           //
  State,          //Province/State
  Country,        //
  CountryCode,    //
  Headline,       //
  Keywords,       //
  Subject,        //Subject Code
  Writer,         //Caption/Description Writer
  ObjectName,     //Title
  JobId,          //Job Identifier
  Credit,         //Credit Line
  Source,         //
  Copyright,      //Copyright Notice
  Rights,         //Rights Usage Terms
};
enum  tagsColumns
{
  tagCol = 0,     //tag id column
  valueCol = 1    //tag value column
};

}  //namespace KbvMeta
#endif // KBVIMAGEMETADATA_H
/****************************************************************************/
