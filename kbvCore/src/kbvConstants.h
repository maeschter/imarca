/*****************************************************************************
 * kbv constants
 * Globally used definitions, enums, constants and coefficients
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-06-13
 * Created: 2009.07.23
 *****************************************************************************/
#ifndef KBVCONSTANTS_H_
#define KBVCONSTANTS_H_
#include <QtCore>
#include <QMetaType>

/* Version: major.minor.lower
 *  Major:  changed when minor > 9
 *  Minor:  change design, add new functions or plugins
 *  Lower:  extend existing or add new ancillary functions or behaviour
 *          solve problems and faults within existing major.minor
 */

namespace Kbv
{
// Types, structures *****************************************************
typedef QMap<int, QVariant> kbvItem;    //see qRegisterMetaType in main()
typedef QList<kbvItem*> kbvItemList;
struct  kbvDirWatchItem
 {
   int          row;
   QString      dirName;
   QModelIndex  parent;
 };

// Parameters for batch processing in kbvImageEditor
struct  kbvBatchParam  //parameters for batch processing
 {
    bool            flipHor;
    bool            flipVer;
    bool            rotExif;
    double          rotAngle;
    int             trimTop;
    int             trimLeft;
    int             trimBottom;
    int             trimRight;
    int             resizeHeight;
    int             resizeWidth;
    double          resizePercent;
    QString         saveFormat;
    int             saveParam;
    QString         targetDir;
    bool            overwrite;
 };


// General ***************************************************************
  #define appName               "Imarca"
  #define appVersion            "0.3.43"                  //application version
  #define copyrightInfo         "Copyright 2009-2021  The Imarca Team"
  #define appMainWindow         "kbvMain"                 //application object name
  #define appTempDir            "ImarcaTmp"               //application temp dir
  #define appInstallDir         "/usr/bin"                //application dir
  #define libInstallDir         "/usr/lib/imarca"         //libraries and plugin dirs
  #define helpInstallDir        "/var/lib/imarca"         //help and translation files
  #define configDir             "/.config/imarca"         //configuration files
  #define imageEditorBatchLog   "/IE_batch.log"           //log file of image editor batch process

//  Special***************************************************************
  #define kbvDBusService        "org.kbv.Imarca"          //dbus service
  #define kbvDBusPath           "/org/kbv/deviceMonitor"  //dbus path
  #define kbvDBusInterface      "org.kbv.deviceMonitor"   //dbus interface
  #define kbvHelpNamespace      "org.kbv.core"            //namespace of compressed help for kbvCore
  #define kbvHelpVirtualFolder  "help"                    //virtual folder for all compressed help

  #define treeColourJoinedDB    16                        //colour of joined databases in db tree
  
 // Database: changes on these definitions may corrupt existing databases
  #define dbIdentifier          "SQLite format 3"         //check for sqlite databases
  #define dbMinVer              "1.0"                     //at least required database version
  #define dbNameExt             ".kbvdb"
  #define keywordBoundary       ","

