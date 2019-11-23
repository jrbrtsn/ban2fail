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
#include <unistd.h>

#include "util.h"
#include "ez_stdlib.h"

/***************************************************/
int _ez_close (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd
      )
{
   int rtn= close (fd);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "close(%d) failed", fd);
      abort();
   }
   return rtn;
}

/***************************************************/
ssize_t _ez_write (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd,
      const void *buf,
      size_t count
      )
{
   ssize_t rtn= write (fd, buf, count);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "write(fd= %d) failed", fd);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_stat (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname,
      struct stat *statbuf
      )
{
   int rtn= stat (pathname, statbuf);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "stat(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_mkdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname,
      mode_t mode
      )
{
   int rtn= mkdir (pathname, mode);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "mkdir(\"%s\", %04x) failed", pathname, (unsigned)mode);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_rmdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      )
{
   int rtn= rmdir (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "rmdir(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_unlink (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      )
{
   int rtn= unlink (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "unlink(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}
