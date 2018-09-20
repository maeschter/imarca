/*****************************************************************************
 * kbv db import/export progress dialog
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2017.01.31
 *****************************************************************************/
#ifndef KBVDBIMEXPORTPROGRESS_H_
#define KBVDBIMEXPORTPROGRESS_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdbimexprogress.h"

/*! Class KbvDBImExportProgress.
 * The class constructs the progress dialog for database import/export.
 */
class Ui_kbvDBImExProgress;

class KbvDBImExportProgress : public QDialog
{
    Q_OBJECT

public:
  KbvDBImExportProgress(QWidget *parent = nullptr);
  virtual ~KbvDBImExportProgress();

signals:
  void  cancel();

public slots:
  void  setTitle(QString text, int type);
  void  setInfotext(QString text);
  void  setFilecount(int count);
  void  setDiskSize(quint64 size);
  void  setProgress(int progress);
  void  closeEvent(QCloseEvent *event);

private slots:
  void  buttonClicked();

private:
  Ui::kbvDBImExProgressClass ui;
  QString textClose, textCancel;
  int     action;
  bool    finished;
};

#endif // KBVDBIMEXPORTPROGRESS_H
/****************************************************************************/
