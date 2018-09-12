/*****************************************************************************
 * kvb rename dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2008.11.12
 *****************************************************************************/
#ifndef KBVRENAMEDIALOG_H_
#define KBVRENAMEDIALOG_H_
#include <QtCore>
#include <QtGui>
#include "ui_kbvrename.h"

class Ui_kbvRenameClass;

class KbvRenameDialog : public QDialog
{
    Q_OBJECT

public:
    KbvRenameDialog(QWidget *parent = nullptr);
    virtual ~KbvRenameDialog();

int       perform(QString &name1, QString &name2, bool multiple);

QString   newFilename = "";     //single rename
QString   prefix = "";
QString   suffix = "";
QString   extension = "";
int       startValue = 0;
int       stepValue = 1;
int       numerals = 0;
int       combination = 0;
bool      multiple = false;

private slots:
void    renStartSpinChanged(const int value);
void    renStepSpinChanged(const int value);
void    renNameEditChanged(const QString &text);
void    renSuffixEditChanged(const QString &text);
void    renExtensionEditChanged(const QString &text);
void    renExtCheckBoxChanged(const int state);

private:
    Ui::kbvRenameClass ui;

void    showPreview();
QString createCountString(int num, int value);
int     getNumerals(QString text, bool zeros);

QString tfNameText="";
QString fileName1="", fileName2="";
QString fileExt1="", fileExt2="";

};

#endif // KBVRENAMEDIALOG_H
/****************************************************************************/
