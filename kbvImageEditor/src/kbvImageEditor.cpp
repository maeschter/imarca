/*****************************************************************************
 * kvb image editor
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.03.23
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvImageEditor.h"
#include "kbvImageEditorUndo.h"


KbvImageEditor::KbvImageEditor(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  
  this->setWindowTitle("Imarca");
  this->setWindowIcon(QIcon(":/icons/kbv-crown.png"));
  this->setMinimumSize(QSize(200, 200));
  
  this->mImageArea = new KbvImageLabel(this);
  this->mImageArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  this->mImageArea->setScaledContents(true);

  this->mScrollArea = new QScrollArea(this);
  this->mScrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  this->mScrollArea->setFrameShape(QFrame::NoFrame);
  this->mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->mScrollArea->setAlignment(Qt::AlignCenter);
  this->mScrollArea->setWidget(mImageArea);
  this->setCentralWidget(mScrollArea);
  
  this->mUndoStack = new QUndoStack(this);
  this->undoAct = mUndoStack->createUndoAction(this, tr("&Undo"));
  this->undoAct->setIcon(QIcon(":/icons/edit-undo.png"));
  this->undoAct->setShortcuts(QKeySequence::Undo);
  this->redoAct = mUndoStack->createRedoAction(this, tr("&Redo"));
  this->redoAct->setIcon(QIcon(":/icons/edit-redo.png"));
  this->redoAct->setShortcuts(QKeySequence::Redo);
  this->mUndoStack->setClean();

  this->createActions();
  this->createToolbars();
  this->createMenus();
  this->createInfoBar();

  this->mFileDialog = new KbvFileDialog(this, nullptr);
  this->batchDialog = new KbvBatchDialog(this);
  this->batchProgress = new KbvBatchProgress(this);
  this->batchThread = new KbvBatchThread(this);

  this->mReader = new KbvImageReader();
  this->mReader->setAutoDetectImageFormat(true);
  this->mWriter = new QImageWriter();

  this->mImage = new QImage();
  this->mQuestionBox = new KbvQuestionBox(this);
  
  mScaleFactor = 1.0;
  mJpegQuality = Kbv::jpegQualityStd;
  mTiffCompression = Kbv::tiffCompressLZW;
  mPngGamma = float(Kbv::pngGammaStd/Kbv::pngGammaScale);

  connect(mImageArea, SIGNAL(selectRectangle(const QRect)), this, SLOT(selectionRect(const QRect)));
  connect(mImageArea, SIGNAL(mousePosition(const QPoint)),  this, SLOT(mousePosition(const QPoint)));

  connect(batchThread,    SIGNAL(progressText(QString)),  batchProgress,  SLOT(setProgressText(QString)));
  connect(batchThread,    SIGNAL(progressValue(int)),     batchProgress,  SLOT(setProgressValue(int)));
  connect(batchThread,    SIGNAL(threadFinished(bool)),   batchProgress,  SLOT(finished(bool)));
  connect(batchProgress,  SIGNAL(rejected()),             batchThread,    SLOT(stopThread()));

  connect(batchThread,    SIGNAL(threadFinished(bool)),   this,  SLOT(updateImage()));
  connect(batchProgress,  SIGNAL(showReport()),           this,  SLOT(openBatchReport()));

  int   prim = mDesktop.primaryScreen();
  mDesktopSize = mDesktop.availableGeometry(prim);
  //qDebug() << "KbvImageEditor::desktop h, w" <<mDesktopSize.height() <<mDesktopSize.width(); //###########

  //ScrollBar.width (height) = scrollBar.extent + some margin (PM_DefaultFrameWidth ???)
  //the additional margin is transparent and shows window background
  mScrollBarWidth = mScrollArea->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, nullptr) +
                 2*mScrollArea->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, nullptr, nullptr);
  
}

KbvImageEditor::~KbvImageEditor()
{
  //qDebug() << "KbvImageEditor::~kbvImageEditor"; //###########
  delete    mReader;
  delete    mWriter;
  delete    mImage;
}

/*************************************************************************//*!
 * Set files for editing in combobox.
 * Populate the combobox in fileToolbar with content of "files" (path,name).
 * The first file is the candidate to load and actualises members.
 */
bool  KbvImageEditor::setFiles(const QList<QPair<QString, QString> > &files)
{
  this->mFileCombo->clear();
  if(!files.empty())
    {
      for(int i=0; i<files.length(); i++)
        {
          //text = file name = second, userData = file path = first
          mFileCombo->addItem(files.at(i).second, QVariant(files.at(i).first));
        }
      mFileCombo->setItemIcon(0, QIcon(":/icons/kbv-triangle-right.png"));
      mFileCombo->setCurrentIndex(0);
      
      mCurrentFile = mFileCombo->currentText();
      mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"
      //qDebug() << "KbvImageEditor::setFiles" <<mCurrentPath <<mCurrentFile; //###########

      mCurrentAltered = false;
      mCurrentIndex = 0;
      mCurrentDBName = "";
      mCurrentDBPath = "";
      mCurrentPK = 0;
      return true;
    }
  else
    {
      return false;
    }
}
/*************************************************************************//*!
 * SLOT: Batch process a list of files stored in fileCombo.
 * This is called from fileView, collectionView or searchView and performs a
 * bunch of modifications on each image in the files list.
 */
void KbvImageEditor::startBatchProcess()
{
  int           ret;

  ret = this->batchDialog->exec(mBatchParams);
  if(ret == Kbv::apply)
    {
      this->batchProgress->open("");
      this->mFileCombo->setCurrentIndex(0);

      this->batchThread->startThread(mFileCombo, mBatchParams);
    }
}
/*************************************************************************//*!
 * SLOT: Finish batch process when the dialog "batchProgress" get closed.
 * We stop the process regardless of it's state. When the process has finished
 * by itself, a message will be shown for the user to close the dialog.
 * When the process is still running we stop it.
 */
