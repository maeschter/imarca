/*****************************************************************************
 * kbv database search dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-05-15 19:45:47 +0200 (Mo, 15. Mai 2017) $
 * $Rev: 1339 $
 * Created: 2012.00.16
 *****************************************************************************/
#ifndef KBVDBSEARCHDIALOG_H_
#define KBVDBSEARCHDIALOG_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdbsearch.h"

class Ui_kbvDBSearchClass;

class KbvDBSearchDialog : public QDialog
{
    Q_OBJECT

public:
    KbvDBSearchDialog(QWidget *parent = nullptr);
    virtual ~KbvDBSearchDialog();

QStringList getKeywordList();
QStringList getPathsList();
int         getKeywordLogic();
int         getSearchLogic();
QDate       getDate(int index);

public slots:
int         perform(const QString name);

private slots:
void    enableDate(int state);
void    enableTermsPaths(const QString text);
void    finishApply(bool checked);
void    finishAbort(bool checked);
void    extractParameter();

private:
Ui::kbvDBSearchClass ui;

QStringList searchKeywords, searchPaths;
QDate       date;
};

#endif // KBVDBSEARCHDIALOG_H
/****************************************************************************/
