/*****************************************************************************
 * kbvTabWidget
 * This is the tab widget for file tab, search tab and collection tabs
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-12-12 10:52:08 +0100 (Di, 12. Dez 2017) $
 * $Rev: 1367 $
 * Created: 2013.01.06
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvConstants.h"
#include "kbvTabWidget.h"


KbvTabWidget::KbvTabWidget(QWidget *parent) : QWidget(parent)
{
  //Insert a vertical box layout with a small control/info bar and below a
  //wide area where the view gets installed.
  //The control/info bar uses a horizontal box layout and consists of a
  //menu button containing the sort criteria (sort roles), two push buttons
  //for sort order and three info buttons to display text.

  tabWidgetLayout = new QVBoxLayout(this);
  tabWidgetLayout->setSpacing(1);
  tabWidgetLayout->setContentsMargins(0,0,0,0);
  this->setLayout(tabWidgetLayout);

  barWidget = new QWidget(this);
  barWidget->setMinimumHeight(21);
  barWidget->setMaximumHeight(21);

  barLayout = new QHBoxLayout(barWidget);
  barLayout->setSpacing(0);
  barLayout->setContentsMargins(0,0,0,0);

  barWidget->setLayout(barLayout);

  tabWidgetLayout->addWidget(barWidget, 0, 0);

  this->createMenu();

  QFont font;
  font.setPointSize(9);
  buttGroup = new QButtonGroup(nullptr);

  sortButton = new QPushButton(barWidget);
  sortButton->setFont(font);
  sortButton->setContentsMargins(0,0,0,0);
  sortButton->setCheckable(false);
  sortButton->setChecked(false);
  sortButton->setText(QString(tr("Sort by")));
  sortButton->setMenu(sortMenu);
  barLayout->addWidget(sortButton, 0, 0);

  button = new QPushButton(generalFunc.iconSortAsc, "", barWidget);
  button->setFont(font);
  button->setMaximumWidth(24);
  button->setCheckable(true);
  button->setChecked(true);
  barLayout->addWidget(button, 0, 0);
  buttGroup->addButton(button, Kbv::up);

  button = new QPushButton(generalFunc.iconSortDesc, "", barWidget);
  button->setFont(font);
  button->setMaximumWidth(24);
  button->setCheckable(true);
  barLayout->addWidget(button, 0, 0);
  buttGroup->addButton(button, Kbv::down);

  viewButton = new QPushButton(generalFunc.iconViewModeIcon, "", barWidget);
  viewButton->setFont(font);
  viewButton->setMaximumWidth(24);
  viewButton->setCheckable(true);
  viewButton->setChecked(false);
  barLayout->addWidget(viewButton, 0, 0);

  infoButton1 = new QPushButton(barWidget);
  infoButton1->setFont(font);
  infoButton1->setMinimumWidth(60);
  infoButton1->setMaximumWidth(60);
  infoButton1->setCheckable(false);
  infoButton1->setChecked(false);
  barLayout->addWidget(infoButton1, 0, 0);

  infoButton2 = new QPushButton(barWidget);
  infoButton2->setFont(font);
  infoButton2->setMinimumWidth(60);
  infoButton2->setMaximumWidth(60);
  infoButton2->setCheckable(false);
  infoButton2->setChecked(false);
  barLayout->addWidget(infoButton2, 0, 0);

  infoButton3 = new QPushButton(barWidget);
  infoButton3->setFont(font);
  infoButton3->setMinimumWidth(150);
  infoButton3->setCheckable(false);
  infoButton3->setChecked(false);
  barLayout->addWidget(infoButton3, 10, 0);

  connect(sortMenu,   SIGNAL(triggered(QAction*)),  this, SLOT(sortRoleSelected(QAction*)));
  connect(buttGroup,  SIGNAL(buttonClicked(int)),   this, SLOT(sortbuttonClicked(int)));
  connect(viewButton, SIGNAL(clicked(bool)),        this, SLOT(viewbuttonClicked(bool)));

  viewMode = QListView::IconMode;
}

KbvTabWidget::~KbvTabWidget()
{
  //qDebug() << "KbvTabWidget::~KbvTabWidget"; //###########
  delete buttGroup;
}
/*************************************************************************//*!
 *Set the main widget
 */
void    KbvTabWidget::addWidget(QWidget *widget)
{
  this->tabWidgetLayout->addWidget(widget, 10, 0);
}
/*************************************************************************//*!
 *SLOTS: Set the text of info buttons 1, 2 and 3
 */