void   KbvImageEditor::endBatchProcess()
{
  this->batchThread->stopThread();
  //qDebug() << "KbvImageEditor::stopBatchProcess finished"; //###########
}
/*************************************************************************//*!
 * SLOT: update current image with the first one of the comboBox.
 * This is called at the end of the batchThread.
 */
void  KbvImageEditor::updateImage()
{
  mFileCombo->setCurrentIndex(0);
  mCurrentFile = mFileCombo->currentText();
  mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"
  this->loadImage(mCurrentPath, mCurrentFile);
}
/*************************************************************************//*!
 * SLOT: Opens the report file of last batch image modification.
 */
void  KbvImageEditor::openBatchReport()
{
  QFileInfo         info;
  QStringList       paramsList;
  QString           path;


  path = QDir::homePath() + QString(configDir) + QString(imageEditorBatchLog);

  info.setFile(path);
  //qDebug() << "KbvImageEditor::openBatchReport" <<path; //##########
  if(info.exists())
    {
      paramsList.append(path);
      generalFunc.openApplication("text/plain", paramsList);
    }
}

/*************************************************************************//*!
 * Load image.
 * This is called from fileView, collectionView and searchView. Model pointer
 * modelindex and database name are required when the image gets altered by
 * the editor and the related models or databases have to be updated.
 * See method save();
 * Important: the parent of each image either is the fileView, the searchView
 * or the collView. So the pointer to the model is valid as long as the view
 * exists. 
 */
/*
void KbvImageEditor::loadImage(QAbstractListModel *model, const QModelIndex index, const QString databaseName)
{
  fileModel = 0;
  collModel = 0;
  searchModel = 0;
  this->mCurrentPK = 0;
  this->mCurrentFile.clear();
  this->mCurrentPath.clear();
  this->mCurrentDBName.clear();
  
  //load image, in case of filemodel databaseName and primaryKey aren't relevant
  if(index.isValid())
    {
      do    //no nested if..then..else
        {
          collModel = dynamic_cast<KbvCollectionModel*> (model);
          if(collModel)
            {
              this->mCurrentFile = collModel->data(index, Kbv::FileNameRole).toString();
              this->mCurrentPath = collModel->data(index, Kbv::FilePathRole).toString();
              this->mCurrentPK = collModel->data(index, Kbv::PrimaryKeyRole).toInt();
              this->mCurrentDBName = databaseName;
              break;
            }
          searchModel = dynamic_cast<KbvSearchModel*> (model);
          if(searchModel)
            {
              this->mCurrentFile = searchModel->data(index, Kbv::FileNameRole).toString();
              this->mCurrentPath = searchModel->data(index, Kbv::FilePathRole).toString();
              this->mCurrentPK = searchModel->data(index, Kbv::PrimaryKeyRole).toInt();
              this->mCurrentDBName = databaseName;
              break;
            }
          fileModel = dynamic_cast<KbvFileModel*> (model);
          if(fileModel)
            {
              this->mCurrentFile = fileModel->data(index, Kbv::FileNameRole).toString();
              this->mCurrentPath = fileModel->data(index, Kbv::FilePathRole).toString();
              break;
            }
        }
      while(false);
      //qDebug() << "KbvImageEditor::loadImage model" <<fileModel <<collModel <<searchModel; //###########
      this->loadImage(mCurrentPath, mCurrentFile);
    }
}
*/
/*************************************************************************//*!
 * Load image if possible.
 * If loading fails the previous image is still present (if any was loaded
 * before) and all collected image parameters are still valid.
 * Path already ends with "/". Metadata get read and stored in an object.
 * This is called from open() and from kbvMain (command line).
 */
void KbvImageEditor::loadImage(const QString path, const QString name)
{
  mUndoStack->clear();  //clear undo stack

  mReader->setFileName(path + name);
  mImageFormat = mReader->format();

  if (mImageFormat == "jpeg")
    {
      mJpegQuality = mReader->quality();
      //qDebug() << "KbvImageEditor::loadImage"<<path <<name <<mImageFormat <<mJpegQuality; //###########
    }
  if (mImageFormat == "png")
    {
      mPngGamma = mReader->gamma();
      //qDebug() << "KbvImageEditor::loadImage"<<path <<name <<mImageFormat <<mPngGamma; //###########
    }
  if (mImageFormat == "tiff")
    {
      mTiffCompression = 1;     //1=LZW, 0=none
      qDebug() << "KbvImageEditor::loadImage"<<path <<name <<mImageFormat; //###########
    }

  mReader->read(mImage);
  if (!mImage->isNull())
    {
      if(mMetaData.loadFromFile(path+name))
        {
          if(mMetaData.hasXmp())
            {
              mMetaData.clearXmp();               //XMP not supported <<<<<<<<<<<<<<<<<<<<<
            }
        }
      adjustAndShow(mImage);
    }
  else
    {
      //question with close button only
      mQuestionBox->exec(QString(tr("Cannot load: %1.").arg(name)), QString(""));
    }
  }
/*************************************************************************//*!
 * Display image and size window to fit to image or screen.
 */
