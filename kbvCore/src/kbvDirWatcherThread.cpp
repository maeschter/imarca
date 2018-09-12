/*****************************************************************************
 * kvb dir watcher thread
 * $LastChangedDate: 2017-05-27 13:53:15 +0200 (Sa, 27. Mai 2017) $
 * $Rev: 1346 $
 * Created: 2011.01.03
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#include <QtDebug>
#include "kbvDirWatcherThread.h"

KbvDirWatcherThread::KbvDirWatcherThread(QObject * parent) : QThread(parent)
{
  abort = false;

  //Note: the timer lives inside main thread, not in this one. So we must
  //retrigger the timer in main thread when the watchTread emits the
  // signal finished().
  watchTimer = new QTimer(this);
  watchTimer->setSingleShot(true);
  watchTimer->setInterval(2000);

  connect(watchTimer, SIGNAL(timeout()),  this, SLOT(startByTimer()));
  connect(this,       SIGNAL(finished()), this, SLOT(startTimer()));

}
/************************************************************************//*!
 * Destructor waits until thread has finished or wakes up the sleeping thread
 * to finish.
 */
KbvDirWatcherThread::~KbvDirWatcherThread()
{
  //qDebug() << "KbvDirWatcherThread::~KbvDirWatcherThread";//###########
  abort = true;
  wait();
}
/************************************************************************//*!
 * Starts the watch timer when the thread is not running. The expiring timer
 * starts the thread. List L1 is the reference and created here. List L2 will
 * be created by the thread reading the directory content.
 */
void    KbvDirWatcherThread::startThread(const QString dir, const QModelIndex index, int cycleTime)
{
  if(!isRunning())
    {
      watchedDir = dir + "/";
      watchedIndex = index;
      watchCycle = cycleTime;
      readDirContent(L1, watchedDir);
      watchTimer->start(watchCycle);
      qDebug() << "KbvDirWatcherThread::start" <<watchedDir; //###########
    }
}
/************************************************************************//*!
 * First stop timer then stop the thread when it's running and wait until the
 * thread has stopped.
 */
void    KbvDirWatcherThread::stopThread()
{
  watchTimer->stop();
  if(isRunning())
    {
      abort = true;
      wait();
    }
}
/************************************************************************//*!
 * Slot: Retriggers the timer.
 */
void    KbvDirWatcherThread::startTimer()
{
  this->watchTimer->start(watchCycle);;
}
/************************************************************************//*!
 * Slot: Starts the thread when the timer expires. As the timer is started at
 * the end of run() this function never meets a running thread.
 */
void    KbvDirWatcherThread::startByTimer()
{
  start(NormalPriority);
}
/************************************************************************//*!
 * Watch directory for content changes
 * The run method realizes a loop to process all directories in watchedDir.
 * List L1 is the reference and list L2 is the actual content of parent.
 * Both lists are string lists containing all subdir names of watchedDir.
 * The method compares L2 against L1 and adjusts the model due to L2.
 * At the end the model is identical to L2 so this list is copied to L1 as
 * reference for next run.
 */
void    KbvDirWatcherThread::run()
  {
    QFileInfo   fileInfo;
    int         i, k, km, n, m;

    readDirContent(L2, watchedDir);

    n = L1.length();
    m = L2.length();
    i = 0;
    k = 0;
    km = 0;

    //qDebug() << "KbvDirWatcherThread::run watchedDir dirs in L1 L2" << watchedDir <<n <<m; //###########
    //Break, when number of subdirs is constant
    if(n == m)
      {
        return;
      }
    //qDebug() << "KbvDirWatcherThread::run watchedDir" << watchedDir; //###########

    //outer loop: step through L1
    while (i < n)
      {
        //directory removed while thread is running -> stop immediately
        fileInfo = QFileInfo(watchedDir);
        if(!fileInfo.exists())
          {
            L1.clear();
            L2.clear();
            abort = false;
            return;
          }
        //when directory names are identical process next item
        if (L1.value(i) == L2.value(k))
          {
            i++;
            k++;
          }
        else
          {
            //item i not found at this position in L2 ->
            //save index and step further through L2 for later occurrence
            km = k;
            //inner loop: check L2 for occurrence of item i
            while (true)
              {
                k++;
                //item i not found in L2 -> remove from model and search item i+1
                //from position in L2 where i wasn't found
                if (k >= m)
                  {
                    //qDebug() << "KbvDirWatcherThread::run: row to remove at" << km; //###########
                    watchedItem.row = km;
                    watchedItem.dirName = "";
                    watchedItem.parent = watchedIndex;
                    emit  removeRow(watchedItem);
                    i++;
                    k = km;
                    break;
                  }
                //item i found in L2 at later position ->
                //insert k-km new items of L2 into model starting at km
                if (L1.value(i) == L2.value(k))
                  {
                    //qDebug() << "KbvDirWatcherThread::run: rows to insert" << k-km << "at" << km; //###########
                    for (int v=km; v<k; v++)
                      {
                        watchedItem.row = v;
                        watchedItem.dirName = watchedDir + L2.value(v);
                        watchedItem.parent = watchedIndex;
                        emit  insertRow(watchedItem);
                        //qDebug() << "KbvDirWatcherThread::run: insert" <<watchedItem.dirName; //###########
                      }
                    i++;
                    k++;
                    break;
                  }
              }
          }
        //Cleanup and terminate thread on abort
        if (abort)
          {
            L1.clear();
            L2.clear();
            abort = false;
            return;
          }
      } //end of outer loop
    //qDebug() << "KbvDirWatcherThread::run: end m k" << m << k; //###########
    //L2 > L1 -> append last m-k new items of L2 to the model
    if (m > k)
      {
        //qDebug() << "KbvDirWatcherThread::run: rows to insert" << m-k; //###########
        for (int v=k; v<m; v++)
          {
            watchedItem.row = v;
            watchedItem.dirName = watchedDir + L2.value(v);
            watchedItem.parent = watchedIndex;
            emit  insertRow(watchedItem);
            //qDebug() << "KbvDirWatcherThread::run: insert" <<watchedItem.dirName; //###########
          }
      }
    //Finish: cleanup and save L2 as new reference L1
    L1.clear();
    L1 << L2;
    //qDebug() << "KbvDirWatcherThread::run check grip" << L1.length(); //###########
    //check all children of watched dir for added or removed grandchildren
    for (i=0; i<L1.length(); i++)
      {
        emit  checkExpansionGrip(watchedIndex);
      }
    //qDebug() << "KbvDirWatcherThread::run: finished"; //###########
  }
/************************************************************************//*!
 * Read content of current directory into a list of strings.
 * Names are sorted alphabetically and only names are considered.
 */
void    KbvDirWatcherThread::readDirContent(QStringList &list, QString &dirpath)
{
  QDir    dir;
  
  list.clear();
  dir.setPath(dirpath);
  dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
  dir.setSorting(QDir::Name | QDir::LocaleAware);
  list.append(dir.entryList());
}
/*****************************************************************************/
