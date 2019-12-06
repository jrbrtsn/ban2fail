
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

#ifdef DEBUG
#       define ez_strptime(s, format, tm) \
   _ez_strptime(__FILE__, __LINE__, __FUNCTION__, s, format, tm)
#else
#       define ez_strptime(s, format, tm) \
   _ez_strptime(s, format, tm)
#endif
char *_ez_strptime(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *s,
      const char *format,
      struct tm *tm
      );

#ifdef DEBUG
#       define ez_access(pathname, mode) \
   _ez_access(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
#else
#       define ez_access(pathname, mode) \
   _ez_access(pathname, mode)
#endif
int _ez_access(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      int mode
      );

#ifdef DEBUG
#       define ez_open(pathname, flags, mode) \
   _ez_open(__FILE__, __LINE__, __FUNCTION__, pathname, flags, mode)
#else
#       define ez_open(pathname, flags, mode) \
   _ez_open(pathname, flags, mode)
#endif
int _ez_open(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      int flags,
      mode_t mode
      );


#ifdef DEBUG
#       define ez_fputs(s, stream) \
   _ez_fputs(__FILE__, __LINE__, __FUNCTION__, s, stream)
#else
#       define ez_fputs(s, stream) \
   _ez_fputs(s, stream)
#endif
int _ez_fputs (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *s,
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fputc(c, stream) \
   _ez_fputc(__FILE__, __LINE__, __FUNCTION__, c, stream)
#else
#       define ez_fputc(c, stream) \
   _ez_fputc(c, stream)
#endif
int _ez_fputc (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int c,
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fprintf(stream, fmt, ...) \
         _ez_fprintf(__FILE__, __LINE__, __FUNCTION__, stream, fmt, ##__VA_ARGS__)
#else
#       define ez_fprintf(stream, fmt, ...) \
         _ez_fprintf(stream, fmt, ##__VA_ARGS__)
#endif
int _ez_fprintf (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      FILE *stream,
      const char *fmt,
      ...
      );

#ifdef DEBUG
#       define ez_popen(command, type) \
         _ez_popen(__FILE__, __LINE__, __FUNCTION__, command, type)
#else
#       define ez_popen(command, type) \
         _ez_popen(command, type)
#endif
FILE* _ez_popen (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *command,
      const char *type
      );

#ifdef DEBUG
#       define ez_fopen(pathname, mode) \
         _ez_fopen(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
#else
#       define ez_fopen(pathname, mode) \
         _ez_fopen(pathname, mode)
#endif
FILE* _ez_fopen (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      const char *mode
      );

#ifdef DEBUG
#       define ez_fclose(stream) \
         _ez_fclose(__FILE__, __LINE__, __FUNCTION__, stream)
#else
#       define ez_fclose(stream) \
         _ez_fclose(stream)
#endif
int _ez_fclose (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fflush(stream) \
         _ez_fflush(__FILE__, __LINE__, __FUNCTION__, stream)
#else
#       define ez_fflush(stream) \
         _ez_fflush(stream)
#endif
int _ez_fflush (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fread(ptr, size, nmemb, stream) \
         _ez_fread(__FILE__, __LINE__, __FUNCTION__, ptr, size, nmemb, stream)
#else
#       define ez_fread(ptr, size, nmemb, stream) \
         _ez_fread(ptr, size, nmemb, stream)
#endif
size_t _ez_fread(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fwrite(ptr, size, nmemb, stream) \
         _ez_fwrite(__FILE__, __LINE__, __FUNCTION__, ptr, size, nmemb, stream)
#else
#       define ez_fwrite(ptr, size, nmemb, stream) \
         _ez_fwrite(ptr, size, nmemb, stream)
#endif
size_t _ez_fwrite(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const void *ptr,
      size_t size,
      size_t nmemb,
      FILE *stream
      );


#ifdef DEBUG
#       define ez_pclose(stream) \
         _ez_pclose(__FILE__, __LINE__, __FUNCTION__, stream)
#else
#       define ez_pclose(stream) \
         _ez_pclose(stream)
#endif
int _ez_pclose (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      FILE *stream
      );

#ifdef DEBUG
#       define ez_fgets(s, size, stream) \
         _ez_fgets(__FILE__, __LINE__, __FUNCTION__, s, size, stream)
#else
#       define ez_fgets(s, size, stream) \
         _ez_fgets(s, size, stream)
#endif
char* _ez_fgets (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      char *s,
      int size,
      FILE *stream
      );

#ifdef DEBUG
#       define ez_remove(pathname) \
         _ez_remove(__FILE__, __LINE__, __FUNCTION__, pathname)
#else
#       define ez_remove(pathname) \
         _ez_remove(pathname)
#endif
int _ez_remove (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname
      );

#ifdef DEBUG
#       define ez_rename(oldpath, newpath) \
         _ez_rename(__FILE__, __LINE__, __FUNCTION__, oldpath, newpath)
#else
#       define ez_rename(oldpath, newpath) \
         _ez_rename(oldpath, newpath)
#endif
int _ez_rename (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *oldpath,
      const char *newpath
      );

#ifdef DEBUG
#       define ez_opendir(name) \
   _ez_opendir(__FILE__, __LINE__, __FUNCTION__, name)
#else
#       define ez_opendir(name) \
   _ez_opendir(name)
#endif
DIR* _ez_opendir (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *name
      );

#ifdef DEBUG
#       define ez_closedir(dirp) \
   _ez_closedir(__FILE__, __LINE__, __FUNCTION__, dirp)
#else
#       define ez_closedir(dirp) \
   _ez_closedir(dirp)
#endif
int _ez_closedir (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DIR *dirp
      );

#ifdef DEBUG
#       define ez_readdir(dirp) \
         _ez_readdir(__FILE__, __LINE__, __FUNCTION__, dirp)
#else
#       define ez_readdir(dirp) \
         _ez_readdir(dirp)
#endif
struct dirent* _ez_readdir (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      DIR *dirp
      );

#ifdef DEBUG
#       define ez_close(fd) \
         _ez_close(__FILE__, __LINE__, __FUNCTION__, fd)
#else
#       define ez_close(fd) \
         _ez_close(fd)
#endif
int _ez_close (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
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

#ifdef DEBUG
#       define ez_write(fd, buf, count) \
         _ez_write(__FILE__, __LINE__, __FUNCTION__, fd, buf, count)
#else
#       define ez_write(fd, buf, count) \
         _ez_write(fd, buf, count)
#endif
ssize_t _ez_write (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      const void *buf,
      size_t count
      );

#ifdef DEBUG
#       define ez_stat(pathname, statbuf) \
         _ez_stat(__FILE__, __LINE__, __FUNCTION__, pathname, statbuf)
#else
#       define ez_stat(pathname, statbuf) \
         _ez_stat(pathname, statbuf)
#endif
int _ez_stat (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
   const char *pathname,
      struct stat *statbuf
      );

#ifdef DEBUG
#       define ez_mkdir(pathname, mode) \
         _ez_mkdir(__FILE__, __LINE__, __FUNCTION__, pathname, mode)
#else
#       define ez_mkdir(pathname, mode) \
         _ez_mkdir(pathname, mode)
#endif
int _ez_mkdir (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      mode_t mode
      );

#ifdef DEBUG
#       define ez_rmdir(pathname) \
         _ez_rmdir(__FILE__, __LINE__, __FUNCTION__, pathname)
#else
#       define ez_rmdir(pathname) \
         _ez_rmdir(pathname)
#endif
int _ez_rmdir (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname
      );

#ifdef DEBUG
#       define ez_unlink(pathname) \
         _ez_unlink(__FILE__, __LINE__, __FUNCTION__, pathname)
#else
#       define ez_unlink(pathname) \
         _ez_unlink(pathname)
#endif
int _ez_unlink (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname
      );

#ifdef DEBUG
#       define ez_unlink(pathname) \
         _ez_unlink(__FILE__, __LINE__, __FUNCTION__, pathname)
#else
#       define ez_unlink(pathname) \
         _ez_unlink(pathname)
#endif
int _ez_unlink (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname
      );

#ifdef DEBUG
#       define ez_getaddrinfo(node, service, hints, res) \
         _ez_getaddrinfo(__FILE__, __LINE__, __FUNCTION__, node, service, hints, res)
#else
#       define ez_getaddrinfo(node, service, hints, res) \
         _ez_getaddrinfo(node, service, hints, res)
#endif
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
      );

#ifdef DEBUG
#       define ez_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags) \
         _ez_getnameinfo(__FILE__, __LINE__, __FUNCTION__, addr, addrlen, host, hostlen, serv, servlen, flags)
#else
#       define ez_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags) \
         _ez_getnameinfo(addr, addrlen, host, hostlen, serv, servlen, flags)
#endif
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
      );

#ifdef DEBUG
#       define ez_flock(fd, operation) \
         _ez_flock(__FILE__, __LINE__, __FUNCTION__, fd, operation)
#else
#       define ez_flock(fd, operation) \
         _ez_flock(fd, operation)
#endif
int _ez_flock (
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      int operation
      );

#ifdef DEBUG
#       define ez_setuid(uid) \
         _ez_setuid(__FILE__, __LINE__, __FUNCTION__, uid)
#else
#       define ez_setuid(uid) \
         _ez_setuid(uid)
#endif
int _ez_setuid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      uid_t uid
      );

#ifdef DEBUG
#       define ez_setgid(gid) \
         _ez_setgid(__FILE__, __LINE__, __FUNCTION__, gid)
#else
#       define ez_setgid(gid) \
         _ez_setgid(gid)
#endif
int _ez_setgid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      gid_t gid
      );


#ifdef DEBUG
#       define ez_seteuid(euid) \
         _ez_seteuid(__FILE__, __LINE__, __FUNCTION__, euid)
#else
#       define ez_seteuid(euid) \
         _ez_seteuid(euid)
#endif
int _ez_seteuid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      uid_t euid
      );

#ifdef DEBUG
#       define ez_setegid(egid) \
         _ez_setegid(__FILE__, __LINE__, __FUNCTION__, egid)
#else
#       define ez_setegid(egid) \
         _ez_setegid(egid)
#endif
int _ez_setegid(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      gid_t egid
      );

#ifdef DEBUG
#       define ez_getgrnam(name) \
         _ez_getgrnam(__FILE__, __LINE__, __FUNCTION__, name)
#else
#       define ez_getgrnam(name) \
         _ez_getgrnam(name)
#endif
struct group* _ez_getgrnam(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *name
      );

#ifdef DEBUG
#       define ez_chown(pathname, owner, group) \
         _ez_chown(__FILE__, __LINE__, __FUNCTION__, pathname, owner, group)
#else
#       define ez_chown(pathname, owner, group) \
         _ez_chown(pathname, owner, group)
#endif
int _ez_chown(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      const char *pathname,
      uid_t owner,
      gid_t group
      );

#ifdef DEBUG
#       define ez_fchown(fd, owner, group) \
         _ez_fchown(__FILE__, __LINE__, __FUNCTION__, fd, owner, group)
#else
#       define ez_fchown(fd, owner, group) \
         _ez_fchown(fd, owner, group)
#endif
int _ez_fchown(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      uid_t owner,
      gid_t group
      );

#ifdef DEBUG
#       define ez_fchmod(fd, mode) \
         _ez_fchmod(__FILE__, __LINE__, __FUNCTION__, fd, mode)
#else
#       define ez_fchmod(fd, mode) \
         _ez_fchmod(fd, mode)
#endif
int _ez_fchmod(
#ifdef DEBUG
   const char *fileName,
   int lineNo,
   const char *funcName,
#endif
      int fd,
      mode_t mode
      );

#ifdef __cplusplus
}
#endif

#endif
