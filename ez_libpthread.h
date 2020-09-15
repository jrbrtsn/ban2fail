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
#ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#endif
#include <pthread.h>

#include "ez.h"

#ifdef __cplusplus
extern "C" {
#endif

ez_proto (int, pthread_mutex_lock,
      pthread_mutex_t *mutex);
#ifdef DEBUG
#       define ez_pthread_mutex_lock(...) \
         _ez_pthread_mutex_lock(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_mutex_lock(...) \
         _ez_pthread_mutex_lock(__VA_ARGS__)
#endif

ez_proto (int, pthread_mutex_unlock,
      pthread_mutex_t *mutex);
#ifdef DEBUG
#       define ez_pthread_mutex_unlock(...) \
         _ez_pthread_mutex_unlock(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_mutex_unlock(...) \
         _ez_pthread_mutex_unlock(__VA_ARGS__)
#endif

ez_proto (int, pthread_create,
      pthread_t *thread,
      const pthread_attr_t *attr,
      void *(*start_routine) (void *),
      void *arg);
#ifdef DEBUG
#       define ez_pthread_create(...) \
         _ez_pthread_create(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_create(...) \
         _ez_pthread_create(__VA_ARGS__)
#endif

ez_proto (int, pthread_cond_signal,
      pthread_cond_t *cond);
#ifdef DEBUG
#       define ez_pthread_cond_signal(...) \
         _ez_pthread_cond_signal(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_cond_signal(...) \
         _ez_pthread_cond_signal(__VA_ARGS__)
#endif

ez_proto (int, pthread_cond_wait,
      pthread_cond_t *cond,
      pthread_mutex_t *mutex);
#ifdef DEBUG
#       define ez_pthread_cond_wait(...) \
         _ez_pthread_cond_wait(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_cond_wait(...) \
         _ez_pthread_cond_wait(__VA_ARGS__)
#endif

ez_proto (int, pthread_join,
      pthread_t thread,
      void **retval);
#ifdef DEBUG
#       define ez_pthread_join(...) \
         _ez_pthread_join(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pthread_join(...) \
         _ez_pthread_join(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif


#endif
