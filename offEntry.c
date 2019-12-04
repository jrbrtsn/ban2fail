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
#define _GNU_SOURCE
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "ban2fail.h"
#include "cntry.h"
#include "ez_libdb.h"
#include "ez_libc.h"
#include "map.h"
#include "offEntry.h"
#include "util.h"
/********************************************************/
/**************** OFFENTRY ******************************/
/********************************************************/

static void
common_constructor(OFFENTRY *self)
/******************************************************************
 * common portion for all constructors.
 */
{
   memset(self, 0, sizeof(*self));
}

OFFENTRY*
OFFENTRY_addr_constructor(OFFENTRY *self, const char *addr)
/********************************************************
 * Prepare for use.
 */
{
   OFFENTRY *rtn= NULL;

   common_constructor(self);

   strncpy(self->addr, addr, sizeof(self->addr)-1);

   const char *cntry= COUNTRY_get_code(self->addr);
   if(cntry)
      strncpy(self->cntry, cntry, 2);

   rtn= self;
abort:
   return rtn;
}

OFFENTRY*
OFFENTRY_cache_constructor(OFFENTRY *self, const char *cacheFileEntry)
/********************************************************
 * Prepare for use.
 */
{
   OFFENTRY *rtn= NULL;

   common_constructor(self);

   long long ll;

   int rc= sscanf(cacheFileEntry, "%u %u %lld %45s %2s"
         , &self->count
         , &self->severity
         , &ll
         , self->addr
         , self->cntry
         );

   if(4 > rc) {
      eprintf("ERROR: failed to interpret \"%s\"", cacheFileEntry);
      goto abort;
   }

   self->latest= ll;

#ifdef qqDEBUG
   if(self->severity) {
eprintf("%s : %u", self->addr, self->severity);
   }
#endif

   rtn= self;
abort:
   return rtn;
}

void*
OFFENTRY_destructor(OFFENTRY *self)
/********************************************************
 * Free resources.
 */
{
   if(self->dns.name)
      free(self->dns.name);

   return self;
}

void
OFFENTRY_register(OFFENTRY *self, unsigned severity, time_t when)
/********************************************************
 * Register the current failure try.
 */
{
   /* Keep track of count */
   ++self->count;

#ifdef qqDEBUG
   if(severity) {
      eprintf("Severity= %u", severity);
   }
#endif

   /* Keep track of most severe match */
   if(self->severity < severity)
      self->severity= severity;

   /* Keep track of the most recent offense time */
   if(self->latest < when)
      self->latest= when;

}


int
OFFENTRY_cacheWrite(OFFENTRY *self, FILE *fh)
/********************************************************
 * Write to the cache file in a form we can read later.
 */
{
   ez_fprintf(fh, "%u %u %lld %s %s\n"
         , self->count
         , self->severity
         , (long long)self->latest
         , self->addr
         , self->cntry
         );
   return 0;
}

int
OFFENTRY_print(OFFENTRY *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh,
"\tLOGENTRY %p { addr= \"%s\", cntry= \"%2s\" count= %u, severity= %u }\n"
         , self
         , self->addr
         , self->cntry
         , self->count
         , self->severity
         );
   return 0;
}

int
OFFENTRY_map_addr(OFFENTRY *self, MAP *h_rtnMap)
/********************************************************
 * Create a map of OFFENTRY objects with composite
 * counts by address.
 */
{
   OFFENTRY *e= MAP_findStrItem(h_rtnMap, self->addr);

   if(!e) {
      OFFENTRY_addr_create(e, self->addr);
      assert(e);
      MAP_addStrKey(h_rtnMap, e->addr, e);
   }

   e->count += self->count;

   if(e->severity < self->severity)
      e->severity= self->severity;

   if(e->latest < self->latest)
      e->latest= self->latest;
   return 0;
}

int
OFFENTRY_offenseCount(OFFENTRY *self, unsigned *h_sum)
/********************************************************
 * Get a count of all offenses for this entry.
 */
{
   *h_sum += self->count;
//eprintf("%s numItems= %u", self->addr, PTRVEC_numItems(&self->rptObj_vec));
   return 0;
}

int
OFFENTRY_list(OFFENTRY *self, FILE *fh, int flags, unsigned nAllowed)
/********************************************************
 * Print in listing form
 */
{
   static const struct bitTuple BlockBitTuples[]= {
      {.name= "BLK", .bit= BLOCKED_FLG},
      {.name= "+blk+", .bit= WOULD_BLOCK_FLG},
      {.name= "-blk-", .bit= UNJUST_BLOCK_FLG},
      {.name= "WL",   .bit= WHITELIST_FLG},
      {/* Terminating member */}
   };

   const static struct bitTuple dns_flagsArr[]= {
      {.name= "~", .bit= PDNS_FWD_FAIL_FLG},
      {.name= "!!", .bit= PDNS_FWD_NONE_FLG},
      {.name= "NXDOMAIN", .bit= PDNS_NXDOMAIN_FLG},
      {.name= "SERVFAIL", .bit= PDNS_SERVFAIL_FLG},
      {}
   };


   const static char *dns_fmt= "%u %-13s %-15s\t%5u/%-4d offenses %s [%s] %s %s\n",
                     *fmt= "%u %-13s %-15s\t%5u/%-4d offenses %s [%s]\n";

   ez_fprintf(fh, self->dns.flags ? dns_fmt : fmt
         , self->severity
         , self->latest ? local_strftime(&self->latest, "%b %d %H:%M") : ""
         , self->addr
         , self->count
         , nAllowed
         , self->cntry[0] ? self->cntry : "--"
         , bits2str(flags, BlockBitTuples)
         , self->dns.name ? self->dns.name : ""
         , bits2str(self->dns.flags, dns_flagsArr)
         );

   return 0;
}
