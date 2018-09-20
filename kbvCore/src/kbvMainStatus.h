/*****************************************************************************
 * kbv main status bar.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.09.27
 ****************************************************************************/
#ifndef KBVMAINSTATUS_H_
#define KBVMAINSTATUS_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class KbvMainStatus : public QStatusBar
{
  Q_OBJECT

public:
  KbvMainStatus(QWidget *parent = nullptr);
  virtual
  ~KbvMainStatus();

public slots:
  void  setProgressValue(int promille); /*! Set value of progress bar */
  void  setStatusText1(QString text);
  void  setStatusText2(QString text);
  void  setStatusText3(QString text);

private:
  QProgressBar  *kbvStatusProgress;
  QLabel        *kbvStatusLbl1, *kbvStatusLbl2, *kbvStatusLbl3;

};
#endif /* KBVMAINSTATUS_H_ */
/****************************************************************************/
