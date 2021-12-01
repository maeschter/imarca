/*****************************************************************************
 * kbv setvalues
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.02.25
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvSetvalues.h"
#include "kbvConstants.h"

/*************************************************************************//*!
 * kbvSetValues provides all values which are adjustable by options dialog.
 * The constructors of depending classes read these variables thus
 * kbvSetValues must be the first object created.
 * When any changes are done in options dialog the dialog writes all values
 * back to the config file and triggers signal "settingChanged" to inform
 * all dependent classes.
 */

KbvSetvalues::KbvSetvalues(Format format, Scope scope, const QString & organization,
                            const QString & application, QObject *parent) :
                            QSettings(format, scope, organization, application, parent)
{
  readValues();
}

KbvSetvalues::~KbvSetvalues()
{
  //qDebug() << "KbvSetvalues::~KbvSetvalues"; //###########
}

/*************************************************************************//*!
 * Read all values from config file.
 * When the config-file doesn't exist the default values are used.
 */
void  KbvSetvalues::readValues()
{
  //General
  this->beginGroup("App");
  this->beginGroup("Options-General");
  dataBaseDir = this->value("dataBaseDir", QDir::homePath() + QString(configDir)).toString();
  this->endGroup();

  // kbvView
  this->beginGroup("Options-View");
  iconSize = this->value("iconSize", KbvConf::stdIconSize).toInt();
  showHiddenFiles = this->value("showHiddenFiles", false).toBool();
  saveDirTreeState = this->value("saveDirTreeState", false).toBool();
  showFileName = this->value("showFileName", false).toBool();
  showFileSize = this->value("showFileSize", false).toBool();
  showImageSize = this->value("showImageSize", false).toBool();
  dirWatchCycle = 1000 * (this->value("dirWatchCycle", 3).toInt());
  this->endGroup();

  this->beginGroup("Options-Slide");
  slideChangeManual = this->value("checkManual", true).toBool();
  slideFullScreen = this->value("checkFullScreen", false).toBool();
  slideStretch = this->value("checkStretch", false).toBool();
  slideStopAtDirEnd = this->value("checkStopAtDirEnd", false).toBool();
  slideDelaySec = this->value("slideDelay", 2).toInt();
  slideChangeId = this->value("slideChangeId", KbvConf::slideReplace).toInt();
  slideBackColour = this->value("slideBackColour", QColor(Qt::black)).value<QColor>();
  this->endGroup();
  this->endGroup(); //group App
}

/*************************************************************************//*!
 * Options dialog confirmed. Read values and inform all dependent classes.
 */
void  KbvSetvalues::setvaluesChanged()
{
  readValues();
  emit settingsChanged();
}

/****************************************************************************/
