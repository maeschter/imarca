/*****************************************************************************
 * kvb general
 * Generally used methods to access system functions
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2018.08.28
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvGeneral.h"

extern  KbvSetvalues            *settings;

KbvGeneral::KbvGeneral() : QObject()
{
  establishTrashDir();
  establishTempDir();
  establishIcons();
  
  tempDirPurgeTimer = new QTimer(this);
  tempDirPurgeTimer->setSingleShot(true);
  tempDirPurgeTimer->setInterval(Kbv::tempDirPurgeDelay);
  connect(tempDirPurgeTimer, SIGNAL(timeout()), this, SLOT(purgeTempDir()));
}

KbvGeneral::~KbvGeneral()
{
  //qDebug() << "KbvGeneral::~KbvGeneral"; //###########
  purgeTempDir();
}
/* Public functions *********************************************************/
/*************************************************************************//*!
 * Find the application for the given mime type and open it in an own process.
 * The parameter string 'params' contains the command line parameters.
 */
void    KbvGeneral::openApplication(const QString mimetype, const QStringList params)
{
  QString       prog;
  QProcess      *app;

  prog = this->getMimeApplication(mimetype);

  if(!prog.isEmpty())
    {
      app = new QProcess(this);
      app->start(prog, params);
      //qDebug() << "KbvGeneral::openApplication" <<prog <<params; //###########
    }
}/*************************************************************************//*!
 * Return file for locale in given dir and with given extension.
 * QLocale delivers "language_country", so we try name.ll_CC and name.ll
 * Return empty string if nothing found
 */
QString   KbvGeneral::getFileForLocale(const QString dir, const QString name, const QString locale, const QString extension)
{
  QFileInfo     info;
  QString       file;

  //1. try name.xx_YY
  file = name + "." + locale + extension;
  //qDebug() << "KbvGeneral::getFileForLocale" <<file; //###########
  info.setFile(dir + file);
  if(!info.exists())
    {
      //2. try name.xx
      file = name + "." + locale.left(2) + extension;
      //qDebug() << "KbvGeneral::getFileForLocale" <<file; //###########
      info.setFile(dir + file);
      if(!info.exists())
        {
          return QString();
        }
    }
  return file;
}
/*************************************************************************//*!
 * Get themed icon from one of the names in "names". When no matching icon
 * was found a null icon is returned.
 * Note: The dirs in themeSearchPaths() must contain an index.theme file
 * describing it's contents. We don't provide this in our icon resource dir
 * so setting an own path to this dir will not work.
 */
QIcon    KbvGeneral::getThemeIcon(const QString names)
{
  QIcon         icon = QIcon();
  QStringList   nameList;
  
  //qDebug() << "KbvGeneral::getThemedIcon theme name" <<QIcon::themeName(); //###########

  nameList = names.split(" ", QString::SkipEmptyParts);
  for(int n=0; n<nameList.length(); n++)
    {
      //qDebug() << "KbvGeneral::getThemedIcon has icon" <<nameList.at(n) <<QIcon::hasThemeIcon(nameList.at(n)); //###########
      if(QIcon::hasThemeIcon(nameList.at(n)))
        {
          icon = QIcon::fromTheme(nameList.at(n));
          break;
        }
    }
  //Use own icon when no theme icon was found
  if(icon.isNull())
    {
      if(names.contains("drive-harddisk"))
        {
          return iconDriveHarddisk;
        }
      else if(names.contains("drive-removable"))
        {
          return iconDriveHarddisk;
        }
      else if(names.contains("camera-photo"))
        {
          return iconCameraPhoto;
        }
      else if(names.contains("camera-video"))
        {
          return iconCameraVideo;
        }
      else if(names.contains("drive-optical"))
        {
          return iconDriveOptical;
        }
      else if(names.contains("media-flash"))
        {
          return iconMediaFlash;
        }
      else if(names.contains("media-optical"))
        {
          return iconMediaOptical;
        }
      else
        {
          return iconDriveHarddisk;
        }
    }
  return  icon;
}
/*************************************************************************//*!
 * Find or establish temporary directory
 */
void    KbvGeneral::establishTempDir()
{
  QString temp = QDir::tempPath() + QString("/") + QString(appTempDir);
  tempDir.setPath(temp);
  
  if(!tempDir.exists())
    {
      tempDir.mkdir(temp);
    }
}
/*************************************************************************//*!
 * Get trash directory or return empty string if not existing.
 */
