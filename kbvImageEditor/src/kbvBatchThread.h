/*****************************************************************************
 * kvbFileModelThread
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-06-16 20:17:33 +0200 (Fr, 16. Jun 2017) $
 * $Rev: 1352 $
 * Created: 2009.01.15
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVBATCHTHREAD_H_
#define KBVBATCHTHREAD_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <opencv2/opencv.hpp>
#include <kbvExiv.h>
#include <kbvConstants.h>
#include "kbvBatchThread.h"

extern "C"
{
#include  <math.h>
}

class   KbvImageEditor;

class KbvBatchThread : public QThread
{
  Q_OBJECT

public:
	KbvBatchThread(QObject * parent = nullptr);
	virtual ~KbvBatchThread();

public slots:
  void  startThread(QComboBox *fileCombo, Kbv::kbvBatchParam &batchParam);
  void  stopThread();

signals:
  void  progressText(QString text);
  void	progressValue(int progress);
  void  threadFinished(bool error);
  void  filesChanged(QString path);
  void  threadNotRunning(bool running);

protected:
void    run();

private:
  bool  flipRotate();
  bool  trimEdges(const bool changed);
  bool  resize(const bool changed);
  void  setSoftwareInfo(const bool changed);
  void  saveFile(const bool changed);

  QComboBox           *mCombo;
  Kbv::kbvBatchParam  mParams;
  KbvExiv             mMetadata;
  bool                mAbort;
  int                 mBatch, mLot;
  float               mStep;
  QString             mPath, mFile, mExtension;
  QStringList         mReport;

  //openCV
  cv::Mat         ocvImage, ocvOutImage;
  cv::String      ocvFileName;

};
#endif /*KBVBATCHTHREAD_H_*/
/****************************************************************************/
