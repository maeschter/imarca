/*****************************************************************************
 * kbv busy animation
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2011.12.04
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVBUSYANIMATION_H_
#define KBVBUSYANIMATION_H_
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include "ui_kbvanimation.h"

class Ui_kbvAnimationClass;

class KbvBusyAnimation : public QWidget
{
  Q_OBJECT

public:
        KbvBusyAnimation();
	virtual ~KbvBusyAnimation();

public slots:
void    run(QString title, bool run);
void    setText1(const QString text);
void    setText2(const QString text);

signals:
  void  running(QString title, bool run);

private slots:
  void  buttonClicked();

private:
Ui::kbvAnimationClass ui;
QMovie  *busyAnim;
QString name;

};
#endif /*KBVBUSYANIMATION_H_*/
/****************************************************************************/
