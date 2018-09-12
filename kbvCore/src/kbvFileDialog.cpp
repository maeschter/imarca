/*****************************************************************************
 * kvb file save dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2016-03-13 12:00:38 +0100 (So, 13. Mär 2016) $
 * $Rev: 1080 $
 * Created: 2009.07.01
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *****************************************************************************/

#include "kbvFileDialog.h"


KbvFileDialog::KbvFileDialog(QWidget *parent, Qt::WindowFlags flags) : QFileDialog(parent, flags)
{
  this->setViewMode(QFileDialog::Detail);
  this->setOption(QFileDialog::DontUseNativeDialog);  //by reason of widget layout

  this->setLabelText(QFileDialog::FileType, tr("Image type:"));
  QStringList filters;
  filters << tr("JPEG image      *.jpg *.jpeg");
  filters << tr("TIFF image      *.tif *.tiff");
  filters << tr("PNG image       *.png");
  this->setNameFilters(filters);

  optionsLabel = new QLabel(tr("Options:"), this, 0);
  optionsLabel->setVisible(true);
  optionsLabel->setAlignment(Qt::AlignTop);

  createOptionStack();

  this->layout()->addWidget(optionsLabel);
  this->layout()->addWidget(optionsStack);

  connect(this, SIGNAL(filterSelected(const QString)), this, SLOT(fileTypeChanged(const QString)));

  connect(jpegQualSlider,  SIGNAL(valueChanged(const int)), this, SLOT(jpegQualityChanged(const int)));
  connect(jpegOrigQuality, SIGNAL(stateChanged(const int)), this, SLOT(enableJpegSlider(const int)));
  connect(jpegMetaData,    SIGNAL(stateChanged(const int)), this, SLOT(checkMetaData(const int)));

  connect(tiffButtons,    SIGNAL(buttonClicked(const int)), this, SLOT(tiffCompressChanged(const int)));

  connect(pngGammaSlider, SIGNAL(valueChanged(const int)), this, SLOT(pngGammaChanged(const int)));
  connect(pngOrigGamma,   SIGNAL(stateChanged(const int)), this, SLOT(enablePngSlider(const int)));

  //initialization
  jpegQualSlider->setValue(90);
  pngGammaSlider->setValue(Kbv::pngGammaStd);
  fileType = Kbv::jpeg;
  jpegQuality = 90;
  tiffCompress = Kbv::tiffCompressLZW;
  pngGamma = (float) Kbv::pngGammaStd/Kbv::pngGammaScale;
}

KbvFileDialog::~KbvFileDialog()
{
  //qDebug() << "KbvFileDialog::~KbvFileDialog";//###########
}

/*************************************************************************//*!
 * Show format options on "save" dialog .
 */
void  KbvFileDialog::showSaveOptions(bool show)
  {
    optionsLabel->setVisible(show);
    optionsStack->setVisible(show);
  }

/*************************************************************************//*!
 * Preset field "Type by extension" in dialog window.
 */
void  KbvFileDialog::setFileTypeParams(QString name, int quality, int compress, float gamma)
{
  int           idx, gam;

  //Strip extension and possibly leading "/"
  nameNoExt = name;
  idx = nameNoExt.lastIndexOf(".");
  nameNoExt = nameNoExt.left(idx);
  idx = nameNoExt.lastIndexOf("/");
  nameNoExt = nameNoExt.right(nameNoExt.length()-idx-1);

  //JPG is the first filter
  this->selectFile(nameNoExt + ".jpg");

  jpegQuality = quality;
  origQuality = quality;
  jpegQualSlider->setValue(quality);
  jpegQualSlider->setEnabled(false);
  jpegOrigQuality->setCheckState(Qt::Checked);

  origCompress = compress;

  pngGamma = gamma;
  origGamma = gamma;
  gam = (int) (gamma*Kbv::pngGammaScale);
  if(gam < Kbv::pngGammaMin)
    {
      gam = Kbv::pngGammaMin;
    }
  if(gam > Kbv::pngGammaMax)
    {
      gam = Kbv::pngGammaMax;
    }
  pngGammaSlider->setValue(gam);
  pngGammaSlider->setEnabled(false);
  pngOrigGamma->setCheckState(Qt::Checked);
  //qDebug() <<"kbvFileDialog::setFileTypeParams gamma" << gam; //###########
}

/*************************************************************************//*!
 * Slot: Set file name extension due to selection in combo box "Image type".
 */
void    KbvFileDialog::fileTypeChanged(QString filter)
{
  QString       str;

  //Find file type
  str = filter.left(3);
  if (str == "JPE")
    {
      optionsStack->setCurrentIndex(Kbv::jpeg);
      this->selectFile(nameNoExt + ".jpg");
      fileType = Kbv::jpeg;
    }
  if (str == "TIF")
    {
      optionsStack->setCurrentIndex(Kbv::tiff);
      this->selectFile(nameNoExt + ".tif");
      fileType = Kbv::tiff;
    }
  if (str == "PNG")
    {
      optionsStack->setCurrentIndex(Kbv::png);
      this->selectFile(nameNoExt + ".png");
      fileType = Kbv::png;
    }
}

/*************************************************************************//*!
 * SLOT: Disable slider and set value to original quality when checkbox
 * "use original quality" is checked and enable slider when unchecked.
 */
