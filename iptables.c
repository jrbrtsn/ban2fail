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
#include <sys/wait.h>
#include <unistd.h>

#include "ban2fail.h"
#include "ez_libc.h"
#include "iptables.h"
#include "limits.h"
#include "map.h"
#include "offEntry.h"
#include "util.h"

/* JDR Fri 26 Feb 2021 09:37:59 AM EST
 * it appears that iptables has a limit on how many
 * addresses it will handle in a single command.
 */
#define IPTABLES_MAX_ADDR 9000

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
#ifdef qqDEBUG
      unsigned count= 0;
#endif
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
#ifdef qqDEBUG
         ++count;
#endif
      }
      ez_pclose(fh);
      regfree(&re);
#ifdef qqDEBUG
eprintf("%s got %u entries", ipv->cmd, count);
#endif
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
run_command(const char *argv[])
/**************************************************************
 * Run a command given argv using fork() and execve(). Wait
 * for command to finish.
 */
{
#ifdef DEBUG
   { // Print argv[] to stderr
      ez_fprintf(stderr, "argv[]= {\n");
      const char **ppstr;
      for(ppstr= argv; *ppstr; ++ppstr)
         ez_fprintf(stderr, "\t%s\n", *ppstr);

      ez_fputs("}\n", stderr);
      ez_fflush(stderr);
   }
#endif
   int out_pipe[2];

   /* Create a connected pipe for output from command */
   ez_pipe(out_pipe);

   // Parent will read from out_pipe[0];

   // Create child process
   pid_t child_pid= ez_fork();

   if(!child_pid) { // Child process

      // Close useless end of pipe
      ez_close(out_pipe[0]);

      // Attach  standard outputs to our pipe
      ez_dup2(out_pipe[1], STDOUT_FILENO);
      ez_dup2(out_pipe[1], STDERR_FILENO);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
      // Execute command
      ez_execve(argv[0], argv, environ);
      // We will never get to here
#pragma GCC diagnostic pop
   }

   // Close useless end of pipe
   ez_close(out_pipe[1]);

#define BUF_SZ 1024
   // Read buffer
   static char buf[BUF_SZ];

   // Loop reading data from child's output
   ssize_t nRead;
   while(0 < (nRead= read(out_pipe[0], buf, BUF_SZ-1)))
      // Relay to our stderr
      ez_write(STDERR_FILENO, buf, nRead);

#undef BUF_SZ

   if(-1 == nRead)
         sys_eprintf("ERROR: read()");

   /* Wait indefinitely for child to finish */
   int wstatus;
   pid_t rc= waitpid(child_pid, &wstatus, 0);

   // Proper exit
   if(WIFEXITED(wstatus))
      return WEXITSTATUS(wstatus);

   // Killed with signal
   if(WIFSIGNALED(wstatus)) {
      eprintf("ERROR: %s killed by signal: %s", argv[0], strsignal(WTERMSIG(wstatus)));
      return -1;
   }

   // Shouldn't ever get here
   assert(0);
}

static int
_control_addresses(const char *cmdFlag, PTRVEC *h_vec)
/**************************************************************
 * (Un)block addresses.
 */
{
   if(!S.is_init)
      initialize();

   int rtn= -1;

   // Need a large string buffer for comma separated address list
   static STR addr_sb;
   STR_sinit(&addr_sb, N_ADDRESSES_HINT*20);

   // argv always same length, with NULL at the end
   static const char *argv[8];

   /**************************************************************************/
   /**************** ip4 addresses *******************************************/
   /**************************************************************************/
   /* Name of executable file */
   argv[0]= IPTABLES;

   /* iptables command string begins like this */
   argv[1]= cmdFlag;
   argv[2]= "INPUT";
   argv[3]= "-s";
   /* argv[4] supplied below */
   argv[5]= "-j";
   argv[6]= "DROP";

   const char *addr= NULL;

   /* Move any ipv6 addresses to the end */
   PTRVEC_sort(h_vec, addrCmp_pvsort);
#ifdef DEBUG
{
   const char *addr;
   unsigned i;
   PTRVEC_loopFwd(h_vec, i, addr) {
      eprintf("%s", addr);
   }
}
#endif

   { /* Place comma separated address list into single string buffer */
      const char *colon=NULL;
      unsigned naddr= 0;

      do {

         addr= PTRVEC_remHead(h_vec);
         if(addr)
            colon= strchr(addr, ':');

         /* We have an ipv4 address */
         if(addr && !colon) {

            /* Need comma after 1st address */
            if(naddr)
               STR_append(&addr_sb, ",", 1);

            /* Put address in buffer */
            STR_append(&addr_sb, addr, -1);

            /* Note we will use this address */
            ++naddr;
         }

         /* Keep adding addresses until we bump up against iptables maximum,
          * or run out of ipv4 addresses
          */
         if(!naddr || (naddr < IPTABLES_MAX_ADDR && (addr && !colon))) 
            continue;

         // Place string buffer in argv
         argv[4]= STR_str(&addr_sb);
         if(run_command(argv)) {
            eprintf("ERROR: run_command() failed.");
            goto abort;
         }
         /* Reset for next command */
         naddr= 0;
         STR_reset(&addr_sb);

      } while(addr && !colon);
   }

   /**************************************************************************/
   /**************** ip6 addresses *******************************************/
   /**************************************************************************/

   { // ipv6 addresses
      argv[0]= IP6TABLES;
      // Prepare to load up ipv6 addresses in string buffer
      STR_reset(&addr_sb);

      unsigned naddr= 0;

      for(;;) { /* Work through ipv6 addresses in the vector */

         if(addr) {
            /* Need comma after 1st address */
            if(naddr)
               STR_append(&addr_sb, ",", 1);

            /* Put address in place */
            STR_append(&addr_sb, addr, -1);

            /* Note that we will use this address */
            ++naddr;
         }
         
         /* See if there is another address */
         addr= PTRVEC_remHead(h_vec);

         /* Break out if nothing remains */
         if(!addr && !naddr)
            break;

         /* Keep adding addresses until we bump up against iptables maximum,
          * or run out of addresses
          */
         if(!naddr || (naddr < IPTABLES_MAX_ADDR && addr)) 
            continue;

         // Place string buffer in argv
         argv[4]= STR_str(&addr_sb);
         if(run_command(argv)) {
            eprintf("ERROR: run_command() failed.");
            goto abort;
         }

         /* Bail out now if we are done with the list */
         if(!addr)
            break;

         /* Reset for next command */
         naddr= 0;
         STR_reset(&addr_sb);
      }
   }

   rtn= 0;
abort:
   return rtn;
}

int
IPTABLES_block_addresses(PTRVEC *h_vec)
/**************************************************************
 * Block addresses.
 */
{
   if(!S.is_init)
      initialize();

   return _control_addresses("-A", h_vec);

}

int
IPTABLES_unblock_addresses(PTRVEC *h_vec)
/**************************************************************
 * Unblock addresses.
 */
{
   if(!S.is_init)
      initialize();

   return _control_addresses("-D", h_vec);

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
