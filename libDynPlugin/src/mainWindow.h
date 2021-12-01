/*****************************************************************************
 * plugin main window.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2018.06.06
 ****************************************************************************/
#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(QWidget *parent=nullptr);
  virtual ~MainWindow();

  QMenu    *mainMenu;

private:
  void      changeEvent(QEvent *e);

//  QAction   *actOpen, *actQuit;

};
#endif // MAINWINDOW_H
/****************************************************************************/
