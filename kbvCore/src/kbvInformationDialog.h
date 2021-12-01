/*****************************************************************************
 * KbvInformationDialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2012.07.02
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVINFORMATIONDIALOG_H_
#define KBVINFORMATIONDIALOG_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class KbvInformationDialog : public QMessageBox
{
  Q_OBJECT

public:
    KbvInformationDialog(QWidget *parent = nullptr);
virtual
    ~KbvInformationDialog();

int     perform(QString caption, QString infoText, int iconIndex);
void    setCaptionAndIcon(QString caption, QString infoText, int iconIndex);

};
#endif /* KBVINFORMATIONDIALOG_H_ */
/****************************************************************************/
