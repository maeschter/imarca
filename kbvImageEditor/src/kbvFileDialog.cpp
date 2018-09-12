/*****************************************************************************
 * kvb file save dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2009.07.01
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvFileDialog.h"

KbvFileDialog::KbvFileDialog(QWidget *parent, Qt::WindowFlags flags) : QFileDialog(parent, flags)
{
  this->setViewMode(QFileDialog::Detail);
  this->setOption(QFileDialog::DontUseNativeDialog);  //by reason of widget layout

  this->setLabelText(QFileDialog::FileType, tr("Image type:"));
  QStringList filters;
  filters << tr("JPEG image       *.jpeg *.jpg");
  filters << tr("PNG image        *.png");
  filters << tr("X11 Pixmap       *.xpm");
  filters << tr("Windows bitmap   *.bmp");
  filters << tr("TIFF image       *.tiff *.tif");
  this->setNameFilters(filters);

  mOptionsLabel = new QLabel(tr("Options:"), this, 0);
  mOptionsLabel->setVisible(true);
  mOptionsLabel->setAlignment(Qt::AlignTop);

  createOptionStack();

  this->layout()->addWidget(mOptionsLabel);
  this->layout()->addWidget(mOptionsStack);

  connect(this, SIGNAL(filterSelected(const QString)), this, SLOT(fileTypeChanged(const QString)));

  connect(mJpegQualSlider,  SIGNAL(valueChanged(const int)), this, SLOT(jpegQualityChanged(const int)));
  connect(mJpegOrigQuality, SIGNAL(stateChanged(const int)), this, SLOT(enableJpegSlider(const int)));
  connect(mJpegMetaData,    SIGNAL(stateChanged(const int)), this, SLOT(checkMetaData(const int)));

  connect(mTiffButtons,    SIGNAL(buttonClicked(const int)), this, SLOT(tiffCompressChanged(const int)));

  connect(mPngGammaSlider,    SIGNAL(valueChanged(const int)), this, SLOT(pngGammaChanged(const int)));
  connect(mPngOrigGamma,      SIGNAL(stateChanged(const int)), this, SLOT(enablePngGammaSlider(const int)));
  connect(mPngCompressSlider, SIGNAL(valueChanged(const int)), this, SLOT(pngCompressChanged(const int)));

  //initialization
  mFileType = Kbv::jpeg;
  mJpegQuality = Kbv::jpegQualityStd;
  mJpegQualSlider->setValue(Kbv::jpegQualityStd);
  mPngGamma = ((float) Kbv::pngGammaStd) / Kbv::pngGammaScale;
  mPngGammaSlider->setValue(Kbv::pngGammaStd);
  mPngCompress = Kbv::pngCompressStd;
  mPngCompressSlider->setValue(Kbv::pngCompressStd);
  mTiffCompress = Kbv::tiffCompressLZW;
}

KbvFileDialog::~KbvFileDialog()
{
  //qDebug() << "KbvFileDialog::~KbvFileDialog";//###########
}
//TODO: pngGamma --> pngCompression, begrenzt auf 0-9, default 6
/*************************************************************************//*!
 * Execute dialog for 'open' or 'save as ..' a file.
 */
int  KbvFileDialog::exec()
{
  if(this->acceptMode() == QFileDialog::AcceptOpen)
    {
      this->setFileMode(QFileDialog::ExistingFiles);
      this->mOptionsLabel->setVisible(false);
      this->mOptionsStack->setVisible(false);
    }
  else if(this->acceptMode() == QFileDialog::AcceptSave)
    {
      this->setFileMode(QFileDialog::AnyFile);
      this->mOptionsLabel->setVisible(true);
      this->mOptionsStack->setVisible(true);
    }
  return  QFileDialog::exec();
}
/*************************************************************************//*!
 * Preset field "Type by extension" in dialog window.
 */
