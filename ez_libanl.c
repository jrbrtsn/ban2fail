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
#include "ez_libanl.h"

/***************************************************/
int _ez_getaddrinfo_a(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int mode,
      struct gaicb *list[],
      int nitems,
      struct sigevent *sevp
      )
{
   errno= 0;
   int rtn= getaddrinfo_a (mode, list, nitems, sevp);
   switch(rtn) {
      case 0:
      case EAI_AGAIN:
         return rtn;
   }

   /* _sys_eprintf() will pass errno to gai_sterror */
   errno= rtn;
   _sys_eprintf(gai_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "getaddrinfo_a() failed");
   abort();
}

/***************************************************/
int _ez_gai_suspend(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const struct gaicb * const list[],
      int nitems,
      const struct timespec *timeout
      )
{
   errno= 0;
   int rtn= gai_suspend (list, nitems, timeout);
   switch(rtn) {
      case 0:
      case EAI_AGAIN:
      case EAI_ALLDONE:
      case EAI_INTR:
         return rtn;
   }

   /* _sys_eprintf() will pass errno to gai_sterror */
   errno= rtn;
   _sys_eprintf(gai_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "gai_suspend() failed");
   abort();
}
