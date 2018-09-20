/*****************************************************************************
 * KbvHelp.cpp
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2010.11.12
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvHelp.h"


KbvHelp::KbvHelp(QWidget *parent) : QMainWindow(parent)
{
  QString       help, nameSpace;
  QSizePolicy   sizepol;
  QStringList   registeredHelp;
  QDir          pluginsDir;
  
  this->hide();
  this->setWindowTitle(QString(appName));
  windowSize = QSize(700, 500);
  windowPosition = QPoint(100, 100);
  this->resize(windowSize);
  this->move(windowPosition);
  
  helpEngine = new QHelpEngine(help, nullptr);
  helpEngine->contentWidget()->setMinimumWidth(300);
  sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
  sizepol.setVerticalPolicy(QSizePolicy::Expanding);
  sizepol.setHorizontalStretch(KbvConf::HorStretchLeft);
  helpEngine->contentWidget()->setSizePolicy(sizepol);
  
  //Load help files for given locale.
  mLocale = QLocale::system().name();
  
  //The installed app loads from "var/lib/imarca" (Debian)
  //The test version loads from application dir (devel environment).
  mAppdir = QCoreApplication::applicationDirPath();
  if(mAppdir.startsWith(QString(appInstallDir)))
    {
      //the real installation
      mAppdir = QString(helpInstallDir);
    }
  else
    {
      //app in development environment /kbvCore/prd, search in /test
      pluginsDir.setPath(QString(mAppdir));
      pluginsDir.cdUp();
      pluginsDir.cdUp();
      pluginsDir.cd("test");
      mAppdir = pluginsDir.absolutePath();
    }
  mAppdir.append("/");
  //qDebug() << "KbvHelp::KbvHelp help dir" << appdir; //###########
  mStaticNameSpaces.clear();

  //setup kbvCore collection file
  //register other help docs when core collection has been setup
  help = generalFunc.getFileForLocale(mAppdir, "kbvCore", mLocale, ".qhc");
  if(!help.isEmpty())
    {
      //setup kbvCore collection file
      helpEngine->setCollectionFile(mAppdir+help);
      helpEngine->setupData();

      //get already registered help files
      registeredHelp = helpEngine->registeredDocumentations();

      //load and register kbvCore help file
      help = generalFunc.getFileForLocale(mAppdir, "kbvCore", mLocale, ".qch");
      //qDebug() << "KbvHelp::KbvHelp file" <<mAppdir+help; //###########
      nameSpace = helpEngine->namespaceName(mAppdir+help);
      if(!registeredHelp.contains(nameSpace))
        {
          helpEngine->registerDocumentation(mAppdir+help);
        }
      mStaticNameSpaces.append(nameSpace);
      //static plugin libImageMetadata, register help file
      help = generalFunc.getFileForLocale(mAppdir, "kbvMetadata", mLocale, ".qch");
      //qDebug() << "KbvHelp::KbvHelp file" <<mAppdir+help; //###########
      nameSpace = helpEngine->namespaceName(mAppdir+help);
      if(!registeredHelp.contains(nameSpace))
        {
          helpEngine->registerDocumentation(mAppdir+help);
        }
      mStaticNameSpaces.append(nameSpace);

      //static plugin libImageEditor, register help file
      help = generalFunc.getFileForLocale(mAppdir, "kbvImageEditor", mLocale, ".qch");
      //qDebug() << "KbvHelp::KbvHelp file" <<mAppdir+help; //###########
      nameSpace = helpEngine->namespaceName(mAppdir+help);
      if(!registeredHelp.contains(nameSpace))
        {
          helpEngine->registerDocumentation(mAppdir+help);
        }
      mStaticNameSpaces.append(nameSpace);

      //qDebug() << "KbvHelp::KbvHelp last error" << helpEngine->error(); //###########
      //qDebug() << "KbvHelp::KbvHelp registered" << helpEngine->registeredDocumentations(); //###########
  }

  helpBrowser = new KbvHelpBrowser(helpEngine, this);
  sizepol.setHorizontalPolicy(QSizePolicy::Expanding);
  sizepol.setVerticalPolicy(QSizePolicy::Expanding);
  sizepol.setHorizontalStretch(KbvConf::HorStretchRight);
  helpBrowser->setSizePolicy(sizepol);

  naviButtons = new QToolBar("Navigation", nullptr);
  naviButtons->setMinimumWidth(120);
  naviButtons->addAction(generalFunc.iconHome, "home");
  naviButtons->addAction(generalFunc.iconGoBack, "left");
  naviButtons->addAction(generalFunc.iconGoForward, "right");
  this->addToolBar(Qt::TopToolBarArea, naviButtons);

  helpPanel = new QSplitter(Qt::Horizontal, this);
  helpPanel->setChildrenCollapsible(false);
  helpPanel->setHandleWidth(4);
  helpPanel->setStretchFactor(0, KbvConf::HorStretchLeft);
  helpPanel->setStretchFactor(1, KbvConf::HorStretchRight);
  helpPanel->insertWidget(0, helpEngine->contentWidget());
  helpPanel->insertWidget(1, helpBrowser);
  this->setCentralWidget(helpPanel);

  connect(helpEngine->contentWidget(), SIGNAL(linkActivated(const QUrl&)), helpBrowser, SLOT(openUrl(const QUrl&)));
  connect(naviButtons, SIGNAL(actionTriggered(QAction*)), this, SLOT(naviClicked(QAction*)));
}
/*************************************************************************/
KbvHelp::~KbvHelp()
{
  //qDebug() << "KbvHelp::~KbvHelp"; //###########
  delete  helpEngine;
  delete  naviButtons;
}
/*************************************************************************//*!
 * SLOT. Called from main menu. Show help content window.
 */
void KbvHelp::showHelpContent()
  {
    this->show();
  }
/*************************************************************************//*!
 * Return namespaces of static plugins help.
 */
QStringList KbvHelp::staticNameSpaces()
{
  return mStaticNameSpaces;
}
/*************************************************************************//*!
 * Navigate forward, backward in history.
 */
void KbvHelp::naviClicked(QAction *act)
{
  if(act->text() == "home")
    {
      //qDebug() << "KbvHelp::naviClicked" <<act->text(); //###########
      helpBrowser->home();
    }
  if(act->text() == "left")
    {
      //qDebug() << "KbvHelp::naviClicked" <<act->text() <<helpBrowser->backwardHistoryCount(); //###########
      helpBrowser->backward();
    }
  if(act->text() == "right")
    {
      //qDebug() << "KbvHelp::naviClicked" <<act->text() <<helpBrowser->forwardHistoryCount(); //###########
      helpBrowser->forward();
    }
}
/*************************************************************************//*!
 * Quit help window. Remember position and size;
 */
void    KbvHelp::closeEvent(QCloseEvent *event)
{
  windowSize = this->size();
  windowPosition = this->pos();
  event->accept();        //action_quit
}
/****************************************************************************/