void  KbvFileDialog::setFileTypeParams(QString name, int quality, int compress, float gamma)
{
  int    idx, gam;

  //qDebug() <<"kbvFileDialog::setFileTypeParams name qual. comp. gamma" <<name <<quality <<compress << gamma; //###########

  //Strip extension and possibly leading "/"
  mNameNoExt = name;
  idx = mNameNoExt.lastIndexOf(".");
  mNameNoExt = mNameNoExt.left(idx);
  idx = mNameNoExt.lastIndexOf("/");
  mNameNoExt = mNameNoExt.right(mNameNoExt.length()-idx-1);

  //JPG is the first filter
  this->selectFile(mNameNoExt + ".jpg");

  mJpegQuality = quality;
  mOrigQuality = quality;
  mJpegQualSlider->setValue(quality);
  mJpegQualSlider->setEnabled(false);
  mJpegOrigQuality->setCheckState(Qt::Checked);

  mOrigCompress = compress;

  mPngGamma = gamma;
  mOrigGamma = gamma;
  gam = (int) (gamma*Kbv::pngGammaScale);
  gam = (gam < Kbv::pngGammaMin) ? Kbv::pngGammaMin : ((gam > Kbv::pngGammaMax) ? Kbv::pngGammaMax : gam);

  mPngGammaSlider->setValue(gam);
  mPngGammaSlider->setEnabled(false);
  mPngOrigGamma->setCheckState(Qt::Checked);
}
/*************************************************************************//*!
 * Slot: Set file name extension due to selection in combo box "Image type".
 */
void    KbvFileDialog::fileTypeChanged(QString filter)
{
  QString       str;

  //Find file type
  str = filter.right(3);
  //qDebug() <<"kbvFileDialog::fileTypeChanged" <<str; //###########
  if (str == "jpg")
    {
      mOptionsStack->setCurrentIndex(Kbv::jpeg);
      this->selectFile(mNameNoExt + ".jpg");
      mFileType = Kbv::jpeg;
    }
  if (str == "tif")
    {
      mOptionsStack->setCurrentIndex(Kbv::tiff);
      this->selectFile(mNameNoExt + ".tif");
      mFileType = Kbv::tiff;
    }
  if (str == "png")
    {
      mOptionsStack->setCurrentIndex(Kbv::png);
      this->selectFile(mNameNoExt + ".png");
      mFileType = Kbv::png;
    }
  if (str == "xpm")
    {
      mOptionsStack->setCurrentIndex(Kbv::xpm);
      this->selectFile(mNameNoExt + ".xpm");
      mFileType = Kbv::xpm;
    }
  if (str == "bmp")
    {
      mOptionsStack->setCurrentIndex(Kbv::bmp);
      this->selectFile(mNameNoExt + ".bmp");
      mFileType = Kbv::bmp;
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
        mJpegQualSlider->setEnabled(false);
        mJpegQualSlider->setValue(mOrigQuality);
      }
    else
      {
        mJpegQualSlider->setEnabled(true);
      }
  }
/*************************************************************************//*!
 * SLOT: Slider was moved. Save value and write it into label.
 */
void  KbvFileDialog::jpegQualityChanged(int value)
  {
    mJpegQuality = value;
    QString str;
    str.setNum(value);
    mJpegLblQuality->setText(str);
  }
/*************************************************************************//*!
 * SLOT: Enable/disable metadata when "save metadata" is checked/unchecked.
 * Fits all formats which can include metadata like exif, iptc, comment.
 */
void  KbvFileDialog::checkMetaData(int state)
  {
    mMetaData = false;
    if (state == Qt::Checked)
      {
        mMetaData = true;
      }
  }
/*************************************************************************//*!
 * SLOT: Disable slider and set value to original gamma when checkbox
 * "use original gamma" is checked and enable slider when unchecked.
 */
void  KbvFileDialog::enablePngGammaSlider(int state)
  {
    if (state == Qt::Checked)
      {
        mPngGammaSlider->setEnabled(false);
        mPngGammaSlider->setValue((int) mOrigGamma * Kbv::pngGammaScale);
      }
    else
      {
        mPngGammaSlider->setEnabled(true);
      }
  }
/*************************************************************************//*!
 * SLOT: png gamma slider was moved. Save value as float and write it into label.
 */
void  KbvFileDialog::pngGammaChanged(int value)
  {
    mPngGamma = (float) value / Kbv::pngGammaScale;
    QString str = QString::number(mPngGamma, 'f', 2);
    mPngLblGamma->setText(str);
  }
/*************************************************************************//*!
 * SLOT: png compression slider was moved. Save value and write it into label.
 */
void  KbvFileDialog::pngCompressChanged(int value)
  {
    mPngCompress = value;
    QString str = QString::number(mPngCompress);
    mPngLblCompression->setText(str);
  }
/*************************************************************************//*!
 * SLOT: Tiff compression selected.
 */
void  KbvFileDialog::tiffCompressChanged(int index)
  {
    mTiffCompress = index;
  }
/*************************************************************************//*!
 * Return values.
 */
