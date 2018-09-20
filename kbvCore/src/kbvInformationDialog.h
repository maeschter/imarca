/*****************************************************************************
 * KbvInformationDialog
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.07.02
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