void  KbvFileDialog::enableJpegSlider(int state)
  {
    if (state == Qt::Checked)
      {
        jpegQualSlider->setEnabled(false);
        jpegQualSlider->setValue(origQuality);
      }
    else
      {
        jpegQualSlider->setEnabled(true);
      }
  }

/*************************************************************************//*!
 * SLOT: Slider was moved. Save value and write it into label.
 */
void  KbvFileDialog::jpegQualityChanged(int value)
  {
    jpegQuality = value;
    QString str;
    str.setNum(value);
    jpegLblQuality->setText(str);
  }

/*************************************************************************//*!
 * SLOT: Enable/disable metadata when "save metadata" is checked/unchecked.
 * Fits all formats which can include metadata like exif, iptc, comment.
 */
void  KbvFileDialog::checkMetaData(int state)
  {
    metaData = false;
    if (state == Qt::Checked)
      {
        metaData = true;
      }
  }

/*************************************************************************//*!
 * SLOT: Disable slider and set value to original gamma when checkbox
 * "use original gamma" is checked and enable slider when unchecked.
 */
void  KbvFileDialog::enablePngSlider(int state)
  {
    if (state == Qt::Checked)
      {
        pngGammaSlider->setEnabled(false);
        pngGammaSlider->setValue((int)origGamma * Kbv::pngGammaScale);
      }
    else
      {
        pngGammaSlider->setEnabled(true);
      }
  }

/*************************************************************************//*!
 * SLOT: png slider was moved. Save value as float and write it into label.
 */
void  KbvFileDialog::pngGammaChanged(int value)
  {
    pngGamma = (float) value / Kbv::pngGammaScale;
    QString str = QString::number(pngGamma, 'f', 2);
    pngLblGamma->setText(str);

    //qDebug() << "kbvfiledialog::pngGammaChanged" << pngGamma; //###########
  }

/*************************************************************************//*!
 * SLOT: Tiff compression selected.
 */
void  KbvFileDialog::tiffCompressChanged(int index)
  {
    tiffCompress = index;

  }

/*************************************************************************//*!
 * Options for "save" dialog.
 */
void  KbvFileDialog::createOptionStack()
  {
    optionsStack = new QStackedWidget(this);
    optionsStack->setMinimumSize(430, 150);
    optionsStack->setVisible(true);
    optionsStack->setFrameStyle(QFrame::Sunken | QFrame::Box);

    tabJpeg = new QWidget();
    jpegTxtQuality = new QLabel(tr("Quality"), tabJpeg);
    jpegTxtQuality->setGeometry(5, 5, 350, 25);

    jpegQualSlider = new QSlider(Qt::Horizontal, tabJpeg);
    jpegQualSlider->setGeometry(5, 30, 380, 25);
    jpegQualSlider->setRange(0, 100);

    jpegLblQuality = new QLabel("", tabJpeg);
    jpegLblQuality->setGeometry(390, 30, 40, 25);

    jpegTxtFileSize = new QLabel(tr("Estimated file size"), tabJpeg);
    jpegTxtFileSize->setGeometry(5, 60, 140, 25);
    jpegFileSize = new QLabel("unkown", tabJpeg);
    jpegFileSize->setGeometry(150, 60, 120, 25);

    jpegOrigQuality = new QCheckBox(tr("Use original Quality"), tabJpeg);
    jpegOrigQuality->setGeometry(5, 90, 350, 25);
    jpegOrigQuality->setCheckState(Qt::Checked);

    jpegMetaData = new QCheckBox(tr("Save metadata (exif, comment, iptc)"), tabJpeg);
    jpegMetaData->setGeometry(5, 115, 350, 25);
    jpegMetaData->setCheckState(Qt::Checked);
    jpegMetaData->setEnabled(false);

    tabTiff = new QWidget();
    tiffButtons = new QButtonGroup(tabTiff);

    tiffNoCompr = new QCheckBox(tr("No compression"), tabTiff);
    tiffNoCompr->setGeometry(5, 10, 350, 25);

    tiffCompr1 = new QCheckBox(tr("LZW compression"), tabTiff);
    tiffCompr1->setGeometry(5, 35, 350, 25);
    tiffCompr1->setChecked(true);

    tiffButtons->addButton(tiffNoCompr);
    tiffButtons->addButton(tiffCompr1);
    tiffButtons->setId(tiffNoCompr, Kbv::tiffCompressNo);
    tiffButtons->setId(tiffCompr1, Kbv::tiffCompressLZW);

    tabPng = new QWidget();
    pngTxtGamma = new QLabel(tr("Gamma"), tabPng);
    pngTxtGamma->setGeometry(5, 5, 350, 25);

    pngGammaSlider = new QSlider(Qt::Horizontal, tabPng);
    pngGammaSlider->setGeometry(5, 30, 380, 25);
    pngGammaSlider->setRange(Kbv::pngGammaMin, Kbv::pngGammaMax);

    pngLblGamma = new QLabel("", tabPng);
    pngLblGamma->setGeometry(390, 30, 40, 25);

    pngOrigGamma = new QCheckBox(tr("Use original gamma"), tabPng);
    pngOrigGamma->setGeometry(5, 60, 350, 25);
    pngOrigGamma->setCheckState(Qt::Checked);

    optionsStack->insertWidget(Kbv::jpeg, tabJpeg);
    optionsStack->insertWidget(Kbv::tiff, tabTiff);
    optionsStack->insertWidget(Kbv::png, tabPng);
  }

/****************************************************************************/
