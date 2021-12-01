/*****************************************************************************
 * plugin main menu
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2018.06.06
 ****************************************************************************/
#ifndef MAINMENU_H_
#define MAINMENU_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>



class MainMenu : public QMenu
{
    Q_OBJECT

public:
  MainMenu(QWidget *parent=nullptr);
  virtual ~MainMenu();

private:
  QAction   *actOpen, *actClear, *actAbout;

};
#endif // MAINMENU_H
/****************************************************************************/
