/*****************************************************************************
 * kbv file save dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2009.07.01
 *****************************************************************************/
#ifndef KBVFILEDIALOG_H_
#define KBVFILEDIALOG_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvConstants.h"

class KbvFileDialog : public QFileDialog
{
  Q_OBJECT

public:
  KbvFileDialog(QWidget *parent, Qt::WindowFlags flags);
  virtual ~KbvFileDialog();

  void  setFileTypeParams(QString name, int quality, int compress, float gamma);
  int   exec();

  int   getJpegQuality();
  int   getTiffCompression();
  float getPngGamma();
  int   getPngCompression();
  int   getFileType();
  bool  keepMetadata();

private slots:
  void  fileTypeChanged(QString filter);
  void  enableJpegSlider(int state);
  void  jpegQualityChanged(int value);
  void  checkMetaData(int state);
  void  enablePngGammaSlider(int state);
  void  pngGammaChanged(int value);
  void  pngCompressChanged(int value);
  void  tiffCompressChanged(int index);

private:
  void  createOptionStack();

  QComboBox     *mFileTypeBox;

  QStackedWidget *mOptionsStack;
  QButtonGroup   *mTiffButtons;
  QWidget     *mTabJpeg, *mTabTiff, *mTabPng, *mTabNone;
  QLabel      *mOptionsLabel;
  QLabel      *mJpegTxtQuality, *mJpegLblQuality;
  QLabel      *mPngTxtGamma, *mPngLblGamma, *mPngTxtCompression, *mPngLblCompression;
  QCheckBox   *mJpegOrigQuality, *mJpegMetaData, *mPngOrigGamma, *mTiffNoCompr, *mTiffCompr1;
  QSlider     *mJpegQualSlider, *mPngGammaSlider, *mPngCompressSlider;

  int         mFileType, mOrigQuality, mOrigCompress, mJpegQuality, mTiffCompress, mPngCompress;
  float       mOrigGamma, mPngGamma;
  bool        mMetaData;
  QString     mNameNoExt;
};

#endif /*KBVFILEDIALOG_H_*/
/****************************************************************************/
