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
#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#define _GNU_SOURCE
#include <openssl/md5.h>
#include <regex.h>
#include <time.h>

#include "cfgmap.h"

typedef struct _TS {
   const char *pattern;
   regex_t re;
   const char *strptime_fmt;
   int64_t flags;
} TS;

#ifdef __cplusplus
extern "C" {
#endif

#define TS_is_prepared(s) \
   ((s)->pattern ? 1 : 0)

int
TS_init(TS *self, CFGMAP *h_map, const char *pfix);
/********************************************************
 * Initialize timestamp from config map.
 */

void*
TS_destructor(TS *self);
/********************************************************
 * Free resources.
 */

int
TS_scan(const TS *self, time_t *rslt, const char *str, const struct tm *pTmRef);
/********************************************************
 * Scan a string to obtain the timestamp. 
 */

int
TS_MD5_update(const TS *self, MD5_CTX *ctx);
/********************************************************
 * For computing MD5 checksum of config data.
 */


#ifdef __cplusplus
}
#endif

#endif





