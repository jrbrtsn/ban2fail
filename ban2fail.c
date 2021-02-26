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
#include <getopt.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>

#include "addrRpt.h"
#include "ban2fail.h"
#include "cntry.h"
#include "ez_libanl.h"
#include "ez_libc.h"
#include "iptables.h"
#include "offEntry.h"
#include "logFile.h"
#include "logType.h"
#include "map.h"
#include "maxoff.h"
#include "pdns.h"
#include "str.h"
#include "util.h"

/*==================================================================*/
/*=================== Support structs ==============================*/
/*==================================================================*/
struct cntryStat {
   char *cntry;
   unsigned nAddr;
};

/* Need this for initialization from configuration file */
struct initInfo {
  const char *symStr;
  int (*init_f)(CFGMAP *map, char *symStr);
};

/*==================================================================*/
/*================= Forward declarations ===========================*/
/*==================================================================*/

static int addrRpt_serial_qsort(const void *p1, const void *p2);
static int cntryStat_count_qsort(const void *p1, const void *p2);
static int configure(CFGMAP *h_cfgmap, const char *pfix);
static int logentry_count_qsort(const void *p1, const void *p2);
static int logentry_latest_qsort(const void *p1, const void *p2);
static int map_byCountries(OFFENTRY *e, MAP *h_map);
static int stub_init(CFGMAP *map, char *symStr);


/*==================================================================*/
/*========================= static data ============================*/
/*==================================================================*/
static const struct bitTuple GlobalFlagBitTuples[]= {
   {.name= "GLB_VERBOSE_FLG", .bit= GLB_VERBOSE_FLG},
   {.name= "GLB_LIST_ADDR_FLG", .bit= GLB_LIST_ADDR_FLG},
   {.name= "GLB_LIST_CNTRY_FLG", .bit= GLB_LIST_CNTRY_FLG},
   {.name= "GLB_DONT_IPTABLE_FLG", .bit= GLB_DONT_IPTABLE_FLG},
   {/* Terminating member */}
};

struct Global G= {
   .cache= {
      .dir= CACHEDIR,
      .dir_mode=0770,
      .file_mode=0660
   },
   .lock= {
      .dir= LOCKDIR,
      .dir_mode= 0770,
      .file_mode=0660
   },

   .version= {
      .major= 0,
      .minor= 14,
      .patch= 5
   },

   .bitTuples.flags= GlobalFlagBitTuples
};

const static struct initInfo S_initInfo_arr[] = {
   {.symStr= "MAX_OFFENSES",     .init_f= MAXOFF_init},
   {.symStr= "LOGTYPE",          .init_f= LOGTYPE_init},
   {/* Terminating member */}
};


/*================ Local only static struct  ======================*/
static struct {

   enum {
      PAGER_RUNNING_FLG= 1<<31
   } flags;

   /* OFFENTRY object indexed by ip address */
   MAP addr2logEntry_map;

   /* CFGMAP containing our configuration information */
   CFGMAP cfgmap;

   /* Vectors for storing ip address which are to be blocked and
    * Unblocked.
    */
   PTRVEC toBlock_vec,
          toUnblock_vec;

   /* Used to place OFFENTRY address objects into linear
    * access container.
    */
   OFFENTRY **lePtrArr;

   /* Avoid multiple instances of filename buffers */
   char fnameBuf[PATH_MAX];

   int cacheLock_fd,
       iptablesLock_fd;

} S= {
   .cacheLock_fd= -1,
   .iptablesLock_fd= -1
};

/*==================================================================*/
/*======================== main() ==================================*/
/*==================================================================*/
/* Enums for long options */
enum {
   VERSION_OPT_ENUM=128, /* Larger than any printable character */
   HELP_OPT_ENUM,
   PRINT_LOGFILE_NAMES_ENUM
};

