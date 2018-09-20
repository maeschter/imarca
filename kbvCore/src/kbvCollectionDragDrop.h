/*****************************************************************************
 * kvbCollectionDragDrop
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2012.03.12
 *****************************************************************************/
#ifndef KBVCOLLECTIONDRAGDROP_H_
#define KBVCOLLECTIONDRAGDROP_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSql>
#include "kbvGlobal.h"
#include "kbvDBInfo.h"
#include "kbvCollectionModel.h"
#include "kbvConstants.h"

class KbvCollectionModel;

class KbvCollectionDragDrop : public QObject
{
  Q_OBJECT

public:
  KbvCollectionDragDrop(QObject *parent=nullptr);
  virtual ~KbvCollectionDragDrop();

void    dropMimeData(const QMimeData *mimeData, Qt::DropAction action,
                     QString branch, KbvDBInfo *info, KbvCollectionModel *cm);

signals:
void    dropFinished(void);
void    infoText1(QString text);
void    infoText2(QString text);
void    infoText3(QString text);

private:
bool    connectDB(QString dbname, bool source);
void    disconnectDB();
void    addToAlbum(int flag, KbvCollectionModel *model);
void    addToCollection(int flag, KbvCollectionModel *model);
void    getUserInput(const QString text1, const QString text2);
bool    removeFromDB(QString path, QString name, bool source);

KbvGlobal       globalFunc;
QSqlDatabase    targetDB, sourceDB;
QString         targetDBName, targetDBLocation, targetCollRoot;
QString         sourceDBName, sourceDBLocation;
int             sourceType, targetType, targetDBIconSize, targetDBKeywordType;

QString         messageNoConnect, messageSameAlbum, messageFaultAlbum, messageFaultCollection,
                messageFaultReason, messageNoTargetDir, messageNoFileCRC;

QStringList     entryList;
QStringList     mimeStrings;
QString         fileDir, errorList;
bool            move, error, yesall, yes, no, cancel;
};

#endif /* KBVCOLLECTIONDRAGDROP_H_ */
/****************************************************************************/
