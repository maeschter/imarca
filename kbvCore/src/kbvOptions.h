/*****************************************************************************
 * kbv options dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.02.18
 *****************************************************************************/
#ifndef KBVOPTIONS_H_
#define KBVOPTIONS_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvoptions.h"

/* The class constructs the options dialog. Hence the constructor reads all
 * initial values from kbvSetValues.
 * When the dialog is hidden with OK button all elements are read and written
 * to the config file. Then a signal from kbvSetValues is triggered which
 * informs other components to actualize.
 */
class Ui_kbvOptionsClass;

class KbvOptions : public QDialog
{
    Q_OBJECT

public:
    KbvOptions(QWidget *parent = nullptr);
    virtual ~KbvOptions();


private slots:
  void  mousePressEvent(QMouseEvent *me);
  void  dialogDatabase();
  void  slider(int pos);
  void  accept(void);
  void  reject(void);

private:
  Ui::kbvOptionsClass ui;

  void  presetElements();
  void  saveElements();

  QColorDialog  colorDialog;
  QFileDialog   dataBaseDialog;
  QString       databaseHome;
  int           halfstep;
};

#endif // KBVOPTIONS_H
/****************************************************************************/
