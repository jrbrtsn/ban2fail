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

#include "ez_libdb.h"
#include "ez_libc.h"
#include "ez_libz.h"
#include "obsvTpl.h"

/* Make sure we don't have padding at the end of this struct */
#pragma pack(push,2)
struct obsv {
   z_off_t pos; /* position in the file */
   uint16_t len; /* length of string to retrieve */
};
#pragma pack(pop)

ObsvTpl*
ObsvTpl_constructor(ObsvTpl *self, LOGFILE *lf, const char *addr)
/*****************************************************************
 * prepare for use.
 */
{
   memset(self, 0, sizeof(*self));

   strncpy(self->addr, addr, sizeof(self->addr)-1);
   self->lf= lf;
   DS_constructor(&self->stack, sizeof(struct obsv), 10000);
   return self;
}

int
ObsvTpl_sinit(ObsvTpl *self, LOGFILE *lf, const char *addr)
/********************************************************
 * Initialize or reset a static instance
 */
{
   if(self->lf) {
      memset(self->addr, 0, sizeof(self->addr));
      strncpy(self->addr, addr, sizeof(self->addr)-1);
      self->lf= lf;
      DS_reset(&self->stack);
   } else {
      ObsvTpl_constructor(self, lf, addr);
   }

   return 0;
}

void*
ObsvTpl_destructor(ObsvTpl *self)
/*****************************************************************
 * Free resources
 */
{

   DS_destructor(&self->stack);

   return self;
}

int
ObsvTpl_addObsv(ObsvTpl *self, z_off_t pos, unsigned len)
/*****************************************************************
 * Add an observation to this report object.
 */
{
   struct obsv obs;
   obs.pos= pos;
   obs.len= len;
   DS_push(&self->stack, &obs);
   return 0;
}

int
ObsvTpl_db_put(ObsvTpl *self, DB *db)
/*****************************************************************
 * Write our contents into the database.
 */
{
   int rtn= -1;

   DBT key= {
           .data= self->addr,
           .size= strlen(self->addr)
       },
       data= {
          .data= DS_ptr(&self->stack),
          .size= DS_numItems(&self->stack) * sizeof(struct obsv)
       };
   
   /* Write data to database */
   ez_db_put(db, NULL, &key, &data, 0);

   rtn= 0;
abort:
   return rtn;
}

int
ObsvTpl_db_get(ObsvTpl *self, DB *db)
/********************************************************
 * Fetch our content from a database.
 */
{
   int rtn= -1;

   DBT key= {
           .data= self->addr,
           .size= strlen(self->addr)
       },
       data;

   memset(&data, 0, sizeof(data));
   
   /* Read from the database */
   int rc= ez_db_get(db, NULL, &key, &data, 0);
   if(DB_NOTFOUND == rc) return rc;

   /* Now put all obsv's in place */
   DS_reset(&self->stack);

   /* Compute number of items we fetched */
   unsigned nItems= data.size / sizeof(struct obsv);

   /* Load them into the stack */
   for(unsigned i= 0; i < nItems; ++i) {
      struct obsv *src= (struct obsv*)data.data + i;
      DS_push(&self->stack, src);
   }

   rtn= 0;
abort:
   return rtn;
}

static int obsv_print(struct obsv *self, FILE *fh)
/********************************************************
 * Print observation to fh
 */
{
   ez_fprintf(fh, "\tpos= %lu, len= %hu\n", (long unsigned)self->pos, self->len);
   return 0;
}


int
ObsvTpl_print(ObsvTpl *self, FILE *fh)
/********************************************************
 * Print a human readable representation of *self.
 */
{
   ez_fprintf(fh, "ObsvTpl %s {\n", self->addr);

   DS_visitAllEntries(&self->stack, (int(*)(void*,void*))obsv_print, fh);
   ez_fprintf(fh, "}\n");
   return 0;
}

struct infoTuple {
   gzFile fh;
   AddrRPT *ar;
   LOGFILE *lf;
};

static int
obsv_load_AddrRPT(struct obsv *self, struct infoTuple *it)
/********************************************************
 * load specific observation into it->ar.
 */
{
   static char lbuf[1024];
   assert(self->len < sizeof(lbuf));
   unsigned len= MIN(self->len, sizeof(lbuf)-1);

   ez_gzseek(it->fh, self->pos, SEEK_SET);
   ez_gzread(it->fh, lbuf, len);
   AddrRPT_addLine(it->ar, it->lf, lbuf);
   return 0;
}

int
ObsvTpl_put_AddrRPT(ObsvTpl *self, gzFile fh, AddrRPT *ar)
/********************************************************
 *  Place contents of self into ar.
 */
{
   struct infoTuple it= {.fh= fh, .ar= ar, .lf= self->lf};
   DS_visitAllEntries(&self->stack,  (int(*)(void*,void*))obsv_load_AddrRPT, &it); 
   return 0;
}

