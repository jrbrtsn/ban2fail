
/***************************************************************************
 *   Copyright (C) 2018 by John D. Robertson                               *
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
/***************************************************************************
                          ez_stdlib.h  -  description                              
stdlib calls with boilerplate error handling.

                             -------------------
    begin                : Tue Nov 13 19:42:23 EST 2018
    email                : john@rrci.com                                     
 ***************************************************************************/
#ifndef EZ_STDLIB_H
#define EZ_STDLIB_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ez_close(fd) \
         _ez_close(__FILE__, __LINE__, __FUNCTION__, fd)
int _ez_close (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd
      );

#if 0
// FIXME: this needs to be implemented and tested
#define ez_read(fd, buf, count) \
         _ez_read(__FILE__, __LINE__, __FUNCTION__, fd, buf, count)
ssize_t _ez_read (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd,
      void *buf,
      size_t count
      );
#endif

#define ez_write(fd, buf, count) \
         _ez_write(__FILE__, __LINE__, __FUNCTION__, fd, buf, count)
ssize_t _ez_write (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd,
      const void *buf,
      size_t count
      );

#define ez_stat(pathname, statbuf) \
         _ez_stat(__FILE__, __LINE__, __FUNCTION__, pathname, statbuf)
int _ez_stat (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname,
      struct stat *statbuf
      );

// FIXME: xxxdir() function should be in ez_unistd.h
#define ez_mkdir(pathname, mode) \
         _ez_mkdir(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
int _ez_mkdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname,
      mode_t mode
      );

#define ez_rmdir(pathname) \
         _ez_rmdir(__FILE__, __LINE__, __FUNCTION__, pathname)
int _ez_rmdir (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      );

#define ez_unlink(pathname) \
         _ez_unlink(__FILE__, __LINE__, __FUNCTION__, pathname)
int _ez_unlink (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      );


#ifdef __cplusplus
}
#endif

#endif
