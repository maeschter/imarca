/*****************************************************************************
 * kbvFileModelThread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.01.15
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvBatchThread.h"

KbvBatchThread::KbvBatchThread(QObject *parent) : QThread(parent)
{
  mAbort = false;
  mBatch = 10;
}

/*************************************************************************//*!
 * Destructor waits until thread has finished or wakes up the sleeping thread
 * to finish.
 */
KbvBatchThread::~KbvBatchThread()
{
  //qDebug() << "KbvFileModelThread::~KbvFileModelThread";//###########
  mAbort = true;
  wait();
}

/*************************************************************************//*!
 * Starts the thread. A running thread will be aborted!
 * The task is devided in batches. The end of a batch will update the progress
 * values.
 */
void  KbvBatchThread::startThread(QComboBox *fileCombo, Kbv::kbvBatchParam &batchParams)
{
  //qDebug() << "KbvvBatchThread::startThread"; //##########
  if(this->isRunning())
    {
      mAbort = true;
      wait();
    }
  mCombo = fileCombo;
  mParams = batchParams;

  //progress bar
  mLot = mCombo->count();
  mBatch = (mLot > 40) ? (mLot/20) : 2;   //20 steps in progress bar for large amount
  mStep = 100.0/mLot;
  //error report
  mReport.clear();

  //Start thread
  start(NormalPriority);
}

/*************************************************************************//*!
 * SLOT: Stops the thread when it's runing and waits until the thread has stopped.
 * When the tread isn't running the signal treadFinished() gets emitted.
 */
void  KbvBatchThread::stopThread()
{
  //qDebug() << "KbvBatchThread::stopThread" <<mFile; //##########
  if(this->isRunning())
    {
      mAbort = true;
      wait();
    }
  else
    {
      emit threadFinished(false);
    }
}

/*************************************************************************//*!
 * Process all files of comboBox "combo" due to parameters set in the batch
 * wizard. The parameters are given in structure Kbv::kbvBatchParam.
 * After performing the files we have to inform all views to update their
 * contents.
 */
void  KbvBatchThread::run()
{
  bool            fileChanged=false;
  QFile           reportFile;
  QTextStream     outReport;
  QString         lineEnd, str;
  QDateTime       dt;

  //qDebug() << "KbvBatchThread::run" <<mFile; //###########
  emit threadNotRunning(false);

  //working loop
  for(int i=0; i<mLot; ++i)
    {
      if(mAbort)
        {
          break;
        }
      fileChanged = false;
      mCombo->setCurrentIndex(i);
      mFile = mCombo->currentText();
      mPath = mCombo->currentData().toString();
      mMetadata.loadFromFile(mPath + mFile);
      //qDebug() << "KbvBatchThread::run open" <<(mPath + mFile); //###########

      try
        {
          ocvFileName = (mPath + mFile).toUtf8().constData();
          ocvImage = cv::imread(ocvFileName, cv::IMREAD_ANYDEPTH + cv::IMREAD_ANYCOLOR + cv::IMREAD_IGNORE_ORIENTATION);
          if(ocvImage.data != NULL)
            {
              //notice when at least one of the files has been altered
              fileChanged |= this->flipRotate();
              fileChanged |= this->trimEdges(fileChanged);
              fileChanged |= this->resize(fileChanged);
              this->setSoftwareInfo(fileChanged);
              this->saveFile(fileChanged);
            }
          else
            {
              //report problems
              mReport.append(QString("Could not open or read: %1 \n").arg(mFile));
            }
        }
      catch(cv::Exception)
        {
          //report problems)
          mReport.append(QString("opencv matrice create error: %1 \n").arg(mFile));
        }

      if((i % mBatch) == 0)
        {
          emit progressText(mFile);
          emit progressValue(int (mStep*i));
        }
    }   //end loop

  emit  threadNotRunning(true);
  mAbort = false;
  if(fileChanged)
    {
      emit  filesChanged(mPath);
    }
  if(mReport.isEmpty())
    {
      emit progressValue(100);
      emit threadFinished(false);
      emit progressText(QString(tr("Process finished.")));
    }
  else
    {
      emit progressValue(100);
      emit threadFinished(true);
      emit progressText(QString(tr("Process finished. Open report for errors.")));

      //save report file in configDir
      lineEnd = QString("\n");
      reportFile.setFileName(QDir::homePath() + QString(configDir) + QString(imageEditorBatchLog));
      outReport.setCodec("UTF-8");
      if(reportFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
          reportFile.resize(0);
          outReport.setDevice(&reportFile);

          //Header and list
          dt = QDateTime::currentDateTime();
          str = dt.toString("yyyy.MM.dd-hh:mm");
          outReport <<QString("Imarca image editor batch processing") <<lineEnd;
          outReport <<QString("Date: %1").arg(str) <<lineEnd <<lineEnd;
          for(int i=0; i<mReport.length(); i++)
            {
              outReport <<mReport.at(i);
            }
          reportFile.flush();
          reportFile.close();
        }
    }
  //qDebug() << "KbvBatchThread::run finished"; //###########
}
/*************************************************************************//*!
 * Flip or rotate the image due to selection in batch wizard page "Rotate".
 * Note: all rotations (flip, by exif orientation or by arbitrary angle are
 * exclusive. This means only one of them can take place and the product is
 * to be found in ocvOutImage with change = true.
 */
