/*****************************************************************************
 * kbv setvalues
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2009.02.25
 *****************************************************************************/
#ifndef KBVSETVALUES_H_
#define KBVSETVALUES_H_
#include <QtCore>
#include <QtGui>


class KbvSetvalues : public QSettings
{
  Q_OBJECT

public:
  KbvSetvalues(Format format, Scope scope, const QString & organization,
               const QString & application = QString(), QObject *parent = nullptr);
  virtual ~KbvSetvalues();

  void  readValues();
  void  setvaluesChanged();

//General
QString dataBaseDir;

// kbvView
int     iconSize;
int     dirWatchCycle;
bool    showHiddenFiles;
bool    saveDirTreeState;
bool    showFileName;
bool    showFileSize;
bool    showImageSize;

//kbvSlideShow
int     slideDelaySec;
int     slideChangeId;
bool    slideChangeManual;
bool    slideFullScreen;
bool    slideStretch;
bool    slideStopAtDirEnd;
QColor  slideBackColour;


signals:
  void  settingsChanged();

};

#endif /*KBVSETVALUES_H_*/
/****************************************************************************/