QString KbvGeneral::getTrashDir()
{
  return  gblTrashDir;
}
/*************************************************************************//*!
 * Return TRUE when accessible trash directory exists.
 */
bool    KbvGeneral::validTrashDir(void)
{
  return  trashvalid;
}

/* Private functions ********************************************************/
/*************************************************************************//*!
 * Find the application which should open a file associated with a mime type.
 * The algorithm follows the recommandations of freedesktop.org "Association
 * between MIME types and applications".
 * Considered are the environment variables:
 * $XDG_CONFIG_HOME/$desktop-mimeapps.list  user overrides, desktop-specific
 * $XDG_CONFIG_HOME/mimeapps.list           user overrides (recommended location for user configuration GUIs)
 * $XDG_CONFIG_DIRS/$desktop-mimeapps.list  sysadmin and ISV overrides, desktop-specific
 * $XDG_CONFIG_DIRS/mimeapps.list           sysadmin and ISV overrides
 * $XDG_DATA_DIRS/applications/$desktop-mimeapps.list   distribution-provided defaults, desktop-specific
 * $XDG_DATA_DIRS/applications/mimeapps.list            distribution-provided defaults
 * The *.list files of a directory are examined and the first found application
 * for matching the mime type get used.
 */
QString KbvGeneral::getMimeApplication(const QString mimetype)
{
  QString       app, env;
  QStringList   strList;


  app.clear();
  env = QString(qgetenv("XDG_CONFIG_HOME"));
  //qDebug() << "KbvGeneral::getMimeApplication $XDG_CONFIG_HOME" <<env; //###########
  if(!env.isEmpty())
    {
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "");
    }

  env = QString(qgetenv("XDG_CONFIG_DIRS"));
  //qDebug() << "KbvGeneral::getMimeApplication $XDG_CONFIG_DIRS" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    {
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "");
    }

  env = QString(qgetenv("XDG_DATA_HOME"));
  //qDebug() << "KbvGeneral::getMimeApplication $XDG_DATA_HOME" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    {
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "applications/");
    }

  env = QString(qgetenv("XDG_DATA_DIRS"));
  //qDebug() << "KbvGeneral::getMimeApplication $XDG_DATA_DIRS" <<env; //###########
  if(!env.isEmpty() && app.isEmpty())
    {
      strList = env.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
      app = mimeAssociation(strList, mimetype, "applications/");
    }

  return app;
}
/*************************************************************************//*!
 * Helper function: Search the application associated to a mime type.
 * Evaluate the content of an environment variable given in parameter
 * 'environment'. Return an empty string when no app could be found.
 */
QString    KbvGeneral::mimeAssociation(const QStringList dirList, const QString mimetype, const QString subPath)
{
  QDir          xdgDir;
  QDir::Filters filter;
  QFile         associations;
  QString       path, line, appDesktop;
  QStringList   fileList, nameFilter;
  QTextStream   stream;
  int           i, k, n;
  bool          found=false;

  filter = QDir::Files;
  nameFilter << "*.list";

  //qDebug() << "KbvGeneral::mimeApplication" <<dirList <<mimetype <<subPath; //###########
  //Outer loop: check all dirs for presence of a *.list file
  appDesktop.clear();
  i = 0;
  while((i < dirList.size()) && !found)
    {
      path = dirList.at(i);
      if(!path.endsWith("/"))
        {
          path.append("/");
        }
      path.append(subPath);
      xdgDir.setPath(path);
      //qDebug() << "KbvGeneral::mimeApplication path" <<path; //###########
      fileList = xdgDir.entryList(nameFilter, filter, QDir::NoSort);
      //Inner loop: check all found files for mime type
      k = 0;
      while((k < fileList.size()) && !found)
        {
          //qDebug() << "KbvGeneral::mimeApplication file" <<fileList.at(k); //###########
          associations.setFileName(path + fileList.at(k));
          if(associations.open(QIODevice::ReadOnly | QIODevice::Text))
            {
              stream.setDevice(&associations);
              while(!stream.atEnd())
                {
                  line = stream.readLine();
                  if(line.startsWith(mimetype))
                    {
                      found = true;
                      n = line.indexOf("=", 0, Qt::CaseInsensitive);
                      appDesktop = line.remove(0, n+1);
                      break;      //break loop through the file
                    }
                }
              associations.close();
            }
          k++;
        }
      i++;
   }
  //Find executable in desktop file (app.desktop)
  //Search line: Exec=/usr/bin/app or Exec=app
  line.clear();
  if(found)
    {
      associations.setFileName("/usr/share/applications/" + appDesktop);
      if(associations.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           stream.setDevice(&associations);
           while (!stream.atEnd())
             {
                line = stream.readLine();
                if(line.startsWith("Exec="))
                  {
                     //qDebug() << "KbvGeneral::mimeApplication application found" <<line; //###########
                     n = line.indexOf("=", 0, Qt::CaseInsensitive);
                     line.remove(0, n+1);
                     n = line.indexOf(" ", 0, Qt::CaseInsensitive);
                     if(n > 0)
                       {
                         line = line.left(n);
                       }
                     //qDebug() << "KbvGeneral::mimeApplication application" <<line; //###########
                     break;      //break loop through file
                  }
              }
            associations.close();
         }
     }
  return line;
}
/*************************************************************************//*!
 * Check if subdir is already existing or create it.
 * Return TRUE if dir exists or is created with owner rights w-r-x
 */
