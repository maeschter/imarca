/*****************************************************************************
 * kvb image viewer
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-06-12
 * Created: 2017.05.01
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvImageViewer.h"

extern  KbvSetvalues            *settings;

KbvImageViewer::KbvImageViewer(QWidget *parent, Qt::WindowFlags flags) : QLabel(parent, flags)
{
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowFlags(Qt::Window);
  this->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  this->setAutoFillBackground(true);
  
  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, settings->slideBackColour);
  this->setPalette(palette);

  //for test only  
  this->setAttribute(Qt::WA_AlwaysShowToolTips);
  this->setMouseTracking(true);

  //Desktop and screen dimensions
  int   prim = desktop.primaryScreen();
  desktopSize = desktop.screenGeometry(prim);     //full monitor size
  screenSize = desktop.availableGeometry(prim);   //screen size for windows (ex title and base panel)
  //qDebug() << "KbvImageViewer desktop" <<desktopSize.left() <<desktopSize.top() <<desktopSize.width() <<desktopSize.height(); //###########
  //qDebug() << "KbvImageViewer screen" <<screenSize.left() <<screenSize.top() <<screenSize.width() <<screenSize.height(); //###########
  
  //full screen size
  geo_full = desktopSize;
  //qDebug() << "KbvImageViewer full geo" <<geo_full.width() <<geo_full.height(); //###########
  
  //TODO: on X11 the titleBarHeight only can be estimated but not calculated
  //normal screen size
  geo_normal = screenSize;
  int titleBarHeight = this->style()->pixelMetric(QStyle::PM_TitleBarHeight, 0, this) + 12;
  int frameWidth = this->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
  geo_normal.setHeight(geo_normal.height() - titleBarHeight - frameWidth - frameWidth);
  geo_normal.setWidth(geo_normal.width() - frameWidth - frameWidth);
  //qDebug() << "KbvImageViewer titlebarheight framewidth" <<titleBarHeight <<frameWidth; //###########
  //qDebug() << "KbvImageViewer normal geo" <<geo_normal.width() <<geo_normal.height(); //###########

  painter = new QPainter();

  //Timer1 enforces a minimal display time by prohibiting the changeover
  timer1 = new QTimer(this);    //automatic slide change
  timer1->setSingleShot(true);
  if(settings->slideChangeManual)
    {
      timer1->setInterval(250);
    }
  else
    {
      timer1->setInterval(settings->slideDelaySec*1000);
    }

  showText = true;
  permitChange = true;
  
  connect(timer1, SIGNAL(timeout()), this, SLOT(permitChangeover()));  
}

KbvImageViewer::~KbvImageViewer()
{
  qDebug() << "KbvImageViewer::~KbvImageViewer"; //###########
  this->clear();
  delete  timer1;
  delete  painter;
  delete  playlist;  
  if(!deletelist.isEmpty())
    {
      emit  (deleteFiles(deletelist));
    }
}

/*************************************************************************//*!
 * Display images of "filelist".
 */
void  KbvImageViewer::showImages(const QList<QPair<QString, QString> > filelist)
{
  if (filelist.isEmpty())
    {
      this->close();
    }
  //save playlist as long as the viewer is running
  playlist = new QList<QPair<QString, QString> >(filelist);
  this->setVisible(true);
  this->activateWindow();
  this->current = 0;
  imageChange(Kbv::current);
}

/*************************************************************************//*!
 * Show prevoius or next image.\m
 * When the last or first item is reached the count will wrap around and
 * continue with the first respectively the last item, depending on settings.
 * If this is not permitted, the viewer stops at first or last item.
 * If nor next item neither previous item get selected the current item will
 * not change. After each change, timer1 guarantees a minimal display time.
 */
void KbvImageViewer::imageChange(int previousnext)
{
  if(permitChange)
    {
      if(previousnext == Kbv::next)
        {
          if(current+1 >= playlist->length())
            {
              if(!settings->slideStopAtDirEnd)
                {
                  current = 0;
                }
            }
          else
            {
              current++;
            }
        }
    
      if(previousnext == Kbv::previous)
        {
          if(current-1 < 0)
            {
              if(!settings->slideStopAtDirEnd)
                {
                  current = playlist->length()-1;
                }
            }
          else
            {
              current--;
            }
        }
      permitChange = false;
      //qDebug() <<"KbvImageViewer::imageChange"<<permitChange <<current; //###########
      this->adjustAndShow(playlist->at(current));
    }
}

