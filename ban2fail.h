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
/***************************************************************************************
 * ban2fail global process declarations and data.
 ***************************************************************************************/

#ifndef BAN2FAIL_H
#define BAN2FAIL_H

#define _GNU_SOURCE
#include <regex.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

#include "cfgmap.h"

/* Maximum number of source ip addresses per iptables command.
 * I have not tested larger numbers, and it would probably
 * work with a much larger number.  The command line maximum
 * is something like 200K characters.
 */
#define IPTABLES_BATCH_SZ 100

/* For sizing maps and vectors, this a starting point */
#define N_ADDRESSES_HINT 10000
#define BUCKET_DEPTH_HINT 10

#define MD5SUM_SZ 32

/* How long to wait for reverse DNS lookups before giving up and
 * moving on with the report.
 */
#ifdef DEBUG
#       define DFLT_DNS_PAUSE_SEC 10
#else
#       define DFLT_DNS_PAUSE_SEC 60
#endif

/* Where to find stuff */
#define CONFIGFILE "/etc/ban2fail/ban2fail.cfg"
#define LOCKDIR "/run/lock/ban2fail"
#define CACHEDIR "/var/cache/ban2fail"
#define IPTABLES "/usr/sbin/iptables" 
#define IP6TABLES "/usr/sbin/ip6tables" 
#define GEOIP_DB "/usr/share/GeoIP/GeoIP.dat"
#define GEOIP6_DB "/usr/share/GeoIP/GeoIPv6.dat"
#define GROUP_NAME "adm"

enum GlobalFlg_enum {
   GLB_VERBOSE_FLG            =1<<0,
   GLB_LIST_ADDR_FLG          =1<<1,
   GLB_LIST_CNTRY_FLG         =1<<2,
   GLB_DONT_IPTABLE_FLG       =1<<3,
   GLB_LIST_SUMMARY_FLG       =1<<4,
   GLB_PRINT_LOGFILE_NAMES_FLG=1<<5,
   GLB_DNS_LOOKUP_FLG         =1<<6,
   GLB_DNS_FILTER_BAD_FLG     =1<<7,
   GLB_FLUSH_CACHE_FLG        =1<<8,
   GLB_CMDLINE_ADDR_FLG       =1<<9,
   GLB_LONG_LISTING_MASK = GLB_LIST_CNTRY_FLG|GLB_LIST_ADDR_FLG
};

enum BlockedFlg_enum {
   BLOCKED_FLG          =1<<0,
   WOULD_BLOCK_FLG      =1<<1,
   UNJUST_BLOCK_FLG     =1<<2,
   WHITELIST_FLG        =1<<3
};

/* Singleton static object with global visibility */
extern struct Global {

   enum GlobalFlg_enum flags;

   MAP logType_map;

   struct {
      char *dir;
      mode_t dir_mode,
             file_mode;
   } cache;

   struct {
      char *dir;
      mode_t dir_mode,
             file_mode;
   } lock;

   /* This should be set to adm */
   gid_t gid;


   struct {
      FILE *fh;
      MAP AddrRPT_map;
   } rpt;

   struct {
      int major,
          minor,
          patch;
   } version;
   
   struct {
      const struct bitTuple *flags;
   } bitTuples;

   struct {
      time_t time_t;
      struct tm tm;
   } begin;

} G;


#endif

