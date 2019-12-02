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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "ban2fail.h"
#include "ez_libc.h"
#include "iptables.h"
#include "offEntry.h"
#include "map.h"
#include "util.h"

static struct {

   int is_init;
   MAP addr_map;

} S;

static void
initialize (void)
/********************************************************
 * Prepare static data, populate index from iptables.
 */
{
   S.is_init= 1;

   MAP_constructor(&S.addr_map, N_ADDRESSES_HINT/BUCKET_DEPTH_HINT, BUCKET_DEPTH_HINT);

   const static struct ipv {
      const char *cmd,
                 *pattern;
   } ipv_arr[] = {
      { .cmd= IPTABLES " -nL INPUT 2>/dev/null",
        .pattern= "DROP[[:space:]]+all[[:space:]]+--[[:space:]]+([0-9.]+)[[:space:]]+0\\.0\\.0\\.0/0"
      },
      { .cmd= IP6TABLES " -nL INPUT 2>/dev/null",
        .pattern= "DROP[[:space:]]+all[[:space:]]+([0-9a-f:]+)[[:space:]]+::/0"
      },
      { /* Terminating member */ }
   };

   /* Take care of all ip versions ... */
   for(const struct ipv *ipv= ipv_arr; ipv->cmd; ++ipv) {

      static char lbuf[1024];
      static char addr[43];
      regex_t re;
      regmatch_t matchArr[2];
      size_t len;
      FILE *fh= NULL;

      if(regex_compile(&re, ipv->pattern, REG_EXTENDED)) {   
         eprintf("ERROR: regex_compile(\"%s\") failed.", ipv->pattern);
         exit(EXIT_FAILURE);
      }   
         
      fh= ez_popen(ipv->cmd, "r");

      while(ez_fgets(lbuf, sizeof(lbuf)-1, fh)) {

         /* Filter all that looks uninteresting */
         if(regexec(&re, lbuf, 2, matchArr, 0)) continue;

         /* Compute the length needed for the address string */
         len= matchArr[1].rm_eo -  matchArr[1].rm_so;

         /* Copy address into a null terminated static buffer */
         strncpy(addr, lbuf + matchArr[1].rm_so, sizeof(addr)-1);
         addr[len]= '\0';

         if(MAP_findStrItem(&S.addr_map, addr))
            eprintf("WARNING: duplicate iptable entry for %s", addr);
         else
            MAP_addStrKey(&S.addr_map, addr, strdup(addr));
      }
      ez_pclose(fh);
      regfree(&re);
   }
}

int
IPTABLES_is_currently_blocked(const char *addr)
/********************************************************
 * This provides an efficient lookup of addresses blocked
 * by iptables in the filter table, INPUT chain.
 * 
 * RETURN:
 * 1 if the supplied addr is blocked by iptables.
 * 0 otherwise.
 */
{
   if(!S.is_init)
      initialize();

   /* See if this addr is in the map */
   if(MAP_findStrItem(&S.addr_map, addr)) return 1;
   return 0;
}

static int
addrCmp_pvsort(const void *const* pp1, const void *const* pp2)
/**************************************************************
 * PTRVEC_sort() comparison function for addresses, puts
 * ipv6 at the bottom.
 */
{
   const char *addr1= *((const char*const*)pp1),
              *addr2= *((const char*const*)pp2);

   if(strchr(addr2, ':')) {
      if(!strchr(addr1, ':')) return -1;
   } else {
      if(strchr(addr1, ':')) return 1;
   }
  
   return 0;
}

