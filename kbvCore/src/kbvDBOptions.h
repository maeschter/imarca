/*****************************************************************************
 * kbv dboptions dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.02.18
 *****************************************************************************/
#ifndef KBVDBOPTIONS_H_
#define KBVDBOPTIONS_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdboptions.h"

/*! Class kbvDBOptions.
 * The class constructs the options dialog for database settings icon size
 * and description. These settings are saved in the database.
 */
class Ui_kbvDBOptionsClass;

class KbvDBOptions : public QDialog
{
    Q_OBJECT

public:
  KbvDBOptions(QWidget *parent = nullptr);
  virtual ~KbvDBOptions();

  int       iconsize();
  int       databaseType();
  int       keywordType();
  QString   rootDir();
  QString   databaseName();
  QString   description();

private slots:
  void  accept(void);
  void  reject(void);
  void  slider(int pos);
  void  typeButton(bool checked);
  void  keywordCheck(int state);
  void  dialogRootDir();

private:
  Ui::kbvDBOptionsClass ui;

  QFileDialog   rootDirDialog;
  QString   dbname, dbcomment, collrootdir, userHome, warntext1, warntext2;
  int       dbtype, dbicon, dbkeywordtype, halfstep;
};

#endif // KBVDBOPTIONS_H
/****************************************************************************/
