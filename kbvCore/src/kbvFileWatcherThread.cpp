/*****************************************************************************
 * kvb file watcher thread
 * version 2, UML diagram KbvFileWatcherThread_run.uxf
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-06-29 15:06:39 +0200 (Do, 29. Jun 2017) $
 * $Rev: 1359 $
 * Created: 2009.09.18
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvFileWatcherThread.h"


KbvFileWatcherThread::KbvFileWatcherThread(QObject * parent) : QThread(parent)
{
  abort = false;
}
/*************************************************************************//*!
 * Destructor waits until thread has finished or wakes up the sleeping thread
 * to finish.
 */
KbvFileWatcherThread::~KbvFileWatcherThread()
{
  //qDebug() << "KbvFileWatcherThread::~KbvFileWatcherThread";//###########
  abort = true;
  wait();
}
/*************************************************************************//*!
 * Stores the parameters and starts the thread when the thread is not running
 * and the timer expires.
 */
void    KbvFileWatcherThread::startThread(const QString &rootdir, QList<QPair<QString, qint64> > *modelList, QSize iconsize)
{
  if(!isRunning())
    {
      //qDebug() << "KbvFileWatcherThread::startThread" << rootdir; //###########
      L1 = modelList;
      watchedDir = rootdir;
      iconSize = iconsize;
      QThread::start(QThread::HighPriority);
    }
}
/*************************************************************************//*!
 * Stop the thread when it's running and wait until the thread has aborted.
 */
void    KbvFileWatcherThread::stopThread()
{
  //qDebug() << "KbvFileWatcherThread::stop"; //###########
  if(isRunning())
    {
      abort = true;
      wait();
    }
}
/*************************************************************************//*!
 * Watch directory for changes\n
 * The run method realizes a loop to process all files in the directory and
 * then stops the thread (returns from run).\n
 * List L1 is the model content as reference and list L2 is the actual dir
 * content. Each item in L1 is a pair containing file name and file size in
 * alphabetically sorted order. L2 only contains file names.\n
 * The method compares L1 against L2. The model gets adjusted by removing
 * orphanded and adding new items due to L2. When the size of a found file
 * is different, the model item get created newly.
 * This detects removed and added files as well as modified files.
 */
void    KbvFileWatcherThread::run()
{
  Kbv::kbvItem    *item;
  QFileInfo       fileInfo;
  QString         filename;
  int             i, k;

  //qDebug() << "KbvFileWatcherThread::run" << watchedDir; //###########
  //directory removed while thread is running -> stop immediately
  fileInfo = QFileInfo(watchedDir);
  if(!fileInfo.exists() || abort)
    {
      abort = false;
      return;
    }
  readDirContent(watchedDir, L2);
  
  //handle removed files, L1.size > L2.size
  k = L1->size();
  if(k > L2.size())
    {
      //qDebug() << "KbvFileWatcherThread::run: removed"; //###########
      for(i=0; i<k; i++)
        {
          if(abort)
            {
              break;
            }
          if(!L2.contains(L1->at(i).first))
            {
              //qDebug() << "KbvDirWatcherThread::run orphaned" <<L1->at(i).first; //###########
              emit  invalidItem(i);
            }
        }
      //empty items get removed by the model when the thread finished
    }
  
  //handle added files, L1.size < L2.size
  else if(k < L2.size())
    {
      //qDebug() << "KbvFileWatcherThread::run: added"; //###########
      for(i=0; i<k; i++)
        {
          L2.removeOne(L1->at(i).first);
        }
      for(i=0; i<L2.size(); i++)
        {
          if(abort)
            {
              break;
            }
          //qDebug() << "KbvFileWatcherThread::run: append" <<L2.at(i); //###########
          item = globalFunc.itemFromFile((watchedDir+"/"), L2.at(i), iconSize);
          emit  newItem(item);
        }
    }

  //handle modified and renamed files, L1.size = L2.size
  else
    {
      //qDebug() << "KbvDirWatcherThread::run modified"; //###########
      for(i=0; i<L1->length(); i++)
        {
          if(abort)
            {
              break;
            }
          //L2 contains the filename of L1[i] ?
          filename = L1->at(i).first;
          if (L2.contains(filename))
            {
              //check sizes too since the file could be modified
              fileInfo = QFileInfo(watchedDir+"/"+filename);
              if (L1->at(i).second != fileInfo.size())
                {
                  //qDebug() << "KbvDirWatcherThread::run size different" <<filename; //###########
                  //same name but different size -> update model at i
                  item = globalFunc.itemFromFile((watchedDir+"/"), filename, iconSize);
                  emit  replaceItem(i, item);
                }
              //item found in L2 -> remove from list
              L2.removeOne(filename);
            }
          else
            {
              //qDebug() << "KbvDirWatcherThread::run orphaned" <<filename; //###########
              //item not found in L2 -> remove renamed item data from model
              emit  invalidItem(i);
            }
        } //end of loop
      //L2 contains renamed only
      for(k=0; k<L2.size(); k++)
        {
          item = globalFunc.itemFromFile((watchedDir+"/"), L2.at(k), iconSize);
          emit  newItem(item);
        }
      //empty items get removed by the model when the thread finished
    }
  //qDebug() << "KbvFileWatcherThread::run end"; //###########
  abort = false;
  emit  threadFinished();
}
/*************************************************************************//*!
 * Read all sorted file names of current directory into a list strings.
 * No support for hidden image files since the reader does not read them.
 */
void    KbvFileWatcherThread::readDirContent(QString dirpath, QStringList &L2)
{
  QDir  dir;

  dir.setPath(dirpath);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  dir.setSorting(QDir::Name | QDir::LocaleAware);
  
  L2.clear();
  L2 = dir.entryList();
  //qDebug() << "KbvFileWatcherThread::readDirContent" <<L2; //###########
}
/*************************************************************************//*!
 * Get the number of files in current directory.
 */
int    KbvFileWatcherThread::getFileCountInDir(QString dirpath)
{
  QDir  dir;
  
  dir.setPath(dirpath);
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
  return dir.entryList().count();
}
/*****************************************************************************/
