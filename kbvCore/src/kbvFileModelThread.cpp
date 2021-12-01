/*****************************************************************************
 * kbvFileModelThread
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.01.15
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvFileModelThread.h"
//#include "kbvFileModel.h"
#include "kbvConstants.h"


KbvFileModelThread::KbvFileModelThread(QObject *parent) : QThread(parent)
{
  abort = false;
}

/*************************************************************************//*!
 * Destructor waits until thread has finished or wakes up the sleeping thread
 * to finish.
 */
KbvFileModelThread::~KbvFileModelThread()
{
  //qDebug() << "KbvFileModelThread::~KbvFileModelThread";//###########
  abort = true;
  wait();
}

/*************************************************************************//*!
 * Starts the thread. A running thread will be aborted!
 * We use QImageReader since it's much faster than QImage
 */
void    KbvFileModelThread::startThread(const QString &rootdir, QStringList *fileentries,
                                        QSize iconsize, int position)
{
  //qDebug() << "KbvFileModelThread::startThread" <<rootdir; //##########
  if(this->isRunning())
    {
      abort = true;
      wait();
    }
    this->iconsize = iconsize;
    filedir = rootdir + "/";
    entries = fileentries;
    location = position;

    //Start thread
    start(NormalPriority);
}

/*************************************************************************//*!
 * Stops the thread when it's runing and waits until the thread has stopped.
 * When the tread isn't running the signal treadFinished() is required for
 * synchronizing the start/stop mechanism with fileModel.
 */
void    KbvFileModelThread::stopThread()
{
  //qDebug() << "KbvFileModelThread::stopThread" <<filedir; //##########
  if(this->isRunning())
    {
      abort = true;
      wait();
    }
  else
    {
      emit threadFinished();
    }
}

/*************************************************************************//*!
 * Fill file model with data (filename, path and icon)
 * The run method realizes a loop to process all files in the directory and
 * then stops the thread (returns from run). When a new selection meets the
 * running thread abort=true finishes the actual work.
 * Note:
 * The run method waits 50ms before sending the finish signal and then returns.
 * This is more than sufficient to deliver all pending signals and an immediately
 * following call with different dir will not catch remaining signals from
 * previous thread.
 * When the application finishes the destructor sets abort=true and wakes up
 * the thread once whereon the run method returns finally.
 * Created item:
 * QString   - Qt::DisplayRole
 * QString   - Kbv::FileNameRole
 * QString   - Kbv::FilePathRole
 * qint64    - Kbv::FileSizeRole
 * QDateTime - Kbv::FileDateRole
 * QSize     - Kbv::ImageDimRole
 * quint64   - Kbv::ImageSizeRole
 */
void    KbvFileModelThread::run()
{
  Kbv::kbvItem   *item;

  //qDebug() << "KbvFileModelThread::run"<<filedir; //##########
  //working loop
  for (int i=0; i < entries->size(); ++i)
    {
      if(abort)
        {
          //stop thread
          break;
        }
      else
        {
          //create new item and insert it into model
          item = globalFunc.itemFromFile(filedir, entries->at(i), iconsize);
          emit newItem(item);
          //qDebug() << "KbvFileModelThread::run insert" <<entries->at(i); //##########
          emit statusText2(QString("%1").arg(i, 6));
        }
    }
  usleep(50000);     //wait for pending signals to be executed
  emit statusText2("");
  emit threadFinished();
  abort = false;
  //qDebug() << "KbvFileModelThread::run finished"; //##########
}

/*************************************************************************//*!
 * Smart scale\n
 * The code is from: qt-labs-graphics-dojo/thumbview.
 * See explanation below.
 */
#define AVG(a,b)  ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )

// assume that the source image is ARGB32 formatted
QImage KbvFileModelThread::smartScale(int width, int height, const QImage &source)
{
    Q_ASSERT(source.format() == QImage::Format_ARGB32);

    QImage dest(width, height, QImage::Format_ARGB32);

    int sw = source.width();
    int sh = source.height();
    int xs = (sw << 8) / width;
    int ys = (sh << 8) / height;
    quint32 *dst = reinterpret_cast<quint32*>(dest.bits());
    int stride = dest.bytesPerLine() >> 2;

    for (int y = 0, yi = ys >> 2; y < height; ++y, yi += ys, dst += stride)
      {
        const quint32 *src1 = reinterpret_cast<const quint32*>(source.scanLine(yi >> 8));
        const quint32 *src2 = reinterpret_cast<const quint32*>(source.scanLine((yi + ys / 2) >> 8));
        for (int x = 0, xi1 = xs / 4, xi2 = xs * 3 / 4; x < width; ++x, xi1 += xs, xi2 += xs)
          {
            quint32 pixel1 = AVG(src1[xi1 >> 8], src1[xi2 >> 8]);
            quint32 pixel2 = AVG(src2[xi1 >> 8], src2[xi2 >> 8]);
            dst[x] = AVG(pixel1, pixel2);
          }
      }
    return dest;
}
/*************************************************************************//*!
 *For the explanation of the trick, check out:
 * - http://www.virtualdub.org/blog/pivot/entry.php?id=116
 * - http://www.compuphase.com/graphic/scale3.htm
 *If you need to average two sets of bitfields using larger word operations,
 *there is a trick derived from a hardware algorithm for constructing adders
 *that can come in handy. The basic comes from the design of a carry-save adder,
 *which splits apart carry and sum signals to allow multiple values to be added
 *with only one carry propagation pass at the end:
 * a+b = ((a & b) << 1) + (a ^ b)
 *(& is the C operator for bitwise AND, ^ is bitwise exclusive OR, and << is
 *a left shift.)
 *To convert this into an average, propagate through the necessary shift:
 * (a+b) >> 1 = (a & b) + ((a ^ b) >> 1)
 *and to apply this to bitfields, just mask off the least significant bits
 *from each bitfield sum to avoid cross-field contamination:
 * (pel0+pel1) >> 1 = (pel0 & pel1) + (((pel0 ^ pel1) & 0xfefefefe)>>1)
 *This can be useful even if you have SIMD hardware that has built-in averaging.
 *For instance, SSE has support for unsigned byte averages, but the above
 *algorithm can be used to average four 565 pixels at a time by using the mask
 *0xf7def7def7def7de.
 *Note that the above algorithm always rounds down, which isn't always appropriate;
 *MPEG motion prediction, for instance, requires rounding up. There is a trick that
 *can be used to average up without using any additional operations.
 *The trick is to recognize that the addition can also be formulated using inclusive
 *OR and subtraction:
 * a+b = ((a & b) << 1) + (a^b) = ((a|b) << 1) - (a^b)
 *Shifting out the LSB from the exclusive OR result then has the effect of rounding
 *up instead of rounding down, giving:
 * (a+b+1) >> 1 = (a|b) - ((a^b)>>1)
 * (pel0+pel1+1)>>1 = (pel0 | pel1) - (((pel0 ^ pel1) & 0xfefefefe)>>1)
 */


/*****************************************************************************/
