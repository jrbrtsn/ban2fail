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
#ifndef AddrRPT_H
#define AddrRPT_H

#define _GNU_SOURCE
#include <stdio.h>

#include "logFile.h"
#include "ptrvec.h"

/* One of these for each offense found in a log file */
typedef struct _AddrRPT {

   unsigned serial;

   char addr[46];

   /* Store match records here */
   PTRVEC vec;
   
} AddrRPT;

#ifdef __cplusplus
extern "C" {
#endif

#define AddrRPT_addr_create(p, addr) \
  ((p)=(AddrRPT_addr_constructor((p)=malloc(sizeof(AddrRPT)), addr) ? (p) : ( p ? realloc(AddrRPT_destructor(p),0) : 0 )))
AddrRPT*
AddrRPT_addr_constructor(AddrRPT *self, const char *addr);
/********************************************************
 * Prepare for use from an address on the command line
 */

#define AddrRPT_destroy(s) \
  {if(AddrRPT_destructor(s)) {free(s); (s)=0;}}
void*
AddrRPT_destructor(AddrRPT *self);
/********************************************************
 * Free resources.
 */

int
AddrRPT_addLine(AddrRPT *self, LOGFILE *lf, const char *line);
/********************************************************
 * Add a matching line to this object.
 */

int
AddrRPT_print(AddrRPT *self, FILE *fh);
/********************************************************
 * Print a human readable representation of *self.
 */


#ifdef __cplusplus
}
#endif

#endif

