/*****************************************************************************
 * kbv database attributes dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2016-03-13 12:00:38 +0100 (So, 13. Mär 2016) $
 * $Rev: 1080 $
 * Created: 2011.11.25
 *****************************************************************************/
#ifndef KBVDBINFO_H_
#define KBVDBINFO_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdbinfo.h"

class Ui_kbvDBInfoClass;

class KbvDBInfo : public QDialog
{
    Q_OBJECT

public:
    KbvDBInfo(QWidget *parent = nullptr);
    virtual ~KbvDBInfo();

void    setDBInfo(int type, int iconsize, int keywordtype, QString version,
                QString name, QString des, QString location, QString collRoot);
int     getType();
int     getIconSize();
int     getKeyWordType();
QString getVersion();
QString getName();
QString getLocation();
QString getRootDir();
QString getDescription();

private slots:
void    description();
void    finish(QAbstractButton *button);

private:
Ui::kbvDBInfoClass ui;

int     dbType, dbIconSize, dbKeywordType;
QString dbVersion, dbName, dbDescription, dbLocation, collectionRootDir,
        dbTypeCollection, dbTypeKeywords, dbIconDim;
};

#endif // KBVDBINFO_H
/****************************************************************************/