/*************************************************************************//*!
 * Size pixmap to fit to screen.\n
 * - Full screen mode: the pixmap is stretched when the appropriate option is
 *   checked, all other cases shrink the pixmap to fit in desktop.
 * - Non full screen mode: the screen size is reduced to 80% of available
 *   screen size and the pixmap gets reduced to fit.
*/
void  KbvImageViewer::adjustAndShow(QPair<QString, QString> path)
{
  KbvExiv     metaData;   //libKbvExiv.so
  QTransform  trans;
  QSize       deltaSize;
  QRect       geo;
  bool        imageHeightLarger, imageWidthLarger, max;

  //full screen and normal screen display
  //set geometries of visible image area
  if(settings->slideFullScreen)
    {
      max = true;
      geo = geo_full;
      this->showFullScreen();
      this->setGeometry(geo);
    }
  else
    {
      max = false;
      geo = geo_normal;
      this->showNormal();
    }
  //qDebug() << "KbvImageViewer::adjustSize available geometry" <<geo.width() <<geo.height(); //###########

  this->setWindowTitle(path.second);
  this->setPixmap(QPixmap());   //show background only, prevents flicker

  //load pixmap and display the "no support icon" on failure
  image = QPixmap(path.first + path.second);
  if(image.isNull())
    {
      this->setWindowTitle("");
      image = QPixmap(":/kbv/icons/kbv_no_support.png");
      if(!max)
        {
          this->setGeometry(image.rect().adjusted(0,0,200,200));
          deltaSize = geo.size() - image.size();
          this->move(deltaSize.width()/2, deltaSize.height()/2);
        }
    }
  else
    {
      //adjust image to orientation given by exif data
      //positive rotation means here: clockwise rotation
      //unspecified  = 0
      //normal       = 1,  rot_90_vflip = 5
      //vflip        = 2,  rot_90       = 6
      //rot_180      = 3,  rot_90_hflip = 7
      //hflip        = 4,  rot_270      = 8
      //hflip: mirrored along the horizontal axis
      //vflip: mirrored along the vertical axis

      QByteArray  ba = metaData.getPhotoAlignmentFromFile(path.first + path.second);
      short orientation = ba[0]*16+ba[1];
      //qDebug() << "KbvImageViewer::adjustAndShow orientation" <<orientation <<ba <<path.second; //###########
      switch(orientation)
        {
          case 2: { trans = QTransform(-1.0, 0.0, 0.0, 1.0, 1.0*image.width(), 0.0);  //vflip
                    image = image.transformed(trans); }
          break;
          case 3: { trans.rotate(180.0);
                    image = image.transformed(trans); }
          break;
          case 4: { trans = QTransform(1.0, 0.0, 0.0, -1.0, 0.0, 1.0*image.height());  //hflip
                    image = image.transformed(trans); }
          break;
          case 5: { trans = QTransform(-1.0, 0.0, 0.0, 1.0, 1.0*image.width(), 0.0);  //vflip
                    image = image.transformed(trans);
                    trans = QTransform(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
                    trans.rotate(-90.0);
                    image = image.transformed(trans); }
          break;
          case 6: { trans = QTransform(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
                    trans.rotate(90.0);
                    image = image.transformed(trans); }
          break;
          case 7: { trans = QTransform(1.0, 0.0, 0.0, -1.0, 0.0, 1.0*image.height());  //hflip
                    image = image.transformed(trans);
                    trans = QTransform(1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
                    trans.rotate(-90.0);
                    image = image.transformed(trans); }
          break;
          case 8: { trans.rotate(-90.0);
                    image = image.transformed(trans); }
        }

      //Set flags due to relation geometry/image.
      deltaSize = geo.size() - image.size();
      //qDebug() << "KbvImageViewer::adjustSize delta" <<deltaSize.width() <<deltaSize.height(); //###########
      imageHeightLarger = (deltaSize.height() >= 0) ? false : true;
      imageWidthLarger  = (deltaSize.width() >= 0) ? false : true;
      
      //image width and height are smaller or equal than geometry
      //set geometry to image size for normal view
      if (!imageWidthLarger && !imageHeightLarger)
        {
          if(!max)
            {
              this->setGeometry(image.rect());
              this->move(screenSize.center() - image.rect().center());
            }
          else
            {
              //full screen, no window frames
              if(settings->slideStretch)
                {
                  image = image.scaled(geo.width(), geo.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                }
            }
        }
      //image width is smaller or equal and height is larger than geometry
      //scale image to geometry keeping aspect ratio, set geometry to image size
      if (!imageWidthLarger && imageHeightLarger)
        {
          image = image.scaled(geo.width(), geo.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
          if(!max)
            {
              this->setGeometry(image.rect());
              this->move(screenSize.center() - image.rect().center());
            }
        }
      //image width is larger and height is smaller or equal than  geometry
      //scale image to geometry keeping aspect ratio, set geometry to image size
      if (imageWidthLarger && !imageHeightLarger)
        {
          image = image.scaled(geo.width(), geo.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
          if(!max)
            {
              this->setGeometry(image.rect());
              this->move(screenSize.center() - image.rect().center());
            }
        }
      //image width and height are larger than  geometry
      //scale image to geometry keeping aspect ratio, set geometry to image size
      if (imageWidthLarger && imageHeightLarger)
        {
          image = image.scaled(geo.width(), geo.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
          if(!max)
            {
              this->setGeometry(image.rect());
              this->move(screenSize.center() - image.rect().center());
            }
        }
    }
  //qDebug() << "KbvImageViewer::adjustSize image" <<image.width() <<image.height(); //###########
  this->showMetaData(showText);
  this->setPixmap(image);         //display image in label
  this->timer1->start();
}

/*************************************************************************//*!
 * Display or hide the image meta data.
 */
void  KbvImageViewer::showMetaData(bool show)
{
  QString   info, path;
  KbvExiv   metaData;   //libKbvExiv.so

  if(show)
    {
      info = QString("File: ");
      info.append(playlist->at(current).second);

      path = playlist->at(current).first + playlist->at(current).second;
      info.append(metaData.getIptcInfo(path));
      info.append(metaData.getExifInfo(path));
      
      QBrush brush = QBrush(Qt::darkGray, Qt::SolidPattern);
      painter->begin(&image);
      painter->setBackgroundMode(Qt::OpaqueMode);
      painter->setBackground(brush);
      painter->setPen(Qt::white);
      painter->setFont(QFont("Arial", 9));
      painter->drawText(5, 5, image.width(), image.height(), Qt::AlignLeft | Qt::AlignTop, info);
      painter->end();
    }
}

/*************************************************************************//*!
 * Slot: timer1 expired.
 * Changeover to next/previous image gets permitted.
 */
void KbvImageViewer::permitChangeover()
{
  permitChange = true;
  if(!settings->slideChangeManual)
    {
      this->imageChange(Kbv::next);
    }
}

/*************************************************************************//*!
 * Show prevoius image on left mouse button press or show next image on right
 * mouse button press.
 */
void KbvImageViewer::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::RightButton)
    {
      this->imageChange(Kbv::next);
    }
  if(event->button() == Qt::LeftButton)
    {
      this->imageChange(Kbv::previous);
    }
}

/*************************************************************************//*!
 * For test only including setAttribute(Qt::WA_AlwaysShowToolTips) and
 * setMouseTracking(true).
 */
void  KbvImageViewer::mouseMoveEvent(QMouseEvent *event)
{
  Q_UNUSED(event)
//  this->setToolTip(QString("%1,%2").arg(event->pos().x()).arg(event->pos().y()));
}

/*************************************************************************//*!
 * SLOT: Key press event
 * Ctrl-Q or ESC key pressed: close viewer by close event
 * Page down or arrow right will show the next item.\n
 * Page up or arrow left key will show the previous item.\n
 * F9 shows/hides the file metadata.
 * DEL key removes the currently shown item from playlist and displays the
 * following item and adds the removed item to the deletelist. At the end of
 * the show the content of deletelist will be removed from model and files
 * system. This is done in this way to keep the list of model indices valid
 * at any time.\n
 */
void KbvImageViewer::keyPressEvent(QKeyEvent *event)
{
  //qDebug() << "KbvImageViewer::keyPressEvent key" <<event->key(); //###########
  
  //show or hide the textual info on pressing F9
  if(event->key() == Qt::Key_F9)
    {
      showText = !showText;
      imageChange(Kbv::current);
    }
  
  //On pressing delete key the actual shown file has to be removed from play list.
  //File path and name is inserted in delete list. When play list is empty the
  //viewer gets closed otherwise the viewer displays the next image.
  //deletelist can keep multiple values (file names) for one key (file path).
  else if(event->key() == Qt::Key_Delete)
    {
      if(playlist->size() > 1)
        {
          deletelist.insertMulti(playlist->at(current).first, playlist->at(current).second);
          playlist->removeAt(current);
          current--;
          imageChange(Kbv::next);
        }
      else
        {
          deletelist.insertMulti(playlist->at(current).first, playlist->at(current).second);
          close();
        }
    }
  //show next or previous image
  else if((event->key() == Qt::Key_Left) || (event->key() == Qt::Key_PageUp))
    {
      this->imageChange(Kbv::previous);
    }
  else if((event->key() == Qt::Key_Right) || (event->key() == Qt::Key_PageDown))
    {
      this->imageChange(Kbv::next);
    }
  
  //close viewer
  else if((event->key() == Qt::Key_Q) && (event->modifiers() == Qt::ControlModifier))
    {
      this->close();
    }
  else if(event->key() == Qt::Key_Escape)
    {
      this->close();
    }
}
/*************************************************************************//*!
 * SLOT: Quit application on menu file|exit or ctrl-q or ESC or close button.
 */
void KbvImageViewer::closeEvent(QCloseEvent *event)
{
  //qDebug() << "KbvImageViewer::closeEvent"; //###########
  event->accept();
}

/****************************************************************************/
