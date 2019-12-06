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
#ifndef EZ_LIBPTHREAD_H
#define EZ_LIBPTHREAD_H

/* Simplified interface to libpthread functions */
#define _GNU_SOURCE
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#       define ez_pthread_mutex_lock(mutex) \
         _ez_pthread_mutex_lock(__FILE__, __LINE__, __FUNCTION__, mutex)
#else
#       define ez_pthread_mutex_lock(mutex) \
         _ez_pthread_mutex_lock(mutex)
#endif
int _ez_pthread_mutex_lock(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_mutex_t *mutex
      );

#ifdef DEBUG
#       define ez_pthread_mutex_unlock(mutex) \
         _ez_pthread_mutex_unlock(__FILE__, __LINE__, __FUNCTION__, mutex)
#else
#       define ez_pthread_mutex_unlock(mutex) \
         _ez_pthread_mutex_unlock(mutex)
#endif
int _ez_pthread_mutex_unlock(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_mutex_t *mutex
      );

#ifdef DEBUG
#       define ez_pthread_create(thread, addr, start_routine, arg) \
         _ez_pthread_create(__FILE__, __LINE__, __FUNCTION__, thread, addr, start_routine, arg)
#else
#       define ez_pthread_create(thread, addr, start_routine, arg) \
         _ez_pthread_create(thread, addr, start_routine, arg)
#endif
int _ez_pthread_create(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_t *thread,
      const pthread_attr_t *attr,
      void *(*start_routine) (void *),
      void *arg
      );

#ifdef DEBUG
#       define ez_pthread_cond_signal(cond) \
         _ez_pthread_cond_signal(__FILE__, __LINE__, __FUNCTION__, cond)
#else
#       define ez_pthread_cond_signal(cond) \
         _ez_pthread_cond_signal(cond)
#endif
int _ez_pthread_cond_signal(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_cond_t *cond
      );

#ifdef DEBUG
#       define ez_pthread_cond_wait(cond, mutex) \
         _ez_pthread_cond_wait(__FILE__, __LINE__, __FUNCTION__, cond, mutex)
#else
#       define ez_pthread_cond_wait(cond, mutex) \
         _ez_pthread_cond_wait(cond, mutex)
#endif
int _ez_pthread_cond_wait(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_cond_t *cond,
      pthread_mutex_t *mutex
      );

#ifdef DEBUG
#       define ez_pthread_join(thread, retval) \
         _ez_pthread_join(__FILE__, __LINE__, __FUNCTION__, thread, retval)
#else
#       define ez_pthread_join(thread, retval) \
         _ez_pthread_join(thread, retval)
#endif
int _ez_pthread_join(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      pthread_t thread,
      void **retval
      );

#ifdef __cplusplus
}
#endif


#endif
