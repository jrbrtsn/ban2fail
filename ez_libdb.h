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
#ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#endif
#include <db.h>

#include "ez.h"

#ifdef __cplusplus
extern "C" {
#endif

ez_proto (int, db_create,
      DB **dbp,
      DB_ENV *dbenv,
      u_int32_t flags);
#ifdef DEBUG
#       define ez_db_create(...) \
         _ez_db_create(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_create(...) \
         _ez_db_create(__VA_ARGS__)
#endif

ez_proto (int, db_open,
      DB *db,
      DB_TXN *txnid,
      const char *file,
      const char *database,
      DBTYPE type,
      u_int32_t flags,
      int mode);
#ifdef DEBUG
#       define ez_db_open(...) \
         _ez_db_open(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_open(...) \
         _ez_db_open(__VA_ARGS__)
#endif

ez_proto (int, db_put,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags);
#ifdef DEBUG
#       define ez_db_put(...) \
         _ez_db_put(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_put(...) \
         _ez_db_put(__VA_ARGS__)
#endif

ez_proto (int, db_get,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      DBT *data,
      u_int32_t flags);
#ifdef DEBUG
#       define ez_db_get(...) \
         _ez_db_get(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_get(...) \
         _ez_db_get(__VA_ARGS__)
#endif

ez_proto (int, db_del,
      DB *db,
      DB_TXN *txnid,
      DBT *key,
      u_int32_t flags);
#ifdef DEBUG
#       define ez_db_del(...) \
         _ez_db_del(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_del(...) \
         _ez_db_del(__VA_ARGS__)
#endif

ez_proto (int, db_close,
      DB *db,
      u_int32_t flags);
#ifdef DEBUG
#       define ez_db_close(...) \
         _ez_db_close(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_close(...) \
         _ez_db_close(__VA_ARGS__)
#endif

ez_proto (int, db_fd,
      DB *db,
      int *fdp);
#ifdef DEBUG
#       define ez_db_fd(...) \
         _ez_db_fd(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_db_fd(...) \
         _ez_db_fd(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif
