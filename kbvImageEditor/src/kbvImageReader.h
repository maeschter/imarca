/*****************************************************************************
 * kbv image reader
 * (C): G. Trauth, Erlangen
 * Created: 2009.07.19
 *****************************************************************************/
#ifndef KBVIMAGEREADER_H_
#define KBVIMAGEREADER_H_
#include <QtCore>
#include <QtGui>
#include <kbvConstants.h>


class KbvImageReader : public QImageReader
{

public:
        KbvImageReader();
	virtual ~KbvImageReader();

	int    quality();
	int    compression();
	float  gamma();
};

#endif /*KBVIMAGEREADER_H_*/
/****************************************************************************/
