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
#include <assert.h>
#include <GeoIP.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "ban2fail.h"
#include "cntry.h"
#include "ez_stdio.h"
#include "map.h"
#include "logEntry.h"
#include "util.h"

/********************************************************/
/**************** LOGENTRY ******************************/
/********************************************************/

static void
common_constructor(LOGENTRY *self)
/******************************************************************
 * common portion for all constructors.
 */
{
   memset(self, 0, sizeof(*self));
}

LOGENTRY*
LOGENTRY_addr_constructor(LOGENTRY *self, const char *addr)
/********************************************************
 * Prepare for use.
 */
{
   LOGENTRY *rtn= NULL;

   common_constructor(self);

   strncpy(self->addr, addr, sizeof(self->addr)-1);

   const char *cntry= COUNTRY_get_code(self->addr);
   if(cntry)
      strncpy(self->cntry, cntry, 2);

   rtn= self;
abort:
   return rtn;
}

LOGENTRY*
LOGENTRY_cache_constructor(LOGENTRY *self, const char *cacheFileEntry)
/********************************************************
 * Prepare for use.
 */
{
   LOGENTRY *rtn= NULL;

   common_constructor(self);

   int rc= sscanf(cacheFileEntry, "%u %45s %2s"
         ,&self->count
         ,self->addr
         ,self->cntry);

   if(2 > rc) {
      eprintf("ERROR: failed to interpret \"%s\"", cacheFileEntry);
      goto abort;
   }

   rtn= self;
abort:
   return rtn;
}

void*
LOGENTRY_destructor(LOGENTRY *self)
/********************************************************
 * Free resources.
 */
{
   return self;
}

void
LOGENTRY_register(LOGENTRY *self)
/********************************************************
 * Register the current failure try.
 */
{
   /* Keep track of count */
   ++self->count;
}


int
LOGENTRY_cacheWrite(LOGENTRY *self, FILE *fh)
/********************************************************
 * Write to the cache file in a form we can read later.
 */
{
   ez_fprintf(fh, "%u %s %s\n"
         , self->count
         , self->addr
         , self->cntry
         );
   return 0;
}

int
LOGENTRY_print(LOGENTRY *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh,
"\tLOGENTRY %p { addr= \"%s\", cntry= \"%2s\" count= %u }\n"
         , self
         , self->addr
         , self->cntry
         , self->count
         );
   return 0;
}

int
LOGENTRY_map_addr(LOGENTRY *self, MAP *h_rtnMap)
/********************************************************
 * Create a map of LOGENTRY objects with composite
 * counts by address.
 */
{
   LOGENTRY *e= MAP_findStrItem(h_rtnMap, self->addr);

   if(!e) {
      LOGENTRY_addr_create(e, self->addr);
      assert(e);
      MAP_addStrKey(h_rtnMap, e->addr, e);
   }

   e->count += self->count;
   return 0;
}

int
LOGENTRY_offenseCount(LOGENTRY *self, unsigned *h_sum)
/********************************************************
 * Get a count of all offenses for this entry.
 */
{
   *h_sum += self->count;
   return 0;
}
