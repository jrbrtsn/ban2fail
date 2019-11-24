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

#include <regex.h>
#include <stdint.h>

#include "cfgmap.h"

/* Maximum number of source ip addresses per iptables command.
 * I have not tested larger numbers, and it would probably
 * work with a much larger number.  The command line maximum
 * is something like 200K characters.
 */
#define IPTABLES_BATCH_SZ 10

/* Where to find stuff */
#define CONFIGFILE "/etc/ban2fail/ban2fail.cfg"
#define LOCKPATH "/run/lock/ban2fail"
#define CACHEDIR "/var/cache/ban2fail"
#define IPTABLES "/usr/sbin/iptables" 
#define GEOIP_DB "/usr/share/GeoIP/GeoIP.dat"
#define GEOIP6_DB "/usr/share/GeoIP/GeoIPv6.dat"


/* Singleton static object with global visibility */
extern struct Global {
   enum {
      GLB_VERBOSE_FLG       =1<<0,
      GLB_LIST_ADDR_FLG     =1<<1,
      GLB_LIST_CNTRY_FLG    =1<<2,
      GLB_DONT_IPTABLE_FLG  =1<<3,
      GLB_PRINT_MASK = GLB_LIST_CNTRY_FLG|GLB_LIST_ADDR_FLG
   } flags;

   MAP logType_map;

   char *cacheDir,
        *lockPath;

   struct {
      int major,
          minor,
          patch;
   } version;

} G;


#endif

