/*****************************************************************************
 * plugin main window
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2018.06.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtGui>
#include <QtDebug>
#include "mainWindow.h"
#include "mainMenu.h"

/*************************************************************************//*!
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent=nullptr)
{
    mainMenu = new  MainMenu(this);

}
/*****************************************************************************
*/
MainWindow::~MainWindow()
{
  //qDebug() << "MainWindow::~MainWindow"; //###########
}

/*****************************************************************************
 * retranslate >>> first load translator then create UI and all translatable widgets
 * >>>>> unused <<<<<
*/
void MainWindow::changeEvent(QEvent *e)
{
  qDebug() << "MainWindow::changeEvent"; //###########
  QMainWindow::changeEvent(e);
}

/****************************************************************************/