void KbvImageEditor::adjustAndShow(QImage *image)
  {
    QSize   centralSize, deltaSize;
    int     w=0, h=0;
    bool    imageHeightLarger, imageWidthLarger;

    //Resize window to desktop size and hide.
    //This delivers the maximum possible size of centralwidget
    this->mImageArea->clear();
    this->show();
    this->resize(mDesktopSize.width(), mDesktopSize.height());
    this->hide();
    centralSize = centralWidget()->size();
    
    //Calculate window size to fit exactly to given image
    mImageArea->setPixmap(QPixmap::fromImage(*image));
    mImageArea->adjustSize();
    mImageSize = image->rect();
    deltaSize = centralSize - mImageArea->size();
    //qDebug() << "KbvImageEditor::adjustAndShow area w h" <<mImageArea->width() <<mImageArea->height(); //###########
    //qDebug() << "KbvImageEditor::adjustAndShow central w h" <<centralSize.width() <<centralSize.height(); //###########
    
    //Step 1. When one image dimension is less than minimum dimension:
    //set minimum window size and center imageArea. This prevents the window
    //getting smaller in step 2.
    if((mImageSize.height() <= KbvConf::viewerMinHeight) | (mImageSize.width() <= KbvConf::viewerMinWidth))
      {
        this->setMinimumHeight(KbvConf::viewerMinHeight);
        mImageArea->setAlignment(Qt::AlignVCenter);
        this->setMinimumWidth(KbvConf::viewerMinWidth);
        mImageArea->setAlignment(Qt::AlignHCenter);
      }

    //Step 2. Set flags due to relation imagesize/desktopsize.
    //Calculate the necessary window size to enclose the image without gap.
    imageHeightLarger = (deltaSize.height() >= 0) ? false : true;
    imageWidthLarger  = (deltaSize.width() >= 0) ? false : true;

    //width and height are smaller or equal than central widget
    //window size = desktop size - selta size
    if (!imageWidthLarger && !imageHeightLarger)
      {
        w = mDesktopSize.width() - deltaSize.width();
        h = mDesktopSize.height() - deltaSize.height();
      }
    //width is smaller or equal and height is larger than central widget
    //enable vertical scrollbar
    if (!imageWidthLarger && imageHeightLarger)
      {
        w = mDesktopSize.width() - deltaSize.width() + mScrollBarWidth;
        h = mDesktopSize.height();
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      }
    //width is larger and height is smaller or equal than central widget
    //enable horizontal scrollbar
    if (imageWidthLarger && !imageHeightLarger)
      {
        w = mDesktopSize.width();
        h = mDesktopSize.height() - deltaSize.height() + mScrollBarWidth;
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      }
    //width and height are larger than central widget, enable both scrollbars
    //window not maximized, otherwise cropping and rotating doesn't work properly
    if (imageWidthLarger && imageHeightLarger)
      {
        w = mDesktopSize.width();
        h = mDesktopSize.height();
        mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        this->setWindowState(Qt::WindowNoState);
      }
    //resizesing of the widget excludes any window frame
    this->move(0, 0);
    this->show();
    this->resize(w, h);
    this->activateWindow();

    //infoLabel 1&2 are used by rubberband;
    updateInfoLabel(0, QString("%1,%2").arg(mImageSize.width()).arg(mImageSize.height()));
    updateInfoLabel(3, QString("%L1%").arg(100*mScaleFactor, 0, 'f', 1));
    updateInfoLabel(4, (mCurrentPath+mCurrentFile));
    //qDebug() << "KbvImageEditor::adjustAndShow window w h" <<w <<h; //###########
  }

/*************************************************************************//*!
 * SLOT: selection in file combobox changed. We use signal activated() since
 * it is not sent when the combobox gets cleared or new items get inserted.
 * The selected file in the combobox will be mark by a triangle.
 * The undo stack will be cleared in function loadImage().
 */
void  KbvImageEditor::comboBoxOpen(int index)
{
  int   ret;
  
  //qDebug() << "KbvImageEditor::imageToOpen new old" <<index <<mCurrentIndex; //###########
  if(index != mCurrentIndex)
    {
      if(mCurrentAltered)
        {
          //image altered but not saved - open save dialog
          ret =  mQuestionBox->exec(QString(tr("The image has been modified")),
                                   QString(tr("If you don't save the image all changes get lost")),
                                   QString(tr("Close without saving")), QString(tr("Cancel")), QString(tr("Save")));

          if((ret == Kbv::save) | (ret == Kbv::discard))
            {
              if(ret == Kbv::save)
                {
                  this->menuSaveAs();
                }
              //read selected image file
              mCurrentFile = mFileCombo->currentText();
              mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"

              //remove icon at old index and insert one at new index
              mFileCombo->setItemIcon(mCurrentIndex, QIcon());
              mFileCombo->setItemIcon(index, QIcon(":/icons/kbv-triangle-right.png"));
              mCurrentIndex = index;
              mCurrentAltered = false;
              mCurrentDBName = "";
              mCurrentDBPath = "";
              mCurrentPK = 0;
              this->loadImage(mCurrentPath, mCurrentFile);
            }
        }
      else
        {
          //image not modified
          mCurrentFile = mFileCombo->currentText();
          mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"

          //remove icon at old index and insert one at new index
          mFileCombo->setItemIcon(mCurrentIndex, QIcon());
          mFileCombo->setItemIcon(index, QIcon(":/icons/kbv-triangle-right.png"));
          mCurrentIndex = index;
          mCurrentAltered = false;
          mCurrentDBName = "";
          mCurrentDBPath = "";
          mCurrentPK = 0;
          this->loadImage(mCurrentPath, mCurrentFile);
        }
    }
}
/*************************************************************************//*!
 * SLOT: menu file|open. Creates a modal file dialog.
 * The first of the selected files will be loaded and marked by a triangle in
 * the combobox.
 * The undo stack will be cleared in function loadImage().
 */
