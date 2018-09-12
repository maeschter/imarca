/*****************************************************************************
 * kvb slide scene
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-04-29 19:20:46 +0200 (Sa, 29. Apr 2017) $
 * $Rev: 1315 $
 * Created: 2009.11.29
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
  *****************************************************************************/
#include <QtDebug>
#include "kbvGlobal.h"
#include "kbvConstants.h"
#include "kbvSetvalues.h"
#include "kbvSlideScene.h"
#include "kbvFileView.h"
#include "kbvPluginInterfaces.h"

extern  KbvSetvalues            *settings;
extern  KbvGlobal               *globalFunc;
extern  KbvIptcExifXmpInterface *metadataPlugin;

KbvSlideScene::KbvSlideScene(QWidget *parent) : QGraphicsScene(parent)
{
  //initialize
  oldSlide = this->addPixmap(globalFunc->pixMimeImage);
  currentSlide = oldSlide;
  changeBusy = false;           //slide change in progress
  direction = Kbv::next;

  textInfo = new QGraphicsTextItem();
  textInfo->setDefaultTextColor(QColor(255,255,255,255));
  textInfo->setPos(5.0, 5.0);
  this->addItem(textInfo);
  metaData = true;

  timer1 = new QTimer(this);    //automatic slide change
  timer1->setInterval(settings->slideDelaySec * 1000);
  timer1->setSingleShot(true);

  timer2 = new QTimer(this);    //fade-in, insert left, insert right
  timer2->setInterval(Kbv::InsertTimer);
  timer2->setSingleShot(true);

  //Desktop and screen dimensions
  int   prim = desktop.primaryScreen();
  desktopSize = desktop.screenGeometry(prim);
  screenSize = desktop.availableGeometry(prim);
  //qDebug() << "KbvSlideScene scene desktopSize" << desktopSize; //###########
  //qDebug() << "KbvSlideScene scene screenSize" << screenSize; //###########

  //Scene rectangle for full screen mode due to slide change mode
  QRectF rectf(desktopSize);
  if (settings->slideFullScreen)
    {
      switch(settings->slideChangeId)
      {
        case KbvConf::slideReplace:
          {
            this->setSceneRect(rectf);
          }
          break;
        case KbvConf::slideFadeIn:
          {
            this->setSceneRect(rectf);
          }
          break;
        case KbvConf::slideInsertLeft:
          {
            rectf.setWidth(qreal (3.0*desktopSize.width()));
            rectf.setHeight(qreal (1.0*desktopSize.height()));
            this->setSceneRect(rectf);
          }
          break;
        case KbvConf::slideInsertRight:
          {
            rectf.setWidth(qreal (3.0*desktopSize.width()));
            rectf.setHeight(qreal (1.0*desktopSize.height()));
            this->setSceneRect(rectf);
          }
          break;
        default:
          {
            this->setSceneRect(rectf);
          }
          break;
      }
    }
  connect(timer1, SIGNAL(timeout()), this, SLOT(slideAutomatic())); //automatic
  connect(timer2, SIGNAL(timeout()), this, SLOT(slideChangeOver())); //fade-in etc
}

KbvSlideScene::~KbvSlideScene()
{
  //qDebug() << "KbvSlideScene::~KbvSlideScene"; //###########
  delete    textInfo;
  delete    playlist;
  delete    currentSlide;  //oldSlide already has been deleted
  if(!deletelist.isEmpty())
    {
      emit  (deleteFiles(deletelist));
    }
}

/*************************************************************************//*!
 * SLOT: mouse press event in full screen only
 * Left mouse button shows the previous item, right button shows the next item
 */
void    KbvSlideScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
  if (settings->slideChangeManual && settings->slideFullScreen)
    {
      if (mouseEvent->button() == Qt::RightButton)
        {
          slidePreviousNext(Kbv::next);
        }
      if (mouseEvent->button() == Qt::LeftButton)
        {
          slidePreviousNext(Kbv::previous);
        }
    }
}
/*************************************************************************//*!
 * SLOT: key press event\n
 * Delete key removes the currently shown file from slide show and removes them
 * from file model and files system at the end of slide show. This is done in
 * this way to keep the list of model indices valid at any time.\n
 * Page down, arrow down or arrow right will show the next item.\n
 * Page up, arrow up or arrow left key will show the previous item.\n
 * F9 shows/hides the file metadata.
 */
