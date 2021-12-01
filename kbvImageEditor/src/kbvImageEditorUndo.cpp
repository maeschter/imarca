/*****************************************************************************
 * kvb image editor undo/redo commands
 * (C): G. Trauth, Erlangen
 * LastChanged: 2021-12-01
 * Created: 2017.04.10
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include "kbvImageEditorUndo.h"

/*************************************************************************//*!
 * Crop image to selection
 */
KbvImageCrop::KbvImageCrop(KbvImageEditor *edit, QImage *image, QRect area, QUndoCommand *parent) : QUndoCommand(parent)
{
  //the constructor calls redo()
  editor = edit;
  pimage = image;
  croparea = area;
  img = image->copy(QRect());   //save copy
  //qDebug() << "KbvImageEditor::crop" <<croparea; //###########
}
void  KbvImageCrop::undo()
{
  *pimage = img.copy(QRect());  //restore copy
  editor->removeSelectionRect();
  editor->adjustAndShow(pimage);
}
void  KbvImageCrop::redo()
{
  *pimage = img.copy(croparea);
  editor->removeSelectionRect();
  editor->adjustAndShow(pimage);
}

/*************************************************************************//*!
 * Rotate image left/right 90°
 */
KbvImageRotate::KbvImageRotate(KbvImageEditor *edit, QImage *image, QTransform  matrix, QUndoCommand *parent) : QUndoCommand(parent)
{
  //the constructor calls redo()
  editor = edit;
  pimage = image;
  trans = matrix;
  img = image->copy(QRect());   //save copy
  //qDebug() << "KbvImageEditor::rotate"; //###########
}
void  KbvImageRotate::undo()
{
  *pimage = img.copy(QRect());  //restore copy
  editor->adjustAndShow(pimage);
}
void  KbvImageRotate::redo()
{
  //perform transformation
  *pimage = img.transformed(trans, Qt::SmoothTransformation);
  editor->adjustAndShow(pimage);
}

/*************************************************************************//*!
 * Rotate 180°, flip vertically and flip horizontically
 */
KbvImageFlip::KbvImageFlip(KbvImageEditor *edit, QImage *image, bool flipH, bool flipV, QUndoCommand *parent) : QUndoCommand(parent)
{
  //the constructors calls redo()
  editor = edit;
  pimage = image;
  hor = flipH;
  ver = flipV;
  img = image->copy(QRect());   //save copy
  //qDebug() << "KbvImageEditor::flip"; //###########
}
void  KbvImageFlip::undo()
{
  *pimage = img.copy(QRect());  //restore copy
  editor->adjustAndShow(pimage);
}
void  KbvImageFlip::redo()
{
  *pimage = img.mirrored(hor, ver);;
  editor->adjustAndShow(pimage);
}


/****************************************************************************/
