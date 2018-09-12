/*****************************************************************************
 * KbvExivGlobal
 * Dyn. library interface to libExiv2 >= 0.25
 * import/export macros for dynamic libKbvExiv.so
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-05-05 17:35:37 +0200 (Fr, 05. Mai 2017) $
 * $Rev: 981 $
 * Created: 2015.10.02
 *****************************************************************************/
#ifndef KBVEXIV_GLOBAL_H
#define KBVEXIV_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(KBVEXIV_LIBRARY)
#  define KBVEXIVSHARED_EXPORT Q_DECL_EXPORT
#else
#  define KBVEXIVSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // KBVEXIV_GLOBAL_H
/****************************************************************************/