void    KbvSlideScene::keyPressEvent(QKeyEvent *keyEvent)
{
  //show or hide the textual info on pressing F9
  if(keyEvent->key() == Qt::Key_F9)
    {
      metaData = !metaData;
      showMetaData(metaData);
    }

  //On pressing delete key the actual shown file has to be removed from play list.
  //File path and name is inserted in delete list. When play list is empty the
  //slide show gets terminated. When the slide show has finished all members of
  //delete list are removed from model and file system.
  //Depending of direction the currentIndex is corrected and the following
  //file displayed.
  if(keyEvent->key() == Qt::Key_Delete && settings->slideChangeManual)
    {
      if(playlist->size() > 1)
        {
          deletelist.insertMulti(playlist->at(currentIndex).first, playlist->at(currentIndex).second);
          playlist->removeAt(currentIndex);
        }
      else
        {
          deletelist.insertMulti(playlist->at(currentIndex).first, playlist->at(currentIndex).second);
          emit finish();
          return;
        }

      if(direction == Kbv::next)
        {
          currentIndex--;
          if(currentIndex < 0)
            {
              currentIndex = playlist->length()-1;
            }
         slidePreviousNext(Kbv::next);
        }
      if(direction == Kbv::previous)
        {
          currentIndex++;
          if(currentIndex >= playlist->length())
            {
              currentIndex = 0;
            }
          slidePreviousNext(Kbv::previous);
        }
      //qDebug() << "KbvSlideScene::keyPressEvent next slide"; //###########
      //qDebug() << "current index" << currentIndex; //###########
      //qDebug() << "file name" << playlist->at(currentIndex); //###########
    }
  //Manual slide change by arrow keys or page keys.
  if (settings->slideChangeManual)
    {
      if (keyEvent->matches(QKeySequence::MoveToNextChar) ||
          keyEvent->matches(QKeySequence::MoveToNextLine) ||
          keyEvent->matches(QKeySequence::MoveToNextPage))
        {
          direction = Kbv::next;
          slidePreviousNext(Kbv::next);
          return;
        }
      if (keyEvent->matches(QKeySequence::MoveToPreviousChar) ||
          keyEvent->matches(QKeySequence::MoveToPreviousLine) ||
          keyEvent->matches(QKeySequence::MoveToPreviousPage))
        {
          direction = Kbv::previous;
          slidePreviousNext(Kbv::previous);
        }
    }
}
/*************************************************************************//*!
 * Display or hide the image meta data.
 */
void    KbvSlideScene::showMetaData(bool show)
{
  QString           info, filePath;

  if(!show)
    {
      textInfo->hide();
      textInfo->setPlainText("");
    }
  else
    {
      filePath = playlist->at(currentIndex).first + playlist->at(currentIndex).second;
      info = QString("File: ");
      info.append(filePath.right(filePath.length() - 1 - filePath.lastIndexOf(QString("/"))));
      info.append(metadataPlugin->getIptcInfo(filePath));
      info.append(metadataPlugin->getExifInfo(filePath));
      textInfo->setPlainText(info);
      textInfo->setZValue(2.0);
      textInfo->show();
    }
}
/*************************************************************************//*!
 * SLOT: timer1 expired
 * Timer1 automatically changes slides time controlled in full screen mode.
 */
void    KbvSlideScene::slideAutomatic(void)
{
  slidePreviousNext(Kbv::next);
}
/*************************************************************************//*!
 * SLOT: timer2 expired\n
 * Timer2 provides a delay between subsequent steps during dynamic slide change
 * in full screen mode. The velocity of change depends on amount of insert steps
 * and on size of the image and is self adjusting.\n
 * The timer is started again as soon as the actual process step accomplishes
 * (adaption for large images) and triggers next step.\n
 * When the change process finishes the busy signal is reset and timer1 has
 * to be started on automatic exchange to display next slide.
 */
void    KbvSlideScene::slideChangeOver(void)
{
  qreal opaque;


  switch(settings->slideChangeId)
    {
      //Fade-in is done in several steps increasing the opacity of the new
      //slide and decreasing the opacity of the old one.
      case KbvConf::slideFadeIn:     //fade-in
        {
          opaque = currentSlide->opacity() + opaque_step;
          currentSlide->setOpacity(opaque);
          oldSlide->setOpacity(1.0 - opaque);
          if (opaque > 0.99)
            {
            //Change completely performed
            //On time controlled change start delay for next slide
              this->removeItem(oldSlide);
              delete oldSlide;
              changeBusy = false;
              if (!settings->slideChangeManual)
                {
                  timer1->start();
                }
            }
          else
            {
              timer2->start(Kbv::InsertTimer);
            }
          break;
        }
      //Insert left is done in x_step_count steps moving both slides right
      //by x_step pixel per step.
      case KbvConf::slideInsertLeft:
        {
          oldSlide->moveBy(x_step, 0.0);
          currentSlide->moveBy(x_step, 0.0);
          x_step_count--;
          if (x_step_count < 1)
            {
              //Change completely performed
              //On time controlled change start delay for next slide
              this->removeItem(oldSlide);
              delete oldSlide;
              changeBusy = false;
              if (!settings->slideChangeManual)
                {
                  timer1->start();
                }
            }
          else
            {
              timer2->start(Kbv::InsertTimer);
            }
          break;
        }
      //Insert right is done in x_step_count steps moving both slides left
      //by x_step pixel per step.
      case KbvConf::slideInsertRight:
        {
          oldSlide->moveBy(-x_step, 0.0);
          currentSlide->moveBy(-x_step, 0.0);
          x_step_count--;
          if (x_step_count < 1)
            {
              //Change completely performed
              //On time controlled change start delay for next slide
              this->removeItem(oldSlide);
              delete oldSlide;
              changeBusy = false;
              if (!settings->slideChangeManual)
                {
                  timer1->start();
                }
            }
          else
            {
              timer2->start(Kbv::InsertTimer);
            }
          break;
        }
    }
}
/*************************************************************************//*!
 * Store a pointer to a list of files and display the first one.
 * When the list is empty finish at once.
 * Memory of playlist is released by destructor.
 */
