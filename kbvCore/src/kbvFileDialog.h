/*****************************************************************************
 * kbv file save dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
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

  void  showSaveOptions(bool show);
  void  setFileTypeParams(QString name, int quality, int compress, float gamma);

  //Return values
  int         jpegQuality, tiffCompress;
  float       pngGamma;
  int         fileType;
  bool        metaData;

private slots:
  void  fileTypeChanged(QString filter);
  void  enableJpegSlider(int state);
  void  jpegQualityChanged(int value);
  void  checkMetaData(int state);
  void  enablePngSlider(int state);
  void  pngGammaChanged(int value);
  void  tiffCompressChanged(int index);

private:
  void  createOptionStack();

  QComboBox     *fileTypeBox;

  QStackedWidget *optionsStack;
  QButtonGroup   *tiffButtons;
  QWidget     *tabJpeg, *tabTiff, *tabPng;
  QLabel      *optionsLabel;
  QLabel      *jpegTxtQuality, *jpegLblQuality, *jpegTxtFileSize, *jpegFileSize;
  QLabel      *pngTxtGamma, *pngLblGamma;
  QCheckBox   *jpegOrigQuality, *jpegMetaData;
  QCheckBox   *pngOrigGamma, *tiffNoCompr, *tiffCompr1;
  QSlider     *jpegQualSlider, *pngGammaSlider;

  int         origQuality, origCompress;
  float       origGamma;
  QString     nameNoExt;
};

#endif /*KBVFILEDIALOG_H_*/
/****************************************************************************/
