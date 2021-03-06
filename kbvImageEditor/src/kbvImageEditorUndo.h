/*****************************************************************************
 * kbv image editor undo/redo commands
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2017.04.10
 *****************************************************************************/
#ifndef KBVIMAGEEDITORUNDO_H
#define KBVIMAGEEDITORUNDO_H

//#include <QtCore>
#include "kbvImageEditor.h"

class KbvImageCrop : public QUndoCommand
{
public:
  KbvImageCrop(KbvImageEditor *edit=0, QImage *image=0, QRect area=QRect(), QUndoCommand *parent=0);
  void  undo();
  void  redo();
  
private:
  KbvImageEditor *editor;
  QImage  *pimage;
  QImage  img;
  QRect   croparea;
};

class KbvImageRotate : public QUndoCommand
{
public:
  KbvImageRotate(KbvImageEditor *edit=0, QImage *image=0, QTransform  matrix=QTransform(), QUndoCommand *parent=0);
  void  undo();
  void  redo();
  
private:
  KbvImageEditor *editor;
  QImage  *pimage;
  QImage  img;
  QTransform  trans;
};

class KbvImageFlip : public QUndoCommand
{
public:
  KbvImageFlip(KbvImageEditor *edit=0, QImage *image=0, bool flipH=false, bool flipV=false, QUndoCommand *parent=0);
  void  undo();
  void  redo();
  
private:
  KbvImageEditor *editor;
  QImage  *pimage;
  QImage  img;
  bool    hor, ver;
};

#endif // KBVIMAGEEDITORUNDO_H
/****************************************************************************/
