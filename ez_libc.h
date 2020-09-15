
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
                          ez_libc.h  -  description                              
glibc calls with boilerplate error handling.

                             -------------------
    begin                : Tue Nov 13 19:42:23 EST 2018
    email                : john@rrci.com                                     
 ***************************************************************************/
#ifndef EZ_LIBC_H
#define EZ_LIBC_H

#ifndef _GNU_SOURCE
#       define _GNU_SOURCE
#endif
#include <dirent.h>
#include <grp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "ez.h"

#ifdef __cplusplus
extern "C" {
#endif

ez_proto (int, access,
      const char *pathname,
      int mode);
#ifdef DEBUG
#       define ez_access(...) \
   _ez_access(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_access(...) \
   _ez_access(__VA_ARGS__)
#endif

ez_proto (int, atexit,
   void(*function)(void));
#ifdef DEBUG
#       define ez_atexit(...) \
         _ez_atexit(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_atexit(...) \
         _ez_atexit(__VA_ARGS__)
#endif

ez_proto (int, chdir,
   const char *path);
#ifdef DEBUG
#       define ez_chdir(...) \
         _ez_chdir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_chdir(...) \
         _ez_chdir(__VA_ARGS__)
#endif

ez_proto (int, chown,
      const char *pathname,
      uid_t owner,
      gid_t group);
#ifdef DEBUG
#       define ez_chown(...) \
         _ez_chown(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_chown(...) \
         _ez_chown(__VA_ARGS__)
#endif

ez_proto (int, close,
      int fd);
#ifdef DEBUG
#       define ez_close(...) \
         _ez_close(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_close(...) \
         _ez_close(__VA_ARGS__)
#endif

ez_proto (int, closedir,
      DIR *dirp);
#ifdef DEBUG
#       define ez_closedir(...) \
   _ez_closedir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_closedir(...) \
   _ez_closedir(__VA_ARGS__)
#endif

ez_proto (int, dup2, int oldfd, int newfd);
#ifdef DEBUG
#       define ez_dup2(...) \
   _ez_dup2(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_dup2(...) \
   _ez_dup2(__VA_ARGS__)
#endif

ez_proto (int, execve,
      const char *filename,
      char *const argv[],
      char *const envp[]);
#ifdef DEBUG
#       define ez_execve(...) \
   _ez_execve(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_execve(...) \
   _ez_execve(__VA_ARGS__)
#endif


ez_proto (int, fchmod,
      int fd,
      mode_t mode);
#ifdef DEBUG
#       define ez_fchmod(...) \
         _ez_fchmod(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fchmod(...) \
         _ez_fchmod(__VA_ARGS__)
#endif

ez_proto (int, fchown,
      int fd,
      uid_t owner,
      gid_t group);
#ifdef DEBUG
#       define ez_fchown(...) \
         _ez_fchown(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fchown(...) \
         _ez_fchown(__VA_ARGS__)
#endif

ez_proto (int, fclose,
      FILE *stream);
#ifdef DEBUG
#       define ez_fclose(...) \
         _ez_fclose(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fclose(...) \
         _ez_fclose(__VA_ARGS__)
#endif

ez_proto (int, fflush,
      FILE *stream);
#ifdef DEBUG
#       define ez_fflush(...) \
         _ez_fflush(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fflush(...) \
         _ez_fflush(__VA_ARGS__)
#endif

ez_proto (char*, fgets,
      char *s,
      int size,
      FILE *stream);
#ifdef DEBUG
#       define ez_fgets(...) \
         _ez_fgets(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fgets(...) \
         _ez_fgets(__VA_ARGS__)
#endif

ez_proto (FILE*,  fdopen, 
      int fd,
      const char *mode);
#ifdef DEBUG
#       define ez_fdopen(...) \
         _ez_fdopen(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fdopen(...) \
         _ez_fdopen(__VA_ARGS__)
#endif

ez_proto (int, flock,
      int fd,
      int operation);
#ifdef DEBUG
#       define ez_flock(...) \
         _ez_flock(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_flock(...) \
         _ez_flock(__VA_ARGS__)
#endif

ez_proto (FILE*,  fopen, 
      const char *pathname,
      const char *mode);
#ifdef DEBUG
#       define ez_fopen(...) \
         _ez_fopen(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fopen(...) \
         _ez_fopen(__VA_ARGS__)
#endif

ez_proto (pid_t,  fork);
#ifdef DEBUG
#       define ez_fork(...) \
         _ez_fork(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fork(...) \
         _ez_fork(__VA_ARGS__)
#endif

ez_proto (int, fprintf,
      FILE *stream,
      const char *fmt,
      ...);
#ifdef DEBUG
#       define ez_fprintf(...) \
         _ez_fprintf(__FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#       define ez_fprintf(...) \
         _ez_fprintf(__VA_ARGS__)
#endif

ez_proto (int, fputc,
      int c,
      FILE *stream);
#ifdef DEBUG
#       define ez_fputc(...) \
   _ez_fputc(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fputc(...) \
   _ez_fputc(__VA_ARGS__)
#endif

ez_proto (int, fputs,
      const char *s,
      FILE *stream);
#ifdef DEBUG
#       define ez_fputs(...) \
   _ez_fputs(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fputs(...) \
   _ez_fputs(__VA_ARGS__)
#endif

ez_proto (size_t, fread,
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream);
#ifdef DEBUG
#       define ez_fread(...) \
         _ez_fread(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fread(...) \
         _ez_fread(__VA_ARGS__)
#endif

ez_proto (size_t, fwrite,
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream);
#ifdef DEBUG
#       define ez_fwrite(...) \
         _ez_fwrite(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_fwrite(...) \
         _ez_fwrite(__VA_ARGS__)
#endif

ez_proto (int, getaddrinfo,
      const char *node,
      const char *service,
      const struct addrinfo *hints,
      struct addrinfo **res);
#ifdef DEBUG
#       define ez_getaddrinfo(...) \
         _ez_getaddrinfo(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_getaddrinfo(...) \
         _ez_getaddrinfo(__VA_ARGS__)
#endif

ez_proto (int, getnameinfo,
      const struct sockaddr *addr,
      socklen_t addrlen,
      char *host,
      socklen_t hostlen,
      char *serv,
      socklen_t servlen,
      int flags);
#ifdef DEBUG
#       define ez_getnameinfo(...) \
         _ez_getnameinfo(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_getnameinfo(...) \
         _ez_getnameinfo(__VA_ARGS__)
#endif

ez_proto (struct group*, getgrnam,
      const char *name);
#ifdef DEBUG
#       define ez_getgrnam(...) \
         _ez_getgrnam(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_getgrnam(...) \
         _ez_getgrnam(__VA_ARGS__)
#endif

ez_proto (int, mkdir,
      const char *pathname,
      mode_t mode);
#ifdef DEBUG
#       define ez_mkdir(...) \
         _ez_mkdir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_mkdir(...) \
         _ez_mkdir(__VA_ARGS__)
#endif

ez_proto (int, mkstemp,
      char *tmpl);
#ifdef DEBUG
#       define ez_mkstemp(...) \
         _ez_mkstemp(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_mkstemp(...) \
         _ez_mkstemp(__VA_ARGS__)
#endif

ez_proto (int, mkstemps, char *tmpl, int suffixlen);
#ifdef DEBUG
#       define ez_mkstemps(...) \
         _ez_mkstemps(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_mkstemps(...) \
         _ez_mkstemps(__VA_ARGS__)
#endif


ez_proto (int, open,
      const char *pathname,
      int flags,
      mode_t mode);
#ifdef DEBUG
#       define ez_open(...) \
   _ez_open(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_open(...) \
   _ez_open(__VA_ARGS__)
#endif

ez_proto (DIR*, opendir,
      const char *name);
#ifdef DEBUG
#       define ez_opendir(...) \
   _ez_opendir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_opendir(...) \
   _ez_opendir(__VA_ARGS__)
#endif

ez_proto (int, pclose,
      FILE *stream);
#ifdef DEBUG
#       define ez_pclose(...) \
         _ez_pclose(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pclose(...) \
         _ez_pclose(__VA_ARGS__)
#endif

ez_proto (int, pipe, int pipefd[2]);
#ifdef DEBUG
#       define ez_pipe(...) \
         _ez_pipe(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_pipe(...) \
         _ez_pipe(__VA_ARGS__)
#endif


ez_proto (FILE*, popen,
      const char *command,
      const char *type);
#ifdef DEBUG
#       define ez_popen(...) \
         _ez_popen(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_popen(...) \
         _ez_popen(__VA_ARGS__)
#endif

ez_proto (struct dirent*, readdir,
      DIR *dirp);
#ifdef DEBUG
#       define ez_readdir(...) \
         _ez_readdir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_readdir(...) \
         _ez_readdir(__VA_ARGS__)
#endif

ez_proto (int, remove, 
      const char *pathname);
#ifdef DEBUG
#       define ez_remove(...) \
         _ez_remove(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_remove(...) \
         _ez_remove(__VA_ARGS__)
#endif

ez_proto (int, rename,
      const char *oldpath,
      const char *newpath);
#ifdef DEBUG
#       define ez_rename(...) \
         _ez_rename(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_rename(...) \
         _ez_rename(__VA_ARGS__)
#endif

ez_proto (int, rmdir,
      const char *pathname);
#ifdef DEBUG
#       define ez_rmdir(...) \
         _ez_rmdir(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_rmdir(...) \
         _ez_rmdir(__VA_ARGS__)
#endif

ez_proto (int, setegid,
      gid_t egid);
#ifdef DEBUG
#       define ez_setegid(...) \
         _ez_setegid(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_setegid(...) \
         _ez_setegid(__VA_ARGS__)
#endif

ez_proto (int, seteuid,
      uid_t euid);
#ifdef DEBUG
#       define ez_seteuid(...) \
         _ez_seteuid(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_seteuid(...) \
         _ez_seteuid(__VA_ARGS__)
#endif

ez_proto (int,  setgid,
      gid_t gid);
#ifdef DEBUG
#       define ez_setgid(...) \
         _ez_setgid(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_setgid(...) \
         _ez_setgid(__VA_ARGS__)
#endif

ez_proto (int, setuid,
      uid_t uid);
#ifdef DEBUG
#       define ez_setuid(...) \
         _ez_setuid(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_setuid(...) \
         _ez_setuid(__VA_ARGS__)
#endif

ez_proto (int, stat,
   const char *pathname,
      struct stat *statbuf);
#ifdef DEBUG
#       define ez_stat(...) \
         _ez_stat(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_stat(...) \
         _ez_stat(__VA_ARGS__)
#endif

ez_proto (char*, strptime,
      const char *s,
      const char *format,
      struct tm *tm);
#ifdef DEBUG
#       define ez_strptime(...) \
   _ez_strptime(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_strptime(...) \
   _ez_strptime(__VA_ARGS__)
#endif

ez_proto (int, unlink,
      const char *pathname);
#ifdef DEBUG
#       define ez_unlink(...) \
         _ez_unlink(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_unlink(...) \
         _ez_unlink(__VA_ARGS__)
#endif

ez_proto (int, vfprintf,
      FILE *stream,
      const char *fmt,
      va_list ap);
#ifdef DEBUG
#       define ez_vfprintf(...) \
         _ez_vfprintf(__FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#       define ez_vfprintf(...) \
         _ez_vfprintf(__VA_ARGS__)
#endif


ez_proto (ssize_t, write,
      int fd,
      const void *buf,
      size_t count);
#ifdef DEBUG
#       define ez_write(...) \
         _ez_write(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_write(...) \
         _ez_write(__VA_ARGS__)
#endif

#if 0
// FIXME: this needs to be implemented and tested
ez_proto (ssize_t read,
      int fd,
      void *buf,
      size_t count);
#ifdef DEBUG
#       define ez_read(...) \
         _ez_read(__FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#       define ez_read(...) \
         _ez_read(__VA_ARGS__)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
