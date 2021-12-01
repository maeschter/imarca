/*****************************************************************************
 * kbv global
 * Globally used methods and information
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-05-19
 * Created: 2009.07.23
 *****************************************************************************/
#ifndef KBVGLOBAL_H_
#define KBVGLOBAL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSql>

#include <png.h>          //libpng.so
#include <jpeglib.h>      //libjpeg.so
#include <kbvExiv.h>      //libKbvExiv.so, needs libexiv2.so
#include <opencv2/core.hpp>
#include "kbvConstants.h"

void    JPEGVersionError(j_common_ptr cinfo);

class KbvGlobal : public QObject
{
  Q_OBJECT

public:
          KbvGlobal();
virtual   ~KbvGlobal();

public:
void            displayImages(QListView *view, QAbstractListModel *model, QSortFilterProxyModel *sortModel);
//void            displaySlideShow(QListView *view, QAbstractListModel *model, QSortFilterProxyModel *sortModel);
QStringList     dropExtractMimeData(const QMimeData *mime);
QVariant        displayRoleData(const Kbv::kbvItem *item, int viewMode) const;
QString         tooltip(const Kbv::kbvItem *item) const;
Kbv::kbvItem*   itemFromFile(const QString path, const QString name, const QSize iconsize);
Kbv::kbvItem*   itemFromRecord(const QSqlQuery &query, const QString &rootDir);
bool            createRecordData(QString path, QString name, QMap<QString, QVariant> &data, int iconSize, int keyWordType);
QString         extractKeywords(QString filename, int mode);
void            getLibraryInfos(QStringList &info);
quint32         crc32FromFile(QString pathName);

/*
QIcon   iconKbvCrown, iconAppExit;
QIcon   iconKbvFolderTree, iconKbvAlbums, iconKbvFind;
QIcon   iconKbvNoSupport, iconUserHome, iconFolder;
QIcon   iconDriveHarddisk, iconCameraPhoto, iconCameraVideo, iconDriveOptical, iconMediaFlash, iconMediaOptical;

QIcon   iconFileRename;
QIcon   iconEditPref, iconViewRefresh;
QIcon   iconDBOpen, iconDBImport, iconDBExport;
QIcon   iconHome, iconGoBack, iconGoForward;
QIcon   iconHelpContent, iconHelpAbout;
QIcon   iconSortDesc, iconSortAsc, iconViewModeIcon, iconViewModeList, iconCircle;
//QIcon   iconSquare, iconTriangleLeft, iconTriangleRight, iconSortFirstLast, iconSortLastFirst;

QIcon   iconPhotoAlbum, iconPhotoCollection, iconSearchResult, iconEye;
QIcon   iconAttention;

QPixmap pixMimeImage, pixMimeText;

QDir    tempDir;
*/
private:
void    crc32BuildTab();

quint32 crcTable[256];
};

#endif /*KBVGLOBAL_H_*/
/****************************************************************************/