void    KbvSlideScene::startShow(QList<QPair<QString, QString> > list)
{
  //save playlist as long as the slide show is running
  playlist = new QList<QPair<QString, QString> >(list);

  if (playlist->isEmpty())
    {
      emit finish();
      return;
    }
  //qDebug() << "KbvSlideScene::startShow" <<playlist->length(); //###########
  
  //Set index on first item and display this
  currentIndex = 0;
  slidePreviousNext(Kbv::current);
}

/*************************************************************************//*!
 * Show next/previous or current item.\n
 * When the last or first item is reached the count will wrap around and
 * continue with the first respectively the last item. When the file is corrupt
 * then next one is selected.\n
 * In full screen mode the slide is loaded, scaled, centered and the selected
 * exchange function get started.\n
 * At last the timer 2 for dynamic change is started when this option is valid.
 * This timer performes a delay between subsequent steps of change. The velocity
 * of change depends on image size and insert steps only and is self adjusting.
 */
void    KbvSlideScene::slidePreviousNext(int previousnextcurrent)
{
  QString   filename;
  QPixmap   pixmap;
  QSize     size;
  qreal     dx, dy;
  int       next;

  if (changeBusy)
    {
      return;
    }
  //Select next/previous entry in play list. When this entry
  //can not be loaded the next/previous item of play list is used.
  //This is done using a loop over the playlist. The loop breakes on valid
  //pixmap but would step through a series of invalid items to find the
  //next valid pixmap.
  for (int i = 0; i < playlist->length(); ++i)
    {
      if(previousnextcurrent == Kbv::next)
        {
          next = currentIndex + 1;
          if(next >= playlist->length())
            {
              if(!settings->slideStopAtDirEnd)
                {
                  currentIndex = 0;
                }
            }
          else
            {
              currentIndex++;
            }
        }
      if(previousnextcurrent == Kbv::previous)
        {
          next = currentIndex - 1;
          if(next < 0)
            {
              if(!settings->slideStopAtDirEnd)
                {
                  currentIndex = playlist->length()-1;
                }
            }
          else
            {
              currentIndex--;
            }
         }
      //Load a pixmap, scale it up/down to desktop size
      filename = playlist->at(currentIndex).first + playlist->at(currentIndex).second;
      pixmap = QPixmap(filename, 0, Qt::AutoColor);
      if(!pixmap.isNull())
        {
          pixmap = adjustSize(pixmap);
          break;    //valid pixmap found
        }
    }
  QPixmapCache::clear();
  
  //qDebug() << "KbvSlideScene::slidePreviousNext" <<playlist->at(currentIndex); //###########

  if (settings->slideFullScreen)
    {
      //Load the slide, center it in scene
      //rectangle and start the selected exchange function.
      changeBusy = true;
      oldSlide = currentSlide;
      currentSlide = this->addPixmap(pixmap);   //item at scene x,y=0,0
      switch(settings->slideChangeId)
        {
        case KbvConf::slideReplace:     //slideReplace
          {
            size = currentSlide->pixmap().size();
            dx = 0.5* qreal(desktopSize.width() - size.width());
            dy = 0.5* qreal(desktopSize.height() - size.height());
            currentSlide->moveBy(dx, dy);
            this->removeItem(oldSlide);
            delete oldSlide;
            changeBusy = false; //ready for next
            break;
          }
        //Fade-in increases opacity of new slide from zero and decreases opacity
        //of old slide in same amount. This achieves a blending where at last the
        //old slide has gone and the new one is fully visible.
        case KbvConf::slideFadeIn:     //fade-in
          {
            size = currentSlide->pixmap().size();
            dx = 0.5* qreal(desktopSize.width() - size.width());
            dy = 0.5* qreal(desktopSize.height() - size.height());
            currentSlide->moveBy(dx, dy);
            currentSlide->setOpacity(0.0);
            opaque_step = qreal (1.0 / Kbv::InsertSteps);
            timer2->start();
            break;
          }
        //The scene rectangle is three times the screen width at same height.
        //The view displays the middle part. The new slide is inserted into
        //the invisible left part. New and old are shifted right till the new
        //is fully visible and the old one is in the invisible right part.
        case KbvConf::slideInsertLeft:     //insert left
          {
            size = currentSlide->pixmap().size();
            dx = 0.5* qreal(desktopSize.width() - size.width());
            dy = 0.5* qreal(desktopSize.height() - size.height());
            currentSlide->moveBy(dx, dy);                   //center in left part
            x_step_count = Kbv::InsertSteps;                //calculate step count and width
            x_step = qreal (desktopSize.width() / x_step_count);
            timer2->start();
            break;
          }
          //The scene rectangle is three times the screen width at same height.
          //The view displays the middle part. The new slide is inserted into
          //the invisible left part then moved to the invisible right part.
          //New and old are shifted left till he new one is fully and the old
          //one is in the invisible left part.
           case KbvConf::slideInsertRight:     //insert right
          {
            size = currentSlide->pixmap().size();
            dx = 0.5* qreal(desktopSize.width() - size.width());
            dy = 0.5* qreal(desktopSize.height() - size.height());
            currentSlide->moveBy(dx, dy);                           //center in left part
            currentSlide->moveBy(qreal (2*desktopSize.width()), 0); //move to right part
            x_step_count = Kbv::InsertSteps;                //calculate step count and width
            x_step = qreal (desktopSize.width() / x_step_count);
            timer2->start();
            break;
          }
        }
    }
  else
    {
      //Non full screen mode: slideReplace slide
      //Load a slide, scale it down to maximum desktop size if too large.
      //Send item path and name for title information
      changeBusy = true;
      this->removeItem(currentSlide);
      delete currentSlide;
      currentSlide = this->addPixmap(pixmap);   //item at scene x,y=0,0
      
      QRectF rectf(QPointF(0,0), (currentSlide->pixmap().size() *1.0));
      emit  (sceneRectChanged(rectf));
      filename = playlist->at(currentIndex).first + playlist->at(currentIndex).second;
      emit  (itemPathName("Imarca - " + filename));
      changeBusy = false; //ready for next
    }
  //On time controlled change, start delay for next slide
  if (!settings->slideChangeManual)
    {
      timer1->start();
    }

  //Show meta data in textInfo
  showMetaData(metaData);
}
/*************************************************************************//*!
 * Size pixmap to fit to screen.\n
 * - Full screen mode: the pixmap is stretched when the appropriate option is
 *   checked, all other cases shrink the pixmap to fit in desktop.
 * - Non full screen mode: if the pixmap including title bar and frame do not
 *   fit in screen size the pixmap is reduced (screen = desktop - task bar).
*/
QPixmap    KbvSlideScene::adjustSize(QPixmap pixmap)
{
  QSize       imageSize;
  double      h_rel, w_rel;
  int         w, h;

  //TODO: intelligente Berechnung des Fensterrahmens -> Methode des kbvImageEditors

  imageSize = pixmap.size();

  if (settings->slideFullScreen)
    {
      h_rel = (double) desktopSize.height() / (double) (imageSize.height());
      w_rel = (double) desktopSize.width() / (double) (imageSize.width());

      //width and height smaller than desktop, keep or stretch due to aspect ratio
      if (w_rel >= 1.0 && h_rel >= 1.0)
        {
          w = desktopSize.width();
          h = desktopSize.height();
          if (settings->slideStretch)
            {
              return pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
          return pixmap;
        }
      //width or height larger than desktop, shrink due to aspect ratio
      w = desktopSize.width();
      h = desktopSize.height();
      return pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  else
    {
      h_rel = (double) screenSize.height() / (double) (imageSize.height() + 24+4);
      w_rel = (double) screenSize.width() / (double) (imageSize.width() + 8);

      //width or height larger than screen size, shrink due to aspect ratio
      if (w_rel < 1.0 || h_rel < 1.0)
        {
          w = screenSize.width() - 8;
          h = screenSize.height() - 24-4;
          //qDebug() << "KbvSlideScene::adjustSize normal scale" << w << h; //###########
          return pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
      //width and height smaller than screen size
      //qDebug() << "KbvSlideScene::adjustSize normal keep" << pixmap.width() << pixmap.height(); //###########
      return pixmap;
    }
}
/****************************************************************************/
