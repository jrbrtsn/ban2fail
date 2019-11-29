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
#include "ez_libc.h"

/***************************************************/
int _ez_fputs (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *s,
      FILE *stream
      )
{
   int rtn= fputs (s, stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fputs() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fputc (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int c,
      FILE *stream
      )
{
   int rtn= fputc (c, stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fputc() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fprintf (
      const char *fileName,
      int lineNo,
      const char *funcName,
      FILE *stream,
      const char *fmt,
      ...
      )
{
   va_list args;

   /* Pass on doctored format string and varargs to vfprintf() */
   va_start(args, fmt);
   int rtn= vfprintf(stream, fmt, args);
   va_end(args);

   if (0 > rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "vfprintf() failed");
      abort();
   }

   return rtn;
}

/***************************************************/
FILE* _ez_popen (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *command,
      const char *type
      )
{
   FILE *rtn= popen (command, type);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "popen(\"%s\", \"%s\") failed", command, type);
      abort();
   }
   return rtn;
}

/***************************************************/
FILE* _ez_fopen (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname,
      const char *mode
      )
{
   FILE *rtn= fopen (pathname, mode);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fopen(\"%s\", \"%s\") failed", pathname, mode);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fclose (
      const char *fileName,
      int lineNo,
      const char *funcName,
      FILE *stream
      )
{
   int rtn= fclose (stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fclose() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
size_t _ez_fread (
      const char *fileName,
      int lineNo,
      const char *funcName,
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      )
{
   size_t rtn= fread (ptr, size, nmemb, stream);
   if (ferror(stream)) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fread() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
size_t _ez_fwrite (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      )
{
   size_t rtn= fwrite (ptr, size, nmemb, stream);
   if (ferror(stream)) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fwrite() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_pclose (
      const char *fileName,
      int lineNo,
      const char *funcName,
      FILE *stream
      )
{
   int rtn= pclose (stream);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "pclose() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
char* _ez_fgets (
      const char *fileName,
      int lineNo,
      const char *funcName,
      char *s,
      int size,
      FILE *stream
      )
{
   char *rtn= fgets (s, size, stream);
   if (!rtn && !feof(stream)) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "fgets() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_remove (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *pathname
      )
{
   int rtn= remove (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "remove(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_rename (
      const char *fileName,
      int lineNo,
      const char *funcName,
      const char *oldpath,
      const char *newpath
      )
{
   int rtn= rename (oldpath, newpath);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror, fileName, lineNo, funcName, "rename(\"%s\", \"%s\") failed", oldpath, newpath);
      abort();
   }
   return rtn;
}

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