int
main(int argc, char **argv)
/***************************************************************
 * Program execution begins here.
 */
{

   int rtn= EXIT_FAILURE;

   char *confFile= CONFIGFILE;

   /* Prepare static data */
   // global
   struct group *gr= ez_getgrnam(GROUP_NAME);
   G.gid= gr->gr_gid;

   /* Default sending listing to stdout */
   G.rpt.fh= stdout;
   MAP_constructor(&G.logType_map, 10, 10);
   MAP_constructor(&G.rpt.AddrRPT_map, 10, 10);

   // local
   MAP_constructor(&S.addr2logEntry_map, N_ADDRESSES_HINT/BUCKET_DEPTH_HINT, BUCKET_DEPTH_HINT);

   PTRVEC_constructor(&S.toBlock_vec, N_ADDRESSES_HINT);
   PTRVEC_constructor(&S.toUnblock_vec, N_ADDRESSES_HINT);

   /* Parse command line arguments */
   { /*==================================================================*/
      int c, errflg= 0;
      extern char *optarg;
      extern int optind, optopt;

      for(;;) {

         static const struct option long_options[]= {
            {"help", no_argument, 0, HELP_OPT_ENUM},
            {"print-lfn", no_argument, 0, PRINT_LOGFILE_NAMES_ENUM},
            {"version", no_argument, 0, VERSION_OPT_ENUM},
            {}
         };

         int c, option_ndx= 0;

         c= getopt_long(argc, argv, ":a::cFst:v", long_options, &option_ndx);

         if(-1 == c) break;

        switch(c) {

            /* print usage help */
            case HELP_OPT_ENUM:
               ++errflg;
               break;

            case 'a':
               G.flags |= GLB_LIST_ADDR_FLG|GLB_DONT_IPTABLE_FLG;
               if(optarg) {
                  if(*optarg == '+') {
                     G.flags |= GLB_DNS_LOOKUP_FLG;
                  } else if(*optarg == '-') {
                     G.flags |= GLB_DNS_LOOKUP_FLG|GLB_DNS_FILTER_BAD_FLG;
                  } else
                     ++errflg;
               }
               break;

            case 'c':
               G.flags |= GLB_LIST_CNTRY_FLG|GLB_DONT_IPTABLE_FLG;
               break;

            case 'F':
               G.flags |= GLB_FLUSH_CACHE_FLG;
               break;

            case 's':
               G.flags |= GLB_LIST_SUMMARY_FLG|GLB_DONT_IPTABLE_FLG;
               break;

            case 't':
               G.flags |= GLB_DONT_IPTABLE_FLG;
               G.cache.dir= CACHEDIR "-test";
               G.lock.dir= LOCKDIR "-test";
               confFile= optarg;
               break;

            case 'v':
               G.flags |= GLB_VERBOSE_FLG;
               break;

            case PRINT_LOGFILE_NAMES_ENUM:
               G.flags |= GLB_PRINT_LOGFILE_NAMES_FLG|GLB_DONT_IPTABLE_FLG;
               break;

            case VERSION_OPT_ENUM:
               ez_fprintf(stdout, "ban2fail v%d.%d.%d\n", G.version.major, G.version.minor, G.version.patch);
               return 0;

            case '?':
               ez_fprintf(stderr, "Unrecognized option: -%c\n", optopt);
               ++errflg;
               break;
         }
      }

      if(errflg) {
         ez_fprintf(stderr, 
"ban2fail v%d.%d.%d Usage:\n"
"%s [options] [-t confFile] [addr1 addr2 ...]\n"
"  addr1 ...  print offending lines from logfiles for these addresses\n"
"  --help\tprint this usage message.\n"
"  -a[+|-]\tList results by Address. '+' to perform DNS lookups, '-' to filter riff raff.\n"
"  -c\t\tlist results by Country\n"
"  -F\t\tFlush the cache\n"
"  -s\t\tlist Summary results only\n"
"  -t confFile\tTest confFile, do not apply iptables rules\n"
"  -v\t\tVerbose information about unrecognized configuration info\n"
"  --print-lfn\tprint the names of primary logfiles to scan\n"
"  --version\tprint the version number and exit.\n"
         , G.version.major, G.version.minor, G.version.patch
         , argv[0]
         );

         goto abort;
      }

      /* Pick up addresses on command line */
      for(; optind < argc; ++optind) {

         AddrRPT *ar;
         const char *addr= argv[optind];

         /* Skip duplicates */
         if(MAP_findStrItem(&G.rpt.AddrRPT_map, addr)) continue;

         /* Create a new address report object */
         AddrRPT_addr_create(ar, addr);
         assert(ar);

         /* Place it in global map */
         MAP_addStrKey(&G.rpt.AddrRPT_map, addr, ar);

         G.flags |= GLB_DONT_IPTABLE_FLG;

      }


   } /* Done with command line arguments */

   char *pager= NULL,
        *rslt= getenv("PAGER");

#if 0
   /* Keep a copy of the pager environment variable */
   if(rslt) pager= strdup(rslt);

   /* So we can run iptables */
   ez_setuid(0);
   ez_setgid(G.gid);

   /* Restore the pager environment variable */
   if(pager) {
      if(setenv("PAGER", pager, 1)) assert(0);
   }
#endif

   /* Get a time when the scan began */
   G.begin.time_t= time(NULL);
   G.begin.tm= *localtime(&G.begin.time_t);

   /* Read the configuration file */
   { /*=========================================================*/
      if(!CFGMAP_file_constructor(&S.cfgmap, confFile)) {
         eprintf("ERROR: failed to read configuration from \"%s\"", confFile);
         goto abort;
      }

      /* For debugging this can be useful */
//      CFGMAP_print(&S.cfgmap, G.rpt.fh);

      /* Just leave the S.cfgmap in place, so all the value strings
       * don't need to be copied.
       */
   }

   /* Obtain a file lock to protect cache files */
   /*===========================================================*/
   {
      if(-1 == ez_access(G.lock.dir, F_OK)) {
         ez_mkdir(G.lock.dir, G.lock.dir_mode);
         ez_chown(G.lock.dir, getuid(), G.gid);
      }

      snprintf(S.fnameBuf, sizeof(S.fnameBuf), "%s/cache", G.lock.dir);
      /* Make sure the file exists by open()'ing */
      S.cacheLock_fd= ez_open(S.fnameBuf, O_CREAT|O_RDONLY|O_CLOEXEC, G.lock.file_mode);
      ez_fchown(S.cacheLock_fd, getuid(), G.gid);

      /* Let's get a exclusive lock */
      // TODO: set SIGALRM to knock us out of blocked wait?
      int rc= ez_flock(S.cacheLock_fd, LOCK_EX);
   }

#ifndef DEBUG
   /* if stdout is a tty, and listing is likely
    * to be long, then use $PAGER.
    */
   if(G.flags & GLB_LONG_LISTING_MASK && isatty(fileno(G.rpt.fh))) {
      S.flags |= PAGER_RUNNING_FLG;
      G.rpt.fh= pager_open();
   }
#endif
   assert(G.rpt.fh);

   /* Open our cache, instance file-specific LOGTYPE objects */
   { /*=============================================================*/
      if(G.flags & GLB_FLUSH_CACHE_FLG &&
         !ez_access(G.cache.dir, F_OK))
      {
         ez_rmdir_recursive(G.cache.dir);
      }

      /* Make the directory if needed */
      if(ez_access(G.cache.dir, F_OK)) {
         /* errno will be set if access() fails */
         errno= 0;

         ez_mkdir(G.cache.dir, G.cache.dir_mode);
         ez_chown(G.cache.dir, getuid(), G.gid);
      }

      if(G.flags & GLB_LONG_LISTING_MASK) {
         ez_fprintf(G.rpt.fh, "=============== ban2fail v%d.%d.%d =============\n"
               , G.version.major
               , G.version.minor
               , G.version.patch
               );
         fflush(G.rpt.fh);
      }

      /* Implement configuration */
      { /*-----------------------------------------------------*/

         if(configure(&S.cfgmap, NULL)) {
            eprintf("ERROR: failed to realize configuration in \"%s\"", confFile);
            goto abort;
         }

         if(G.flags & GLB_VERBOSE_FLG) { /* Warn about unused symbols */
            CFGMAP_print_unused_symbols(&S.cfgmap, G.rpt.fh);
            fflush(G.rpt.fh);
         }

         /* Just leave the S.cfgmap in place, so all the value strings
          * don't need to be copied.
          */

      } /* End implement configuration */

      /*----------------- Print logfile names short circuiting --------------*/
      if(G.flags & GLB_PRINT_LOGFILE_NAMES_FLG) {
         /* Shortcut any further processing or reporting */
         rtn= 0;
         goto abort;
      }

      /* Check cache for logType directories not in our current map, and remove them */
      { /*---------------------------------------------------------------------*/
         DIR *dir= ez_opendir(G.cache.dir);
         struct dirent *entry;

         while((entry= ez_readdir(dir))) {

            /* Skip uninteresting entries */
            if('.' == *entry->d_name) continue;

            LOGTYPE *t= MAP_findStrItem(&G.logType_map, entry->d_name);
            /* If there is a matching entry, then do not delete results */
            if(t)
               continue;

            /* Make the path with filename */
            snprintf(S.fnameBuf, sizeof(S.fnameBuf), "%s/%s", G.cache.dir, entry->d_name);

            /* Remove unused directory & contents. */
            ez_rmdir_recursive(S.fnameBuf);

         }
         ez_closedir(dir);
      } /* End of cache management */

      /* We're done with disk I/O, so release lock */
      /*-----------------------------------------------------------------------*/
      if(-1 != S.cacheLock_fd) {
         ez_flock(S.cacheLock_fd, LOCK_UN);
         ez_close(S.cacheLock_fd);
         S.cacheLock_fd= -1;
      }

      /* Processing only for long listings */
      /*-----------------------------------------------------------------------*/
      if(G.flags & GLB_LONG_LISTING_MASK) {
         MAP map;
         MAP_constructor(&map, N_ADDRESSES_HINT/BUCKET_DEPTH_HINT, BUCKET_DEPTH_HINT);

         unsigned nOffFound= 0,
                  nAddrFound;
         MAP_visitAllEntries(&G.logType_map, (int(*)(void*,void*))LOGTYPE_offenseCount, &nOffFound);
         /* Collect unique addresses into a map */
         MAP_visitAllEntries(&G.logType_map, (int(*)(void*,void*))LOGTYPE_map_addr, &map);

         /* Number of items in map is number of unique addresses */
         nAddrFound= MAP_numItems(&map);

         ez_fprintf(G.rpt.fh,
"===== Found %u total offenses (%u addresses) =====\n"
            , nOffFound
            , nAddrFound
            );
         fflush(G.rpt.fh);

         /* Clean up map used for counting */
         MAP_clearAndDestroy(&map, (void*(*)(void*))OFFENTRY_destructor);
         MAP_destructor(&map);
      }
   } /* End of cache and logfile-specific OFFENTRY objects */

   /* Now get a map of OFFENTRY objects that have combined counts.
    * Perform all remaining processing and reporting.
    */
   { /*=======================================================================*/

      /* List by address. Make a addr_map of OFFENTRY objects with composite counts */
      MAP_visitAllEntries(&G.logType_map, (int(*)(void*,void*))LOGTYPE_map_addr, &S.addr2logEntry_map);

      /* So we can run iptables */
      ez_setuid(0);
      ez_setgid(G.gid);

      /* Pick up remaining blocked addresses */
      IPTABLES_fill_in_missing(&S.addr2logEntry_map);

      unsigned nItems= MAP_numItems(&S.addr2logEntry_map);

      /* allocate this array, let it leak */
      S.lePtrArr= malloc(sizeof(void*) * nItems);
      assert(S.lePtrArr);

      MAP_fetchAllItems(&S.addr2logEntry_map, (void**)S.lePtrArr);
      qsort(S.lePtrArr, nItems, sizeof(OFFENTRY*), logentry_latest_qsort);

      /* Special processing for DNS lookups */
      if(G.flags & GLB_DNS_LOOKUP_FLG) {

         ez_fprintf(G.rpt.fh, "Performing DNS lookups for up to %d seconds ...\n", DFLT_DNS_PAUSE_SEC);
         fflush(G.rpt.fh);

         int64_t begin_ms= clock_gettime_ms(CLOCK_REALTIME);
         int rc= PDNS_lookup(S.lePtrArr, nItems, DFLT_DNS_PAUSE_SEC*1000);
         assert(-1 != rc);
         int64_t ms= clock_gettime_ms(CLOCK_REALTIME) - begin_ms;
         ez_fprintf(G.rpt.fh, "\t==> Completed %d of %u lookups in %.1f seconds\n", rc, nItems, (double)ms/1000.);
      }

      /* Process each OFFENTRY item */
      for(unsigned i= 0; i < nItems; ++i) {
         int flags=0;

         OFFENTRY *e= S.lePtrArr[i];

         if(IPTABLES_is_currently_blocked(e->addr))
            flags |= BLOCKED_FLG;

         int nAllowed= MAXOFF_allowed(e->addr);

         if(-1 == nAllowed)
             flags |= WHITELIST_FLG;

         if((flags & WHITELIST_FLG || e->count <= nAllowed) &&
            (flags & BLOCKED_FLG))
         {

             flags |= UNJUST_BLOCK_FLG;
             PTRVEC_addTail(&S.toUnblock_vec, e->addr);
         }

         if(!(flags & BLOCKED_FLG) &&
            !(flags & WHITELIST_FLG) &&
            e->count > nAllowed)
         {

             flags |= WOULD_BLOCK_FLG;
             PTRVEC_addTail(&S.toBlock_vec, e->addr);
         }

         /* Print out only for list option */
         if(G.flags & GLB_LIST_ADDR_FLG &&
            !(G.flags & GLB_DNS_FILTER_BAD_FLG && e->dns.flags & PDNS_BAD_MASK))
         {
            OFFENTRY_list(e, G.rpt.fh, flags, nAllowed);
         }

      } /*--- End of OFFENTRY processing ---*/

      unsigned currBlocked= MAP_numItems(&S.addr2logEntry_map);

      /* List offenses by country if directed to do so */
      if(G.flags & (GLB_LIST_CNTRY_FLG|GLB_LIST_SUMMARY_FLG)) {

         /* Map for indexing cntryStat objects */
         static MAP byCntry_map;
         MAP_sinit(&byCntry_map, 100, 10);

         /* Build index by trawling existing by-address map */
         MAP_visitAllEntries(&S.addr2logEntry_map, (int(*)(void*,void*))map_byCountries, &byCntry_map);

         /* Now get all cntStat handles in a vector */
         unsigned vec_sz= MAP_numItems(&byCntry_map);
         struct cntryStat *rtn_vec[vec_sz];

         MAP_fetchAllItems(&byCntry_map, (void**)rtn_vec);

         /* Sort high to low */
         qsort(rtn_vec, vec_sz, sizeof(struct cntryStat*), cntryStat_count_qsort);

         if(G.flags & GLB_LIST_CNTRY_FLG) {
            /* Print results */
            for(unsigned i= 0; i < vec_sz; ++i) {

               struct cntryStat *cs= rtn_vec[i];
               ez_fprintf(G.rpt.fh, "%2s  %5u blocked addresses\n"
                     , cs->cntry[0] ? cs->cntry : "--"
                     , cs->nAddr
                     );
            }

            ez_fprintf(G.rpt.fh, "===============================================\n");
         }

         ez_fprintf(G.rpt.fh, "%6u countries affected\n" , vec_sz);

      }

      /* Take care of summary blocking and reporting */
      unsigned n2Block= PTRVEC_numItems(&S.toBlock_vec);
      unsigned n2Unblock= PTRVEC_numItems(&S.toUnblock_vec);
   
      if(G.flags & GLB_LIST_ADDR_FLG && !(G.flags & (GLB_LIST_SUMMARY_FLG|GLB_LIST_CNTRY_FLG)))
         ez_fprintf(G.rpt.fh, "===============================================\n");

      if(!(G.flags & GLB_DONT_IPTABLE_FLG)) {

         if(n2Block || n2Unblock) {
            snprintf(S.fnameBuf, sizeof(S.fnameBuf), "%s/iptables", G.lock.dir);
            /* Make sure the file exists by open()'ing */
            S.iptablesLock_fd= ez_open(S.fnameBuf, O_CREAT|O_WRONLY|O_CLOEXEC, G.lock.file_mode);
            ez_fchown(S.iptablesLock_fd, getuid(), G.gid);
            /* Get an exclusive lock on the lockfile */
            ez_flock(S.iptablesLock_fd, LOCK_EX);
         }

         if(n2Block) {

            if(IPTABLES_block_addresses(&S.toBlock_vec)) {
               eprintf("ERROR: cannot block addresses!");
               goto abort;
            }
            ez_fprintf(G.rpt.fh, "Blocked %u new hosts\n", n2Block);
         }

         if(n2Unblock) {

            if(IPTABLES_unblock_addresses(&S.toUnblock_vec)) {
               eprintf("ERROR: cannot unblock addresses!");
               goto abort;
            }
            ez_fprintf(G.rpt.fh, "Unblocked %u hosts\n", n2Unblock);
         }

         /* Release the lock */
         if(-1 != S.iptablesLock_fd) {
            ez_flock(S.iptablesLock_fd, LOCK_UN);
            ez_close(S.iptablesLock_fd);
            S.iptablesLock_fd= -1;
         }

      } else {

         if(n2Block) 
            ez_fprintf(G.rpt.fh, "Would block %u new hosts\n", n2Block);

         if(n2Unblock)
            ez_fprintf(G.rpt.fh, "Would unblock %u hosts\n", n2Unblock);
      }

      if(G.flags & (GLB_LIST_ADDR_FLG|GLB_LIST_SUMMARY_FLG))
            ez_fprintf(G.rpt.fh, "%6u addresses currently blocked\n" , currBlocked + n2Block - n2Unblock);

   }

   /* Print out address reports */
   { /*==========================================================*/
      unsigned nItems= MAP_numItems(&G.rpt.AddrRPT_map);
      AddrRPT *arArr[nItems];

      MAP_fetchAllItems(&G.rpt.AddrRPT_map, (void**)arArr);
      qsort(arArr, nItems, sizeof(AddrRPT*), addrRpt_serial_qsort);
      for(unsigned i= 0; i < nItems; ++i) {
         AddrRPT_print(arArr[i], G.rpt.fh);
      }
   }

   fflush(G.rpt.fh);

   rtn= EXIT_SUCCESS;
abort:

   /* Wait for pager to finish, if it is running */
   if(S.flags & PAGER_RUNNING_FLG)
      ez_pclose(G.rpt.fh);

   /* Make sure lock file is unlocked */
   if(-1 != S.cacheLock_fd) {
      ez_flock(S.cacheLock_fd, LOCK_UN);
      ez_close(S.cacheLock_fd);
   }

   if(-1 != S.iptablesLock_fd) {
      ez_flock(S.iptablesLock_fd, LOCK_UN);
      ez_close(S.iptablesLock_fd);
   }
   return rtn;
}


