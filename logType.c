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
#include <errno.h>
#include <limits.h>
#include <openssl/md5.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ban2fail.h"
#include "logEntry.h"
#include "logFile.h"
#include "logType.h"
#include "ez_dirent.h"
#include "ez_stdio.h"
#include "ez_stdlib.h"
#include "str.h"
#include "util.h"


static int
cmp_pvsort(const void *const* pp1, const void *const* pp2)
/******************************************************************
 * PTRVEC sort comaprision function to sort log file names "naturally".
 */
{
   const char *path[2]= {
      *((const char*const*)pp1),
      *((const char*const*)pp2)
   };

   static int is_init;
   static regex_t re;

   /* Use a regular expression to pick-off the log file number, if any */
   if(!is_init) {
      is_init= 1;
      regex_compile(&re, "\\.([[:digit:]]+)(\\.gz)?$", REG_EXTENDED);
   }

   int num[2]= {-1,-1};

   for(unsigned i= 0; i < 2; ++i) {
      /* Not interested in all the precedes the file name */
      const char *fname= strrchr(path[i], '/');
      assert(fname && *fname);
      /* Skip the slash */
      ++fname;
      regmatch_t matchArr[2];
      /* If no match, num[i] == -1 as initialized above */
      if(regexec(&re, fname, 2, matchArr, 0)) continue;

      /* Convert text to integer */
      int rc= sscanf(fname+matchArr[1].rm_so, "%d", num+i);
      assert(1 == rc);
   }

   /* Sort primarily by number */
   if(num[0] < num[1]) return -1;
   if(num[0] > num[1]) return 1;

   /* If numbers are the same, resort to strcmp() */
   return strcmp(path[0], path[1]);
}

#define LOGTYPE_proto_create(s, proto) \
  ((s)=(LOGTYPE_proto_constructor((s)=malloc(sizeof(LOGTYPE)), proto) ? (s) : ( s ? realloc(LOGTYPE_destructor(s),0) : 0 )))
