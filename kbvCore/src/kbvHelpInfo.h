/*****************************************************************************
 * kbv help content dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2009.10.17
 *****************************************************************************/
#ifndef KBVHELPINFO_H_
#define KBVHELPINFO_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvhelpinfo.h"

class Ui_kbvHelpInfoClass;

class KbvHelpInfo : public QDialog
{
    Q_OBJECT

public:
    KbvHelpInfo(QWidget *parent = nullptr);
    virtual ~KbvHelpInfo();

void    showEvent(QShowEvent *event);
void    setPluginsList(const QStringList &plugins);
void    setLibraryInfos(const QStringList &info);

private slots:
void    accept(void);
void    scrollAuthorsByTimer();
void    pluginsTabActivated(int index);
void    specialTabActivated(int index);
void    specialApply(void);
void    comboItemActivated(int index);

private:
Ui::kbvHelpInfoClass ui;

QStringList author, pluginsList;
QTimer      authorTimer;
int         index;
QString     database;
};

#endif // KBVHELPINFO_H
/****************************************************************************/
