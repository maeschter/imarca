/*****************************************************************************
 * kvb replace dialog
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-03-08 19:47:29 +0100 (Do, 08. Mär 2018) $
 * $Rev: 1480 $
 * Created: 2011.08.19
 *****************************************************************************/
#ifndef KBVREPLACEDIALOG_H_
#define KBVREPLACEDIALOG_H_
#include <QtCore>
#include <QtGui>
#include "ui_kbvreplace.h"

class Ui_kbvReplaceClass;

class KbvReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    KbvReplaceDialog(QWidget *parent = nullptr);
    virtual ~KbvReplaceDialog();

int     perform(QString &oldfile, QString &newfile);

private slots:
void    buttonClicked(QAbstractButton *button);

private:
    Ui::kbvReplaceClass ui;

};

#endif // KBVREPLACEDIALOG_H
/****************************************************************************/
