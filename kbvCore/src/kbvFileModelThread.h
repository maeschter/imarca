/*****************************************************************************
 * kvbFileModelThread
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.01.15
 *****************************************************************************/
#ifndef KBVFILEMODELTHREAD_H_
#define KBVFILEMODELTHREAD_H_
#include <QtCore>
#include <QtGui>
#include "kbvGlobal.h"
#include "kbvConstants.h"

class   KbvFileModel;

class KbvFileModelThread : public QThread
{
  Q_OBJECT

public:
        KbvFileModelThread(QObject * parent = nullptr);
	virtual ~KbvFileModelThread();

void    startThread(const QString &str, QStringList *fileentries,
                    QSize iconsize, int position);
void    stopThread();

signals:
void    newItem(Kbv::kbvItem*);
void    statusText2(QString text);
void    threadFinished();

protected:
void    run();

private:
QImage      smartScale(int width, int height, const QImage &source);

KbvGlobal       globalFunc;
QStringList     *entries;
QString         filedir;
int             location;
QSize           iconsize;
bool            abort;

};
#endif /*KBVFILEMODELTHREAD_H_*/
/****************************************************************************/
