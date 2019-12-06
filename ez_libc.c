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
#include <sys/file.h>
#include <unistd.h>

#include "util.h"
#include "ez_libc.h"

/***************************************************/
int _ez_fputs (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *s,
      FILE *stream
      )
{
   int rtn= fputs (s, stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fputs(\"%s\") failed", s);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fputc (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int c,
      FILE *stream
      )
{
   int rtn= fputc (c, stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fputc('%c') failed", (unsigned char)c);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fprintf (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
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
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "vfprintf() failed");
      abort();
   }

   return rtn;
}

/***************************************************/
FILE* _ez_popen (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *command,
      const char *type
      )
{
   errno= 0;
   FILE *rtn= popen (command, type);
   if (!rtn || errno) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "popen(\"%s\", \"%s\") failed", command, type);
      abort();
   }
   return rtn;
}

/***************************************************/
FILE* _ez_fopen (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname,
      const char *mode
      )
{
   FILE *rtn= fopen (pathname, mode);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fopen(\"%s\", \"%s\") failed", pathname, mode);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fclose (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      FILE *stream
      )
{
   int rtn= fclose (stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fclose() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_fflush (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      FILE *stream
      )
{
   int rtn= fflush (stream);
   if (EOF == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fflush() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
size_t _ez_fread (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      )
{
   size_t rtn= fread (ptr, size, nmemb, stream);
   if (ferror(stream)) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fread() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
size_t _ez_fwrite (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      )
{
   size_t rtn= fwrite (ptr, size, nmemb, stream);
   if (ferror(stream)) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fwrite() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_pclose (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      FILE *stream
      )
{
   int rtn= pclose (stream);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "pclose() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
char* _ez_fgets (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      char *s,
      int size,
      FILE *stream
      )
{
   char *rtn= fgets (s, size, stream);
   if (!rtn && !feof(stream)) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fgets() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_remove (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname
      )
{
   int rtn= remove (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "remove(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_rename (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *oldpath,
      const char *newpath
      )
{
   int rtn= rename (oldpath, newpath);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "rename(\"%s\", \"%s\") failed", oldpath, newpath);
      abort();
   }
   return rtn;
}

/***************************************************/
DIR* _ez_opendir (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *name
      )
{
   DIR *rtn= opendir (name);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "opendir(\"%s\") failed", name);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_closedir (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      DIR *dirp
      )
{
   int rtn= closedir (dirp);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "closedir() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
struct dirent* _ez_readdir (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      DIR *dirp
      )
{

   /* Pass on doctored format string and varargs to vfprintf() */
   errno= 0;
   struct dirent* rtn= readdir(dirp);

   if (!rtn && errno) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "readdir() failed");
      abort();
   }

   return rtn;
}

/***************************************************/
int _ez_close (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int fd
      )
{
   int rtn= close (fd);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "close(%d) failed", fd);
      abort();
   }
   return rtn;
}

/***************************************************/
ssize_t _ez_write (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int fd,
      const void *buf,
      size_t count
      )
{
   ssize_t rtn= write (fd, buf, count);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "write(fd= %d) failed", fd);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_stat (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname,
      struct stat *statbuf
      )
{
   int rtn= stat (pathname, statbuf);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "stat(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_mkdir (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname,
      mode_t mode
      )
{
   int rtn= mkdir (pathname, mode);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "mkdir(\"%s\", %04x) failed", pathname, (unsigned)mode);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_rmdir (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname
      )
{
   int rtn= rmdir (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "rmdir(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_unlink (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *pathname
      )
{
   int rtn= unlink (pathname);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "unlink(\"%s\") failed", pathname);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_getaddrinfo(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *node,
      const char *service,
      const struct addrinfo *hints,
      struct addrinfo **res
      )
{
   errno= 0;
   int rtn= getaddrinfo (node, service, hints, res);
   switch(rtn) {
      case 0:
      case EAI_AGAIN:
      case EAI_FAIL:
      case EAI_NODATA:
      case EAI_NONAME:
         return rtn;

      case EAI_SYSTEM:
         _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
               , fileName, lineNo, funcName
#endif
               , "getaddrinfo(\"%s:%s\") failed", node, service);
         abort();
   }

   /* _sys_eprintf() will pass errno to gai_sterror */
   errno= rtn;
   _sys_eprintf(gai_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "getaddrinfo(\"%s:%s\") failed", node, service);
   abort();
}

/***************************************************/
int _ez_getnameinfo(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const struct sockaddr *addr,
      socklen_t addrlen,
      char *host,
      socklen_t hostlen,
      char *serv,
      socklen_t servlen,
      int flags
      )
{
   errno= 0;
   int rtn= getnameinfo (addr, addrlen, host, hostlen, serv, servlen, flags);
   switch(rtn) {
      case 0:
      case EAI_AGAIN:
      case EAI_FAIL:
      case EAI_NONAME:
         return rtn;

      case EAI_SYSTEM:
         _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
               , fileName, lineNo, funcName
#endif
               , "getnameinfo() failed");
         abort();
   }

   /* _sys_eprintf() will pass errno to gai_sterror */
   errno= rtn;
   _sys_eprintf(gai_strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "getnameinfo() failed", rtn);
   abort();
}

/***************************************************/
int _ez_flock (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      int operation
      )
{
   errno= 0;
   int rtn= flock (fd, operation);
   if(0 == rtn) return 0;

   switch(errno) {
      case EINTR:
      case EWOULDBLOCK:
         return rtn;
         break;
   }

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "flock() failed");
   abort();

}

/***************************************************/
int _ez_open(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      int flags,
      mode_t mode
      )
{
   errno= 0;
   int rtn= open (pathname, flags, mode);
   if(0 <= rtn) return rtn;

   switch(errno) {
      case EINTR:
      case EWOULDBLOCK:
         return rtn;
         break;
   }

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "open(\"%s\") failed", pathname);
   abort();

}

/***************************************************/
int _ez_access(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      int mode
      )
{
   errno= 0;
   int rtn= access (pathname, mode);
   if(0 == rtn) return rtn;

   switch(errno) {
      case ENOENT:
         return rtn;
         break;
   }

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "access(\"%s\") failed", pathname);
   abort();

}

/***************************************************/
char *_ez_strptime(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *s,
      const char *format,
      struct tm *tm
      )
{
   char *rtn= strptime (s, format, tm);
   if(rtn) return rtn;

   _eprintf(
#ifdef DEBUG
         fileName, lineNo, funcName,
#endif
         "strptime(\"%s\", \"%s\") failed", s, format);
   abort();

}

/***************************************************/
int _ez_seteuid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      uid_t euid
      )
{
   int rtn= seteuid (euid);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "seteuid(%d) failed", (int)euid);
   abort();
}

