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

#include "cntry.h"
#include "maxoff.h"

static struct {
   int is_init;
   MAP cntry_map,
       addr_map;
} S;

static void
initialize(void)
/********************************************************
 * Perform one-time initializations.
 */
{
   S.is_init= 1;
   MAP_constructor(&S.cntry_map, 10, 10);
   MAP_constructor(&S.addr_map, 1000, 200);
}

// Compiler doesn't like that we use integers in place of item pointers */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"

int
MAXOFF_allowed(const char *addr)
/********************************************************
 * Returns the maximum number of allowed offenses.
 * -1 means unlimited.
 */
{
   if(!S.is_init)
      initialize();

   /* Default is no offenses allowed */
   int rtn= 0;

   /* Check for by-address specification */
   rtn= MAP_findStrItem(&S.addr_map, addr);

   /* by-address spec trumps by-country spec */
   if(!rtn) {
      const char *cntry;
      cntry= COUNTRY_get_code(addr);
      /* if the source country of an address is not known,
       * then it will be keyed in our map under "unknown".
       */
      cntry= cntry ? cntry : "unknown";
      rtn= MAP_findStrItem(&S.cntry_map, cntry);
   }

   return rtn;
}

int
MAXOFF_init(CFGMAP *h_cfgmap, char *pfix)
/**************************************************************
 * Initialize objects from configuration map.
 */
{
   if(!S.is_init)
      initialize();

   int rtn= -1;

   size_t len= strlen(pfix)+1024;
   char symBuf[len];
   unsigned arr_sz= CFGMAP_numTuples(h_cfgmap);
   struct CFGMAP_tuple rtn_arr[arr_sz];
   int nAllowed;

   /* Get the allowed number from pfix */
   const char *str= strrchr(pfix, '\\');
   ++str;
   if(1 != sscanf(str, "%d", &nAllowed)) {
      eprintf("ERROR: \"%s\" is not an integer number.", str);
      goto abort;
   }

   { /*--- Register IP entries ---*/
      snprintf(symBuf, len, "%s\\IP", pfix);

      unsigned nFound= CFGMAP_find_tuples(h_cfgmap, rtn_arr, symBuf);
      for(unsigned i= 0; i < nFound; ++i) {
         const struct CFGMAP_tuple *tpl= rtn_arr + i; 

         /* Break the address into prefix & subnet bits */
         static char addr[43];
         unsigned bits;

         int rc= sscanf(tpl->value, "%42[0-9a-zA-Z:.]/%u", addr, &bits);

         if(1 != rc && 2 != rc) {
            eprintf("ERROR: \"%s\" is neither IP address or CIDR.", tpl->value);
            goto abort;
         }

         /* If it's simply an IP address, place in map & keep going */
         if(1 == rc) {
            /* Place in the map the number allowed as if it were
             * a pointer to an object (there is no object in this case).
             */
            MAP_addStrKey(&S.addr_map, addr, (void*)nAllowed);
            continue;
         }

         /* Avoid stupidity right off the hop */
         if(strchr(addr, ':')) {
            eprintf("ERROR: CIDR not implemented for ipv6 \"%s\"", tpl->value);
            goto abort;
         } else if(16 > bits) {
            eprintf("ERROR: Subnet is too large! \"%s\"", tpl->value);
            goto abort;
         }

         /*--- Expand the CIDR notation to addresses ---*/
         unsigned q1, q2, q3, q4;

         /* Convert quad values from text to binary */
         rc= sscanf(addr, "%u.%u.%u.%u", &q1, &q2, &q3, &q4);

         /* Convert quads into useful parameters */
         uint32_t baddr= (q1 << 24) | (q2 << 16) | (q3 << 8) | q4,
                  mask= 0xffffffff << (32-bits),
                  net= baddr & mask,
                  nAddr= 0xffffffff >> bits;

         assert(4 == rc);

         /* Generate individual addresses, and place them in the map */
         for(unsigned i= 0; i < nAddr; ++i) {
            baddr= net | i;
            q1= (baddr & 0xff000000) >> 24;
            q2= (baddr & 0x00ff0000) >> 16;
            q3= (baddr & 0x0000ff00) >> 8;
            q4= baddr & 0x000000ff;

            /* Convert back to text form */
            snprintf(addr, sizeof(addr), "%u.%u.%u.%u", q1, q2, q3, q4);
            /* Add to the map */
            MAP_addStrKey(&S.addr_map, addr, (void*)nAllowed);
         }
      }
   }

   { /*--- Register COUNTRY entries ---*/
      snprintf(symBuf, len, "%s\\COUNTRY", pfix);
      unsigned nFound= CFGMAP_find_tuples(h_cfgmap, rtn_arr, symBuf);

      for(unsigned i= 0; i < nFound; ++i) {
         const struct CFGMAP_tuple *tpl= rtn_arr + i; 

         /* Place in the map the number allowed as if it were
          * a pointer to an object (superflous in this case).
          */
         MAP_addStrKey(&S.cntry_map, tpl->value, (void*)nAllowed);

      }
   }

   rtn= 0;
abort:
   return rtn;
}

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

