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
#ifndef LOGFILE_H
#define LOGFILE_H

#define _GNU_SOURCE
#include <stdio.h>

#include "logType.h"
#include "map.h"

/* One of these for each log file which is scanned. */
typedef struct _LOGFILE {
   int flags;
   char *logFname;
   struct {
      MAP offEntry_map,
          obsvTpl_map;
   } addr; /* addr denotes the map keys */
   unsigned nOffenses;
} LOGFILE;

#define LOGFILE_logFname(self) \
   (const char*)(self)->logFname

#ifdef __cplusplus
extern "C" {
#endif

#define LOGFILE_cache_create(s, cacheFname, logFname) \
  ((s)=(LOGFILE_cache_constructor((s)=malloc(sizeof(LOGFILE)), cacheFname, logFname) ? (s) : ( s ? realloc(LOGFILE_destructor(s),0) : 0 )))
LOGFILE*
LOGFILE_cache_constructor(
      LOGFILE *self,
      const char *cacheFname,
      const char *logFname
      );
/******************************************************************
 * Initialize an instance of a LOGFILE class.
 */

#define LOGFILE_log_create(s, h_protoType, logFname) \
  ((s)=(LOGFILE_log_constructor((s)=malloc(sizeof(LOGFILE)), h_protoType, logFname) ? (s) : ( s ? realloc(LOGFILE_destructor(s),0) : 0 )))
LOGFILE*
LOGFILE_log_constructor(
      LOGFILE *self,
      const struct logProtoType *h_protoType,
      const char *logFname
      );
/******************************************************************
 * Initialize an instance of a LOGFILE class.
 */

#define LOGFILE_destroy(s) \
  {if(LOGFILE_destructor(s)) {free(s);(s)=0;}}
LOGFILE*
LOGFILE_destructor(LOGFILE *self);
/******************************************************************
 * Free resources associated with a LOGFILE object.
 */

#if 0
void
LOGFILE_set_logFname(LOGFILE *self, const char *path);
/******************************************************************
 * Set the log file name by making a copy of the path.
 */
#endif

int
LOGFILE_writeCache(LOGFILE *self, const char *fname);
/******************************************************************
 * Create a cache file and dump contents of this object.
 */

int
LOGFILE_print(LOGFILE *self, FILE *fh);
/********************************************************
 * Print a human readable representation of *self.
 */

int
LOGFILE_map_addr(LOGFILE *self, MAP *h_rtnMap);
/********************************************************
 * Create a map of OFFENTRY objects with composite
 * counts by address.
 */

int
LOGFILE_offenseCount(LOGFILE *self, unsigned *h_sum);
/********************************************************
 * Get a count of all offenses for this file.
 */

int
LOGFILE_addressCount(LOGFILE *self, unsigned *h_sum);
/********************************************************
 * Get a count of all unique addresses for this file.
 */

#ifdef __cplusplus
}
#endif

#endif /* LOGFILE_H */
