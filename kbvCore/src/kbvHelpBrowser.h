/*****************************************************************************
 * KbvHelpBrowser
 * This is the tool to display and browse help.
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2014.05.19
 ******************************************************************************/
#ifndef KBVHELPBROWSER_H_
#define KBVHELPBROWSER_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtHelp>

class KbvHelpBrowser : public QTextBrowser
{
  Q_OBJECT

public:
  KbvHelpBrowser(QHelpEngine* helpEngine, QWidget *parent = nullptr);
  virtual ~KbvHelpBrowser();
  
  QVariant  loadResource(int type, const QUrl& url);
  
public slots:
  void      openUrl(const QUrl& url);
  
private:
  QHelpEngine     *helpEng;
  QUrl            activeUrl;

};
#endif // KBVHELPBROWSER_H
/****************************************************************************/