bool    KbvGeneral::checkCreateSubdDir(QString subdir)
{
  QDir          dir;
  QFile         file;
  QString       str;

  str = gblTrashDir + subdir;
  if (dir.exists(str))
    {
      if (!file.setPermissions(str, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner))
        {
          return false;
        }
    }
  else
    {
      if (!dir.mkdir(str))
        {
          return false;
        }
      if (!file.setPermissions(str, QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner))
        {
          return false;
        }
    }
  return  true;
}
/*************************************************************************//*!
 * Read all necessary icons.
 */
void    KbvGeneral::establishIcons(void)
{
//mainWindow, menu, toolbars
  iconKbvCrown = QIcon(":/kbv/icons/imarca.png");
  iconKbvFolderTree = QIcon(":/kbv/icons/kbv_folder_tree_22x22.png");
  iconKbvAlbums = QIcon(":/kbv/icons/kbv_alben_22x32.png");
  iconKbvFind = QIcon(":/kbv/icons/kbv_find_24x24.png");
  iconFileRename = QIcon(":/kbv/icons/file-rename.png");
  iconEditPref = QIcon(":/kbv/icons/app-system.png");
  iconViewRefresh = QIcon(":/kbv/icons/view-refresh.png");
  iconAppExit = QIcon(":/kbv/icons/app-exit.png");
  iconHelpContent = QIcon(":/kbv/icons/help-contents.png");
  iconHelpAbout = QIcon(":/kbv/icons/help-about.png");
  iconDBOpen = QIcon(":/kbv/icons/database-add.png");
  iconDBImport = QIcon(":/kbv/icons/database-import.png");
  iconDBExport = QIcon(":/kbv/icons/database-export.png");
//mainWindow, tree, tabs
  iconPhotoAlbum = QIcon(":/kbv/icons/di.png");
  iconPhotoCollection = QIcon(":/kbv/icons/bookcase.png");
  iconSearchResult = QIcon(":/kbv/icons/folder-image.png");
  iconEye = QIcon(":/kbv/icons/eye.png");
  iconAttention = QIcon(":/kbv/icons/attention.png");

//dirModel
  iconKbvNoSupport = QIcon(":/kbv/icons/kbv_no_support.png");
  iconUserHome = QIcon(":/kbv/icons/user-home.png");
  iconFolder = QIcon(":/kbv/icons/folder.png");
  iconDriveHarddisk = QIcon(":/kbv/icons/drive-harddisk.png");
  iconDriveOptical = QIcon(":/kbv/icons/drive-optical.png");
  iconCameraPhoto = QIcon(":/kbv/icons/camera-photo.png");
  iconCameraVideo = QIcon(":/kbv/icons/camera-video.png");
  iconMediaFlash = QIcon(":/kbv/icons/media-flash.png");
  iconMediaOptical = QIcon(":/kbv/icons/media-optical.png");

//collectionTabs
  iconSortDesc = QIcon(":/kbv/icons/kbv_triangle-down.png");
  iconSortAsc = QIcon(":/kbv/icons/kbv_triangle-up.png");
  iconViewModeIcon = QIcon(":/kbv/icons/kbv_iconview.png");
  iconViewModeList = QIcon(":/kbv/icons/kbv_listview.png");
  iconCircle = QIcon(":/kbv/icons/kbv_circle.png");

//help
  iconHome = QIcon(":/kbv/icons/home.png");
  iconGoBack = QIcon(":/kbv/icons/arrow_back.png");
  iconGoForward = QIcon(":/kbv/icons/arrow_forward.png");

  //iconSortFirstLast = QIcon(":/kbv/icons/sort_first-last.png");
  //iconSortLastFirst = QIcon(":/kbv/icons/sort_last-first.png");

  pixMimeImage = QPixmap(":/kbv/icons/image-x-generic.png");
  pixMimeText = QPixmap(":/kbv/icons/text-x-generic.png");
}
/*************************************************************************//*!
 * Search or establish trash directory:\n
 * Due to XDG Base Dir Spec version 0.6 (http://standards.freedesktop.org/)
 * $XDG_DATA_HOME defines the base directory relative to which user specific
 * data files should be stored. If $XDG_DATA_HOME is either not set or empty,
 * a default equal to $HOME/.local/share is attempt to use.
 * gblTrashDir holds the string $HOME/.local/share/Trash when a trash dir was
 * found or has been established. Otherwise gblTrashDir holds an empty string.
 */