static LOGTYPE*
LOGTYPE_proto_constructor(LOGTYPE *self, const struct logProtoType *proto)
/******************************************************************
 * Initialize an instance of a LOGTYPE class.
 */
{
   LOGTYPE *rtn= NULL;

   int rc;

   /* Not reentrant! */
   static char CacheDname[PATH_MAX],
               CacheFname[PATH_MAX];

   memset(self, 0, sizeof(*self));

   MAP_constructor(&self->file_map, 10, 10);
   self->dir= strdup(proto->dir);
   self->pfix= strdup(proto->pfix);

   if(G.flags & GLB_PRINT_LOGFILE_NAMES_FLG)
      ez_fprintf(G.listing_fh, "%s/%s\n", proto->dir, proto->pfix);

   size_t pfix_len= strlen(self->pfix);

   { /* Compute md5sum of all patterns put together */
      MD5_CTX md5ctx;
      MD5_Init(&md5ctx);
      const struct target *t;
      for(t= proto->targetArr; t->pattern; ++t) {
         MD5_Update(&md5ctx, t->pattern, strlen(t->pattern));
      }
      static unsigned char sum[16];
      MD5_Final(sum, &md5ctx);
      const char *str= bytes_2_hexStr(self->patterns_md5sum, sizeof(self->patterns_md5sum), sum, 16);
      assert(str);
   }

   /* At this point we can call LOGTYPE_cacheName() */

   /* Keep the name of this logType's cache directory */
   rc= snprintf(CacheDname, sizeof(CacheDname), "%s/%s", G.cacheDir, LOGTYPE_cacheName(self));

   { /*** Compute md5sum for each log file, then scan of read from cache ***/

      static PTRVEC fname_vec;

      PTRVEC_sinit(&fname_vec, 100);

      { /*--- Get file names from log directory and place then in a vector ---*/
         static char LogFname[PATH_MAX];

         /* Open the logfile directory, look for candidate files */
         DIR *dir= ez_opendir(self->dir);
         struct dirent *entry;

         while((entry= ez_readdir(dir))) {

            /* Skip uninteresting entries */
            if(!strcmp(".", entry->d_name) ||
               !strcmp("..", entry->d_name) ||
               strncmp(self->pfix, entry->d_name, pfix_len)) continue;

            snprintf(LogFname, sizeof(LogFname), "%s/%s", self->dir, entry->d_name);

            /* Add log file name to the vector */
            PTRVEC_addTail(&fname_vec, strdup(LogFname));

         }
         ez_closedir(dir);
      }

      PTRVEC_sort(&fname_vec, cmp_pvsort);

      /* Now scan files in lexigraphical order */
      char *log_fname;
      while((log_fname= PTRVEC_remHead(&fname_vec))) {

         /* Now compute the checksum */
         static char buf[1024];
         MD5_CTX md5ctx;
         static char sumStr[33];

         MD5_Init(&md5ctx);

         { /* Compute the checksum of the log file */

            FILE *fh= ez_fopen(log_fname, "r");
            int rc;
            while((rc= ez_fread(buf, 1, sizeof(buf), fh))) {
               MD5_Update(&md5ctx, buf, rc);
            }

            static unsigned char sum[16];
            MD5_Final(sum, &md5ctx);
            bytes_2_hexStr(sumStr, sizeof(sumStr), sum, 16);

            ez_fclose(fh);
         }

         if(G.flags & GLB_LONG_LISTING_FLG) {
            ez_fprintf(G.listing_fh, "Scanning \"%s\" ...", log_fname);
            fflush(G.listing_fh);
         }

         /* Now we have the checksum of the log file */
         /* See if the cached results exist */
         rc= snprintf(CacheFname, sizeof(CacheFname), "%s/%s", CacheDname, sumStr);
         LOGFILE *f;

         if(!access(CacheFname, F_OK)) {

            /* Construct object from cache file */
            LOGFILE_cache_create(f, CacheFname);
            assert(f);
            LOGFILE_set_logFilePath(f, log_fname);
         } else {

            if(access(CacheDname, F_OK)) {
               ez_mkdir(CacheDname, 0770);
            }

            /* Construct object from log file */
            LOGFILE_log_create(f, proto, log_fname);
            assert(f);
            LOGFILE_set_logFilePath(f, log_fname);
            if(LOGFILE_writeCache(f, CacheFname))
               assert(0);
         }
         assert(f);

         unsigned nOffFound= 0,
                  nAddrFound= 0;

         LOGFILE_offenseCount(f, &nOffFound);
         LOGFILE_addressCount(f, &nAddrFound);

         if(G.flags & GLB_LONG_LISTING_FLG) {
            ez_fprintf(G.listing_fh, " found %u offenses (%u addresses)\n", nOffFound, nAddrFound);
            fflush(G.listing_fh);
         }

         MAP_addStrKey(&self->file_map, sumStr, f);

         free(log_fname);
      }
   }

   { /*** clean up unused cache files ***/

      /* Open the cache directory, check each cache file's name against the file_map */
      DIR *dir= ez_opendir(CacheDname);
      struct dirent *entry;

      while((entry= ez_readdir(dir))) {

         /* Skip uninteresting entries */
         if(!strcmp(".", entry->d_name) ||
            !strcmp("..", entry->d_name)) continue;

         LOGFILE *f= MAP_findStrItem(&self->file_map, entry->d_name);
         if(f) continue;

         rc= snprintf(CacheFname, sizeof(CacheFname), "%s/%s", CacheDname, entry->d_name);
         ez_unlink(CacheFname);
      }

      ez_closedir(dir);
   }

   unsigned nOffFound= 0,
            nAddrFound;
   LOGTYPE_offenseCount(self, &nOffFound);
   nAddrFound= LOGTYPE_addressCount(self);

   if(G.flags & GLB_LONG_LISTING_FLG) {
      ez_fprintf(G.listing_fh, ">>>> Found %u offenses (%u addresses) for %s/%s*\n"
            , nOffFound
            , nAddrFound
            , self->dir
            , self->pfix
            );
      fflush(G.listing_fh);
   }

   rtn= self;
abort:
   return rtn;
}

int
LOGTYPE_init(CFGMAP *h_map, char *pfix)
/**************************************************************
 * Initialize object from configuration map.
 */
{
   int rtn= -1;

   struct logProtoType proto;
   memset(&proto, 0, sizeof(proto));

   size_t len= strlen(pfix)+1024;
   char symBuf[len];
   unsigned arr_sz= CFGMAP_numTuples(h_map);
   struct CFGMAP_tuple rtn_arr[arr_sz];

   { /*--- Check for "dir" symbol ---*/
      snprintf(symBuf, len, "%s\\DIR", pfix);
      proto.dir= CFGMAP_find_last_value(h_map, symBuf);

      if(!proto.dir) {
         eprintf("ERROR: cannot find \"DIR\" entry for LOGTYPE %s", pfix);
         goto abort;
      }
   }

   { /*--- Check for "prefix" symbol ---*/
      snprintf(symBuf, len, "%s\\PREFIX", pfix);
      proto.pfix= CFGMAP_find_last_value(h_map, symBuf);

      if(!proto.pfix) {
         eprintf("ERROR: cannot find \"PREFIX\" entry for LOGTYPE %s", pfix);
         goto abort;
      }
   }

   { /*--- Get all regex entries ---*/
      snprintf(symBuf, len, "%s\\REGEX", pfix);

      unsigned nFound= CFGMAP_find_tuples(h_map, rtn_arr, symBuf);

      /* Get enough object to include a terminating entry */
      struct target targetArr[nFound+1];
      /* Clear all bits in array */
      memset(targetArr, 0, sizeof(struct target)*(nFound+1));

      proto.targetArr= targetArr;

      for(unsigned i= 0; i < nFound; ++i) {
         const struct CFGMAP_tuple *tpl= rtn_arr + i; 
         struct target *tg= targetArr + i;
         tg->pattern= tpl->value;
         if(regex_compile(&tg->re, tg->pattern, REG_EXTENDED)) {   
            eprintf("ERROR: regex_compile(\"%s\") failed.", tg->pattern);
            goto abort;
         }   
      }

      /* Create the LOGTYPE object, place in global map */
      LOGTYPE *obj;
      LOGTYPE_proto_create(obj, &proto);
      assert(obj);

      /* Place int the global map */
      MAP_addStrKey(&G.logType_map, LOGTYPE_cacheName(obj), obj);

      /* Free regex pattern data */
      for(unsigned i= 0; i < nFound; ++i) {
         struct target *tg= targetArr + i;
         regfree(&tg->re);
      }
   }

   rtn= 0;
abort:
   return rtn;
}