void    KbvTabWidget::setInfoText1(QString text)
{
  infoButton1->setText(text);
}
void    KbvTabWidget::setInfoText2(QString text)
{
  infoButton2->setText(text);
}
void    KbvTabWidget::setInfoText3(QString text)
{
  infoButton3->setText(text);
}
/*************************************************************************//*!
 *SLOT: view button (list view / icon view) has been clicked
 */
void    KbvTabWidget::viewbuttonClicked(bool checked)
{
  Q_UNUSED(checked)

  if(viewMode == QListView::IconMode)
    {
      viewMode = QListView::ListMode;
      viewButton->setIcon(generalFunc.iconViewModeList);
    }
  else
    {
      viewMode = QListView::IconMode;
      viewButton->setIcon(generalFunc.iconViewModeIcon);
    }
  //qDebug() << "KbvTabWidget::viewButtonClicked" << viewMode;//###########
  emit  viewModeChanged(viewMode);
}
/*************************************************************************//*!
 *SLOT: a sort button (triangle up / down) has been clicked
 */
void    KbvTabWidget::sortbuttonClicked(int index)
{
  //qDebug() << "KbvTabWidget::sortbuttonClicked" <<index;//###########
  emit  sortButtonClicked(index);
}
/*************************************************************************//*!
 *SLOT: a sort role has been selected by the sort button
 */
void    KbvTabWidget::sortRoleSelected(QAction *action)
{
  int   sortRole;

  actFileName->setIconVisibleInMenu(false);
  actFileSize->setIconVisibleInMenu(false);
  actFileDate->setIconVisibleInMenu(false);
  actImageSize->setIconVisibleInMenu(false);
  actUserDate->setIconVisibleInMenu(false);

  sortRole = action->data().toInt();
  //qDebug() << "KbvTabWidget::sortRoleSelected role" <<sortRole;//###########
  switch(sortRole)
  {
    case Kbv::sortFileName:
    {
      sortButton->setText(actFileName->text());
      actFileName->setIconVisibleInMenu(true);
      emit  sortRoleChanged(sortRole);
    }
    break;
    case Kbv::sortFileSize:
    {
      sortButton->setText(actFileSize->text());
      actFileSize->setIconVisibleInMenu(true);
      emit  sortRoleChanged(sortRole);
    }
    break;
    case Kbv::sortFileDate:
    {
      sortButton->setText(actFileDate->text());
      actFileDate->setIconVisibleInMenu(true);
      emit  sortRoleChanged(sortRole);
    }
    break;
    case Kbv::sortImageSize:
    {
      sortButton->setText(actImageSize->text());
      actImageSize->setIconVisibleInMenu(true);
      emit  sortRoleChanged(sortRole);
    }
    break;
    case Kbv::sortUserDate:
    {
      sortButton->setText(actUserDate->text());
      actUserDate->setIconVisibleInMenu(true);
      emit  sortRoleChanged(sortRole);
    }
    break;
    default:
      {
        emit  sortRoleChanged(Kbv::sortNone);
      }
  }
}
/*************************************************************************//*!
 *Create the menu for sort button
 */
void    KbvTabWidget::createMenu()
{
  actFileName = new QAction(tr("File name"), this);
  actFileName->setData(QVariant(Kbv::sortFileName));
  actFileName->setIcon(generalFunc.iconCircle);
  actFileName->setIconVisibleInMenu(true);
  
  actFileSize = new QAction(tr("File size"), this);
  actFileSize->setData(QVariant(Kbv::sortFileSize));
  actFileSize->setIcon(generalFunc.iconCircle);
  actFileSize->setIconVisibleInMenu(false);
  
  actFileDate = new QAction(tr("File date"), this);
  actFileDate->setData(QVariant(Kbv::sortFileDate));
  actFileDate->setIcon(generalFunc.iconCircle);
  actFileDate->setIconVisibleInMenu(false);
  
  actImageSize = new QAction(tr("Image size"), this);
  actImageSize->setData(QVariant(Kbv::sortImageSize));
  actImageSize->setIcon(generalFunc.iconCircle);
  actImageSize->setIconVisibleInMenu(false);
  
  actUserDate = new QAction(tr("User date"), this);
  actUserDate->setData(QVariant(Kbv::sortUserDate));
  actUserDate->setIcon(generalFunc.iconCircle);
  actUserDate->setIconVisibleInMenu(false);

  sortMenu = new QMenu(this);
  sortMenu->addAction(actFileName);
  sortMenu->addAction(actFileSize);
  sortMenu->addAction(actFileDate);
  sortMenu->addAction(actImageSize);
  sortMenu->addAction(actUserDate);
}
/****************************************************************************/
