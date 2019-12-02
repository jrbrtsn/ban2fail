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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "addrRpt.h"
#include "ban2fail.h"
#include "ez_libc.h"
#include "logFile.h"
#include "logType.h"
#include "util.h"

/*==================================================================*/
/*================= Match ==========================================*/
/*==================================================================*/
/* One of these object for each LOGFILE
 * which contains matches.
 */
typedef struct _Match {
   LOGFILE *lf;
   char *line;
} Match;

#define Match_create(p, logfile, line) \
  ((p)=(Match_constructor((p)=malloc(sizeof(Match)), logfile, line) ? (p) : ( p ? realloc(Match_destructor(p),0) : 0 )))
static Match*
Match_constructor(Match *self, LOGFILE *lf, const char *line)
/********************************************************
 * Prepare for use.
 */
{
   memset(self, 0, sizeof(*self));
   self->lf= lf;
   self->line= strdup(line);
   return self;
}

#define Match_destroy(s) \
  {if(Match_destructor(s)) {free(s); (s)=0;}}
static void*
Match_destructor(Match *self)
/********************************************************
 * Free resources.
 */
{
   if(self->line)
      free(self->line);
   return self;
}

/*==================================================================*/
/*================= AddrRPT ========================================*/
/*==================================================================*/
AddrRPT*
AddrRPT_addr_constructor(AddrRPT *self, const char *addr)
/********************************************************
 * Prepare for use from an address on the command line
 */
{
   memset(self, 0, sizeof(*self));

   strncpy(self->addr, addr, sizeof(self->addr)-1);
   PTRVEC_constructor(&self->vec, 100);

   /* Stamp with serial number so reporting at the end can be
    * in the order addresses were supplied on the command
    * line.
    */
   static unsigned count;
   self->serial= ++count;

   return self;
}

void*
AddrRPT_destructor(AddrRPT *self)
/********************************************************
 * Free resources.
 */
{
   Match *m;
   while((m= PTRVEC_remHead(&self->vec)))
      Match_destroy(m);

   PTRVEC_destroy(&self->vec);
   return self;
}

int
AddrRPT_addLine(AddrRPT *self, LOGFILE *lf, const char *line)
/********************************************************
 * Add a matching line to this object.
 */
{
   Match *m;
   Match_create(m, lf, line);
   assert(m);
   PTRVEC_addTail(&self->vec, m);
   return 0;
}

int
AddrRPT_print(AddrRPT *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   Match *m;
   unsigned i;
   LOGFILE *lf= NULL;
   ez_fprintf(fh, "============== Report for %s\r\n", self->addr);

   PTRVEC_loopFwd(&self->vec, i, m) {

      if(lf != m->lf) {
         ez_fprintf(fh, "%s\r\n", LOGFILE_logFilePath(m->lf));
         lf= m->lf;
      }
      ez_fprintf(fh, "%s\r\n", m->line);
   }

   return 0;
}