static int
_control_addresses(int cmdFlag, PTRVEC *h_vec, unsigned batch_sz)
/**************************************************************
 * (Un)block addresses in batches of batch_sz.
 */
{
   if(!S.is_init)
      initialize();

   int rtn= -1;

   /* Sanity check for debugging */
   assert(batch_sz > 0 && batch_sz <= 100);

   /* Use string buffer to form command */
   static STR cmd_sb;

   const char *addr;

   /* Put any ipv6 addresses at end */
   PTRVEC_sort(h_vec, addrCmp_pvsort);

   /* Work through ipv4 addresses in the vector */
   while((addr= PTRVEC_remHead(h_vec)) &&
         !strchr(addr, ':'))
   {
      /* Initialize / reset string buffer */
      STR_sinit(&cmd_sb, 256+batch_sz*42);

      /* Beginning of command string, with first source address */
      STR_sprintf(&cmd_sb, IPTABLES " 2>&1 -%c INPUT -s %s", cmdFlag, addr);

      /* Append additional source addresses */
      unsigned i= 1;
      while(i < batch_sz &&
            (addr= PTRVEC_remHead(h_vec)) &&
            !strchr(addr, ':'))
      {
         /* employ multiple source addresses for batching */
         STR_sprintf(&cmd_sb, ",%s", addr);
         ++i;
      }

      /* Put the end of the command in place */
      STR_sprintf(&cmd_sb, " -j DROP");

      /* Run iptables */
      FILE *fh= ez_popen(STR_str(&cmd_sb), "r");
      /* Display any output from iptables */
      static char lbuf[1024];
      while(ez_fgets(lbuf, sizeof(lbuf), fh))
         ez_fprintf(stderr, "NOTE: iptables output: %s", lbuf);

      /* All done */
      ez_pclose(fh);

      /* If the last address pulled was ipv6, move on */
      if(addr && strchr(addr, ':')) break;
   }

   /* Work through ipv6 addresses in the vector */
   for( ; addr; (addr= PTRVEC_remHead(h_vec))) {

      /* Initialize / reset string buffer */
      STR_sinit(&cmd_sb, 256+batch_sz*42);

      /* Beginning of command string, with first source address */
      STR_sprintf(&cmd_sb, IP6TABLES " 2>&1 -%c INPUT -s %s", cmdFlag, addr);

      /* Append additional source addresses */
      unsigned i= 1;
      while(i < batch_sz && (addr= PTRVEC_remHead(h_vec))) {

         /* employ multiple source addresses for batching */
         STR_sprintf(&cmd_sb, ",%s", addr);
         ++i;
      }

      /* Put the end of the command in place */
      STR_sprintf(&cmd_sb, " -j DROP");

      /* Run iptables */
      FILE *fh= ez_popen(STR_str(&cmd_sb), "r");
      /* Display any output from iptables */
      static char lbuf[1024];
      while(ez_fgets(lbuf, sizeof(lbuf), fh))
         ez_fprintf(stderr, "NOTE: ip6tables output: %s", lbuf);

      /* All done */
      ez_pclose(fh);

   }

   rtn= 0;
abort:
   return rtn;
}

int
IPTABLES_block_addresses(PTRVEC *h_vec, unsigned batch_sz)
/**************************************************************
 * Block addresses in batches of batch_sz.
 */
{
   if(!S.is_init)
      initialize();

   return _control_addresses('A', h_vec, batch_sz);

}

int
IPTABLES_unblock_addresses(PTRVEC *h_vec, unsigned batch_sz)
/**************************************************************
 * Block addresses in batches of batch_sz.
 */
{
   if(!S.is_init)
      initialize();

   return _control_addresses('D', h_vec, batch_sz);

}

static int
fill_in_missing(char *blocked_addr, MAP *h_rtn_map)
/**************************************************************
 * If blocked_addr is not in h_rtn_map, create an object and
 * place it their.
 */
{
   if( MAP_findStrItem(h_rtn_map, blocked_addr)) return 0;

   /* Create a new faux logentry object */
   OFFENTRY *e;
   OFFENTRY_addr_create(e, blocked_addr);
   assert(e);

   /* Place in the return map */
   MAP_addStrKey(h_rtn_map, blocked_addr, e);

   return 0;
}

int
IPTABLES_fill_in_missing(MAP *h_rtn_map)
/**************************************************************
 * Fill in all blocked IP's which are not already in *h_map.
 */
{
   if(!S.is_init)
      initialize();

   int rtn= -1;

   MAP_visitAllEntries(&S.addr_map, (int(*)(void*,void*))fill_in_missing, h_rtn_map);



   rtn= 0;
abort:
   return rtn;
}
