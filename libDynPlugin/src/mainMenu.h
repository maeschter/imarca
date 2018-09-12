/*****************************************************************************
 * plugin main menu
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-01-19 15:49:55 +0100 (Fr, 19. Jan 2018) $
 * $Rev: 1387 $
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