/*==================================================================*/
/*============== Supporting functions ==============================*/
/*==================================================================*/
static int
logentry_latest_qsort(const void *p1, const void *p2)
/***************************************************************
 * qsort functor puts large counts on top.
 */
{
   const OFFENTRY *le1= *(const OFFENTRY *const*)p1,
                  *le2= *(const OFFENTRY *const*)p2;

   if(le1->latest > le2->latest) return -1;
   if(le1->latest < le2->latest) return 1;
   return logentry_count_qsort(p1, p2);
}


static int
logentry_count_qsort(const void *p1, const void *p2)
/***************************************************************
 * qsort functor puts large counts on top.
 */
{
   const OFFENTRY *le1= *(const OFFENTRY *const*)p1,
                  *le2= *(const OFFENTRY *const*)p2;

   if(le1->count > le2->count) return -1;
   if(le1->count < le2->count) return 1;
   return 0;
}

static int
cntryStat_count_qsort(const void *p1, const void *p2)
/***************************************************************
 * qsort functor puts large counts on top.
 */
{
   const struct cntryStat
      *cs1= *(const struct cntryStat *const*)p1,
      *cs2= *(const struct cntryStat *const*)p2;

   if(cs1->nAddr > cs2->nAddr) return -1;
   if(cs1->nAddr < cs2->nAddr) return 1;
   return 0;
}

