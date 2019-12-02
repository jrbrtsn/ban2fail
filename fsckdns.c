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
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>

#include "ez_libc.h"
#include "util.h"
const static struct addrinfo rev_hints= {
   .ai_flags = AI_NUMERICHOST, /* doing reverse lookups */
   .ai_family = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
   .ai_socktype= SOCK_DGRAM,
   .ai_protocol= IPPROTO_UDP
};

const static struct addrinfo fwd_hints= {
   .ai_family= AF_UNSPEC,    /* Allow IPv4 or IPv6 */
   .ai_socktype= SOCK_DGRAM,
   .ai_protocol= IPPROTO_UDP
};

int
main(int argc, char **argv)
{
   int rtn= EXIT_FAILURE;
   const char *addr= NULL;
   static char hostBuf[PATH_MAX];

   extern char *optarg;
   extern int optind, optopt;

   for(optind= 1; optind < argc; ++optind)
   {
      addr= argv[optind];

      /*============ Reverse DNS lookup ================*/
      /* Get a populated addrinfo object */
      struct addrinfo *res= NULL;
      int rc= ez_getaddrinfo(addr, NULL, &rev_hints, &res);
      assert(0 == rc);
      assert(res && res->ai_addr && res->ai_addrlen);
      addrinfo_print(res, stdout);
      ez_fflush(stdout);

      /* Now do blocking reverse lookup */
      rc= ez_getnameinfo(res->ai_addr, res->ai_addrlen, hostBuf, sizeof(hostBuf)-1, NULL, 0, NI_NAMEREQD);
      if(rc) {
         ez_fprintf(stdout, "%s\n", gai_strerror(rc));
         continue;
      }

      ez_fprintf(stdout, "RevDNS= \"%s\"\n", hostBuf);
      ez_fflush(stdout);

      /*============ Forward DNS lookup ================*/
      rc= ez_getaddrinfo(hostBuf, NULL, &fwd_hints, &res);
      const char *msg= NULL;

      if(rc) {
         ez_fprintf(stdout, "%s\n", gai_strerror(rc));
         continue;
      }

      addrinfo_print(res, stdout);
      ez_fflush(stdout);

   }

   rtn= EXIT_SUCCESS;
abort:
   return rtn;
}
