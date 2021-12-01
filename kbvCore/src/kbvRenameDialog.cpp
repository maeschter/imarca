/*****************************************************************************
 * kvb rename dialog handles multiple rename
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * 2009.09.06 Renamed
 * Created: 2008.11.12
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvRenameDialog.h"


KbvRenameDialog::KbvRenameDialog(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);

  this->setWindowTitle(QString(tr("Rename")));
  this->setModal(true);

  ui.renGroupPreview->setTitle(QString(tr("Preview")));
  ui.renGroupName->setTitle(QString(tr("Name")));
  ui.renPrefixLbl->setText(QString(tr("Prefix &Name Count or Prefix Count Name")));
  ui.renSuffixLbl->setText(QString(tr("&Postfix")));
  ui.renGroupCount->setTitle(QString(tr("Count")));
  ui.renStartLbl->setText(QString(tr("&Start value")));
  ui.renStepLbl->setText(QString(tr("S&tep value")));
  ui.renKeepExtEdit->setText(QString());
  ui.renKeepExtCheckBox->setText(QString(tr("Keep file extension")));

  connect(ui.renStartSpin,      SIGNAL(valueChanged(const int)),    this, SLOT(renStartSpinChanged(const int)));
  connect(ui.renStepSpin,       SIGNAL(valueChanged(const int)),    this, SLOT(renStepSpinChanged(const int)));
  connect(ui.renNameEdit,       SIGNAL(textChanged(const QString)), this, SLOT(renNameEditChanged(const QString)));
  connect(ui.renSuffixEdit,     SIGNAL(textChanged(const QString)), this, SLOT(renSuffixEditChanged(const QString)));
  connect(ui.renKeepExtEdit,    SIGNAL(textChanged(const QString)), this, SLOT(renExtensionEditChanged(const QString)));
  connect(ui.renKeepExtCheckBox,SIGNAL(stateChanged(const int)),    this, SLOT(renExtCheckBoxChanged(const int)));
  connect(ui.renButtonBox,      SIGNAL(accepted()), this, SLOT(accept()));
  connect(ui.renButtonBox,      SIGNAL(rejected()), this, SLOT(reject()));

  ui.renGroupName->setToolTip(tr("The new name is composed from prefix, name, count value and suffix.\n"
                                  "Use prefix, name and suffix in arbitrary combination.\n"
                                  "% specifies the original filename. The count value may be located\n"
                                  "before or after the filename.\n"
                                  "Use # for count.\n"
                                  "Use up to five # for number numerals of count value (unused leading\n"
                                  "digits are filled with zeros."));
  ui.renGroupCount->setToolTip(tr("Use the spin boxes to set start and a step values\n"
                                  "for counter. Watch preview."));
  ui.renGroupPreview->setToolTip(tr("Preview on next two filenames."));
  ui.renKeepExtEdit->setToolTip(tr("Define a new file extension."));

  ui.renStartSpin->setValue(0);
  ui.renStepSpin->setValue(1);
  ui.renSuffixEdit->setText("");

  showPreview();
}

KbvRenameDialog::~KbvRenameDialog()
{
  //qDebug() << "KbvRenameDialog::~KbvRenameDialog"; //###########
}

/*************************************************************************//*!
 * Execute modal dialog.\n
 * Multiple rename: Set name of first two selected files for preview labels.\n
 * Single rename: Set name of selected file in text field.
 */
int    KbvRenameDialog::perform(QString &name1, QString &name2, bool multiple)
{
  int i;

  this->multiple = multiple;
  fileExt1 = "";
  i = name1.lastIndexOf(".");
  fileName1 = name1.left(i);
  if (i >= 0)
    {
      fileExt1 = name1.right(name1.length() - i);
    }

  if (multiple)
    {
      fileExt2 = "";
      i = name2.lastIndexOf(".");
      fileName2 = name2.left(i);
      if (i >= 0)
        {
          fileExt2 = name2.right(name2.length() - i);
        }
      ui.renNameEdit->setFocus();
      ui.renNameEdit->setText(fileName1 + "#");
      ui.renGroupCount->setEnabled(true);
    }
  else
    {
      //Single rename: disable count group
      ui.renNameEdit->setFocus();
      ui.renNameEdit->setText(fileName1);
      ui.renGroupCount->setEnabled(false);
    }
  return  QDialog::exec();
}

