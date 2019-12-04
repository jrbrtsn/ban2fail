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
#include "ez_libc.h"
#include "timestamp.h"
#include "util.h"


enum Flags {
   GUESS_YR_FLG= 1<<0
};

const static struct bitTuple FlagsBitTupleArr[]= {
   {.name= "GUESS_YEAR", .bit= GUESS_YR_FLG},
   {/* Terminating member */}
};

int
TS_init(TS *self, CFGMAP *h_map, const char *pfix)
/********************************************************
 * Initialize timestamp from config map.
 */
{
   int rtn= -1;
   memset(self, 0, sizeof(*self));

   size_t len= strlen(pfix)+1024;
   char symBuf[len];

   { /*--- Check for "REGEX" symbol ---*/
      snprintf(symBuf, len, "%s\\REGEX", pfix);
      /* CFGMAP is left in place, so we don't need to strdup() */
      self->pattern= CFGMAP_find_last_value(h_map, symBuf);

      if(!self->pattern) {
         eprintf("ERROR: cannot find \"REGEX\" entry for ENTRY_TIMESTAMP %s", pfix);
         goto abort;
      }

      if(regex_compile(&self->re, self->pattern, REG_EXTENDED)) {   
         eprintf("ERROR: regex_compile(\"%s\") failed.", self->pattern);
         goto abort;
      }
   }

   { /*--- Check for "STRPTIME" symbol ---*/
      snprintf(symBuf, len, "%s\\STRPTIME", pfix);
      /* CFGMAP is left in place, so we don't need to strdup() */
      self->strptime_fmt= CFGMAP_find_last_value(h_map, symBuf);

      if(!self->strptime_fmt) {
         eprintf("ERROR: cannot find \"STRPTIME\" entry for ENTRY_TIMESTAMP %s", pfix);
         goto abort;
      }
   }

   { /*--- Check for "FLAGS" symbol ---*/
      const char *flagStr;
      snprintf(symBuf, len, "%s\\FLAGS", pfix);

      flagStr= CFGMAP_find_last_value(h_map, symBuf);
      /* This is optional */
      if(flagStr) {
         int rc= str2bits(&self->flags, flagStr, FlagsBitTupleArr);
         if(rc) {
            eprintf("ERROR: cannot interpret \"FLAGS\" entry for ENTRY_TIMESTAMP %s", pfix);
            goto abort;
         }
      }
   }

   rtn= 0;
abort:
   return rtn;
}

void*
TS_destructor(TS *self)
/********************************************************
 * Free resources.
 */
{
   if(self->pattern)
         regfree(&self->re);
   return self;
}

int
TS_scan(const TS *self, time_t *rslt, const char *str, const struct tm *pTmRef)
/********************************************************
 * Scan a string to obtain the timestamp. 
 */
{
   int rtn= -1;

   /* If there is no match, continue looking */
   regmatch_t matchArr[2];
   if(0 != regexec(&self->re, str, 2, matchArr, 0) || -1 == matchArr[1].rm_so) {
      eprintf("ERROR: failed to identify date in \"%s\"", str);
      goto abort;
   }

   unsigned len= matchArr[1].rm_eo - matchArr[1].rm_so;
   static char date[128];
   strncpy(date, str+matchArr[1].rm_so, sizeof(date)-1);
   date[MIN(len, sizeof(date)-1)]= '\0';
   static struct tm tm;
   memset(&tm, 0, sizeof(tm));
   tm.tm_isdst= -1;

   const char *lc= ez_strptime(date, self->strptime_fmt, &tm);

   /* We may have to guess the year */
   if(self->flags & GUESS_YR_FLG) {

      tm.tm_year= pTmRef->tm_year;

      if(tm.tm_mon > pTmRef->tm_mon)
         --tm.tm_year;
   }

   *rslt= mktime(&tm);
//eprintf("Date string= \"%s\", lc= \"%s\", =? \"%s\"", date, lc, ctime(rslt));

   rtn= 0;
abort:
   return rtn;
}

int
TS_MD5_update(const TS *self, MD5_CTX *ctx)
/********************************************************
 * For computing MD5 checksum of config data.
 */
{
   if(self->pattern)
      MD5_Update(ctx, self->pattern, strlen(self->pattern));

   if(self->strptime_fmt)
      MD5_Update(ctx, self->strptime_fmt, strlen(self->strptime_fmt));

   MD5_Update(ctx, &self->flags, sizeof(self->flags));
   return 0;
}
