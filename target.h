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
#ifndef TARGET_H
#define TARGET_H

#define _GNU_SOURCE
#include <openssl/md5.h>
#include "cfgmap.h"

struct TargetRx {
   const char *pattern;
   regex_t re;
};

typedef struct _Target {
   enum {
      TARGET_INIT_FLG=1<<0
   } flags;
   unsigned severity,
            nRx;
   struct TargetRx *rxArr;
} Target;

#define Target_severity(s) \
   ((const unsigned)(s)->severity)

#define Target_is_init(s) \
   ((const int)(s)->flags)


#ifdef __cplusplus
extern "C" {
#endif

int
Target_init(Target *self, CFGMAP *h_map, const char *pfix);
/********************************************************
 * Prepare for use from config map
 */

void*
Target_destructor(Target *self);
/********************************************************
 * Free resources.
 */

int
Target_scan(const Target *self, char *rsltBuf, unsigned buf_sz, const char *str);
/********************************************************
 * Scan a string to obtain the address. 
 */

int
Target_MD5_update(const Target *self, MD5_CTX *ctx);
/********************************************************
 * For computing MD5 checksum of cumulative patterns.
 */



#ifdef __cplusplus
}
#endif

#endif