/*************************************************************************//*!
 * SLOT: start value of counter has changed
 */
void    KbvRenameDialog::renStartSpinChanged(const int value)
{
  startValue = value;
  showPreview();
}

/*************************************************************************//*!
 * SLOT: step value of counter has changed
 */
void    KbvRenameDialog::renStepSpinChanged(const int value)
{
  stepValue = value;
  showPreview();
}

/*************************************************************************//*!
 * SLOT: text for prefix and name has changed
 */
void    KbvRenameDialog::renNameEditChanged(const QString &text)
{
  tfNameText = text;
  showPreview();
}

/*************************************************************************//*!
 * SLOT:text for suffix has changed
 */
void    KbvRenameDialog::renSuffixEditChanged(const QString &text)
{
  suffix = text;
  showPreview();
}

/*************************************************************************//*!
 * SLOT: text for extension has changed
 */
void    KbvRenameDialog::renExtensionEditChanged(const QString &text)
{
  extension = text;
  showPreview();
}

/*************************************************************************//*!
 * SLOT: check box "Keep File Extension" has changed
 */
void    KbvRenameDialog::renExtCheckBoxChanged(const int state)
{
  //qDebug() << "kbvRename::renExtCheckBoxChanged state" <<state; //###########
  if (state == Qt::Checked)
    {
      ui.renKeepExtEdit->setEnabled(false);
      ui.renKeepExtEdit->setText("");
      extension = "";
    }
  else
    {
      ui.renKeepExtEdit->setEnabled(true);
    }
}

/*************************************************************************//*!
 * Multiple rename: Calculate preview for next two names.\
 * Restrictions: max. 250 characters (prefix - count - suffix)
 * Prefix: 0 - n characters, suffix: 0 - m characters
 * count: 1 - 5 numerals (leading zeros with # as space holder)
 * genuine filename: % as space holder before or after count
 * The method delivers prefix, suffix, start value, increment and number of
 * numerals for count. Prefix, suffix and name my be empty.
 * The combination of these parameters is coded for rename method. This method
 * always tries to arrange:\n
 * prefix-name-count-suffix or prefix-count-name-suffix.\n
 * 0=no count -> nothing to rename,\n
 * 1=count only, 2=count before name, 3=count after name
 */