void KbvImageEditor::menuOpen()
{
  QStringList strlist;
  QString     str;
  int         ret, n;


  mFileDialog->setWindowTitle(tr("Open file"));
  mFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
  mFileDialog->setDirectory(mCurrentPath);

  //qDebug() << "KbvImageEditor::menuOpen"; //###########
  if(mCurrentAltered)
    {
      //image altered but not saved - open save dialog
      ret =  mQuestionBox->exec(QString(tr("The image has been modified")),
                               QString(tr("If you don't save the image all changes get lost")),
                               QString(tr("Close without saving")), QString(tr("Cancel")), QString(tr("Save")));

      //do nothing on QMessageBox::Cancel
      if((ret == Kbv::save) | (ret == Kbv::discard))
        {
          if(ret == Kbv::save)
            {
              this->menuSave();
            }
          if(mFileDialog->exec() == QDialog::Accepted)
            {
              strlist = mFileDialog->selectedFiles();
              if(!strlist.empty())
                {
                  //qDebug() << "KbvImageEditor::menuOpen modified" <<strlist; //###########
                  mFileCombo->clear();
                  for(int i=0; i<strlist.length(); i++)
                    {
                      n = strlist.at(i).lastIndexOf("/");
                      str = strlist.at(i).right(strlist.at(i).length() - n - 1);
                      //text = file name, userData = file path
                      mFileCombo->addItem(str, QVariant(strlist.at(i).left(n+1)));
                    }
                  mCurrentIndex = 0;
                  mFileCombo->setCurrentIndex(0);
                  mFileCombo->setItemIcon(0, QIcon(":/icons/kbv-triangle-right.png"));
                  mCurrentFile = mFileCombo->currentText();
                  mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"
                  //qDebug() << "KbvImageEditor::menuOpen modified" <<mCurrentPath <<mCurrentFile; //###########
                  loadImage(mCurrentPath, mCurrentFile);
                }
            }
        }
    }
  else
   {
      //image not modified
      if(mFileDialog->exec() == QDialog::Accepted)
        {
          strlist = mFileDialog->selectedFiles();
          if(!strlist.empty())
            {
              //qDebug() << "KbvImageEditor::menuOpen not modified" <<strlist; //###########
              mFileCombo->clear();
              for(int i=0; i<strlist.length(); i++)
                {
                  n = strlist.at(i).lastIndexOf("/");
                  str = strlist.at(i).right(strlist.at(i).length() - n - 1);
                  //text = file name, userData = file path
                  mFileCombo->addItem(str, QVariant(strlist.at(i).left(n+1)));
                }
              mCurrentIndex = 0;
              mFileCombo->setCurrentIndex(0);
              mFileCombo->setItemIcon(0, QIcon(":/icons/kbv-triangle-right.png"));
              mCurrentFile = mFileCombo->currentText();
              mCurrentPath = mFileCombo->currentData().toString();    //including trailing "/"
              //qDebug() << "KbvImageEditor::menuOpen not modified" <<mCurrentPath <<mCurrentFile; //###########
              loadImage(mCurrentPath, mCurrentFile);
            }
        }
   }
}
/*************************************************************************//*!
 * SLOT: menu file|save. The altered image overwrites the origin.
 * If the metadata object contains data they get written back to the image and
 * the tag Exif.Image.ProcessingSoftware will be updated with "Imarca".
 * The model (from which the image was displayed) and the database will be
 * updated either by fileWatchThread or collectionWatchThread.
 * The undo stack will be cleared in function loadImage().
 */
void KbvImageEditor::menuSave()
{
  //set parameters to original values
  mWriter->setFormat(mImageFormat);
  if (mImageFormat == "jpg")
    {
      mWriter->setQuality(mJpegQuality);
    }
  if (mImageFormat == "tif")
    {
      mWriter->setCompression(mTiffCompression);
    }
  if (mImageFormat == "png")
    {
      mWriter->setGamma(mPngGamma);
      mWriter->setCompression(Kbv::pngCompressStd);
    }
  mWriter->setFileName(mCurrentPath + mCurrentFile);
  //qDebug() << "KbvImageEditor::menuSave" <<mCurrentFile; //###########

  if(mWriter->write(*mImage))
    {
      createImarcaExifInfo();
      mMetaData.writeToFile(mCurrentPath + mCurrentFile);
      mCurrentAltered = false;
      emit  fileChanged(mCurrentPath);
    }
  else
    {
      //question with close button only
      mQuestionBox->exec(QString(tr("Cannot save: %1.").arg(mCurrentFile)), QString(""));
    }
}
/*************************************************************************//*!
 * SLOT: menu file|save as.
 * If the metadata object contains data they get written back to the image and
 * the tag Exif.Image.ProcessingSoftware will be updated with "Imarca".
 * The undo stack will be cleared in function loadImage().
 */
void KbvImageEditor::menuSaveAs()
{
  QStringList strlist;
  QString     str;
  int         idx;

  //Prepare dialog
  mFileDialog->setWindowTitle(tr("Save as ..."));
  mFileDialog->setAcceptMode(QFileDialog::AcceptSave);
  mFileDialog->setDirectory(mCurrentPath);

  //Get file name without extension,
  //mImageFormat contains the new format
  idx = mCurrentFile.lastIndexOf(".");
  if(idx > 0)
    {
      str = mCurrentFile.left(idx);
    }
  else
    {
      str = mCurrentFile;
    }

  if (mImageFormat == "jpg")
    {
      str = str + ".jpg";
    }
  if (mImageFormat == "tif")
    {
      str = str + ".tif";
    }
  if (mImageFormat == "png")
    {
      str = str + ".png";
    }
  if (mImageFormat == "xpm")
    {
      str = str + ".xpm";
    }
  if (mImageFormat == "bmp")
    {
      str = str + ".bmp";
    }
  //Preset dialog values
  mFileDialog->setFileTypeParams(str, mJpegQuality, mTiffCompression, mPngGamma);

  //Save as ... dialog
  if(mFileDialog->exec() == QDialog::Accepted)
    {
      strlist = mFileDialog->selectedFiles();
      if(!strlist.first().isEmpty())
        {
          str = strlist.first();

          int format = mFileDialog->getFileType();
          switch (format)
            {
              case Kbv::jpeg:
                {
                  mWriter->setFormat("jpg");
                  mWriter->setQuality(mFileDialog->getJpegQuality());
                  qDebug() << "KbvImageEditor::menuSaveAs jpg" <<str <<mFileDialog->getJpegQuality(); //###########
                  break;
                }
              case Kbv::tiff:
                {
                  mWriter->setFormat("tif");
                  mWriter->setCompression(mFileDialog->getTiffCompression());
                  qDebug() << "KbvImageEditor::menuSaveAs tif" <<str <<mFileDialog->getTiffCompression(); //###########
                  break;
                }
              case Kbv::png:
                {
                  mWriter->setFormat("png");
                  mWriter->setGamma(mFileDialog->getPngGamma());
                  mWriter->setCompression(mFileDialog->getPngCompression());
                  qDebug() << "KbvImageEditor::menuSaveAs png" <<str <<mFileDialog->getPngGamma() <<mFileDialog->getPngCompression(); //###########
                  break;
                }
            case Kbv::xpm:
              {
                mWriter->setFormat("xpm");
                qDebug() << "KbvImageEditor::menuSaveAs png" <<str; //###########
               break;
              }
            case Kbv::bmp:
              {
                mWriter->setFormat("bmp");
                qDebug() << "KbvImageEditor::menuSaveAs png" <<str; //###########
               break;
              }
            default:
              {
                mWriter->setFormat("bmp");
                str = QString("error-no-format-use-bmp");
                qDebug() << "KbvImageEditor::menuSaveAs error" <<str; //###########
               break;
              }
            }

          mWriter->setFileName(str);
          if(mWriter->write(*mImage))
            {
              if(mFileDialog->keepMetadata())
                {
                  createImarcaExifInfo();
                  mMetaData.writeToFile(str);
                }
              mCurrentAltered = false;
              emit  fileChanged(str);
            }
          else
            {
              //question with close button only
              mQuestionBox->exec(QString(tr("Cannot save: %1.").arg(mWriter->errorString())), QString(""));
            }
        }
    }
}
/*************************************************************************//*!
 * SLOT: menu file|print.
 */
