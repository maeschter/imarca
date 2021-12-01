/*****************************************************************************
 * kbv data record dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2013.12.03
 *****************************************************************************/
#ifndef KBVRECORDINFO_H_
#define KBVRECORDINFO_H_
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "kbvGeneral.h"
#include "ui_kbvrecordinfo.h"

class Ui_KbvRecordInfoClass;

class KbvRecordInfo : public QDialog
{
    Q_OBJECT

public:
    KbvRecordInfo(QWidget *parent = nullptr);
    virtual ~KbvRecordInfo();

  int     perform(QList<QVariant> pkl, QString dbname, QString dbPath);

signals:

private slots:
  void    addToAllRecords(bool checked);
  void    saveRecordData(bool checked);
  void    discard(bool checked);
  void    next(bool checked);
  void    previous(bool checked);
  void    today(bool checked);
  void    enableUserDate(int state);
  void    userDate(QDate date);
  void    userComment();
  void    userKeywords();

private:
  Ui::kbvRecordInfoClass ui;
  void    updateRecordInfo();
  void    readRecord(QVariant pk);
  void    updateRecord(QVariant pk);
  void    keyPressEvent(QKeyEvent *event);
  void    closeEvent(QCloseEvent *event);

  KbvGeneral      generalFunc;
  QString         recName, recPath, recImageSize, recKeywords, recDateChanged,
                  recFileSize, recCRC32, recUserComment, recUserKeywords;
  QPixmap         recIcon;
  QDate           recUserdate, nonsenseDate;
  QSqlDatabase    infoDB;
  QString         dbconnection, dbName;
  int             dbKeywordType, indexNo;
  QList<QVariant>         pkList;
  QMap<QString, QVariant> dataSet;
};

#endif // KBVRECORDINFO_H
/****************************************************************************/
