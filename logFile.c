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
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "cntry.h"
#include "logEntry.h"
#include "logFile.h"
#include "ez_stdio.h"
#include "ez_gzfile.h"
#include "util.h"

#define NOFFENSES_CACHED_FLG (1<<0)

/*==================================================================*/
/*=================== LOGFILE ======================================*/
/*==================================================================*/

static void
common_constructor(LOGFILE *self)
/******************************************************************
 * common portion for all constructors.
 */
{
   memset(self, 0, sizeof(*self));
   MAP_constructor(&self->addr_map, 1000, 200);
}

LOGFILE*
LOGFILE_cache_constructor(LOGFILE *self, const char *fname)
/******************************************************************
 * Initialize an instance of a LOGFILE class from a cache file.
 */
{
   LOGFILE *rtn= NULL;

   common_constructor(self);

   FILE *fh= ez_fopen(fname, "r");

   static char buf[256];
   while(ez_fgets(buf, sizeof(buf), fh)) {
      LOGENTRY *e;
      LOGENTRY_cache_create(e, buf);
      if(!e) goto abort;
      MAP_addStrKey(&self->addr_map, e->addr, e);
   }

   rtn= self;

abort:
   if(fh) ez_fclose(fh);
   return rtn;
}

LOGFILE*
LOGFILE_log_constructor(LOGFILE *self, const struct logProtoType *h_protoType, const char *fname)
/******************************************************************
 * Initialize an instance of a LOGFILE from an actual log file.
 */
{
   LOGFILE *rtn= NULL;

   common_constructor(self);

   static char lbuf[1024];

   /* Open the log file for reading */
   gzFile fh= ez_gzopen(fname, "r");

   /* Loop through one line at a time */
   while(ez_gzgets(fh, lbuf, sizeof(lbuf)-1)) {
      size_t len= strlen(lbuf);
      /* Get rid of trailing newline, spaces */
      while(len && isspace(lbuf[len-1]))
         lbuf[--len]= '\0';

      /* Search for a match in the targets */
      for(struct target *tg= h_protoType->targetArr; tg->pattern; ++tg) {

         /* If there is no match, continue looking */
         regmatch_t matchArr[2];
         if(0 != regexec(&tg->re, lbuf, 2, matchArr, 0) || -1 == matchArr[1].rm_so)
            continue;

         static char addr[128];
         unsigned len;
         len= matchArr[1].rm_eo - matchArr[1].rm_so;
         strncpy(addr, lbuf+matchArr[1].rm_so, sizeof(addr)-1);
         addr[MIN(len, sizeof(addr)-1)]= '\0';

         LOGENTRY *e= MAP_findStrItem(&self->addr_map, addr);
         if(!e) {
            LOGENTRY_addr_create(e, addr);
            if(!e) goto abort;

            /* Add to the addr_map */
            MAP_addStrKey(&self->addr_map, e->addr, e);
         }
         LOGENTRY_register(e);
      }
   }

   rtn= self;
abort:
   if(fh) ez_gzclose(fh);

   return rtn;
}

LOGFILE*
LOGFILE_destructor(LOGFILE *self)
/******************************************************************
 * Free resources associated with LOGFILE object.
 */
{
   if(self->logFilePath)
      free(self->logFilePath);

   MAP_clearAndDestroy(&self->addr_map, (void*(*)(void*))LOGENTRY_destructor);
   MAP_destructor(&self->addr_map);

  return self;
}

void
LOGFILE_set_logFilePath(LOGFILE *self, const char *path)
/******************************************************************
 * Set the log file name by making a copy of the path.
 */
{
   if(self->logFilePath)
      free(self->logFilePath);

   self->logFilePath= strdup(path);
}

int
LOGFILE_writeCache(LOGFILE *self, const char *fname)
/******************************************************************
 * Create a cache file and dump contents of this object.
 */
{
   int rc, rtn= -1;

   FILE *fh= ez_fopen(fname, "w");
   rc= MAP_visitAllEntries(&self->addr_map, (int(*)(void*,void*))LOGENTRY_cacheWrite, fh);
   if(rc) goto abort;

   rtn= 0;
abort:
   if(fh) ez_fclose(fh);
   return rtn;
}

int
LOGFILE_print(LOGFILE *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh, "LOGFILE %p \"%s\" {\n", self, self->logFilePath);
   MAP_visitAllEntries(&self->addr_map, (int(*)(void*,void*))LOGENTRY_print, fh);
   ez_fprintf(fh, "}\n");

   return 0;
}

int
LOGFILE_map_addr(LOGFILE *self, MAP *h_rtnMap)
/********************************************************
 * Create a addr_map of LOGENTRY objects with composite
 * counts by address.
 */
{
   MAP_visitAllEntries(&self->addr_map, (int(*)(void*,void*))LOGENTRY_map_addr, h_rtnMap);
   return 0;
}

int
LOGFILE_offenseCount(LOGFILE *self, unsigned *h_sum)
/********************************************************
 * Get a count of all offenses for this file.
 */
{
   if(!(self->flags & NOFFENSES_CACHED_FLG)) {
      MAP_visitAllEntries(&self->addr_map, (int(*)(void*,void*))LOGENTRY_offenseCount, &self->nOffenses);
      self->flags |= NOFFENSES_CACHED_FLG;
   }

   *h_sum += self->nOffenses;

   return 0;
}
