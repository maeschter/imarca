/*****************************************************************************
 * kvb rubberband
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2018-02-27 14:49:17 +0100 (Di, 27. Feb 2018) $
 * $Rev: 1465 $
 * Created: 2009.05.30
 *****************************************************************************/
#ifndef KBVRUBBERBAND_H_
#define KBVRUBBERBAND_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class KbvRubberBand : public QRubberBand
{
  Q_OBJECT

public:
    KbvRubberBand(Shape s=QRubberBand::Rectangle, QWidget *p=0);
    virtual ~KbvRubberBand();
};

#endif /*KBVRUBBERBAND_H_*/
/****************************************************************************/
