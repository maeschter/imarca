/*****************************************************************************
 * kbv question box
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2017.04.28
 *****************************************************************************/
#ifndef KBVBATCHDIALOG_H
#define KBVBATCHDIALOG_H
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvConstants.h"
#include "ui_kbvBatchWizard.h"

class UI_kbvBatchWizardClass;

class KbvBatchDialog : public QDialog
{
  Q_OBJECT
  
public:
  explicit KbvBatchDialog(QWidget *parent = 0);
  virtual ~KbvBatchDialog();
  
public slots:
  int   exec(Kbv::kbvBatchParam &params);
  
private slots:
  void  leftButtonPressed(bool checked);
  void  middleButtonPressed(bool checked);
  void  rightButtonPressed(bool checked);
  void  buttonFlipHorizontal(bool checked);
  void  buttonFlipVertical(bool checked);
  void  buttonRotateExif(bool checked);
  void  dialRotate(int val);
  void  doubleSpinRotate(double val);
  void  spinHeightPixels(int val);
  void  spinWidthPixels(int val);
  void  spinPercent(double val);
  void  comboFileFormat(const int index);
  void  buttonSelectPressed(bool checked);
  void  buttonStartPressed(bool checked);

private:
  void  closeEvent(QCloseEvent *event);
  
  Ui::kbvBatchWizardClass  ui;
  int mResult, mPages;
};

#endif // KBVBATCHDIALOG_H
/****************************************************************************/
