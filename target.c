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
#include <regex.h>
#include <stdlib.h>

#include "target.h"
#include "util.h"

int
Target_init(Target *self, CFGMAP *h_map, const char *pfix)
/********************************************************
 * Prepare for use from config map
 */
{
   int rtn= -1;
   memset(self, 0, sizeof(*self));

   self->flags |= TARGET_INIT_FLG;

   unsigned arr_sz= CFGMAP_numTuples(h_map);
   struct CFGMAP_tuple rtn_arr[arr_sz];

   size_t len= strlen(pfix)+1024;
   char symBuf[len];

   { /*--- Check for "SEVERITY" symbol ---*/
      snprintf(symBuf, len, "%s\\SEVERITY", pfix);

      if(CFGMAP_query_last_uint(h_map, &self->severity, 0, symBuf)) {
         eprintf("ERROR: cannot interpret \"SEVERITY\" entry for REGEX %s", pfix);
         goto abort;
      }
#ifdef qqDEBUG
eprintf("%s = %u", symBuf, self->severity);
#endif
   }

   { /*--- Get all REGEX entries ---*/
      snprintf(symBuf, len, "%s\\REGEX", pfix);
      self->nRx= CFGMAP_find_tuples(h_map, rtn_arr, symBuf);

      self->rxArr= calloc(self->nRx, sizeof(struct TargetRx));
      assert(self->rxArr);

      for(unsigned i= 0; i < self->nRx; ++i) {

         const struct CFGMAP_tuple *tpl= rtn_arr + i; 
         struct TargetRx *rx= self->rxArr + i;

         /* Compile regular expression */
         rx->pattern= tpl->value;
         if(regex_compile(&rx->re, rx->pattern, REG_EXTENDED)) {   
            eprintf("ERROR: regex_compile(\"%s\") failed.", rx->pattern);
            goto abort;
         }
      }
   }

   rtn= 0;
abort:
   return rtn;
}

void*
Target_destructor(Target *self)
/********************************************************
 * Free resources.
 */
{
   if(self->nRx && self->rxArr) {
      for(unsigned i= 0; i < self->nRx; ++i) {
         struct TargetRx *rx= self->rxArr + i;
         regfree(&rx->re);
      }
      free(self->rxArr);
   }

   return self;
}

int
Target_scan(const Target *self, char *rsltBuf, unsigned buf_sz, const char *str)
/********************************************************
 * Scan a string to obtain the address. 
 */
{
   int rtn= EOF;

   /* Exit on first match */
   for(unsigned i= 0; i < self->nRx; ++i) {

      const struct TargetRx *rx= self->rxArr+i;

      regmatch_t matchArr[2];
      if(0 != regexec(&rx->re, str, 2, matchArr, 0) || -1 == matchArr[1].rm_so)
         continue;

      unsigned len= matchArr[1].rm_eo - matchArr[1].rm_so;

      strncpy(rsltBuf, str+matchArr[1].rm_so, buf_sz-1);
      rsltBuf[MIN(len, buf_sz-1)]= '\0';
      rtn= 0;
      break;
   }

abort:
   return rtn;
}

int
Target_MD5_update(const Target *self, MD5_CTX *ctx)
/********************************************************
 * For computing MD5 checksum of cumulative patterns.
 */
{
   for(unsigned i= 0; i < self->nRx; ++i) {
      const struct TargetRx *rx= self->rxArr+i;
      MD5_Update(ctx, rx->pattern, strlen(rx->pattern));
   }
   return 0;
}
