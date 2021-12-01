/* kbvTabWidget
 * This is the tab widget for file tab, search tab and collection tabs
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2013.01.06
 ****************************************************************************/
#ifndef KBVTABWIDGET_H_
#define KBVTABWIDGET_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "kbvGeneral.h"

class KbvTabWidget : public QWidget
{
  Q_OBJECT

public:
  KbvTabWidget(QWidget *parent = nullptr);
  virtual
  ~KbvTabWidget();

void  addWidget(QWidget *widget);

public slots:
void  setInfoText1(QString text);
void  setInfoText2(QString text);
void  setInfoText3(QString text);

signals:
void    sortRoleChanged(int role);
void    sortButtonClicked(int button);
void    viewModeChanged(int mode);

private slots:
void    sortRoleSelected(QAction *action);
void    sortbuttonClicked(int index);
void    viewbuttonClicked(bool checked);

private:
void    createMenu();

KbvGeneral      generalFunc;
QVBoxLayout     *tabWidgetLayout;
QHBoxLayout     *barLayout;
QButtonGroup    *buttGroup;
QPushButton     *button, *sortButton, *viewButton, *infoButton1, *infoButton2, *infoButton3;
QComboBox       *combo;
QWidget         *barWidget;

QMenu           *sortMenu;
QAction         *actFileName, *actFileSize, *actFileDate, *actImageSize, *actUserDate;
int             viewMode;
};
#endif /* KBVTABWIDGET_H_ */
/****************************************************************************/
