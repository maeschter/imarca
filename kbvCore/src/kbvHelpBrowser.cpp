/*****************************************************************************
 * KbvHelpBrowser
 * This is the tool to display and browse help.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2014.05.19
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvHelpBrowser.h"

KbvHelpBrowser::KbvHelpBrowser(QHelpEngine* helpEngine, QWidget *parent) :  QTextBrowser(parent), helpEng(helpEngine)
{
  
}
KbvHelpBrowser::~KbvHelpBrowser()
{
  //qDebug() << "KbvHelpBrowser::~KbvHelpBrowser"; //###########
}

/*************************************************************************//*!
 * SLOT. Called from help window.
 * Load file from compressed help and set content of browser.
 */
void  KbvHelpBrowser::openUrl(const QUrl& url)
{
  QUrl        newUrl;
  QVariant    help;
  QByteArray  ba;
  QString     helptext, anchor;
  
  if (url.scheme() == "qthelp")
    {
      //get fragment and remove it from url
      anchor = url.fragment();
      newUrl = url;
      newUrl.setFragment(QString());
  
      //remember url, load help and convert to text or html
      if(activeUrl != newUrl)
        {
          activeUrl = newUrl;
          help = this->loadResource(QTextDocument::HtmlResource, activeUrl);
          //qDebug() << "KbvHelpBrowser::openUrl url anchor type"<<newUrl <<anchor <<help.type(); //###########
      
          if(help.canConvert(QMetaType::QString))
            {
              helptext = help.toString();
              this->setHtml(helptext);
            }
          if(help.canConvert(QMetaType::QByteArray))
            {
              ba = help.toByteArray();
              QTextCodec *codec = Qt::codecForHtml(ba);
              helptext = codec->toUnicode(ba);
              this->setHtml(helptext);
              this->scrollToAnchor(anchor);
            }
        }
      else
        {
          //qDebug() << "KbvHelpBrowser::openUrl scroll to" <<anchor; //###########
          this->scrollToAnchor(anchor);
        }
    }
}
/*************************************************************************//*!
 * Helpbrowser or openUrl() requests a file from compressed help.
 */
QVariant KbvHelpBrowser::loadResource(int type, const QUrl& url)
{
  Q_UNUSED(type)
  QVariant        var;
  QString         str, fileUrl;
  
  if (url.scheme() == "qthelp")
    {
      var = QVariant(helpEng->fileData(url));
    }
  else
    {
      //create a resource url like "qthelp://org.kbv.core/help/images/picture-name.png"
      //from relative file paths for loading images
      str = url.toString(QUrl::RemoveFragment);
      
      fileUrl = "qthelp://" + QString(kbvHelpNamespace) + "/" + QString(kbvHelpVirtualFolder) + "/";
      fileUrl.append(url.path());
      var = QVariant(helpEng->fileData(QUrl(fileUrl)));
    }
  //qDebug() << "KbvHelpBrowser::loadResource url str fileUrl"<<url <<str <<fileUrl; //###########
  return var;
}
/****************************************************************************/