bool  KbvBatchThread::flipRotate()
{
  QByteArray  ba;
  QSize       dim;
  short       orientation=0;
  bool        change = false;
  cv::Mat     ocvExt, ocvS, ocvR;
  cv::Point2f center;
  cv::Size    size;
  double      alfa;
  int         dx, dy;

  alfa = mParams.rotAngle;
  //flip horizontically or vetrically
  if(mParams.flipHor)
    {
      //qDebug() << "KbvBatchThread::flipRotate hor"; //###########
      cv::flip(ocvImage, ocvOutImage, 1);
      if(mMetadata.hasExif())
        {
          mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
        }
      change = true;
    }
  else if(mParams.flipVer)
    {
      //qDebug() << "KbvBatchThread::flipRotate ver"; //###########
      cv::flip(ocvImage, ocvOutImage, 0);
      if(mMetadata.hasExif())
        {
          mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
        }
      change = true;
    }
  //-----------------------------------------------------------------------------------------------
  //align image to orientation given by exif data
  else if(mParams.rotExif)
    {
      //qDebug() << "KbvBatchThread::run rotate exif"; //###########
      //unspecified  = 0,  normal = 1, do nothing
      //vflip        = 2,  rot_90       = 6
      //rot_180      = 3,  rot_90_hflip = 7
      //hflip        = 4,  rot_270      = 8
      //rot_90_vflip = 5,  unspecified  > 8
      //hflip: mirrored along the vertical axis
      //vflip: mirrored along the horizontal axis
      //positive rotation means clockwise rotation
      if(mMetadata.hasExif())
        {
          ba = mMetadata.getPhotoAlignmentFromFile(mFile);
          orientation = ba[0]*16+ba[1];
          dim = QSize(ocvImage.cols, ocvImage.rows);

          //qDebug() << "KbvBatchThread::flipRotate exif" <<orientation <<ba <<mFile; //###########
          switch(orientation)
            {
            case 0: { ocvOutImage = ocvImage;
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                      change = true; }
              break;
            case 1: { ocvOutImage = ocvImage;
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                      change = true; }
              break;
            case 2: { cv::flip(ocvImage, ocvOutImage, 0);  //vflip
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                      change = true; }
              break;
            case 3: { cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_180); //rot_180
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                      change = true; }
              break;
            case 4: { cv::flip(ocvImage, ocvOutImage, 1);  //hflip
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                      change = true; }
              break;
            case 5: { cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_CLOCKWISE);  //rot_90_vflip
                      ocvImage = ocvOutImage;
                      cv::flip(ocvImage, ocvOutImage, 0);
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
                      change = true; }
              break;
            case 6: { cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_CLOCKWISE);  //rot_90
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
                      change = true; }
              break;
            case 7: { cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_CLOCKWISE);  //rot_90_hflip
                      ocvImage = ocvOutImage;
                      cv::flip(ocvImage, ocvOutImage, 1);
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
                      change = true; }
              break;
             case 8: { cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_COUNTERCLOCKWISE);  //-rot_90, rot_270
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
                      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
                      change = true; }
              break;
             default: { ocvOutImage = ocvImage;
                        mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
                        mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
                        change = true;}
            }
          mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
        }
      else
        {
          //Do nothing without exif data
        }
    }
  //-----------------------------------------------------------------------------------------------
  //Arbitray rotation angle !=0 .. +-180deg. Positive values mean counter-clockwise rotation!
  //Note: angles are double values and their presentation is exact, so comparison angle == 90.0
  //is working correctly.
  else if(!mParams.flipHor && !mParams.flipVer && !mParams.rotExif)
    {
      dim = QSize(ocvImage.cols, ocvImage.rows);
      if(alfa > 89.99 && alfa < 90.01)
        {
          //qDebug() << "KbvBatchThread::flipRotate angle 90°"; //###########
          cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_CLOCKWISE);
          change = true;
          if(mMetadata.hasExif())
            {
              mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
              mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
              mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
            }
        }
      else if(alfa < -89.99 && alfa > -90.01)
        {
          //qDebug() << "KbvBatchThread::flipRotate angle -90°"; //###########
          cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_COUNTERCLOCKWISE);
          change = true;
          if(mMetadata.hasExif())
            {
              mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.height());
              mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.width());
              mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
            }
        }
      else if(alfa > 179.99 || alfa < -179.99)
        {
          //qDebug() << "KbvBatchThread::flipRotate angle +-180°"; //###########
          cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_180);
          change = true;
          if(mMetadata.hasExif())
            {
              mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", dim.width());
              mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", dim.height());
              mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
            }
        }
      else
        {
          //Arbitrary angle != 0°, +-90°, +-180°
          //The image size will be calculated to fit the rotated image. To achieve this
          //the calculation must respect 3 areas: 0..45°, 45..90°and 90..135°, 135..180°
          //each for positive and negative angles.
          double alfa_r = fabs(alfa/180.0*Kbv::PI);

          //calculate the size of the new rotated image and generate a matrice
          size.width =  int (abs(double (ocvImage.cols*cos(alfa_r))) + double(ocvImage.rows*sin(alfa_r)));
          size.height = int (abs(double (ocvImage.rows*cos(alfa_r))) + double(ocvImage.cols*sin(alfa_r)));
          ocvExt = cv::Mat(size, ocvImage.type(), cv::Scalar());

          //qDebug() << "KbvBatchThread::flipRotate origin type" <<ocvImage.type(); //###########
          //qDebug() << "KbvBatchThread::flipRotate origin size" <<ocvImage.cols <<ocvImage.rows; //###########
          //qDebug() << "KbvBatchThread::flipRotate new size" <<size.width <<size.height; //###########

          //First area -45°..0..45° (0.0 excluded)
          //calculate shift matrice for origin image (into the new image, center to center)
          //shift into new image, calculate rotation matrce and rotate into output image
          if(fabs(alfa) > 0.0 && fabs(alfa) <= 45.0)
            {
              //qDebug() << "KbvBatchThread::flipRotate -45°..0..45°"; //###########
              dx = abs(size.width - ocvImage.cols)/2;
              dy = abs(size.height - ocvImage.rows)/2;
              ocvS = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
              center = cv::Point2f(0.5*ocvExt.cols, 0.5*ocvExt.rows);
              ocvR = cv::getRotationMatrix2D(center, -1.0*alfa, 1.0);

              //qDebug() << "KbvBatchThread::flipRotate rotation center" <<center.x <<center.y; //###########
              //qDebug() << "KbvBatchThread::flipRotate dx dy" <<dx <<dy; //###########
              cv::warpAffine(ocvImage, ocvExt, ocvS, size, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
              cv::warpAffine(ocvExt, ocvOutImage, ocvR, size, cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
              change = true;
            }

          //Second area: 45..90° and 90..135° or -45..-90° and -90..-135°
          //Rotate by 90°, calculate shift matrice for 90°-image
          //45..90°: shift into new image then rotate back by alfa-90°
          //90..135°: shift into new image then rotate forward by alfa-90°
          else if((fabs(alfa) > 45.0 && fabs(alfa) < 90.0) || (fabs(alfa) > 90.0 && fabs(alfa) <= 135.0))
            {
              if(alfa >0.0)
                {
                  //qDebug() << "KbvBatchThread::flipRotate 45..90° or 90..135°"; //###########
                  cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_CLOCKWISE);
                  dx = abs(size.width - ocvOutImage.cols)/2;
                  dy = abs(size.height - ocvOutImage.rows)/2;
                  ocvS = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
                  center = cv::Point2f(0.5*ocvExt.cols, 0.5*ocvExt.rows);
                  ocvR = cv::getRotationMatrix2D(center, -1.0*(alfa-90), 1.0);
                }
              else
                {
                  //qDebug() << "KbvBatchThread::flipRotate -45..-90° or -90..-135°"; //###########
                  cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_90_COUNTERCLOCKWISE);
                  dx = abs(size.width - ocvOutImage.cols)/2;
                  dy = abs(size.height - ocvOutImage.rows)/2;
                  ocvS = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
                  center = cv::Point2f(0.5*ocvExt.cols, 0.5*ocvExt.rows);
                  ocvR = cv::getRotationMatrix2D(center, -1.0*(alfa+90), 1.0);
                }
              //qDebug() << "KbvBatchThread::flipRotate center" <<center.x <<center.y; //###########
              //qDebug() << "KbvBatchThread::run dx dy" <<dx <<dy; //###########
              cv::warpAffine(ocvOutImage, ocvExt, ocvS, size, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
              cv::warpAffine(ocvExt, ocvOutImage, ocvR, size, cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
              change = true;
            }
          //Third area: 135..180° or -135..-180
          //Rotate by 180°, calculate shift matrice for 180°-image
          //shift into new image then rotate back by alfa-180°
          else if(fabs(alfa) > 135.0 && fabs(alfa) < 180.0)
            {
              if(alfa >0.0)
                {
                  //qDebug() << "KbvBatchThread::flipRotate 135..180°"; //###########
                  cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_180);
                  dx = abs(size.width - ocvOutImage.cols)/2;
                  dy = abs(size.height - ocvOutImage.rows)/2;
                  ocvS = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
                  center = cv::Point2f(0.5*ocvExt.cols, 0.5*ocvExt.rows);
                  ocvR = cv::getRotationMatrix2D(center, -1.0*(alfa-180), 1.0);
                }
              else
                {
                  //qDebug() << "KbvBatchThread::flipRotate -135..-180°"; //###########
                  cv::rotate(ocvImage, ocvOutImage, cv::ROTATE_180);
                  dx = abs(size.width - ocvOutImage.cols)/2;
                  dy = abs(size.height - ocvOutImage.rows)/2;
                  ocvS = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
                  center = cv::Point2f(0.5*ocvExt.cols, 0.5*ocvExt.rows);
                  ocvR = cv::getRotationMatrix2D(center, -1.0*(alfa+180), 1.0);
                }
              //qDebug() << "KbvBatchThread::flipRotate center" <<center.x <<center.y; //###########
              //qDebug() << "KbvBatchThread::flipRotate dx dy" <<dx <<dy; //###########
              cv::warpAffine(ocvOutImage, ocvExt, ocvS, size, cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
              cv::warpAffine(ocvExt, ocvOutImage, ocvR, size, cv::INTER_CUBIC, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
              change = true;
            }
          if(mMetadata.hasExif())
            {
              mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", ocvOutImage.cols);
              mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", ocvOutImage.rows);
              mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
            }
        }
    }
  else
    {
      //qDebug() << "KbvBatchThread::flipRotate no rotation"; //###########
      //nothing selected
    }
  return change;
}
/*************************************************************************//*!
 * Trim edges due to selection in batch wizard page "Trim".
 * Note: when changed==true the input is expected in matrice ocvOutImage
 * instead of ocvImage.
 */
bool  KbvBatchThread::trimEdges(const bool changed)
{
  int           top, bottom, left, right;
  cv::Mat       ocvIn;
  cv::Size      rectSize;
  cv::Point2f   rectCenter;

  top = mParams.trimTop;
  bottom = mParams.trimBottom;
  left = mParams.trimLeft;
  right = mParams.trimRight;

  if((top+left+bottom+right) == 0)
    {
      return false;
    }

  if(changed)
    {
      ocvIn = cv::Mat(ocvOutImage);
    }
  else
    {
      ocvIn = cv::Mat(ocvImage);
    }

  if((top+bottom >= ocvIn.rows) || (left+right >= ocvIn.cols))
    {
      //report problems
      mReport.append(QString("Trimming exceeds with or height: %1 \n").arg(mFile));
      return false;
    }

  rectSize = cv::Size(ocvIn.cols-left-right, ocvIn.rows-top-bottom);
  rectCenter = cv::Point2f(1.0*left + 0.5*rectSize.width, 1.0*top + 0.5*rectSize.height);
  cv::getRectSubPix(ocvIn, rectSize, rectCenter, ocvOutImage);

  if(mMetadata.hasExif())
    {
      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", ocvOutImage.cols);
      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", ocvOutImage.rows);
      mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
    }

  //qDebug() << "KbvBatchThread::trimEdges change true"; //###########
  return true;
}
/*************************************************************************//*!
 * Change size due to selection in batch wizard page "Resize".
 * Note: when changed==true the input is expected in matrice ocvOutImage
 * instead of ocvImage.
 */
bool  KbvBatchThread::resize(const bool changed)
{
  bool      change = false;
  cv::Mat   ocvIn;
  double    scaleX, scaleY;

  if(changed)
    {
      ocvIn = cv::Mat(ocvOutImage);
    }
  else
    {
      ocvIn = cv::Mat(ocvImage);
    }

  //resize due to given height while keeping aspect ratio
  if(mParams.resizeHeight > 10)
    {
      //qDebug() << "KbvBatchThread::resize height" <<mParams.resizeHeight; //###########
      scaleY = double (mParams.resizeHeight) / double (ocvIn.rows);
      scaleX = scaleY;
      cv::resize(ocvIn, ocvOutImage, cv::Size(0, 0), scaleX, scaleY, cv::INTER_CUBIC);
      change = true;
    }
  //resize due to given width while keeping aspect ratio
  else if(mParams.resizeWidth > 10)
    {
      //qDebug() << "KbvBatchThread::resize width" <<mParams.resizeWidth; //###########
      scaleX = double (mParams.resizeWidth) / double (ocvIn.cols);
      scaleY = scaleX;
      cv::resize(ocvIn, ocvOutImage, cv::Size(0, 0), scaleX, scaleY, cv::INTER_CUBIC);
      change = true;
    }
  //resize due to given percentage of origin size
  //the percentage comes as double value
  else if(mParams.resizePercent > 10.0)
    {
      //qDebug() << "KbvBatchThread::resize percent" <<mParams.resizePercent; //###########
      scaleX = mParams.resizePercent/100.0;
      scaleY = scaleX;
      cv::resize(ocvIn, ocvOutImage, cv::Size(0, 0), scaleX, scaleY, cv::INTER_CUBIC);
      change = true;
    }
  else
    {
      //nothing to do here
      change = false;
    }

  if(mMetadata.hasExif() && change)
    {
      mMetadata.setExifTagDataLong("Exif.Photo.PixelXDimension", ocvOutImage.cols);
      mMetadata.setExifTagDataLong("Exif.Photo.PixelYDimension", ocvOutImage.rows);
      mMetadata.removeExifTag(QString("Exif.Image.Orientation"));
    }
  return change;
}
/*************************************************************************//*!
 * Set exif tag "processing software" on changes.
 */
void  KbvBatchThread::setSoftwareInfo(const bool changed)
{
  if(mMetadata.hasExif() && changed)
    {
      QString info = QString(appName) + " " + QString(appVersion);
      mMetadata.setExifTagDataString("Exif.Image.ProcessingSoftware", info);
    }
}
/*************************************************************************//*!
 * Save file due to batch wizard page "Save".
 * Opencv selects the appropriate format due to file extension. Unsupported
 * formats cause an entry in error report.
 */
void  KbvBatchThread::saveFile(const bool changed)
{
  QString     path, file, format;
  QFileInfo   info;
  cv::Mat     ocvIn;
  int         n;
  std::vector<int>  params;

  //save image and metadata
  path = mParams.targetDir;
  file = mFile;
  if(path.isEmpty())
    {
      path = mPath;
    }
  if(!path.endsWith("/"))
    {
      path.append("/");
    }

  format = mParams.saveFormat;

  //Get file name without extension,
  //mParams.saveFormat contains the new format = extension
  n = mFile.lastIndexOf(".");
  if(n > 0)
    {
      file = mFile.left(n);
    }
  else
    {
      file = mFile;
    }
  file.append("." + mParams.saveFormat);

  info = QFileInfo(path + file);
  while(!mParams.overwrite && info.exists())
    {
      file.prepend("#");
      info = QFileInfo(path + file);
    }
  ocvFileName = (path+file).toUtf8().constData();

  //get the right image for saving
  if(changed)
    {
      ocvIn = cv::Mat(ocvOutImage);
    }
  else
    {
      ocvIn = cv::Mat(ocvImage);
    }

  //Parameters for file formats: jpeg quality, png compression
  if(mParams.saveFormat.contains("jpg"))
    {
      n = (mParams.saveParam<0) ? 0 : ((mParams.saveParam>100) ? 100 : mParams.saveParam);
      params.push_back(cv::IMWRITE_JPEG_QUALITY);
      params.push_back(n);
      //qDebug() << "KbvBatchThread::saveFile as" <<path <<file <<n <<mParams.saveParam; //###########
    }
  if(mParams.saveFormat.contains("png"))
    {
      n = (mParams.saveParam<0) ? 0 : ((mParams.saveParam>9) ? 9 : mParams.saveParam);
      params.push_back(cv::IMWRITE_PNG_COMPRESSION);
      params.push_back(n);
      //qDebug() << "KbvBatchThread::saveFile as"<<path <<file <<n <<mParams.saveParam; //###########
    }

  try
    {
      if(cv::imwrite(ocvFileName, ocvIn, params))
        {
          //qDebug() << "KbvBatchThread::saveFile saved" <<QString::fromStdString(std::string(ocvFileName)); //###########
          //the write function doesn't check the presence of metadata
          if(mMetadata.hasExif() || mMetadata.hasIptc() || mMetadata.hasComments() /*|| mMetadata.hasXmp()*/)
            {
              mMetadata.writeToFile(path+file);
            }
        }
      else
        {
          //report problems on not existing or not writable dir
          info = QFileInfo(path);
          mReport.append(QString("Could not save: %1 \n").arg(file));
          if(!info.exists())
            {
              mReport.append(QString("    Directory doesn't exist: %1 \n").arg(path));
            }
          if(!info.isWritable())
            {
              mReport.append(QString("    Directory not writable: %1 \n").arg(path));
            }
        }
    }
  catch(cv::Exception)
    {
      //report problems on wrong or missing file extensions (unsupported image formats)
      mReport.append(QString("Missing or unsupported file (extension): %1 \n").arg(file));
    }
}

/*****************************************************************************/
