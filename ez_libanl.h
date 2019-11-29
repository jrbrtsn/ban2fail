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
#ifndef EZ_LIBANL_H
#define EZ_LIBANL_H

/* Simplified interface to libanl functions */
#define _GNU_SOURCE
#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ez_getaddrinfo_a(mode, list, nItems, sevp) \
         _ez_getaddrinfo_a(__FILE__, __LINE__, __FUNCTION__, mode, list, nItems, sevp)
int _ez_getaddrinfo_a(
   const char *fileName,
   int lineNo,
   const char *funcName,
      int mode,
      struct gaicb *list[],
      int nitems,
      struct sigevent *sevp
      );

#define ez_gai_suspend(list, nItems, timeout) \
         _ez_gai_suspend(__FILE__, __LINE__, __FUNCTION__, list, nItems, timeout)
int _ez_gai_suspend(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const struct gaicb * const list[],
      int nitems,
      const struct timespec *timeout
      );


#ifdef __cplusplus
}
#endif


#endif
