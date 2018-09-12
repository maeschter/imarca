/*****************************************************************************
 * KbvCountrySelector
 * Select Country and ISO country code
 * Derived from libkexiv2
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-03-28 21:01:06 +0200 (Di, 28. Mär 2017) $
 * $Rev: 1199 $
 * Created: 2015.10.18
 * This program is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation.
 *****************************************************************************/
#ifndef KBVCOUNTRYSELECTOR_H
#define KBVCOUNTRYSELECTOR_H
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

typedef QMap<QString, QString>  CountryMap;

class KbvCountrySelector : public  QComboBox
{
  Q_OBJECT

public:
  explicit	KbvCountrySelector(QWidget* parent=0);            //Standard constructor
  explicit	KbvCountrySelector(const KbvCountrySelector &d);  //Copy constructor

  virtual   ~KbvCountrySelector();

  KbvCountrySelector& operator=(const KbvCountrySelector &d);
  QString   getCountryFromIndex(int index);

private:
  void  setupMap();

  CountryMap  countryCodeMap;

};

#endif /*KBVCOUNTRYSELECTOR_H*/
/****************************************************************************/