void KbvImageEditor::menuPrint()
{
  Q_ASSERT(mImageArea->pixmap());
#ifndef QT_NO_PRINTER
  QPrintDialog dialog(&printer, this);
  if (dialog.exec())
    {
      QPainter painter(&printer);
      QRect rect = painter.viewport();
      QSize size = mImageArea->pixmap()->size();
      size.scale(rect.size(), Qt::KeepAspectRatio);
      painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
      painter.setWindow(mImageArea->pixmap()->rect());
      painter.drawPixmap(0, 0, *mImageArea->pixmap());
    }
#endif
}
/*************************************************************************//*!
 * SLOT: Quit application on ESC-key.
 */
void  KbvImageEditor::keyPressEvent(QKeyEvent * event)
  {
    if (event->key() == Qt::Key_Escape)
      {
        this->close();
      }
 }
/*************************************************************************//*!
 * SLOT: Quit application on menu file|exit or ctrl-q or ESC or close button
 * but first ask for saving a modified image.
 */
void  KbvImageEditor::closeEvent(QCloseEvent *event)
{
  int ret;
  
  if(mCurrentAltered)
    {
      //image altered but not saved - open save dialog
      ret = mQuestionBox->exec(QString(tr("The image has been modified")),
                              QString(tr("If you don't save the image all changes get lost")),
                              QString(tr("Close without saving")), QString(tr("Cancel")), QString(tr("Save")));

      //do nothing on QMessageBox::Cancel
      if((ret == Kbv::save) | (ret == Kbv::discard))
        {
          if(ret == Kbv::save)
            {
              this->menuSave();
            }
          //read selected image file
          
          mCurrentAltered = false;
          mCurrentIndex = 0;
          mCurrentDBName = "";
          mCurrentDBPath = "";
          mCurrentPK = 0;
          event->accept();        //action_quit
        }
      else
        {
          event->ignore();
        }
    }
  else
    {
      event->accept();
    }
}

/*************************************************************************//*!
 * SLOT: Track selection rectangle.
 * Location of start point is shown in info label 1 while dimensions of
 * selection rect are displayed in info label 2.
 */
void  KbvImageEditor::selectionRect(QRect rect)
  {
    QString     strRect, strSize;

    mSelectedRect = rect;

    //update infolabel 1 with start point of selection rectangle
    strRect = "(";
    strRect.append(QString("%L1").arg(rect.topLeft().x(), 0));
    strRect.append(",");
    strRect.append(QString("%L1").arg(rect.topLeft().y(), 0));

    //update infolabel 2 with size of selection rectangle
    strSize = QString("%L1").arg(rect.width(), 0);
    strSize.append("x");
    strSize.append(QString("%L1").arg(rect.height(), 0));

    updateInfoLabel(1, strRect);
    updateInfoLabel(2, strSize);
  }

/*************************************************************************//*!
 * SLOT: Track mouse movement and scroll.
 * When the mouse position exceeds the bounds of central widget scrolling the
 * imagelabel is performed if necessary. Pos is given in imagelabel coordinates.
 * Here we need to check the position inside the visible part of central widget.
 * Left upper corner of a scaled image can be located at negative coordinates
 * when it was scrolled!
 */
void  KbvImageEditor::mousePosition(const QPoint position)
  {
    int         val;
    QRect       geo;
    QScrollBar  *hScroll, *vScroll;

    hScroll = mScrollArea->horizontalScrollBar();
    vScroll = mScrollArea->verticalScrollBar();
    geo = mImageArea->visibleRegion().boundingRect();
    //scroll left
    if(position.x() <= geo.x())
      {
        val = hScroll->value() - hScroll->singleStep();
        hScroll->setValue(val > hScroll->minimum() ? val : hScroll->minimum());
      }
    //scroll up
    if(position.y() <= geo.y())
      {
        val = vScroll->value() - vScroll->singleStep();
        vScroll->setValue(val > vScroll->minimum() ? val : vScroll->minimum());
      }
    //scroll right
    if(position.x() >= (geo.x() + geo.width()))
      {
        val = hScroll->value() + hScroll->singleStep();
        hScroll->setValue(val < hScroll->maximum() ? val : hScroll->maximum());
      }
    //scroll down
    if(position.y() >= (geo.y() + geo.height()))
      {
        val = vScroll->value() + vScroll->singleStep();
        vScroll->setValue(val < vScroll->maximum() ? val : vScroll->maximum());
      }
  }