/***************************************************/
int _ez_setegid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      gid_t egid
      )
{
   int rtn= setegid (egid);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "setegid(%d) failed", (int)egid);
   abort();
}

/***************************************************/
struct group* _ez_getgrnam(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *name
      )
{
   errno= 0;
   struct group *rtn= getgrnam (name);

   if(rtn) return rtn;

   switch(errno) {
      case EINTR:
      case EIO:
      case EMFILE:
      case ENFILE:
      case ENOMEM:
      case ERANGE:
         return NULL;

   }
      
   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "getgrnam(\"%s\") failed", name);
   abort();

}

/***************************************************/
int _ez_chown(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      uid_t owner,
      gid_t group
      )
{
   int rtn= chown (pathname, owner, group);
   if(0 == rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "chown(\"%s\", %d, %d) failed", pathname, (int)owner, (int)group);
   abort();
}

/***************************************************/
int _ez_fchown(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      uid_t owner,
      gid_t group
      )
{
   int rtn= fchown (fd, owner, group);
   if(0 == rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "fchown() failed");
   abort();
}

/***************************************************/
int _ez_fchmod(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      mode_t mode
      )
{
   int rtn= fchmod (fd, mode);
   if(0 == rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "fchmod() failed");
   abort();
}

/***************************************************/
int _ez_setuid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      uid_t uid
      )
{
   int rtn= setuid (uid);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "setuid(%d) failed", (int)uid);
   abort();
}

/***************************************************/
int _ez_setgid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      gid_t gid
      )
{
   int rtn= setgid (gid);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "setgid(%d) failed", (int)gid);
   abort();
}
