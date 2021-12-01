/*****************************************************************************
 * kbv dbimport dialog
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2016.12.08
 *****************************************************************************/
#ifndef KBVDBIMPORT_H_
#define KBVDBIMPORT_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvdbimport.h"

/*! Class kbvDBImport.
 * The class constructs the import dialog for databases.
 */
class Ui_kbvDBImportClass;

class KbvDBImport : public QDialog
{
    Q_OBJECT

public:
  KbvDBImport(QWidget *parent = nullptr);
  virtual ~KbvDBImport();

  QString   destination();
  QString   databaseName();

private slots:
  void  accept(void);
  void  reject(void);
  void  dialogDestination();
  void  dialogName();

private:
  Ui::kbvDBImportClass ui;

  QFileDialog           fileDialog;
  QString               dbName, homeDir;
};

#endif
// KBVDBIMPORT_H
/****************************************************************************/
