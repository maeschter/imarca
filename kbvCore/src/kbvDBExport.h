/*****************************************************************************
 * kbv dbexport dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2016.12.08
 *****************************************************************************/
#ifndef KBVDBEXPORT_H_
#define KBVDBEXPORT_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdbexport.h"

/*! Class kbvDBExport.
 * The class constructs the export dialog for databases.
 */
class Ui_kbvDBExportClass;

class KbvDBExport : public QDialog
{
    Q_OBJECT

public:
  KbvDBExport(QWidget *parent = nullptr);
  virtual ~KbvDBExport();

  QString   destination();
  QString   databaseName();

  int   perform(const QStringList dblist);

private slots:
  void  accept(void);
  void  reject(void);
  void  dialogDestination();
  void  comboDBActivated(int index);

private:
  Ui::kbvDBExportClass ui;

  QFileDialog           fileDialog;
  QString               dbName, targetDir;
};

#endif // KBVDBEXPORT_H
/****************************************************************************/
