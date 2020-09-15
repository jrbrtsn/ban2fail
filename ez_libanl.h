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
#ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#endif
#include <netdb.h>
#include "ez.h"

#ifdef __cplusplus
extern "C" {
#endif

ez_proto(int, getaddrinfo_a,
      int mode,
      struct gaicb *list[],
      int nitems,
      struct sigevent *sevp);
#ifdef DEBUG
#       define ez_getaddrinfo_a(...) \
         _ez_getaddrinfo_a(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_getaddrinfo_a(...) \
         _ez_getaddrinfo_a(__VA_ARGS__)
#endif

ez_proto (int, gai_suspend,
      const struct gaicb * const list[],
      int nitems,
      const struct timespec *timeout);
#ifdef DEBUG
#       define ez_gai_suspend(...) \
         _ez_gai_suspend(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_gai_suspend(...) \
         _ez_gai_suspend(__VA_ARGS__)
#endif


#ifdef __cplusplus
}
#endif


#endif
