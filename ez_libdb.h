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
#ifndef EZ_LIBDB_H
#define EZ_LIBDB_H

/* Simplified interface to libanl functions */
#define _GNU_SOURCE
#include <db.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#       define ez_db_create(dbp, dbenv, flags) \
         _ez_db_create(__FILE__, __LINE__, __FUNCTION__, dbp, dbenv, flags)
#else
#       define ez_db_create(dbp, dbenv, flags) \
         _ez_db_create(dbp, dbenv, flags)
#endif
int _ez_db_create(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB **dbp,
      DB_ENV *dbenv,
      u_int32_t flags
     );

#ifdef DEBUG
#       define ez_db_open(db, txnid, file, database, type, flags, mode ) \
         _ez_db_open(__FILE__, __LINE__, __FUNCTION__, db, txnid, file, database, type, flags, mode)
#else
#       define ez_db_open(db, txnid, file, database, type, flags, mode ) \
         _ez_db_open(db, txnid, file, database, type, flags, mode)
#endif
int _ez_db_open(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      DB_TXN *txnid,
      const char *file,
      const char *database,
      DBTYPE type,
      u_int32_t flags,
      int mode
      );

#ifdef DEBUG
#       define ez_db_put(db, txnid, key, data, flags) \
         _ez_db_put(__FILE__, __LINE__, __FUNCTION__, db, txnid, key, data, flags)
#else
#       define ez_db_put(db, txnid, key, data, flags) \
         _ez_db_put(db, txnid, key, data, flags)
#endif
int _ez_db_put(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags
      );

#ifdef DEBUG
#       define ez_db_get(db, txnid, key, data, flags) \
         _ez_db_get(__FILE__, __LINE__, __FUNCTION__, db, txnid, key, data, flags)
#else
#       define ez_db_get(db, txnid, key, data, flags) \
         _ez_db_get(db, txnid, key, data, flags)
#endif
int _ez_db_get(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags
      );

#ifdef DEBUG
#       define ez_db_del(db, txnid, key, flags) \
         _ez_db_del(__FILE__, __LINE__, __FUNCTION__, db, txnid, key, flags)
#else
#       define ez_db_del(db, txnid, key, flags) \
         _ez_db_del(db, txnid, key, flags)
#endif
int _ez_db_del(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      u_int32_t flags
      );

#ifdef DEBUG
#       define ez_db_close(db, flags) \
         _ez_db_close(__FILE__, __LINE__, __FUNCTION__, db, flags)
#else
#       define ez_db_close(db, flags) \
         _ez_db_close(db, flags)
#endif
int _ez_db_close(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      u_int32_t flags
      );

#ifdef DEBUG
#       define ez_db_fd(db, fdp) \
         _ez_db_fd(__FILE__, __LINE__, __FUNCTION__, db, fdp)
#else
#       define ez_db_fd(db, fdp) \
         _ez_db_fd(db, fdp)
#endif
int _ez_db_fd(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DB *db,
      int *fdp
      );

#ifdef __cplusplus
}
#endif


#endif
