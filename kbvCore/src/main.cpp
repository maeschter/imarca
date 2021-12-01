/****************************************************************************
 * kvb main.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2008.11.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QApplication>
#include <QtPlugin>
#include "kbvMain.h"
#include "kbvConstants.h"

int main(int argc, char *argv[])
{
  //register types und structures. needs "kbvConstants.h"
  qRegisterMetaType<Kbv::kbvItem>("kbvItem");
  qRegisterMetaType<Kbv::kbvItemList>("kbvItemList");
  qRegisterMetaType<Kbv::kbvDirWatchItem>("kbvDirWatchItem");
  qRegisterMetaType<Kbv::kbvDirWatchItem>("Kbv::kbvBatchParam");

  QTranslator qtTranslator, coreTranslator, metadataTranslator, imageEditTranslator;
  QString     appdir, file;
  QDir        pluginsDir;

  Q_INIT_RESOURCE(kbv);

  QApplication imarca(argc, argv);
  imarca.setApplicationName(QString(appName));

  //install Qt translator
  qtTranslator.load("qt_" + QLocale::system().name(),
  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  imarca.installTranslator(&qtTranslator);
  
  //Load translation files (DEBIAN file system).
  //The installed app and static plugins load from helpInstallDir
  //The test version loads from application dir (~workspace/test).
  appdir = QCoreApplication::applicationDirPath();
  if(appdir.startsWith(QString(appInstallDir)))
    {
      //installed app, plugin and help install dir
      pluginsDir.setPath(QString(helpInstallDir));
    }
  else
    {
      //app in development environment /kbv/prd, search in /test
      pluginsDir.setPath(QString(appdir));
      pluginsDir.cdUp();
      pluginsDir.cdUp();
      pluginsDir.cd("test");
    }
   appdir = pluginsDir.absolutePath();

  //The translator tries in this order:
  //translation.xx_XX.qm, translation.xx.qm, translation.qm
  //Plugin translations get loaded in the initialise-function of plugin
  file = QString("kbvCore." + QLocale::system().name() + ".qm");
  if(coreTranslator.load(file, appdir))
    {
      imarca.installTranslator(&coreTranslator);
      //qDebug() << "KbvCore translator" <<appdir <<file; //###########
    }
  file = QString("kbvMetadata." + QLocale::system().name() + ".qm");
  if(metadataTranslator.load(file, appdir))
    {
      imarca.installTranslator(&metadataTranslator);
      //qDebug() << "KbvMetadata translator" <<appdir <<file; //###########
    }
  file = QString("kbvImageEditor." + QLocale::system().name() + ".qm");
  if(imageEditTranslator.load(file, appdir))
    {
      imarca.installTranslator(&imageEditTranslator);
      //qDebug() << "KbvImageEditor translator" <<appdir <<file; //###########
    }

  KbvMain mainWin;
  mainWin.show();
  mainWin.cmdLineArgs();
  if(argc>=2) {qDebug() << "main" <<argc <<argv[0] <<argv[1];} //###########

  return imarca.exec();
}
/****************************************************************************/
