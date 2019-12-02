/***************************************************************************
 *   Copyright (C) 2019 by John D. Robertson                               *
 *   john@rrci.com                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef LOGTYPE_H
#define LOGTYPE_H

#include <stdio.h>

#include "ban2fail.h"
#include "cfgmap.h"
#include "map.h"

/*==================================================================*/
/*===================== target =====================================*/
/*==================================================================*/
struct target {
   const char *pattern;
   regex_t re;
};

/*==================================================================*/
/*===================== log file prototype =========================*/
/*==================================================================*/
struct logProtoType {
   const char *dir,
              *pfix;
   struct target *targetArr;
};

/*==================================================================*/
/*===================== LOGTYPE ====================================*/
/*==================================================================*/
/* One of these for each log file type to be scanned */

typedef struct _LOGTYPE {
   char *dir,
        *pfix,
        patterns_md5sum[33];
   MAP file_map;
} LOGTYPE;

#ifdef __cplusplus
extern "C" {
#endif

int
LOGTYPE_init(CFGMAP *h_map, char *pfix);
/**************************************************************
 * Initialize objects from configuration map.
 */

#define LOGTYPE_destroy(s) \
  {if(LOGTYPE_destructor(s)) {free(s);(s)=0;}}
LOGTYPE*
LOGTYPE_destructor(LOGTYPE *self);
/******************************************************************
 * Free resources associated with a LOGTYPE object.
 */

const char*
LOGTYPE_cacheName(const LOGTYPE *self);
/******************************************************************
 * Return in a static buffer the name of the cache directory which
 * will hold results for this log type.
 */

const char*
LOGTYPE_fname_2_cacheName(const char *fname);
/******************************************************************
 * Return in a static buffer the name of the cache directory which
 * will hold results for the log type indicated by fname.
 */

int
LOGTYPE_print(LOGTYPE *self, FILE *fh);
/********************************************************
 * Print a human readable representation of *self.
 */

int
LOGTYPE_map_addr(LOGTYPE *self, MAP *h_rtnMap);
/********************************************************
 * Create a map of OFFENTRY objects with composite
 * counts by address.
 */

int
LOGTYPE_offenseCount(LOGTYPE *self, unsigned *h_sum);
/********************************************************
 * Get a count of all offenses for this log type.
 */

int
LOGTYPE_addressCount(LOGTYPE *self);
/********************************************************
 * Get a count of all addresses for this log type.
 */

#ifdef __cplusplus
}
#endif

#endif /* LOGTYPE_H */
