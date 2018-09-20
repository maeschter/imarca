/*****************************************************************************
 * plugin main menu
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2018.06.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtGui>
#include <QtDebug>
#include "constants.h"
#include "mainMenu.h"

/*************************************************************************//*!
 */
MainMenu::MainMenu(QWidget *parent) : QMenu(parent=nullptr)
{
    actOpen = new QAction(QIcon(":/plugin/icons/folder-open.png"), tr("Open"), this);
    actOpen->setToolTip(tr("Open an arbitrary object"));
    actClear = new QAction(QIcon(":/plugin/icons/edit-clear.png"), tr("Clear"), this);
    actClear->setIconVisibleInMenu(true);
    actAbout = new QAction(QIcon(":/plugin/icons/help-about.png"), tr("About"), this);

    this->setTitle(QString(pluginName));
    this->setToolTipsVisible(true);
    this->addAction(actOpen);
    this->addSeparator();
    this->addAction(actClear);
    this->addSeparator();
    this->addAction(actAbout);
}
/*****************************************************************************
*/
MainMenu::~MainMenu()
{
  qDebug() << "MainMenu::~MainMenu"; //###########
}

/****************************************************************************/
