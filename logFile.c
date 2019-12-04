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
#include <ctype.h>
#include <db.h>
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "addrRpt.h"
#include "cntry.h"
#include "dynstack.h"
#include "ez_libc.h"
#include "ez_libdb.h"
#include "ez_libz.h"
#include "logFile.h"
#include "obsvTpl.h"
#include "offEntry.h"
#include "util.h"

/*==================================================================*/
/*=================== LOGFILE ======================================*/
/*==================================================================*/
enum {
   NOFFENSES_CACHED_FLG =1<<0
};

static void
common_constructor(LOGFILE *self)
/******************************************************************
 * common portion for all constructors.
 */
{
   memset(self, 0, sizeof(*self));
   MAP_constructor(&self->addr.offEntry_map, N_ADDRESSES_HINT/BUCKET_DEPTH_HINT, BUCKET_DEPTH_HINT);
   MAP_constructor(&self->addr.obsvTpl_map, N_ADDRESSES_HINT/BUCKET_DEPTH_HINT, BUCKET_DEPTH_HINT);
}

LOGFILE*
LOGFILE_cache_constructor(
      LOGFILE *self,
      const char *cacheFname,
      const char *logFname
      )
/******************************************************************
 * Initialize an instance of a LOGFILE class from a cache file.
 */
{
   LOGFILE *rtn= NULL;


   common_constructor(self);

   /* Not reentrant! */
   gzFile log_fh= NULL;

   self->logFname= strdup(logFname);

   /* Open our database, if it exists */
   DB *db= NULL;

   static char dbFname[PATH_MAX];
   snprintf(dbFname, sizeof(dbFname), "%s.db", cacheFname);
   if(0 == ez_access(dbFname, F_OK)) {
      ez_db_create(&db, NULL, 0);
      ez_db_open(db, NULL, dbFname, NULL, DB_BTREE, DB_RDONLY, 0664);
   }

   /* Open the cache file */
   FILE *cache_fh= ez_fopen(cacheFname, "r");

   static char buf[256];
   while(ez_fgets(buf, sizeof(buf), cache_fh)) {
      OFFENTRY *e;
      OFFENTRY_cache_create(e, buf);
      assert(e);
      MAP_addStrKey(&self->addr.offEntry_map, e->addr, e);

      AddrRPT *ar= MAP_findStrItem(&G.rpt.AddrRPT_map, e->addr);
      if(ar && db) {
         static ObsvTpl otpl;
         /* Initialize or reset static instance */
         ObsvTpl_sinit(&otpl, self, e->addr);

         /* May not exist in database */
         /* Fetch from database */
         if(0 == ObsvTpl_db_get(&otpl, db)) {

            if(!log_fh)
               log_fh= ez_gzopen(logFname, "r");

            /* Place observations in ar */
            ObsvTpl_put_AddrRPT(&otpl, log_fh, ar);
         }
      }

   }

   rtn= self;

abort:
   if(db) ez_db_close(db, 0);
   if(log_fh) ez_gzclose(log_fh);
   if(cache_fh) ez_fclose(cache_fh);
   return rtn;
}

LOGFILE*
LOGFILE_log_constructor(
      LOGFILE *self,
      const struct logProtoType *h_protoType,
      const char *logFname
      )
/******************************************************************
 * Initialize an instance of a LOGFILE from an actual log file.
 */
{
   LOGFILE *rtn= NULL;

   common_constructor(self);

   self->logFname= strdup(logFname);

   static char lbuf[1024];

   /* Open the log file for reading */
   gzFile gzfh= ez_gzopen(logFname, "r");

   /* Loop through one line at a time */
   while(gzfh) {
      z_off_t pos= ez_gztell(gzfh);
      if(!ez_gzgets(gzfh, lbuf, sizeof(lbuf)-1)) break;

      size_t line_len= strlen(lbuf);
      /* Get rid of trailing newline, spaces */
      while(line_len && isspace(lbuf[line_len-1]))
         lbuf[--line_len]= '\0';

      /* Search for a match in the targets */
      for(Target *tg= h_protoType->targetArr; Target_is_init(tg); ++tg) {

         static char addr[128];
         if(Target_scan(tg, addr, sizeof(addr), lbuf)) continue;

         OFFENTRY *e= MAP_findStrItem(&self->addr.offEntry_map, addr);
         if(!e) {
            OFFENTRY_addr_create(e, addr);
            if(!e) goto abort;

            /* Add to the addr.offEntry_map */
            MAP_addStrKey(&self->addr.offEntry_map, e->addr, e);
         }

         /* See if we can extract the date */
         time_t when= 0;
         if(TS_is_prepared(&h_protoType->ts)) {
            if(TS_scan(&h_protoType->ts, &when, lbuf, &G.begin.tm)) {
               eprintf("WARNING: TS_scan() failed.");
            }
         }

         OFFENTRY_register(e, Target_severity(tg), when);

         { /* Keep ObsvTpl record of offense line in log file */
            ObsvTpl *ot= MAP_findStrItem(&self->addr.obsvTpl_map, e->addr);
            if(!ot) {
               ObsvTpl_create(ot, self, e->addr);
               assert(ot);
               MAP_addStrKey(&self->addr.obsvTpl_map, e->addr, ot);
            }

            ObsvTpl_addObsv(ot, pos, line_len);
         }
      } /* End of Target loop */
   } /* End of line reading loop */

   /* Take car of possible address reporting */
   unsigned nItems= MAP_numItems(&G.rpt.AddrRPT_map);
   if(nItems) { /* Take care of address reports now */
      AddrRPT *rtnArr[nItems];
      MAP_fetchAllItems(&G.rpt.AddrRPT_map, (void**)rtnArr);
      for(unsigned i= 0; i < nItems; ++i) {
         AddrRPT *ar= rtnArr[i];
         ObsvTpl *ot= MAP_findStrItem(&self->addr.obsvTpl_map, ar->addr);
         if(!ot) continue;
         ObsvTpl_put_AddrRPT(ot, gzfh, ar);
      }
   }

   rtn= self;
abort:
   if(gzfh) ez_gzclose(gzfh);

   return rtn;
}