/*************************************************************************//*!
 * SLOT: zoom into image in steps of 20%. Zoom is limited to 300%.
 */
void KbvImageEditor::zoomIn()
  {
    mScaleFactor += 0.25;
    mScaleFactor = (mScaleFactor > 3.0) ? 3.0 : mScaleFactor;
    scaleImage();
    updateInfoLabel(3, QString("%L1%").arg(100*mScaleFactor, 0, 'f', 1));
  }
/*************************************************************************//*!
 * SLOT: zoom out of image in steps of -20%. Zoom is limited to 20%.
 */
void KbvImageEditor::zoomOut()
  {
    mScaleFactor -= 0.25;
    mScaleFactor = (mScaleFactor < 0.25) ? 0.25 : mScaleFactor;
    scaleImage();
    updateInfoLabel(3, QString("%L1%").arg(100*mScaleFactor, 0, 'f', 1));
  }
/*************************************************************************//*!
 * SLOT: reset zoom to 100%.
 */
void KbvImageEditor::zoomNormal()
  {
    mScaleFactor = 1.0;
    scaleImage();
    updateInfoLabel(3, QString("%L1%").arg(100*mScaleFactor, 0, 'f', 1));
  }

/*************************************************************************//*!
 * Scale image due to zoom factor and keep the focal point to centre of
 * visible image area. Only the imagearea is scaled, the image itself is not
 * affected.
 */
void KbvImageEditor::scaleImage()
  {
    Q_ASSERT(mImageArea->pixmap());

    float       fpx, fpy;
    int         dx, dy;

    //Focal point = mid of visible part of image relative to image
    mVisibleRect = mImageArea->visibleRegion().boundingRect();
    fpx = float ((mVisibleRect.x() + 0.5*mVisibleRect.width()) / mImageArea->width());
    fpy = float ((mVisibleRect.y() + 0.5*mVisibleRect.height()) / mImageArea->height());

    //Scale: the pixmap keeps its original size but the image area gets resized
    mImageArea->resize(mScaleFactor * mImageArea->pixmap()->size());

    //Set parameters of horizontal scrollbar:
    //min = 0, max = image.width - visiblerectangle.width, pagestep = visiblerectangle.width
    //value = offset to shift. Offset to shift is calculated relative to new image size
    if (mImageArea->width() > mVisibleRect.width())
      {
        dx = int(fpx*mImageArea->width()-0.5*mVisibleRect.width());
        mScrollArea->horizontalScrollBar()->setRange(0, mImageArea->width()-mVisibleRect.width());
        mScrollArea->horizontalScrollBar()->setPageStep(mVisibleRect.width());
        mScrollArea->horizontalScrollBar()->setValue(dx);
      }
    if (mImageArea->height() > mVisibleRect.height())
      {
        dy = int(fpy*mImageArea->height()-0.5*mVisibleRect.height());
        mScrollArea->verticalScrollBar()->setRange(0, mImageArea->height()-mVisibleRect.height());
        mScrollArea->verticalScrollBar()->setPageStep(mVisibleRect.height());
        mScrollArea->verticalScrollBar()->setValue(dy);
      }
  }
/*************************************************************************//*!
 * Crop image due to selection rectangle. The image is altered.
 * Cropping only is performed when the image is unscaled (original size) or
 * the selection rectangle is larger than 16x16 pixel.
 */
void  KbvImageEditor::imageCrop()
  {
    if (mScaleFactor != 1.0)
      {
        //question with close button only
        mQuestionBox->exec(QString(tr("Image is zoomed!")),
                          QString(tr("Result is unpredicted.\nPlease adjust to original size before cropping.")));
        return;
      }
    if ((mSelectedRect.height() < 16) || (mSelectedRect.width() < 16))
      {
        //question with close button only
        mQuestionBox->exec(QString(tr("Crop image")),
                          QString(tr("Selected area is too small!\nMinimum selection is 16x16.")));
        return;
      }
    mUndoStack->push(new KbvImageCrop(this, mImage, mSelectedRect));
    mCurrentAltered = true;
  }
/*************************************************************************//*!
 * Rotation left 90° and right 90° uses bilinear filtering. The function
 * always returns an image of original size. The image is altered.
 */
void  KbvImageEditor::imageRotLeft()
  {
    imageRotate(-90);
  }
void  KbvImageEditor::imageRotRight()
  {
    imageRotate(90);
  }
void  KbvImageEditor::imageRotate(qreal angle)
  {
    QTransform  matrix;

    matrix.translate(0, 0);
    matrix.rotate(angle);
    matrix.scale(1.0, 1.0);

    mScaleFactor = 1.0;
    mUndoStack->push(new KbvImageRotate(this, mImage, matrix));
    mCurrentAltered = true;
  }
/*************************************************************************//*!
 * Rotation 180°, flip vertically and flip horizontically are done with the
 * mirrored method of QImage. The function always returns an image of original
 * size. The image is altered.
 */
void  KbvImageEditor::imageRot180()
  {
    imageFlip(true, true);
  }
void  KbvImageEditor::imageFlipVert()
  {
    imageFlip(false, true);
  }
void  KbvImageEditor::imageFlipHor()
  {
    imageFlip(true, false);
  }
void  KbvImageEditor::imageFlip(bool hor, bool ver)
  {
    mScaleFactor = 1.0;
    mUndoStack->push(new KbvImageFlip(this, mImage, hor, ver));
    mCurrentAltered = true;
  }
/*************************************************************************//*!
 * Prepare a subset of exif tags when the image get saved by Imarca.
 */