void    KbvGeneral::establishTrashDir()
{
  const char    *xdghome;
  QDir          dir;
  QString       xdgDataHome;


  xdghome = getenv("XDG_DATA_HOME");
  if (xdghome != NULL)
    {
      xdgDataHome = QString(xdghome);
      gblTrashDir = xdgDataHome + "/.local/share/Trash";
    }
  else
    {
      gblTrashDir = dir.homePath() + "/.local/share/Trash";
    }
  //qDebug() << "KbvGeneral::establishTrashDir $XDG_Data_HOME"<<xdgDataHome; //###########
  //check if dir exists
  trashvalid = true;
  if (!dir.exists(gblTrashDir))
    {
      //try to create one
      if (!dir.mkdir(gblTrashDir))
        {
          gblTrashDir = "";
          trashvalid = false;
          //qDebug() << "KbvGeneral::establishTrashDir: can't find or create trash dir"; //###########
          return;
        }
    }
  //now we have an existing dir -> look for /info, /files and sufficient rights
  if (!checkCreateSubdDir("/info"))
    {
      gblTrashDir = "";
      trashvalid = false;
      //qDebug() << "KbvGeneral::establishTrashDir: can't find or create trashdir/info"; //###########
    }
  if (!checkCreateSubdDir("/files"))
    {
      gblTrashDir = "";
      trashvalid = false;
      //qDebug() << "KbvGeneral::establishTrashDir: can't find or create trashdir/files"; //###########
    }
}
/*************************************************************************//*!
 * Returns TRUE when the file is moved to dustbin.
 */
bool    KbvGeneral::moveToTrash(QString path, QString name)
{
  QString       trashinfo, trashfiles, info;
  QFile         file;
  QByteArray    data;
  QDateTime     deltime;

  trashinfo = gblTrashDir + "/info/";
  trashfiles = gblTrashDir + "/files/";

  //only on access to dustbin
  if(file.copy(path+name, trashfiles+name))
    {
      file.setFileName(path+name);
      if (file.remove())
        {
        //add infofile to dustbin
          deltime = QDateTime::currentDateTime();
          data = "[Trash Info]\nPath=" + path.toUtf8() + name.toUtf8();
          data += "\nDeletionDate=" + deltime.toString("yyyy-MM-ddThh:mm:ss").toUtf8();
          data += "\n";

          file.setFileName(trashinfo + name + ".trashinfo");
          file.open(QIODevice::WriteOnly|QIODevice::Truncate);
          file.write(data, qstrlen(data));
          file.close();
          return true;
        }
      else
        {
          //not deleted, remove file from dustbin
          file.setFileName(trashfiles+name);
          file.remove();
        }
    }
  return false;
}
/*************************************************************************//*!
 * Purge temporary directory when the retriggerable delay timer expires
 */
void    KbvGeneral::purgeTempDir()
{
  QStringList templist;

  templist = tempDir.entryList(QDir::Files, QDir::NoSort);
  for(int i=0; i<templist.size(); i++)
    {
      tempDir.remove(templist.at(i));
    }
}
/*************************************************************************//*!
 * Start timer for delayed purge of temporary directory
 */
void    KbvGeneral::tempDirDelayedCleanUp()
{
  tempDirPurgeTimer->start();
}
/****************************************************************************/
