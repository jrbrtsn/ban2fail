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

#include "ban2fail.h"
#include "cntry.h"
#include "map.h"
#include "util.h"

/*==================================================================*/
/*=================== Local static data ============================*/
/*==================================================================*/
static struct {

   int is_init;
   GeoIP *gip,
         *gip6;

} S;

static void
init()
/********************************************************
 * Perform any 1 time initialization.
 */
{
   /* Note that initialization has happened */
   S.is_init= 1;

   /* Open the GeoIP database */
   S.gip= GeoIP_open(GEOIP_DB, 0);
   if(!S.gip) {
      eprintf("PANIC: GeoIP_open(\"%s\") failed!", GEOIP_DB);
      exit(EXIT_FAILURE);
   }

   S.gip6= GeoIP_open(GEOIP6_DB, 0);
   if(!S.gip6) {
      eprintf("PANIC: GeoIP_open(\"%s\") failed!", GEOIP6_DB);
      exit(EXIT_FAILURE);
   }
}

const char*
COUNTRY_get_code(const char *addr)
/********************************************************
 * Try to get the country code.
 */
{
   if(!S.is_init) init();

   if(strchr(addr, ':'))
         return GeoIP_country_code_by_addr_v6(S.gip6, addr);

   return GeoIP_country_code_by_addr(S.gip, addr);
}

