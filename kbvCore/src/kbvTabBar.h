/* kbvCollectionTabs
 * This is the tab widget for file, search, album and collection tabs
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2011.10.12
 ****************************************************************************/
#ifndef KBVTABBAR_H_
#define KBVTABBAR_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class KbvTabBar : public QTabBar
{
  Q_OBJECT

public:
  KbvTabBar(QWidget * parent = nullptr);
  virtual
  ~KbvTabBar();

signals:
void    keyOrMouseSelTab(int index);
void    closeTab(int index);

private:
void    mouseDoubleClickEvent(QMouseEvent *event);
void    keyPressEvent(QKeyEvent *event);

};
#endif /* KBVTABBAR_H_ */
/****************************************************************************/
