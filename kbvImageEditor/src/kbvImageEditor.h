/*****************************************************************************
 * kbv image editor
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.03.23
 *****************************************************************************/
#ifndef KBVIMAGEEDITOR_H_
#define KBVIMAGEEDITOR_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QPrinter>
#include <QPrintDialog>
#include <QMetaType>
#include <kbvExiv.h>
#include <kbvGeneral.h>
#include <kbvConstants.h>
#include "kbvFileDialog.h"
#include "kbvImageLabel.h"
#include "kbvImageReader.h"
#include "kbvQuestionBox.h"
#include "kbvBatchDialog.h"
#include "kbvBatchProgress.h"
#include "kbvBatchThread.h"

class KbvImageEditor : public QMainWindow
{
  Q_OBJECT

public:
  KbvImageEditor(QWidget *parent=0, Qt::WindowFlags flags=0);
  virtual ~KbvImageEditor();

  bool  setFiles(const QList<QPair<QString, QString> > &files);
  void  loadImage(const QString path, const QString name);
  //void  loadImage(QAbstractListModel *model=0, const QModelIndex index=QModelIndex(), const QString databaseName=QString());
  void  adjustAndShow(QImage *image);
  void  removeSelectionRect();

  KbvBatchDialog    *batchDialog;
  KbvBatchProgress  *batchProgress;
  KbvBatchThread    *batchThread;

public slots:
  void  startBatchProcess();
  void  endBatchProcess();
  void  openBatchReport();

signals:
  void  fileChanged(QString path);

private slots:
  void  menuOpen();
  void  menuSave();
  void  menuSaveAs();
  void  menuPrint();
  void  comboBoxOpen(int index);
  void  imageCrop();
  void  imageRotLeft();
  void  imageRotRight();
  void  imageRot180();
  void  imageFlipVert();
  void  imageFlipHor();
  void  zoomIn();
  void  zoomOut();
  void  zoomNormal();
  void  updateImage();
  void  updateInfoLabel(int label, QString value);
  void  selectionRect(QRect rect);
  void  mousePosition(const QPoint position);

private:
  void  closeEvent(QCloseEvent *event);
  void  createInfoBar();
  void  createActions();
  void  createToolbars();
  void  createMenus();
  void  setTitle(QString pathName);
  void  scaleImage();
  void  imageFlip(bool hor, bool vert);
  void  imageRotate(qreal angle);
  void  createImarcaExifInfo();
  void  contextMenuEvent(QContextMenuEvent *event);
  void  keyPressEvent(QKeyEvent * event);


#ifndef QT_NO_PRINTER
  QPrinter      printer;
#endif

  KbvGeneral        generalFunc;
  KbvFileDialog     *mFileDialog;
  KbvImageLabel     *mImageArea;
  QScrollArea       *mScrollArea;
  QStatusBar        *mInfoBar;
  QToolBar          *mFileToolBar, *mTransformToolBar, *mEditToolBar, *mExitToolBar;
  QUndoStack        *mUndoStack;
  KbvQuestionBox    *mQuestionBox;

  QMenu             *mfileMenu, *mViewMenu, *mImageMenu, *mPopupMenu;
  QAction           *undoAct, *redoAct;
  QAction           *openAct, *saveAct, *saveAsAct, *printAct, *exitAct;
  QAction           *zoomInAct, *zoomOutAct, *zoomNormalAct;
  QAction           *cropAct, *rotLeftAct, *rotRightAct, *rot180Act;
  QAction           *flipHorAct, *flipVertAct, *batchProcessAct;

  QComboBox         *mFileCombo;
  QLabel            *mInfoLabel0, *mInfoLabel1, *mInfoLabel2, *mInfoLabel3, *mInfoLabel4;

  QDesktopWidget    mDesktop;
  QRect             mDesktopSize, mImageSize, mSelectedRect, mVisibleRect;
  double            mScaleFactor;

  Kbv::kbvBatchParam  mBatchParams;
  KbvExiv             mMetaData;   //libKbvExiv.so
  QImage              *mImage;
  KbvImageReader      *mReader;
  QImageWriter        *mWriter;
  QByteArray          mImageFormat;
  QString             mCurrentPath, mCurrentFile, mCurrentDBName, mCurrentDBPath;
  int                 mCurrentPK, mCurrentIndex;
  bool                mCurrentAltered, mStopBatchProcess;
  int                 mJpegQuality, mTiffCompression;
  float               mPngGamma;
  int                 mScrollBarWidth;
};

#endif /*KBVIMAGEEDITOR_H_*/
/****************************************************************************/
