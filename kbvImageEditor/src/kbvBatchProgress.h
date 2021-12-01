/*****************************************************************************
 * kbv question box
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.04.28
 *****************************************************************************/
#ifndef KBVBATCHPROGRESS_H
#define KBVBATCHPROGRESS_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvBatchProgress.h"

class UI_kbvBatchProgressClass;

class KbvBatchProgress : public QDialog
{
  Q_OBJECT
  
public:
  explicit KbvBatchProgress(QWidget *parent = 0);
  virtual ~KbvBatchProgress();

signals:
  void  showReport();
  
public slots:
  void  open(const QString file);
  void  setProgressText(QString text);
  void  setProgressValue(int value);
  void  finished(bool report);

private slots:
  void  buttonClosePressed(bool checked);
  void  buttonCancelPressed(bool checked);
  void  buttonReportPressed(bool checked);

private:
  void  closeEvent(QCloseEvent *event);
  
  Ui::kbvBatchProgressClass  ui;
  int mResult;
};

#endif // KBVBATCHPROGRESS_H
/****************************************************************************/
