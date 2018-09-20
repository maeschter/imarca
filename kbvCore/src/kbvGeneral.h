/*****************************************************************************
 * kbv global
 * Globally used methods and information
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.07.23
 *****************************************************************************/
#ifndef KBVGENERAL_H_
#define KBVGENERAL_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvConstants.h"

class KbvGeneral : public QObject
{
  Q_OBJECT

public:
          KbvGeneral();
virtual   ~KbvGeneral();

public:
  void    openApplication(const QString mimetype, const QStringList params);
  QString getFileForLocale(const QString dir, const QString name, const QString locale, const QString extension);
  QIcon   getThemeIcon(const QString names);
  bool    moveToTrash(QString path, QString name);
  bool    validTrashDir(void);
  QString getTrashDir(void);

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

private slots:
  void    purgeTempDir();

private:
  QString getMimeApplication(const QString mimetype);
  QString mimeAssociation(const QStringList dirList, const QString mimetype, const QString subPath);
  void    establishIcons(void);
  void    establishTrashDir();
  void    establishTempDir();
  void    tempDirDelayedCleanUp();
  bool    checkCreateSubdDir(QString subdir);

  QString gblTrashDir;
  bool    trashvalid;
  QTimer  *tempDirPurgeTimer;
};

#endif /*KBVGENERAL_H_*/
/****************************************************************************/
