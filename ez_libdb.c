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
#include <stdlib.h>

#include "util.h"
#include "ez_libdb.h"

/***************************************************/
int _ez_db_create(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB **dbp,
      DB_ENV *dbenv,
      u_int32_t flags
     )
{
   int rtn= db_create (dbp, dbenv, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "db_create() failed");
   abort();
}

/***************************************************/
int _ez_db_open(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB *db,
      DB_TXN *txnid,
      const char *file,
      const char *database,
      DBTYPE type,
      u_int32_t flags,
      int mode
      )
{
//eprintf("Opening file (%p) \"%s\"", db, file);
   int rtn= db->open(db, txnid, file, database, type, flags, mode);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "DB->open(\"%s\") failed", database);
   abort();
}

/***************************************************/
int _ez_db_put(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags
      )
{
   int rtn= db->put(db, txnid, key, data, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "DB->put() failed");
   abort();
}

/***************************************************/
int _ez_db_get(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags
      )
{
   int rtn= db->get(db, txnid, key, data, flags);

   switch(rtn) {
      case 0:
      case DB_NOTFOUND:
         return rtn;
   }

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "DB->get() failed");
   abort();
}

/***************************************************/
int _ez_db_del(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      u_int32_t flags
      )
{
   int rtn= db->del(db, txnid, key, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "DB->del() failed");
   abort();
}

/***************************************************/
int _ez_db_close(
   const char *fileName,
   int lineNo,
   const char *funcName,
      DB *db,
      u_int32_t flags
      )
{
//eprintf("Closing database (%p)", db);
   int rtn= db->close(db, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror, fileName, lineNo, funcName, "DB->close() failed");
   abort();
}