static int
addrRpt_serial_qsort(const void *p1, const void *p2)
/***************************************************************
 * qsort functor sorts by serial number, low to high
 */
{
   const AddrRPT *ar1= *(const AddrRPT *const*)p1,
                 *ar2= *(const AddrRPT *const*)p2;

   if(ar1->serial < ar2->serial) return -1;
   if(ar1->serial > ar2->serial) return 1;
   return 0;
}

static int
configure(CFGMAP *h_cfgmap, const char *pfix)
/*****************************************************************
 * dynamic initialization from contents of configuration
 * dictionary.
 */
{
  int rtn= 1;
  const CFGMAP_ENTRY *pCde;
  const struct initInfo *pIi;

  for(pIi= S_initInfo_arr; pIi->symStr; ++pIi) {
    char buf[1024];
    /* Create the symbol we will look for */
    snprintf(buf, sizeof(buf), "%s\\%s", pfix ? pfix : "", pIi->symStr);

    if((pCde= CFGMAP_find(h_cfgmap, buf))) {
      unsigned i;
      for(i= 0; i < CFGMAP_ENTRY_numValues(pCde); ++i) {
        /* Create the name for this object */
        snprintf(buf, sizeof(buf), "%s\\%s", pfix ? pfix : "", CFGMAP_ENTRY_value(pCde, i));
        /* Call the initialization function */
        if((*pIi->init_f)(h_cfgmap, buf)) goto abort;
        /* recurse with longer pfix */
        if(configure(h_cfgmap, buf)) {
          eprintf("ERROR: initialization function failed.");
          goto abort;
        }
      }
    }
  }

  rtn= 0;

abort:
  return rtn;
}

#ifdef DEBUG
static int
stub_init(CFGMAP *map, char *symStr)
/*****************************************************************
 * Stand-in xxx_init() function until a proper one is implemented.
 */
{
   eprintf("HERE, symStr= \"%s\"", symStr);
   return 0;
}
#endif

static int
map_byCountries(OFFENTRY *e, MAP *h_map)
/**************************************************************
 * Generate a "by country" map of cntryStat objects.
 */
{
   struct cntryStat *cs= MAP_findStrItem(h_map, e->cntry);
   if(!cs) {
      cs= calloc(1, sizeof(*cs));
      cs->cntry= e->cntry;
      MAP_addStrKey(h_map, cs->cntry, cs);
   }

   ++cs->nAddr;

   return 0;
}
