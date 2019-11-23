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
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "ez_dirent.h"

/***************************************************/
DIR* _ez_opendir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *name
      )
{
   DIR *rtn= opendir (name);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "opendir(\"%s\") failed", name);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_closedir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      DIR *dirp
      )
{
   int rtn= closedir (dirp);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "closedir() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
struct dirent* _ez_readdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      DIR *dirp
      )
{

   /* Pass on doctored format string and varargs to vfprintf() */
   errno= 0;
   struct dirent* rtn= readdir(dirp);

   if (!rtn && errno) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "readdir() failed");
      abort();
   }

   return rtn;
}