LOGTYPE*
LOGTYPE_destructor(LOGTYPE *self)
/******************************************************************
 * Free resources associated with LOGTYPE object.
 */
{
   if(self->dir) free(self->dir);
   if(self->pfix) free(self->pfix);

   MAP_clearAndDestroy(&self->file_map, (void*(*)(void*))LOGFILE_destructor);
   MAP_destructor(&self->file_map);
  return self;
}

const char*
LOGTYPE_cacheName(const LOGTYPE *self)
/******************************************************************
 * Return in a static buffer the name of the cache directory which
 * will hold results for this log type.
 * Result will take this form:
 * dir= /var/log
 * pfix= auth
 * patterns_md5sum= 71f9514f13bb7acfe4ea2fb0ca2158b7
 * result= :var:log;auth;71f9514f13bb7acfe4ea2fb0ca2158b7
 */
{
   assert(self->dir &&
          self->pfix &&
          strlen(self->patterns_md5sum));

   /* Not reentrant! */
   static STR sb;
   STR_sinit(&sb, PATH_MAX);

   /* Encode the dir section replacing '/' with ':' */
   const char *pc;
   for(pc= self->dir; *pc; ++pc) {
      if(*pc == '/') {
         STR_putc(&sb, ':');
         continue;
      }

      STR_putc(&sb, *pc);
   }

   /* Semicolon, the the pfix part */
   STR_putc(&sb, ';');
   STR_append(&sb, self->pfix, strlen(self->pfix));
   /* Semicolon, and the md5sum */
   STR_putc(&sb, ';');
   STR_append(&sb, self->patterns_md5sum, strlen(self->patterns_md5sum));
   /* Return static buffer */
   return STR_str(&sb);
}

int
LOGTYPE_print(LOGTYPE *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh,
"LOGTYPE %p { dir= \"%s\", pfix= \"%s\", patterns_md5sum= \"%s\" }\n"
         , self
         , self->dir
         , self->pfix
         , self->patterns_md5sum
         );

   MAP_visitAllEntries(&self->file_map, (int(*)(void*,void*))LOGFILE_print, fh);
   return 0;
}

int
LOGTYPE_map_addr(LOGTYPE *self, MAP *h_rtnMap)
/********************************************************
 * Create a map of LOGENTRY objects with composite
 * counts by address.
 */
{
   MAP_visitAllEntries(&self->file_map, (int(*)(void*,void*))LOGFILE_map_addr, h_rtnMap);
   return 0;
}

int
LOGTYPE_offenseCount(LOGTYPE *self, unsigned *h_sum)
/********************************************************
 * Get a count of all offenses for this log type.
 */
{
   unsigned nFound= 0;
   MAP_visitAllEntries(&self->file_map, (int(*)(void*,void*))LOGFILE_offenseCount, &nFound);
   *h_sum += nFound;
   return 0;
}

int
LOGTYPE_addressCount(LOGTYPE *self)
/********************************************************
 * Get a count of all addresses for this log type.
 * NOT REENTRANT!
 */
{
   /* We'll need a map in which to collect unique addresses */
   static MAP smap;
   MAP_sinit(&smap, 1000, 100);

   /* Collect results for all LOGILE objects we own */
   MAP_visitAllEntries(&self->file_map, (int(*)(void*,void*))LOGFILE_map_addr, &smap);
   /* For clarity */
   unsigned nFound= MAP_numItems(&smap);
   
   /* Cleanup for next time */
   MAP_clearAndDestroy(&smap, (void*(*)(void*))LOGENTRY_destructor);
   return nFound;
}