void    KbvRenameDialog::showPreview()
{
  int       i, j, k;
  bool      zeros;
  QString   count1, count2, preview1, preview2, ext1, ext2;

  //file extension
  if (extension.isEmpty())
    {
      ext1 = fileExt1;
      ext2 = fileExt2;
    }
  else
    {
      ext1 = "." + extension;
      ext2 = ext1;
    }
  
  if (multiple)
    {
      //multiple rename
      zeros = true;
      k=0;
      prefix = "";
      i = tfNameText.indexOf("#");
      j = tfNameText.indexOf("%");
      if (i<0)
        {
          if (j<0)    {k=0;}  //no count, no prefix, no filename
          if (j>0)    {k=7;}  //no count, prefix, filename
          if (j==0)
            {
              if(!suffix.isEmpty())
                {
                  k=7;  //no count, no prefix, filename, suffix
                }
            }
        }
      if (i==0)
        {
          if (j<0)    {k=1;}  //count, no prefix, no filename
          if (j>0)    {k=2;}  //count, no prefix, filename,
        }
      if (i>0)
        {
          if (j<0)    {k=3;}  //prefix - count, no filename
          if (j==0)   {k=4;}  //no prefix, filename - count
          if (j>0)
            {
              if (i<j)
                {
                  k=5;    //prefix - count - filename
                }
              else
                {
                  k=6;    //prefix - filename - count
                }
            }
        }

      numerals = getNumerals(tfNameText, zeros);
      switch (k)
        {
        case 0: //i<0, j<0: no count, no filename, prefix?
          prefix = "";
          preview1 = QString(tr("Error!"));
          preview2 = QString(tr("Use '#' for count value or '%' with prefix and/or suffix"));
          combination = 0;
          break;
        case 1:     //i=0, j<0: count only
          prefix = "";
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = count1 + suffix + ext1;
          preview2 = count2 + suffix + ext2;
          combination = 1;
          break;
        case 2:     //i=0, j>0: count - name, no prefix
          prefix = "";
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = count1 + "_" + fileName1 + suffix + ext1;
          preview2 = count2 + "_" + fileName2 + suffix + ext2;
          combination = 2;
          break;
        case 3:     //i>0, j<0: prefix - count, no filename
          prefix = tfNameText.left(i);
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = prefix + count1 + suffix + ext1;
          preview2 = prefix + count2 + suffix + ext2;
          combination = 1;
          break;
        case 4:     //i>0, j=0: name - count
          prefix = "";
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = fileName1 + "_" + count1 + suffix + fileExt1;
          preview2 = fileName1 + "_" + count2 + suffix + fileExt2;
          combination = 3;
          break;
        case 5:     //i>0, j>0, i<j: prefix - count - filename
          prefix = tfNameText.left(i);
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = prefix + count1 + "_" + fileName1 + suffix + ext1;
          preview2 = prefix + count2 + "_" + fileName2 + suffix + ext2;
          combination = 2;
          break;
        case 6:     //i>0, j>0, j<i: prefix - name - count
          prefix = tfNameText.left(j);
          count1 = createCountString(numerals, startValue);
          count2 = createCountString(numerals, (startValue + stepValue));
          preview1 = prefix + fileName1 + "_" + count1 + suffix + ext1;
          preview2 = prefix + fileName2 + "_" + count2 + suffix + ext2;
          combination = 3;
          break;
        case 7:     //i<0, j>=0: no count, filename, prefix or suffix
          prefix = tfNameText.left(j);
          preview1 = prefix + fileName1 + suffix + ext1;
          preview2 = prefix + fileName2 + suffix + ext2;
          combination = 4;
          break;
        default:    //no prefix, no filename, no count
          combination = 0;
          preview1 = QString(tr("error"));
          preview2 = preview1;
        break;
        } 
      //qDebug() << "kbvRename::showPreview numerals" << numerals; //###########

      if (preview1.length() > 250)
        {
          preview1 = QString(tr("Name too long!"));
          preview2 = preview1;
        }
      ui.renPreview1->setText(preview1);
      ui.renPreview2->setText(preview2);
    }
  else
    {
      //single rename
      j = tfNameText.indexOf("%");
      if (j<0)  //filename, suffix, ext
        {
          prefix = tfNameText;
          preview1 = prefix + suffix + ext1;
          combination = 0;
        }
      if (j>=0)  //prefix, filename, suffix, ext
        {
          prefix = tfNameText.left(j);
          preview1 = prefix + fileName1 + suffix + ext1;
          combination = 4;
        }
      ui.renPreview1->setText(preview1);
      ui.renPreview2->setText("");
    }
}

/*************************************************************************//*!
* Convert count value to string with/without leading zeros and num numerals.
*/
QString KbvRenameDialog::createCountString(int num, int value)
  {
    return  QString("%1").arg(value, num, 10, QLatin1Char('0'));
  }

/*************************************************************************//*!
* Extract number of numerals from text
*/
int KbvRenameDialog::getNumerals(QString text, bool zeros)
  {
    int width;

    width = 1 + text.lastIndexOf("#") - text.indexOf("#");
    if (width > 5)
      {
        width=5;
      }
    if (!zeros)
      {
        width = 1;
      }
    return width;
  }

/****************************************************************************/
