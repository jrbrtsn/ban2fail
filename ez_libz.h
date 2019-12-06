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
#ifndef EZ_GZFILE_H
#define EZ_GZFILE_H

/* Simplified interface to libz file functions */
#define _GNU_SOURCE
#include <zlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#       define ez_gzopen(path, mode) \
   _ez_gzopen(__FILE__, __LINE__, __FUNCTION__, path, mode)
#else
#       define ez_gzopen(path, mode) \
   _ez_gzopen(path, mode)
#endif
gzFile _ez_gzopen(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      const char *path,
      const char *mode
      );

#ifdef DEBUG
#       define ez_gzclose(file) \
   _ez_gzclose(__FILE__, __LINE__, __FUNCTION__, file)
#else
#       define ez_gzclose(file) \
   _ez_gzclose(file)
#endif
int _ez_gzclose(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file
      );

#ifdef DEBUG
#       define ez_gzwrite(file, buf, len) \
   _ez_gzwrite(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
#else
#       define ez_gzwrite(file, buf, len) \
   _ez_gzwrite(file, buf, len)
#endif
int _ez_gzwrite(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file,
      voidpc buf,
      unsigned len
      );

#ifdef DEBUG
#       define ez_gzread(file, buf, len) \
   _ez_gzread(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
#else
#       define ez_gzread(file, buf, len) \
   _ez_gzread(file, buf, len)
#endif
int _ez_gzread(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file,
      voidp buf,
      unsigned len
      );

#ifdef DEBUG
#       define ez_gzflush(file, flush) \
   _ez_gzflush(__FILE__, __LINE__, __FUNCTION__, file, flush)
#else
#       define ez_gzflush(file, flush) \
   _ez_gzflush(file, flush)
#endif
int _ez_gzflush(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file,
      int flush 
      );

#ifdef DEBUG
#       define ez_gzseek(file, offset, whence) \
   _ez_gzseek(__FILE__, __LINE__, __FUNCTION__, file, offset, whence)
#else
#       define ez_gzseek(file, offset, whence) \
   _ez_gzseek(file, offset, whence)
#endif
z_off_t _ez_gzseek(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file,
      z_off_t offset,
      int whence
      );

#ifdef DEBUG
#       define ez_gztell(file) \
   _ez_gztell(__FILE__, __LINE__, __FUNCTION__, file)
#else
#       define ez_gztell(file) \
   _ez_gztell(file)
#endif
z_off_t _ez_gztell(
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file
      );

#ifdef DEBUG
#       define ez_gzgets(file, buf, len) \
   _ez_gzgets(__FILE__, __LINE__, __FUNCTION__, file, buf, len)
#else
#       define ez_gzgets(file, buf, len) \
   _ez_gzgets(file, buf, len)
#endif
char* _ez_gzgets (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      gzFile file,
      char *buf,
      int len
      );

#ifdef __cplusplus
}
#endif


#endif
