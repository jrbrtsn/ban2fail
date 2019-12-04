
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

#define _GNU_SOURCE
#include <dirent.h>
#include <grp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ez_strptime(s, format, tm) \
   _ez_strptime(__FILE__, __LINE__, __FUNCTION__, s, format, tm)
char *_ez_strptime(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *s,
      const char *format,
      struct tm *tm
      );

#define ez_access(pathname, mode) \
   _ez_access(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
int _ez_access(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname,
      int mode
      );

#define ez_open(pathname, flags, mode) \
   _ez_open(__FILE__, __LINE__, __FUNCTION__, pathname, flags, mode)
int _ez_open(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname,
      int flags,
      mode_t mode
      );


#define ez_fputs(s, stream) \
   _ez_fputs(__FILE__, __LINE__, __FUNCTION__, s, stream)
int _ez_fputs (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *s,
      FILE *stream
      );

#define ez_fputc(c, stream) \
   _ez_fputc(__FILE__, __LINE__, __FUNCTION__, c, stream)
int _ez_fputc (
   const char *fileName,
   int lineNo,
   const char *funcName,
      int c,
      FILE *stream
      );

#define ez_fprintf(stream, fmt, ...) \
         _ez_fprintf(__FILE__, __LINE__, __FUNCTION__, stream, fmt, ##__VA_ARGS__)
int _ez_fprintf (
   const char *fileName,
   int lineNo,
   const char *funcName,
      FILE *stream,
      const char *fmt,
      ...
      );

#define ez_popen(command, type) \
         _ez_popen(__FILE__, __LINE__, __FUNCTION__, command, type)
FILE* _ez_popen (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *command,
      const char *type
      );

#define ez_fopen(pathname, mode) \
         _ez_fopen(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
FILE* _ez_fopen (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname,
      const char *mode
      );

#define ez_fclose(stream) \
         _ez_fclose(__FILE__, __LINE__, __FUNCTION__, stream)
int _ez_fclose (
   const char *fileName,
   int lineNo,
   const char *funcName,
      FILE *stream
      );

#define ez_fflush(stream) \
         _ez_fflush(__FILE__, __LINE__, __FUNCTION__, stream)
int _ez_fflush (
   const char *fileName,
   int lineNo,
   const char *funcName,
      FILE *stream
      );

#define ez_fread(ptr, size, nmemb, stream) \
         _ez_fread(__FILE__, __LINE__, __FUNCTION__, ptr, size, nmemb, stream)
size_t _ez_fread(
   const char *fileName,
   int lineNo,
   const char *funcName,
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      );

#define ez_fwrite(ptr, size, nmemb, stream) \
         _ez_fwrite(__FILE__, __LINE__, __FUNCTION__, ptr, size, nmemb, stream)
size_t _ez_fwrite(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      );


#define ez_pclose(stream) \
         _ez_pclose(__FILE__, __LINE__, __FUNCTION__, stream)
int _ez_pclose (
   const char *fileName,
   int lineNo,
   const char *funcName,
      FILE *stream
      );

#define ez_fgets(s, size, stream) \
         _ez_fgets(__FILE__, __LINE__, __FUNCTION__, s, size, stream)
char* _ez_fgets (
   const char *fileName,
   int lineNo,
   const char *funcName,
      char *s,
      int size,
      FILE *stream
      );

#define ez_remove(pathname) \
         _ez_remove(__FILE__, __LINE__, __FUNCTION__, pathname)
int _ez_remove (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname
      );

#define ez_rename(oldpath, newpath) \
         _ez_rename(__FILE__, __LINE__, __FUNCTION__, oldpath, newpath)
int _ez_rename (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *oldpath,
      const char *newpath
      );

#define ez_opendir(name) \
   _ez_opendir(__FILE__, __LINE__, __FUNCTION__, name)
DIR* _ez_opendir (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *name
      );

#define ez_closedir(dirp) \
   _ez_closedir(__FILE__, __LINE__, __FUNCTION__, dirp)
int _ez_closedir (
   const char *fileName,
   int lineNo,
   const char *funcName,
      DIR *dirp
      );

#define ez_readdir(dirp) \
         _ez_readdir(__FILE__, __LINE__, __FUNCTION__, dirp)
struct dirent* _ez_readdir (
   const char *fileName,
   int lineNo,
   const char *funcName,
      DIR *dirp
      );

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

#define ez_unlink(pathname) \
         _ez_unlink(__FILE__, __LINE__, __FUNCTION__, pathname)
int _ez_unlink (
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname
      );

#define ez_getaddrinfo(node, service, hints, res) \
         _ez_getaddrinfo(__FILE__, __LINE__, __FUNCTION__, node, service, hints, res)
int _ez_getaddrinfo(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *node,
      const char *service,
      const struct addrinfo *hints,
      struct addrinfo **res
      );

#define ez_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags) \
         _ez_getnameinfo(__FILE__, __LINE__, __FUNCTION__, addr, addrlen, host, hostlen, serv, servlen, flags)
int _ez_getnameinfo(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const struct sockaddr *addr,
      socklen_t addrlen,
      char *host,
      socklen_t hostlen,
      char *serv,
      socklen_t servlen,
      int flags
      );

#define ez_flock(fd, operation) \
         _ez_flock(__FILE__, __LINE__, __FUNCTION__, fd, operation)
int _ez_flock (
   const char *fileName,
   int lineNo,
   const char *funcName,
      int fd,
      int operation
      );

#define ez_setuid(uid) \
         _ez_setuid(__FILE__, __LINE__, __FUNCTION__, uid)
int _ez_setuid(
   const char *fileName,
   int lineNo,
   const char *funcName,
      uid_t uid
      );

#define ez_setgid(gid) \
         _ez_setgid(__FILE__, __LINE__, __FUNCTION__, gid)
int _ez_setgid(
   const char *fileName,
   int lineNo,
   const char *funcName,
      gid_t gid
      );


#define ez_seteuid(euid) \
         _ez_seteuid(__FILE__, __LINE__, __FUNCTION__, euid)
int _ez_seteuid(
   const char *fileName,
   int lineNo,
   const char *funcName,
      uid_t euid
      );

#define ez_setegid(egid) \
         _ez_setegid(__FILE__, __LINE__, __FUNCTION__, egid)
int _ez_setegid(
   const char *fileName,
   int lineNo,
   const char *funcName,
      gid_t egid
      );

#define ez_getgrnam(name) \
         _ez_getgrnam(__FILE__, __LINE__, __FUNCTION__, name)
struct group* _ez_getgrnam(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *name
      );

#define ez_chown(pathname, owner, group) \
         _ez_chown(__FILE__, __LINE__, __FUNCTION__, pathname, owner, group)
int _ez_chown(
   const char *fileName,
   int lineNo,
   const char *funcName,
      const char *pathname,
      uid_t owner,
      gid_t group
      );

#define ez_fchown(fd, owner, group) \
         _ez_fchown(__FILE__, __LINE__, __FUNCTION__, fd, owner, group)
int _ez_fchown(
   const char *fileName,
   int lineNo,
   const char *funcName,
      int fd,
      uid_t owner,
      gid_t group
      );

#define ez_fchmod(fd, mode) \
         _ez_fchmod(__FILE__, __LINE__, __FUNCTION__, fd, mode)
int _ez_fchmod(
   const char *fileName,
   int lineNo,
   const char *funcName,
      int fd,
      mode_t mode
      );

#ifdef __cplusplus
}
#endif

#endif
