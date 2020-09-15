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
#include <errno.h>
#include <stdlib.h>

#include "ez_libpthread.h"
#include "util.h"

/***************************************************/
ez_proto (int, pthread_mutex_lock, pthread_mutex_t *mutex)
{
   int rtn= pthread_mutex_lock (mutex);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_mutex_lock() failed");
   abort();
}

/***************************************************/
ez_proto (int, pthread_mutex_unlock, pthread_mutex_t *mutex)
{
   int rtn= pthread_mutex_unlock (mutex);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_mutex_unlock() failed");
   abort();
}

/***************************************************/
ez_proto (int, pthread_create,
      pthread_t *thread,
      const pthread_attr_t *attr,
      void *(*start_routine) (void *),
      void *arg)
{
   int rtn= pthread_create (thread, attr, start_routine, arg);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_mutex_create() failed");
   abort();
}

/***************************************************/
ez_proto (int, pthread_cond_signal, pthread_cond_t *cond)
{
   int rtn= pthread_cond_signal (cond);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_cond_signal() failed");
   abort();
}

/***************************************************/
ez_proto (int, pthread_cond_wait,
      pthread_cond_t *cond,
      pthread_mutex_t *mutex)
{
   int rtn= pthread_cond_wait (cond, mutex);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_cond_wait() failed");
   abort();
}

/***************************************************/
ez_proto (int, pthread_join,
      pthread_t thread,
      void **retval)
{
   int rtn= pthread_join (thread, retval);
   if(0 == rtn) return 0;

   errno= rtn;
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
      , fileName, lineNo, funcName
#endif
            , "pthread_join() failed");
   abort();
}