LOGFILE*
LOGFILE_destructor(LOGFILE *self)
/******************************************************************
 * Free resources associated with LOGFILE object.
 */
{
   if(self->logFname)
      free(self->logFname);

   /* Clean up the maps and their contents */
   MAP_clearAndDestroy(&self->addr.offEntry_map, (void*(*)(void*))OFFENTRY_destructor);
   MAP_destructor(&self->addr.offEntry_map);

   MAP_clearAndDestroy(&self->addr.obsvTpl_map, (void*(*)(void*))ObsvTpl_destructor);
   MAP_destructor(&self->addr.obsvTpl_map);

  return self;
}

int
LOGFILE_writeCache(LOGFILE *self, const char *fname)
/******************************************************************
 * Create a cache file and dump contents of this object.
 */
{
   int rc, rtn= -1;
   DB *dbh= NULL;

   FILE *fh= ez_fopen(fname, "w");
   ez_fchown(fileno(fh), getuid(), G.gid);
   ez_fchmod(fileno(fh), G.cache.file_mode);

   /* Writes all OFFENTRY object to fh */
   rc= MAP_visitAllEntries(&self->addr.offEntry_map, (int(*)(void*,void*))OFFENTRY_cacheWrite, fh);
   if(rc) goto abort;


   /* Write RptObj's to database if any exist */
   if(MAP_numItems(&self->addr.obsvTpl_map)) {

      static char dbFname[PATH_MAX];
      snprintf(dbFname, sizeof(dbFname), "%s.db", fname);

      ez_db_create(&dbh, NULL, 0);
      assert(dbh);
      ez_db_open(dbh, NULL, dbFname, NULL, DB_BTREE, DB_CREATE, G.cache.file_mode);
      ez_chown(dbFname, getuid(), G.gid);

      /* This will write all entries in the map do the database */
      MAP_visitAllEntries(&self->addr.obsvTpl_map, (int(*)(void*,void*))ObsvTpl_db_put, dbh);
   }

   rtn= 0;
abort:
   if(dbh) ez_db_close(dbh, 0);
   if(fh) ez_fclose(fh);
   return rtn;
}

int
LOGFILE_print(LOGFILE *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh, "LOGFILE %p \"%s\" {\n", self, self->logFname);
   MAP_visitAllEntries(&self->addr.offEntry_map, (int(*)(void*,void*))OFFENTRY_print, fh);
   ez_fprintf(fh, "}\n");

   return 0;
}

int
LOGFILE_map_addr(LOGFILE *self, MAP *h_rtnMap)
/********************************************************
 * Create a addr.offEntry_map of OFFENTRY objects with composite
 * counts by address.
 */
{
   MAP_visitAllEntries(&self->addr.offEntry_map, (int(*)(void*,void*))OFFENTRY_map_addr, h_rtnMap);
   return 0;
}

int
LOGFILE_offenseCount(LOGFILE *self, unsigned *h_sum)
/********************************************************
 * Get a count of all offenses for this file.
 */
{
   if(!(self->flags & NOFFENSES_CACHED_FLG)) {
      MAP_visitAllEntries(&self->addr.offEntry_map, (int(*)(void*,void*))OFFENTRY_offenseCount, &self->nOffenses);
      self->flags |= NOFFENSES_CACHED_FLG;
   }

   *h_sum += self->nOffenses;

   return 0;
}

int
LOGFILE_addressCount(LOGFILE *self, unsigned *h_sum)
/********************************************************
 * Get a count of all unique addresses for this file.
 */
{
   *h_sum += MAP_numItems(&self->addr.offEntry_map);
   return 0;
}

