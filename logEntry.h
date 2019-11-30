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
#ifndef LOGENTRY_H
#define LOGENTRY_H

#include <stdio.h>
#include <time.h>

#include "map.h"

/* One of these for each offense found in a log file */
typedef struct _LOGENTRY {
   unsigned logfile_ndx;
   char addr[46],
        cntry[3];
   unsigned count;
   char *dnsName;
} LOGENTRY;

#ifdef __cplusplus
extern "C" {
#endif

#define LOGENTRY_addr_create(p, addr) \
  ((p)=(LOGENTRY_addr_constructor((p)=malloc(sizeof(LOGENTRY)), addr) ? (p) : ( p ? realloc(LOGENTRY_destructor(p),0) : 0 )))
LOGENTRY*
LOGENTRY_addr_constructor(LOGENTRY *self, const char *addr);
/********************************************************
 * Prepare for use from an address found in a log entry.
 */

#define LOGENTRY_cache_create(p, cacheFileEntry) \
  ((p)=(LOGENTRY_cache_constructor((p)=malloc(sizeof(LOGENTRY)), cacheFileEntry) ? (p) : ( p ? realloc(LOGENTRY_destructor(p),0) : 0 )))
LOGENTRY*
LOGENTRY_cache_constructor(LOGENTRY *self, const char *cacheFileEntry);
/********************************************************
 * Prepare for use with entry from cache file.
 */

#define LOGENTRY_destroy(s) \
  {if(LOGENTRY_destructor(s)) {free(s); (s)=0;}}
void*
LOGENTRY_destructor(LOGENTRY *self);
/********************************************************
 * Free resources.
 */

void
LOGENTRY_register(LOGENTRY *self);
/********************************************************
 * Register the current failure try.
 */

#if 0
int
LOGENTRY_is_blocked_country(const LOGENTRY *self);
/********************************************************
 * Return 1 if the country is blocked, or 0.
 */
#endif

int
LOGENTRY_cacheWrite(LOGENTRY *self, FILE *fh);
/********************************************************
 * Write to the cache file in a form we can read later.
 */

int
LOGENTRY_print(LOGENTRY *self, FILE *fh);
/********************************************************
 * Print a human readable representation of *self.
 */

int
LOGENTRY_map_addr(LOGENTRY *self, MAP *h_rtnMap);
/********************************************************
 * Create a map of LOGENTRY objects with composite
 * counts by address.
 */

int
LOGENTRY_offenseCount(LOGENTRY *self, unsigned *h_sum);
/********************************************************
 * Get a count of all offenses for this entry.
 */

#ifdef __cplusplus
}
#endif

#endif

