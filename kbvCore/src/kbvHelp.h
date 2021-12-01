/*****************************************************************************
 * KbvHelp.h
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2010.11.12
 ****************************************************************************/
#ifndef KBVHELP_H_
#define KBVHELP_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtHelp>
#include "kbvGeneral.h"
#include "kbvHelpBrowser.h"

class KbvHelp : public QMainWindow
{
  Q_OBJECT

public:
  KbvHelp(QWidget *parent=nullptr);

  virtual   ~KbvHelp();

  void  showHelpContent();
  QStringList staticNameSpaces();

  QHelpEngine     *helpEngine;

private slots:
void    naviClicked(QAction *act);

private:
  KbvGeneral      generalFunc;
  QSplitter       *helpPanel;
  KbvHelpBrowser  *helpBrowser;
  QSize           windowSize;
  QPoint          windowPosition;
  QToolBar        *naviButtons;
  QString         mLocale, mAppdir;
  QStringList     mStaticNameSpaces;

  
  void  closeEvent(QCloseEvent *event);
};
#endif /* KBVHELP_H_ */
/****************************************************************************/
