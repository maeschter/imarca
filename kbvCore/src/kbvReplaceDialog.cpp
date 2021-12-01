/*****************************************************************************
 * kvb replace dialog handles replacing of files
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.08.19
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvReplaceDialog.h"


KbvReplaceDialog::KbvReplaceDialog(QWidget *parent) : QDialog(parent)
{
  ui.setupUi(this);

  this->setWindowTitle(QString(tr("Replace file")));

  ui.textLbl2->setText(QString(tr("Replace")));
  ui.textLbl3->setText(QString(tr("with")));

  connect(ui.replButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

KbvReplaceDialog::~KbvReplaceDialog()
{
  //qDebug() << "KbvReplaceDialog::~KbvReplaceDialog"; //###########
}

/*************************************************************************//*!
 * Prepare dialog.\n
 * Set text due to file name, size and date of creation
 */
int    KbvReplaceDialog::perform(QString &oldfile, QString &newfile)
{
  QString       oldname, newname;
  QFileInfo     fileinfo;
  QPixmap       pixmap;

  ui.replPixLbl1->clear();
  ui.replTextLbl1->clear();
  ui.replPixLbl2->clear();
  ui.replTextLbl2->clear();

  //Original file
  fileinfo = QFileInfo(oldfile);
  oldname = fileinfo.fileName() + "\n";
  oldname.append(QString::number(int (fileinfo.size() / 1024.0), 10) + "  " + tr("KiB") + ("\n"));
  oldname.append(fileinfo.lastModified().toString("dd.MM.yyyy - hh:mm:ss"));

  pixmap = QPixmap(oldfile);
  ui.replPixLbl1->setPixmap(pixmap.scaled(ui.replPixLbl1->width(), ui.replPixLbl1->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  ui.replTextLbl1->setText(oldname);

  //new file
  fileinfo = QFileInfo(newfile);
  newname = fileinfo.fileName() + "\n";
  newname.append(QString::number (int (fileinfo.size() / 1024.0), 10) + "  " + tr("KiB") + ("\n"));
  newname.append(fileinfo.lastModified().toString("dd.MM.yyyy - hh:mm:ss"));

  pixmap = QPixmap(newfile);
  ui.replPixLbl2->setPixmap(pixmap.scaled(ui.replPixLbl2->width(), ui.replPixLbl2->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  ui.replTextLbl2->setText(newname);

  return  QDialog::exec();
}

/*************************************************************************//*!
 * Button clicked. Set result code due to button.
 */
void    KbvReplaceDialog::buttonClicked(QAbstractButton *button)
{
  int r = ui.replButtonBox->buttonRole(button);
  int b = ui.replButtonBox->standardButton(button);

  //qDebug() << "KbvReplaceDialog::buttonClicked  role button" << r << b; //#####
  if (r == QDialogButtonBox::YesRole)
    {
      switch (b)
      {
        case QDialogButtonBox::Yes: {setResult(QDialogButtonBox::Yes);}
        break;
        case QDialogButtonBox::YesToAll: {setResult(QDialogButtonBox::YesToAll);}
        break;
        default:{setResult(QDialogButtonBox::No);}
        break;
      }
      done(this->result());
    }
  if (r == QDialogButtonBox::NoRole)
    {
      setResult(QDialogButtonBox::No);
      done(this->result());
    }
  if (r == QDialogButtonBox::RejectRole)
    {
      setResult(QDialogButtonBox::Cancel);
      done(this->result());
    }
}
/****************************************************************************/