void  KbvImageEditor::createImarcaExifInfo()
{
  QString text, gpsdate, gpstime;
  QString description = "", comment = "";
  double  alt=0.0, lat=0.0, lon=0.0;
  bool    gps=false;

  if(mMetaData.hasExif())
    {
      description = mMetaData.getExifDescription();
      comment = mMetaData.getExifComment();
      gpsdate = mMetaData.getGPSDateString();
      gpstime = mMetaData.getGPSTimeString();
      gps = mMetaData.getGPSCoordinates(alt, lat, lon);
      mMetaData.clearExif();
    }
  //qDebug() << "KbvImageEditor::createImarcaExifInfo gps" <<gps <<gpsdate <<gpstime <<lon <<lat <<alt; //###########
  text = QString(appName);
  text.append(" ");
  text.append(QString(appVersion));
  mMetaData.setExifTagDataString("Exif.Image.ProcessingSoftware", text);

  mMetaData.setExifDateTime("Exif.Image.DateTime", QDateTime::currentDateTime());
  mMetaData.setExifTagDataLong("Exif.Image.ImageWidth", mImage->size().width());
  mMetaData.setExifTagDataLong("Exif.Image.ImageLength", mImage->size().height());
  mMetaData.setExifCommentDescription(comment, description);
  if(gps)
    {
      mMetaData.setGPSCoordinates(alt, lat, lon);
      mMetaData.setGPSDateTimeStamp(gpsdate, gpstime);
    }
}
/*************************************************************************//*!
 * SLOT: remove selection rectangle
 */
void  KbvImageEditor::removeSelectionRect()
{
  mImageArea->removeSelectionRect();
}
/*************************************************************************//*!
 * SLOT: set value to infoLabel 0-4
 */
void  KbvImageEditor::updateInfoLabel(int label, QString value)
{
  switch (label)
    {
      case 0:   mInfoLabel0->setText(value);    //Image size
      break;
      case 1:   mInfoLabel1->setText(value);    //Mouse position on image
      break;
      case 2:   mInfoLabel2->setText(value);    //Size of selection rectangle
      break;
      case 3:   mInfoLabel3->setText(value);    //Zoom
      break;
      case 4:   mInfoLabel4->setText(value);    //File path and name
      break;
    }
}
/*************************************************************************//*!
 * Display image path/name in titlebar
 */
void KbvImageEditor::setTitle(QString mCurrentPath)
  {
    QString str = "Imarca - ";
    str += mCurrentPath;
    this->setWindowTitle(str);
  }
/*****************************************************************************
 */
void KbvImageEditor::createActions()
  {
    openAct = new QAction(QIcon(":/icons/folder.png"), tr("Open"), this);
    openAct->setShortcut(QKeySequence(tr("Ctrl+O")));
    connect(openAct, SIGNAL(triggered()), this, SLOT(menuOpen()));

    saveAct = new QAction(QIcon(":/icons/document-save.png"), tr("Save"), this);
    saveAct->setShortcut(QKeySequence(tr("Ctrl+S")));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(menuSave()));

    saveAsAct = new QAction(QIcon(":/icons/document-save-as.png"), tr("Save &as..."), this);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(menuSaveAs()));

    printAct = new QAction(QIcon(":/icons/document-print.png"), tr("Print"), this);
    printAct->setShortcut(QKeySequence(tr("Ctrl+P")));
    printAct->setEnabled(false);
    connect(printAct, SIGNAL(triggered()), this, SLOT(menuPrint()));

    exitAct = new QAction(QIcon(":/icons/app-exit.png"), tr("Quit"), this);
    exitAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    zoomInAct = new QAction(QIcon(":/icons/zoom-in.png"), tr("Zoom In (20%)"), this);
    zoomInAct->setShortcut(QKeySequence(tr("Ctrl++")));
    zoomInAct->setEnabled(true);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/icons/zoom-out.png"), tr("Zoom Out (20%)"), this);
    zoomOutAct->setShortcut(QKeySequence(tr("Ctrl+-")));
    zoomOutAct->setEnabled(true);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    zoomNormalAct = new QAction(QIcon(":/icons/zoom-original.png"), tr("Normal size"), this);
    zoomNormalAct->setShortcut(QKeySequence(tr("Ctrl+0")));
    zoomNormalAct->setEnabled(true);
    connect(zoomNormalAct, SIGNAL(triggered()), this, SLOT(zoomNormal()));

    cropAct = new QAction(QIcon(":/icons/object-crop.png"), tr("Crop to selection"), mImageArea);
    cropAct->setShortcut(QKeySequence(tr("Shift+Alt+Del")));
    cropAct->setEnabled(true);
    connect(cropAct, SIGNAL(triggered()), this, SLOT(imageCrop()));

    rotLeftAct = new QAction(QIcon(":/icons/object-rotate-left.png"), tr("Rotate left 90 deg"), mImageArea);
    rotLeftAct->setShortcut(QKeySequence(tr("Shift+Alt+Left")));
    rotLeftAct->setEnabled(true);
    connect(rotLeftAct, SIGNAL(triggered()), this, SLOT(imageRotLeft()));

    rotRightAct = new QAction(QIcon(":/icons/object-rotate-right.png"), tr("Rotate right 90 deg"), mImageArea);
    rotRightAct->setShortcut(QKeySequence(tr("Shift+Alt+Right")));
    rotRightAct->setEnabled(true);
    connect(rotRightAct, SIGNAL(triggered()), this, SLOT(imageRotRight()));

    rot180Act = new QAction(QIcon(":/icons/object-rotate-180deg.png"), tr("Rotate 180 deg"), mImageArea);
    rot180Act->setShortcut(QKeySequence(tr("Shift+Alt+Down")));
    rot180Act->setEnabled(true);
    connect(rot180Act, SIGNAL(triggered()), this, SLOT(imageRot180()));

    flipVertAct = new QAction(QIcon(":/icons/object-flip-vertical.png"), tr("Flip vertically"), mImageArea);
    flipVertAct->setShortcut(QKeySequence(tr("Shift+Alt+V")));
    flipVertAct->setEnabled(true);
    connect(flipVertAct, SIGNAL(triggered()), this, SLOT(imageFlipVert()));

    flipHorAct = new QAction(QIcon(":/icons/object-flip-horizontal.png"), tr("Flip horizontically"), mImageArea);
    flipHorAct->setShortcut(QKeySequence(tr("Shift+Alt+H")));
    flipHorAct->setEnabled(true);
    connect(flipHorAct, SIGNAL(triggered()), this, SLOT(imageFlipHor()));

    batchProcessAct = new QAction(QIcon(":/icons/kbv-batch-process.png"), tr("Batch process"), mImageArea);
    batchProcessAct->setShortcut(QKeySequence(tr("Shift+Alt+B")));
    batchProcessAct->setEnabled(true);
    connect(batchProcessAct, SIGNAL(triggered()), this, SLOT(startBatchProcess()));
}

