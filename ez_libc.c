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
ez_proto (int, fputs,
      const char *s,
      FILE *stream)
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
ez_proto (int,  fputc,
      int c,
      FILE *stream)
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
ez_proto (int,  fprintf,
      FILE *stream,
      const char *fmt,
      ...)
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
            , "vfprintf() failed returning %d", rtn);
      abort();
   }

   return rtn;
}

/***************************************************/
ez_proto (FILE*, popen,
      const char *command,
      const char *type)
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
ez_proto (FILE*,  fdopen, 
      int fd,
      const char *mode)
{
   FILE *rtn= fdopen (fd, mode);
   if (!rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fdopen(%d, \"%s\") failed", fd, mode);
      abort();
   }
   return rtn;
}


/***************************************************/
ez_proto (FILE*,  fopen, 
      const char *pathname,
      const char *mode)
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
ez_proto (pid_t,  fork)
{
   int rtn= fork ();
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "fork() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
ez_proto (int, fclose,
      FILE *stream)
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
ez_proto (int,  fflush,
      FILE *stream)
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
ez_proto (size_t, fread,
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream)
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
ez_proto (size_t, fwrite,
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream)
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
ez_proto (int,  pclose,
      FILE *stream)
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
ez_proto (int, pipe, int pipefd[2])
{
   int rtn= pipe (pipefd);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "pipe() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
ez_proto (char*, fgets,
      char *s,
      int size,
      FILE *stream)
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
ez_proto (int,  remove, 
      const char *pathname)
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
ez_proto (int,  rename,
      const char *oldpath,
      const char *newpath)
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
ez_proto (DIR*, opendir,
      const char *name)
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
ez_proto (int,  closedir,
      DIR *dirp)
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
ez_proto (int, dup2, int oldfd, int newfd)
{
   int rtn= dup2 (oldfd, newfd);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "dup2() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
ez_proto (int, execve,
      const char *filename,
      char *const argv[],
      char *const envp[])
{
   int rtn= execve(filename, argv, envp);
   if (-1 == rtn) {
      _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
            , fileName, lineNo, funcName
#endif
            , "execve() failed");
      abort();
   }
   return rtn;
}

/***************************************************/
ez_proto (struct dirent*, readdir, DIR *dirp)
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
ez_proto (int, close, int fd)
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
ez_proto (ssize_t, write,
      int fd,
      const void *buf,
      size_t count)
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
ez_proto (int,  stat,
   const char *pathname,
      struct stat *statbuf)
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
ez_proto (int, mkdir,
      const char *pathname,
      mode_t mode)
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
ez_proto (int, rmdir,
      const char *pathname)
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
ez_proto (int, unlink,
      const char *pathname)
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
ez_proto (int, getaddrinfo,
      const char *node,
      const char *service,
      const struct addrinfo *hints,
      struct addrinfo **res)
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
ez_proto (int,  getnameinfo,
      const struct sockaddr *addr,
      socklen_t addrlen,
      char *host,
      socklen_t hostlen,
      char *serv,
      socklen_t servlen,
      int flags)
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
         , "getnameinfo() failed");
   abort();
}

/***************************************************/
ez_proto (int, flock,
      int fd,
      int operation)
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
ez_proto (int, open,
      const char *pathname,
      int flags,
      mode_t mode)
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
ez_proto (int, access,
      const char *pathname,
      int mode)
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
ez_proto (char*, strptime,
      const char *s,
      const char *format,
      struct tm *tm)
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
ez_proto (int, seteuid, uid_t euid)
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
ez_proto (int, setegid, gid_t egid)
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
ez_proto (struct group*, getgrnam, const char *name)
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
ez_proto (int, chown,
      const char *pathname,
      uid_t owner,
      gid_t group)
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
ez_proto (int, fchown,
      int fd,
      uid_t owner,
      gid_t group)
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
ez_proto (int, fchmod, int fd, mode_t mode)
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
ez_proto (int, setuid, uid_t uid)
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
ez_proto (int,  setgid, gid_t gid)
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

/***************************************************/
ez_proto (int, atexit,
   void(*function)(void))
{
   int rtn= atexit (function);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "atexit() failed");
   abort();
}

/***************************************************/
ez_proto (int, chdir,
   const char *path)
{
   int rtn= chdir (path);
   if(0 == rtn) return 0;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "chdir(\"%s\") failed", path);
   abort();
}

/***************************************************/
ez_proto (int, mkstemp,
      char *template)
{
   int rtn= mkstemp (template);
   if(-1 != rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "mkstemp(\"%s\") failed", template);
   abort();
}

/***************************************************/
ez_proto (int, mkstemps,
      char *template,
      int suffixlen)
{
   int rtn= mkstemps (template, suffixlen);
   if(-1 != rtn) return rtn;

   _sys_eprintf((const char*(*)(int))strerror
#ifdef DEBUG
         , fileName, lineNo, funcName
#endif
         , "mkstemps(\"%s\") failed", template);
   abort();
}


/***************************************************/
ez_proto (int, vfprintf,
      FILE *stream,
      const char *fmt,
      va_list ap)
{
   va_list arglist;
   va_copy(arglist, ap);
   int rtn= vfprintf (stream, fmt, arglist);
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