int  KbvFileDialog::getJpegQuality()
{
  return  mJpegQuality;
}
int  KbvFileDialog::getTiffCompression()
{
  return  mTiffCompress;
}
float  KbvFileDialog::getPngGamma()
{
  return  mPngGamma;
}
int  KbvFileDialog::getPngCompression()
{
  return  mPngCompress;
}
int  KbvFileDialog::getFileType()
{
  return  mFileType;
}
bool  KbvFileDialog::keepMetadata()
{
  return  mMetaData;
}

/*************************************************************************//*!
 * Options for "save" dialog.
 */
void  KbvFileDialog::createOptionStack()
  {
    mOptionsStack = new QStackedWidget(this);
    mOptionsStack->setMinimumSize(430, 150);
    mOptionsStack->setVisible(true);
    mOptionsStack->setFrameStyle(QFrame::Sunken | QFrame::Box);

    mTabJpeg = new QWidget();
    mJpegTxtQuality = new QLabel(tr("Quality"), mTabJpeg);
    mJpegTxtQuality->setGeometry(5, 5, 350, 25);

    mJpegQualSlider = new QSlider(Qt::Horizontal, mTabJpeg);
    mJpegQualSlider->setGeometry(5, 25, 370, 25);
    mJpegQualSlider->setRange(0, 100);

    mJpegLblQuality = new QLabel("", mTabJpeg);
    mJpegLblQuality->setGeometry(390, 25, 40, 25);

    mJpegOrigQuality = new QCheckBox(tr("Use original Quality"), mTabJpeg);
    mJpegOrigQuality->setGeometry(5, 50, 380, 25);
    mJpegOrigQuality->setCheckState(Qt::Checked);

    mJpegMetaData = new QCheckBox(tr("Save metadata (exif, comment, iptc)"), mTabJpeg);
    mJpegMetaData->setGeometry(5, 90, 380, 25);
    mJpegMetaData->setCheckState(Qt::Checked);
    mJpegMetaData->setEnabled(false);

    mTabTiff = new QWidget();
    mTiffButtons = new QButtonGroup(mTabTiff);

    mTiffNoCompr = new QCheckBox(tr("No compression"), mTabTiff);
    mTiffNoCompr->setGeometry(5, 10, 350, 25);

    mTiffCompr1 = new QCheckBox(tr("LZW compression"), mTabTiff);
    mTiffCompr1->setGeometry(5, 35, 350, 25);
    mTiffCompr1->setChecked(true);

    mTiffButtons->addButton(mTiffNoCompr);
    mTiffButtons->addButton(mTiffCompr1);
    mTiffButtons->setId(mTiffNoCompr, Kbv::tiffCompressNo);
    mTiffButtons->setId(mTiffCompr1, Kbv::tiffCompressLZW);

    mTabPng = new QWidget();
    mPngTxtGamma = new QLabel(tr("Gamma"), mTabPng);
    mPngTxtGamma->setGeometry(5, 5, 350, 25);

    mPngGammaSlider = new QSlider(Qt::Horizontal, mTabPng);
    mPngGammaSlider->setGeometry(5, 25, 370, 25);
    mPngGammaSlider->setRange(Kbv::pngGammaMin, Kbv::pngGammaMax);

    mPngLblGamma = new QLabel("", mTabPng);
    mPngLblGamma->setGeometry(390, 25, 40, 25);

    mPngOrigGamma = new QCheckBox(tr("Use original gamma"), mTabPng);
    mPngOrigGamma->setGeometry(5, 50, 350, 25);
    mPngOrigGamma->setCheckState(Qt::Checked);

    mPngTxtCompression = new QLabel(tr("Compression"), mTabPng);
    mPngTxtCompression->setGeometry(5, 90, 350, 25);

    mPngCompressSlider = new QSlider(Qt::Horizontal, mTabPng);
    mPngCompressSlider->setGeometry(5, 110, 370, 25);
    mPngCompressSlider->setRange(Kbv::pngCompressMin, Kbv::pngCompressMax);

    mPngLblCompression = new QLabel("", mTabPng);
    mPngLblCompression->setGeometry(390, 110, 40, 25);

    mTabNone = new QWidget();
    mOptionsStack->insertWidget(Kbv::jpeg, mTabJpeg);
    mOptionsStack->insertWidget(Kbv::tiff, mTabTiff);
    mOptionsStack->insertWidget(Kbv::png, mTabPng);
    mOptionsStack->insertWidget(Kbv::bmp, mTabNone);
    mOptionsStack->insertWidget(Kbv::xpm, mTabNone);

  }

/****************************************************************************/