  //Physical and mathematical stuff.
  const double PI  =3.141592653589793238463;


// Misc. *****************************************************************
// Constants
  enum kbviconsizes
    {
      IconList = 24,
      Icon50  = 50,
      Icon75  = 75,
      Icon100 = 100,
      Icon150 = 150,
      Icon200 = 200,
      Icon250 = 250
    };
  //flags
  enum kbvflags
    {
      noflag=0, flag1=1, flag2=2, flag3=4, flag4=8, flag5=16, flag6=32, flag7=64, flag8=128
    };
  //define roles for use in models and views
  enum kbvroles
    {
      FilePathRole = Qt::UserRole,
      FileNameRole = Qt::UserRole+1,
      FileSizeRole = Qt::UserRole+2,
      FileCRCRole = Qt::UserRole+3,
      ImageSizeRole = Qt::UserRole+4,
      ImageDimRole = Qt::UserRole+5,
      PrimaryKeyRole = Qt::UserRole+6,
      FileDateRole = Qt::UserRole+7,
      UserDateRole = Qt::UserRole+8,
      CollectionTypeRole = Qt::UserRole+9,
      CollectionNameRole = Qt::UserRole+10,
      CollectionRootDirRole = Qt::UserRole+11,
      DatabaseLocationRole = Qt::UserRole+12,
      DummyRole = Qt::UserRole+20,
      NoneRole = Qt::UserRole+21
    };
  //types of albums and collections for CollectionTypeRole
  enum kbvcollectiontypes
    {
      TypeNone=0, TypeAlbumAnchor=1, TypeCollectionAnchor=2, TypeAlbumRoot=4, TypeCollectionRoot=8,
      TypeAlbum=16, TypeCollection=32, TypeSearch=64, TypeFile=128, TypeJoined=256
    };
  //types of sorting
  enum kbvsorting
    {
      sortNone, sortFileName, sortFileSize,
      sortFileDate, sortImageSize, sortUserDate
    };
  //types of keywords
  enum kbvkeywords
    {
      keywordNone = 0,
      keywordWordsOnly = 1,
      keywordWordsNumbers = 2,
      keywordFiletype = 64,
      keywordExif = 128
    };
  //image formats and image handling
  enum kbvimageparams
    {
      jpeg=0, tiff, png, xpm, bmp, raw,
      StdLumCoeff54 = 120,     //jpeg std luminance table coefficient 54
      pngGammaMin = 50, pngGammaStd = 220, pngGammaMax = 250, pngGammaScale = 100,
      pngCompressMin = 0, pngCompressMax = 9, pngCompressStd = 5,
      tiffCompressNo = 0, tiffCompressLZW = 1, jpegQualityStd = 90
    };
  //Common constants for database handling, image viewer, slide show and so on.
  //Insert steps and timer for fade in, right and left insertion and
  //speed of change in s
  enum kbvcommon
    {
      none, populate, import, readin, readout, insert, remove, replace,
      rename, check, search, update, settings, dbimport, dbexport, dbexportcd,
      current=1, previous, next, first, last, accurate, inaccurate,
      up=1, down, left, right, forward, backward,
      cancel=1, discard, apply, save, open, close, ok,
      InsertSteps=30, InsertTimer=5,
      logicAND=1, logicOR, logicXOR,
      tempDirPurgeDelay = 60000
    };

} //namespace Kbv

//Own types, for use with QVariant
Q_DECLARE_METATYPE(Kbv::kbvItem)
Q_DECLARE_METATYPE(Kbv::kbvItemList)
Q_DECLARE_METATYPE(Kbv::kbvDirWatchItem)
Q_DECLARE_METATYPE(Kbv::kbvBatchParam)


/*****************************************************************************
 * Options for csvChecker as OR combination
 */
namespace KbvCheck
{
  enum kbvchecker
    {
      resultsize=8,               //number of result fields
      threadMaxId=99,             //max. id number of thread
      threadCreateCSV=997,        //thread ids for special purpose
      threadCopyUpload=998,       //these don't create results
      threadMoveColl=999,
      checkCollection=1,          //check collection due to csv-file
      copyToUploadDir=2,          //copy files to upload directory
      moveToCollection=4,         //move files from download dir to collection
      createCsvFile=8,            //create CSV file
      rename=32,                  //rename files with wrong name
      includePath=64,             //ECSV: check for correct paths too
      correctFilesInReport = 128, //report includes correct files
      ecsvLinux = 256,            //ECSV file in Linux style (UTF8)
      ecsvWindows = 512           //ECSV file in Windows style (ISO 8859-15)
    };
}
/*****************************************************************************
 * Changes on this name space can corrupt the config-file and Imarca
 * crashes at start or when image viewer or slide show get opened!
 */
namespace KbvConf
{
  //main window behaviour, left and right side of splitter, image viewer, slide show
  //icon size in fileView, insert steps and timer for fade in, right and left insertion
  //and speed of change in s
  enum  kbvconfiguration
    {
      MainPosX=100, MainPosY=100,
      MainMimimumHeight=400, MainMimimumWidth=600,
      TabMinimumHeight=300, TabMinimumWidth=240,
      HorStretchLeft=2, HorStretchRight=8,
      DirTreeTab=0, CollTreeTab,
      fileViewTabIndex=0, searchViewTabIndex=1, collectionViewTabIndex=2,
      stdIconSize = 150,
      slideFadeIn=10, slideInsertLeft, slideInsertRight, slideReplace,
      viewerMinHeight = 400, viewerMinWidth = 600
    };
}
#endif /*KBVCONSTANTS_H_*/
/****************************************************************************/
