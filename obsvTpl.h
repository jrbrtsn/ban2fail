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
#ifndef OBSVTPL_H
#define OBSVTPL_H

#define _GNU_SOURCE
#include <db.h>
#include <stdio.h>
#include <zlib.h>

#include "addrRpt.h"
#include "dynstack.h"
#include "logFile.h"

typedef struct _ObsvTpl {
   char addr[43];
   LOGFILE *lf;
   DS stack;
} ObsvTpl;


#ifdef __cplusplus
extern "C" {
#endif

#define ObsvTpl_create(p, lf, addr) \
  ((p)=(ObsvTpl_constructor((p)=malloc(sizeof(ObsvTpl)), lf, addr) ? (p) : ( p ? realloc(ObsvTpl_destructor(p),0) : 0 )))
ObsvTpl*
ObsvTpl_constructor(ObsvTpl *self, LOGFILE *lf, const char *addr);
/********************************************************
 * Prepare for use from an address on the command line
 */

int
ObsvTpl_sinit(ObsvTpl *self, LOGFILE *lf, const char *addr);
/********************************************************
 * Initialize or reset a static instance
 */

#define ObsvTpl_destroy(s) \
  {if(ObsvTpl_destructor(s)) {free(s); (s)=0;}}
void*
ObsvTpl_destructor(ObsvTpl *self);
/********************************************************
 * Free resources.
 */

int
ObsvTpl_addObsv(ObsvTpl *self, z_off_t pos, unsigned len);
/*****************************************************************
 * Add an observation to this object.
 */

int
ObsvTpl_db_get(ObsvTpl *self, DB *db);
/********************************************************
 * Fetch our content from a database.
 */

int
ObsvTpl_db_put(ObsvTpl *self, DB *db);
/*****************************************************************
 * Write our contents into the database.
 */

int
ObsvTpl_put_AddrRPT(ObsvTpl *self, gzFile fh, AddrRPT *ar);
/********************************************************
 *  Place contents of self into ar.
 */

int
ObsvTpl_print(ObsvTpl *self, FILE *fh);
/********************************************************
 * Print a human readable representation of *self.
 */


#ifdef __cplusplus
}
#endif

#endif

