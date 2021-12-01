/*****************************************************************************
 * Dynamic plugin interface
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2018.06.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "constants.h"
#include "interface.h"

Interface::~Interface()
{
  //qDebug() << "Interface::~Interface libDynPlugin";//###########
  //cleanupPluginResource();

  delete mainWin;
}
/*************************************************************************//*!
 * Initialise ressources etc. Do this when the related translation has been
 * loaded in kbvCore and retranslation of gui isn't neccessary.
 */
void   Interface::initialisePlugin()
{
  //initPluginResource();
  mainWin = new MainWindow(nullptr);

  pluginInfo.name = QString(pluginName);
  pluginInfo.version = QString(pluginVersion);
  pluginInfo.author = QString(pluginAuthor);
  pluginInfo.abstract = QString(pluginAbstract);

  //qDebug() << "Interface::initialisePlugin done"; //###########
}
/*************************************************************************//*!
 * Plugin info
 */
KbvDynamicPluginInterface::PLUGIN_INFO   Interface::getPluginInfo() const
{
  return pluginInfo;
}
/*************************************************************************//*!
 * Provide menu
 */
QMenu*  Interface::getPluginMenu() const
{
  return mainWin->mainMenu;
}

/****************************************************************************/