/*****************************************************************************
 */
void KbvImageEditor::createMenus()
  {
    mfileMenu = menuBar()->addMenu(tr("&File"));
    mfileMenu->addAction(openAct);
    mfileMenu->addAction(saveAct);
    mfileMenu->addAction(saveAsAct);
    mfileMenu->addAction(printAct);
    mfileMenu->addSeparator();
    mfileMenu->addAction(exitAct);

    mViewMenu = menuBar()->addMenu(tr("&View"));
    mViewMenu->addAction(zoomInAct);
    mViewMenu->addAction(zoomOutAct);
    mViewMenu->addAction(zoomNormalAct);
    mViewMenu->addSeparator();
    mViewMenu->addAction(undoAct);
    mViewMenu->addAction(redoAct);

    mImageMenu = menuBar()->addMenu(tr("&Image"));
    mImageMenu->addAction(cropAct);
    mImageMenu->addSeparator();
    mImageMenu->addAction(rotLeftAct);
    mImageMenu->addAction(rotRightAct);
    mImageMenu->addAction(rot180Act);
    mImageMenu->addAction(flipVertAct);
    mImageMenu->addAction(flipHorAct);
    mImageMenu->addSeparator();
    mImageMenu->addAction(batchProcessAct);

    mPopupMenu = new QMenu(mImageArea);
    mPopupMenu->addAction(cropAct);
    mPopupMenu->addSeparator();
    mPopupMenu->addAction(rotLeftAct);
    mPopupMenu->addAction(rotRightAct);
    mPopupMenu->addAction(rot180Act);
    mPopupMenu->addAction(flipVertAct);
    mPopupMenu->addAction(flipHorAct);
  }
/*************************************************************************//*!
 * Toolbars
 */
void    KbvImageEditor::createToolbars()
{
  mFileCombo = new QComboBox(this);
  mFileCombo->setMaximumSize(151, 28);
  mFileCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  mFileCombo->setToolTip(tr("Select file for editing"));
  
  mFileToolBar = new QToolBar("File", this);
  mFileToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea);
  mFileToolBar->addAction(openAct);
  mFileToolBar->addAction(saveAct);
  mFileToolBar->addAction(saveAsAct);
  mFileToolBar->addWidget(mFileCombo);
  mFileToolBar->setVisible(true);
  this->addToolBar(mFileToolBar);
  connect(mFileCombo, SIGNAL(activated(int)), this, SLOT(comboBoxOpen(int)));

  mTransformToolBar = new QToolBar("Transform", this);
  mTransformToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea);
  mTransformToolBar->addAction(cropAct);
  mTransformToolBar->addAction(rotLeftAct);
  mTransformToolBar->addAction(rotRightAct);
  mTransformToolBar->addAction(rot180Act);
  mTransformToolBar->addAction(flipVertAct);
  mTransformToolBar->addAction(flipHorAct);
  this->addToolBar(mTransformToolBar);
  
  mEditToolBar = new QToolBar("Edit", this);
  mEditToolBar->addAction(zoomInAct);
  mEditToolBar->addAction(zoomOutAct);
  mEditToolBar->addAction(zoomNormalAct);
  mEditToolBar->addSeparator();
  mEditToolBar->addAction(undoAct);
  mEditToolBar->addAction(redoAct);
  this->addToolBar(mEditToolBar);
  
  mExitToolBar  = new QToolBar("App exit", this);
  mExitToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea);
  mExitToolBar->addAction(exitAct);
  this->addToolBar(mExitToolBar);
}
/*************************************************************************//*!
 * Popup menu for image operations: crop, rotate left 90°, right 90°, 180°,
 * flip horizontal and flip vertikal
 */
void    KbvImageEditor::contextMenuEvent(QContextMenuEvent *event)
  {
    mPopupMenu->exec(event->globalPos());
  }
/*************************************************************************//*!
 */
void    KbvImageEditor::createInfoBar()
  {
    mInfoLabel0 = new QLabel("", nullptr, 0);    //Image size
    mInfoLabel0->setMinimumSize(70, 20);
    mInfoLabel0->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mInfoLabel1 = new QLabel("", nullptr, 0);    //Mouse position on image
    mInfoLabel1->setMinimumSize(70, 20);
    mInfoLabel1->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mInfoLabel2 = new QLabel("", nullptr, 0);    //Size of selection rectangle
    mInfoLabel2->setMinimumSize(70, 20);
    mInfoLabel2->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mInfoLabel3 = new QLabel("", nullptr, 0);    //Zoom
    mInfoLabel3->setMinimumSize(70, 20);
    mInfoLabel3->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mInfoLabel4 = new QLabel("", nullptr, 0);    //File path and name
    mInfoLabel4->setMinimumSize(100, 20);
    mInfoLabel4->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    
    mInfoBar = new QStatusBar();
    mInfoBar->insertWidget(0, mInfoLabel0, 0);
    mInfoBar->insertWidget(1, mInfoLabel1, 0);
    mInfoBar->insertWidget(2, mInfoLabel2, 0);
    mInfoBar->insertWidget(3, mInfoLabel3, 0);
    mInfoBar->insertWidget(4, mInfoLabel4, 10);
    this->setStatusBar(mInfoBar);
  }

/****************************************************************************/
