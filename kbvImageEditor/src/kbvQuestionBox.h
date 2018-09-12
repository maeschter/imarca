/*****************************************************************************
 * kbv question box
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2017.04.28
 *****************************************************************************/
#ifndef KBVQUESTIONBOX_H
#define KBVQUESTIONBOX_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvQuestionBox.h"

class UI_kbvQuestionBoxClass;

class KbvQuestionBox : public QDialog
{
  Q_OBJECT
  
public:
  explicit KbvQuestionBox(QWidget *parent = 0);
  virtual ~KbvQuestionBox();
  
public slots:
  int   exec(const QString caption="", const QString text="",
             const QString leftButtonText="", const QString middleButtonText="", const QString rightButtonText="");
  
private slots:
  void  leftButtonPressed(bool checked);
  void  middleButtonPressed(bool checked);
  void  rightButtonPressed(bool checked);
  
private:
  void  closeEvent(QCloseEvent *event);
  
  Ui::kbvQuestionBoxClass  ui;
  int mResult;
};

#endif // KBVQUESTIONBOX_H
/****************************************************************************/
