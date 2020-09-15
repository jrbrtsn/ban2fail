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
ez_proto (int, create,
      DB **dbp,
      DB_ENV *dbenv,
      u_int32_t flags)
{
   int rtn= db_create (dbp, dbenv, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "db_create() failed");
   abort();
}

/***************************************************/
ez_proto (int, db_open,
      DB *db,
      DB_TXN *txnid,
      const char *file,
      const char *database,
      DBTYPE type,
      u_int32_t flags,
      int mode)
{
   int rtn= db->open(db, txnid, file, database, type, flags, mode);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->open(\"%s\") failed", database);
   abort();
}

/***************************************************/
ez_proto (int, db_put,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags)
{
   int rtn= db->put(db, txnid, key, data, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->put() failed");
   abort();
}

/***************************************************/
ez_proto (int, db_get,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags)
{
   int rtn= db->get(db, txnid, key, data, flags);

   switch(rtn) {
      case 0:
      case DB_NOTFOUND:
         return rtn;
   }

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->get() failed");
   abort();
}

/***************************************************/
ez_proto (int, db_del,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      u_int32_t flags)
{
   int rtn= db->del(db, txnid, key, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->del() failed");
   abort();
}

/***************************************************/
ez_proto (int, db_close, DB *db, u_int32_t flags)
{
   int rtn= db->close(db, flags);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->close() failed");
   abort();
}

/***************************************************/
ez_proto (int, db_fd, DB *db, int *fdp)
{
   int rtn= db->fd(db, fdp);

   if(!rtn) return 0;

   /* _sys_eprintf() will pass errno to db_sterror */
   errno= rtn;
   _sys_eprintf((const char*(*)(int))db_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "DB->fd() failed");
   abort();
}
